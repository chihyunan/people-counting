# Room Occupancy Counter

Door-mounted, real-time, WiFi-capable people counter using ESP32 + dual IR break-beams + AMG8833 thermal camera.

---

## Overview

This system tracks room occupancy by counting people entering and exiting through a standard doorway. Two staggered IR break-beams provide fast, deterministic direction detection for the common case. An AMG8833 GridEye thermal camera handles edge cases — simultaneous crossings, partial entries, and wheelchair users — that a single-sensor approach cannot resolve.

The system runs entirely on a 10 000 mAh power bank (~49 hours continuous), mounts to the door frame without permanent modification, and serves a live dashboard over WiFi. Total BOM cost: **$71.45** at bulk (1 000 units).

Developed for EECS 300: Electrical Engineering Systems Design II at the University of Michigan, by a five-person team.

---

## System architecture

```
Power Bank (5 V)
    └── Custom PCB
            ├── ESP32 ──── IR Break-Beam A  (GPIO 2)
            │         ├── IR Break-Beam B  (GPIO 15)
            │         ├── AMG8833 GridEye  (I²C: SDA 21 / SCL 22)
            │         └── WiFi (SoftAP) ──► Laptop dashboard
            ├── Break-Beam TX pairs (5 V)
            └── Voltage dividers (5 V → 3.3 V on RX signal lines)
```

Both sensor outputs feed a frame-level decision rule:

| Condition | Action |
|---|---|
| A break-beam is stuck or obstructed | Fall back entirely to GridEye |
| Both sensors agree on direction | Use GridEye count (handles simultaneous crossings) |
| Sensors disagree on direction | Use break-beam output (more precise under conflict) |

---

## Hardware

### Pinout summary

| Signal | ESP32 pin |
|---|---|
| Break-Beam A (RX) | GPIO 2 |
| Break-Beam B (RX) | GPIO 15 |
| GridEye SDA | GPIO 21 |
| GridEye SCL | GPIO 22 |

Break-beam receivers output up to 5 V; a resistor voltage divider (4.7 kΩ / 3 kΩ) on each signal line steps this down to 3.3 V before the ESP32 GPIO. The ESP32's 3.3 V rail powers the GridEye directly.

### Design decisions

**Why break-beam as primary, GridEye as secondary?** Break-beam direction detection is deterministic: beam A then beam B = entering, B then A = exiting. It adds no latency overhead and consumes negligible power in interrupt-driven mode. The GridEye covers the two failure modes break-beams cannot handle — simultaneous crossings that collapse into a single blocking pattern, and doorway obstruction by a wide object.

**Why GridEye over a visible-light camera?** The 8×8 thermal grid is orders of magnitude less data-heavy, works in the dark, and avoids privacy concerns in a classroom setting. The resolution (5.8 cm per pixel at head height, 0.46 m coverage width at 1.6 m) is sufficient for blob detection without needing precise localisation.

**Interrupt latency vs. minimum event window.** Hardware interrupt latency on the ESP32 is approximately 2 µs — 0.014% of the 14 ms minimum event window. Latency is negligible at all human movement timescales.

**Noise floor vs. debounce threshold.** EMI spikes on the signal lines measure ~30 µs. The debounce thresholds are set at 5 ms and 10 ms respectively — 167× above the noise floor.

**Mount design.** Velcro attachment with a 5.5 cm moment arm. Safety factor ≈ 10× against peel under worst-case sensor mass and door vibration. A single Velcro strip is sufficient.

**Beam misalignment tolerance.** Signal starts degrading beyond 6° misalignment and fails at 10°. Mount tolerances kept below 5°; sensor orientation eliminates cross-talk between the two beam pairs.

### File locations

```
Hardware_Altium/     — PCB schematic, layout, Gerbers
Hardware_Solidworks/ — door-frame mount CAD files
```

---

## Firmware

### Repository layout

```
people-counting.ino        — main sketch (setup + loop, sensor fusion logic)
src/ir_beam/
    ir_beam.cpp / .h       — interrupt-driven dual-beam direction detection
src/grideye/
    grideye.h              — AMG8833 driver and I²C polling
    grideye_helper.h       — temperature baseline and threshold utilities
    blob_motion.h          — peak filter, blob grouping, centroid tracking
    scanner.h              — frame-level scan loop
src/wifi/
    wifi_ap.h              — SoftAP configuration
    softap_http.cpp        — HTTP endpoints and TCP log stream
Makefile                   — compile / flash / monitor targets
```

### Break-beam algorithm

The algorithm is interrupt-driven and finite-state:

