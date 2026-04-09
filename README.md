# people-counting

ESP32 + two IR beams (GPIO **36** and **39**). Tracks **in room**, cumulative **enter**, and **exit**.

**WiFi:** AP **`esp32-people`** / **`people123`** (passed to `wifiBegin(...)` in `people-counting.ino`). Open **`http://192.168.4.1/`** or **`/json`** / **`/state`**.

**Serial:** **115200**, `make monitor`.

**Build / flash:** `make flash` ([arduino-cli](https://arduino.github.io/arduino-cli/) + ESP32 core). Port: `make flash PORT=/dev/cu.YOURPORT`.

**Layout:** `people-counting.ino` + `lib/ir_beam/` (IR only) + `lib/wifi/` (SoftAP + HTTP only). Include **`wifi_ap.h`**. Source is **`softap_http.cpp`** (not `wifi.cpp` — macOS can treat it like the core `WiFi.cpp` and break the link).
