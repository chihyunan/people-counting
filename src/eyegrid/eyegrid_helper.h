#ifndef EYEGRID_HELPER_H
#define EYEGRID_HELPER_H

#include <Arduino.h>
#include <stdint.h>

namespace EyegridHelper {

struct BinaryGrid8x8 {
  uint8_t cell[8][8];
};

inline BinaryGrid8x8 thresholdMask8x8(const float pixels[64], float thresholdC) {
  BinaryGrid8x8 out{};
  for (uint8_t y = 0; y < 8; ++y) {
    for (uint8_t x = 0; x < 8; ++x) {
      const float tempC = pixels[(y * 8) + x];
      out.cell[y][x] = (tempC >= thresholdC) ? 1 : 0;
    }
  }
  return out;
}

/** Multiplier applied to scanner threshold; printed °C only for cells >= threshold * this. */
static const float PRINT_TEMP_ABOVE_THRESHOLD_RATIO = 1.10f;

inline bool anyCellAtLeast(const float pixels[64], float minC) {
  for (uint8_t i = 0; i < 64; ++i) {
    if (pixels[i] >= minC) {
      return true;
    }
  }
  return false;
}

inline void printTemperatureGrid8x8(const float pixels[64], Stream &out = Serial,
                                    uint8_t decimals = 1,
                                    unsigned long sampleMs = 0,
                                    float printMinC = 0.0f) {
  out.println(F("========== Grid-EYE 8x8 °C =========="));
  out.print(F("sample_ms="));
  out.println(sampleMs);
  out.print(F("threshold_110pct_C="));
  out.println(printMinC, 2);
  out.println(F("--- columns (x) ->"));
  out.print(F("        "));
  for (uint8_t x = 0; x < 8; ++x) {
    out.print(F("x"));
    out.print(x);
    if (x < 7) {
      out.print(F("    "));
    }
  }
  out.println();
  out.println(F("--- rows (y); number = °C if >= threshold_110pct ---"));
  for (uint8_t y = 0; y < 8; ++y) {
    out.print(F("y"));
    out.print(y);
    out.print(F(" | "));
    for (uint8_t x = 0; x < 8; ++x) {
      const float t = pixels[(y * 8) + x];
      if (t >= printMinC) {
        out.print(t, decimals);
      } else {
        out.print(F("."));
      }
      if (x < 7) {
        out.print(F("  "));
      }
    }
    out.println();
  }
  out.println(F("====================================="));
}

} // namespace EyegridHelper

#endif
