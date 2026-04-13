# people-counting

ESP32 + two IR beams (GPIO **36** and **39**). Tracks **in room**, cumulative **enter**, and **exit**.

**WiFi:** AP **`esp32-people`** / **`people123`** (passed to `wifiBegin(...)` in `people-counting.ino`). Open **`http://192.168.4.1/`** or **`/json`** / **`/state`**.

**Serial:** **115200**, `make monitor`.

**Build / flash:** `make flash` ([arduino-cli](https://arduino.github.io/arduino-cli/) + ESP32 core). Port: `make flash PORT=/dev/cu.YOURPORT`.

**Layout:** `people-counting.ino` + `lib/ir_beam/` (IR only) + `lib/wifi/` (SoftAP + HTTP only). Include **`wifi_ap.h`**. Source is **`softap_http.cpp`** (not `wifi.cpp` — macOS can treat it like the core `WiFi.cpp` and break the link).

## Test harnesses

- **Beam jumper harness:** `make flash-beam-test PORT=/dev/cu.YOURPORT`
  - Sketch: `test/beam-jumper/beam-jumper.ino`
  - Wiring: GPIO25 -> GPIO36 (A), GPIO26 -> GPIO39 (B), common GND
  - Purpose: exercise `lib/ir_beam` + counting + WiFi without optical break-beam hardware
- **WiFi-only harness:** `make flash-wifi-test PORT=/dev/cu.YOURPORT`
  - Sketch: `test/wifi-only/wifi-only.ino`
  - Purpose: validate SoftAP + `/json` + `/state` with synthetic counters

You can also override sketch directly: `make flash SKETCH_DIR=test/wifi-only PORT=/dev/cu.YOURPORT`.
