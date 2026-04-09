# people-counting

ESP32 + two IR beams (GPIO **36** and **39**). Serial prints entry / exit and occupancy @ **115200** baud.

**Build / flash:** `make flash` (needs [arduino-cli](https://arduino.github.io/arduino-cli/) + ESP32 core). Other port: `make flash PORT=/dev/cu.YOURPORT`.

**Serial:** `make monitor`

Sketch: `people-counting.ino`. IR logic: Arduino library under `lib/ir_beam/` (`src/ir_beam.cpp`, `ir_beam.h`).