1. **Arm / direction** — whichever beam breaks first sets the candidate direction (A→B = enter, B→A = exit). Events are rejected if both beams break near-simultaneously (noise) or if timing context is stale.
2. **Commit / validation** — both beams must clear in the same order they broke. A minimum inter-event gap suppresses double-counting. Events that do not complete within the timeout window are discarded.

### GridEye algorithm

Runs every frame after a 10-second startup calibration:

1. **Peak filtering** — cells meaningfully hotter than their neighbours and above the room-temperature baseline are retained. Gradual warm patches from sunlight or HVAC are rejected.
2. **Blob grouping** — adjacent hot cells (8-connected) are merged into blobs; each blob's centroid represents one person.
3. **Motion and direction tracking** — blobs inactive for more than 5 seconds are dropped. A blob that crosses the doorway midline registers an entry or exit, with a per-blob cooldown to prevent duplicate counts.

---

## Performance

All tests conducted on a standard interior doorway.

### Baseline scenarios — target ≥ 90%

| Scenario | Result |
|---|---|
| Single person, slow walk, long gap | 100% |
| Single person, fast walk, long gap | 100% |
| Multiple people, same direction, ~1.5 m spacing | 100% |
| Multiple people, random directions, no simultaneous crossing | 100% |

### Edge cases — target ≥ 75%

| Scenario | Result |
|---|---|
| Two people, same direction, shoulder-to-shoulder | 100% |
| Two people, opposite directions, shoulder-to-shoulder | 80% |
| Partial entry (person steps in then backs out) | 100% |
| Wheelchair user, unassisted | 100% |
| Wheelchair user, assisted (two people) | 100% |
| Bystanders near doorway during crossing | 100% |

---

## Quick start

### Dependencies

Install [arduino-cli](https://arduino.github.io/arduino-cli/) with the ESP32 core, then install the following libraries through the Arduino IDE or arduino-cli:

- `Adafruit AMG88xx` (GridEye driver)
- `Wire` (I²C, bundled with ESP32 core)

### Flash

```bash
make flash                       # compile + flash (default port /dev/cu.usbserial-0001)
make flash PORT=/dev/cu.OTHER    # override port
make monitor                     # serial monitor @ 115200
```

### WiFi

The ESP32 creates a SoftAP named `esp32-people` (password: `people123`, configurable in `people-counting.ino`).

| Endpoint | Description |
|---|---|
| `http://192.168.4.1/` | Live dashboard (auto-refreshing) |
| `/json` | `{"inRoom": N, "entered": N, "exited": N}` |
| `/state` | Plain-text one-liner |

Serial logs are mirrored to a TCP stream on port 23:

```bash
nc 192.168.4.1 23
```

Only one WiFi log client is accepted at a time. USB serial remains available at 115 200 baud.

---

## Bill of materials

Bulk pricing at 1 000 units. Does not include labour, fabrication overhead, or shipping.

| Component | Unit cost | Qty | Total |
|---|---|---|---|
| ESP32 | $10.00 | 1 | $10.00 |
| PCB (bulk) | $0.40 | 1 | $0.40 |
| IR break-beam pair | $6.00 | 2 | $12.00 |
| AMG8833 GridEye | $30.00 | 1 | $30.00 |
| PLA mounts (174 g @ $0.02/g) | — | — | $3.48 |
| 10 000 mAh power bank | $10.00 | 1 | $10.00 |
| 22 AWG wire (4 m) | — | — | $1.57 |
| Female header pins | — | 60 pins | $1.03 |
| SMD resistors | $0.005 | 4 | $0.02 |
| USB-C breakout | $2.95 | 1 | $2.95 |
| **Total** | | | **$71.45** |

---

## Known limitations

- **Simultaneous opposing crossings** are handled by the GridEye, but the 8×8 resolution means two people of very similar thermal signature crossing at the exact same midline pixel may register as one blob. Accuracy remains within spec but degrades toward the target floor in this case.
- **Beam obstruction** (e.g., someone holding the door open continuously) disables break-beam direction detection; the system falls back to GridEye only for the duration.
- **Detection zone depth** — beams are positioned a few inches inward from the frame to avoid interruption by the door swing. The outermost centimetres of the doorway depth are outside the detection zone.
- **Startup calibration** — the 10-second room-temperature baseline calibration must complete in an unoccupied doorway. Calibrating with a person standing in the doorway will raise the baseline threshold.

---

## Credits

GridEye blob-tracking logic adapted from [zhaochenyuangit/IR-room-monitor](https://github.com/zhaochenyuangit/IR-room-monitor) (no licence provided on original).

All enhancements in this repository are licensed under MIT.

Reference: H. Mohammadmoradi et al., "Measuring People-Flow through Doorways using Easy-to-Install IR Array Sensors," DCOSS 2017.
