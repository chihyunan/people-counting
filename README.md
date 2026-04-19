# people-counting

ESP32 + two IR break-beams (GPIO **2** / **15**). Tracks **in-room** occupancy with cumulative **enter** and **exit** counts.

## Quick start

```bash
make flash                    # compile + flash (default port /dev/cu.usbserial-0001)
make flash PORT=/dev/cu.OTHER # override port
make monitor                  # serial @ 115200
```

## WiFi

SoftAP **`esp32-people`** / password **`people123`** (set in `people-counting.ino`).

| Endpoint | Description |
|----------|-------------|
| `http://192.168.4.1/` | Live dashboard (auto-refreshing) |
| `/json` | `{"inRoom":N,"entered":N,"exited":N}` |
| `/state` | Plain-text one-liner |

### Wi-Fi log stream

Serial logs are mirrored to a TCP stream on port `23`.

```bash
nc 192.168.4.1 23
# or
telnet 192.168.4.1 23
```

Notes:
- USB serial monitor remains available at `115200`.
- Only one Wi-Fi log client is accepted at a time; additional clients are rejected as busy.

## Layout

```
people-counting.ino        — main sketch (setup + loop)
lib/ir_beam/               — dual IR beam direction detection
lib/wifi/                  — SoftAP + HTTP server (wifi_ap.h / softap_http.cpp)
Makefile                   — compile / flash / monitor
```

Requires [arduino-cli](https://arduino.github.io/arduino-cli/) with the ESP32 core installed.
