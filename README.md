# people-counting

ESP32 + AMG8833 Grid-EYE (8x8 thermal). Detects warm blobs via connected-component flood-fill and reports count changes over serial.

## Quick start

```bash
make flash                    # compile + flash
make flash PORT=/dev/cu.OTHER # override port
make monitor                  # serial @ 115200
```

## Layout

```
people-counting.ino    — main sketch (Grid-EYE → blob count → serial)
src/eyegrid/eyegrid.h  — thermal sampling, zone FSM, blob detection
Makefile               — compile / flash / monitor
```

## Blob tuning

| Constant | Default | Description |
|----------|---------|-------------|
| `BLOB_DELTA_C` | 2.0 | Degrees above frame mean to threshold |
| `BLOB_MIN_SIZE` | 2 | Minimum pixels for a blob to count |

Edit in `src/eyegrid/eyegrid.h`. Active values print on boot.

Requires [arduino-cli](https://arduino.github.io/arduino-cli/) with the ESP32 core and `Adafruit_AMG88xx` library installed.
