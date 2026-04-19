#line 1 "/Users/richardding/Documents/GitHub/people-counting/src/eyegrid/eyegrid_helper.h"
#ifndef EYEGRID_HELPER_H
#define EYEGRID_HELPER_H

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

namespace EyegridHelper {

/** Multiplier applied to scanner threshold; gate + print use cells >= threshold * this (+5%). */
static const float PRINT_TEMP_ABOVE_THRESHOLD_RATIO = 1.05f;

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
                                    float printMinC = 0.0f,
                                    uint64_t peakMask = 0ULL) {
  // Wider columns: numbers, or "* 23.4" for peaks (star + space + °C).
  const int cellW = 5 + static_cast<int>(decimals) + 1 + 4;
  char buf[24];
  char inner[20];

  out.println(F("========== Grid-EYE 8x8 °C =========="));
  out.print(F("sample_ms="));
  out.println(sampleMs);
  out.print(F("threshold_105pct_C="));
  out.println(printMinC, 2);
  out.println(F("--- columns (x) ->"));
  out.print(F("     "));
  for (uint8_t x = 0; x < 8; ++x) {
    char xlab[6];
    snprintf(xlab, sizeof(xlab), "x%u", x);
    snprintf(buf, sizeof(buf), "%*s", cellW, xlab);
    out.print(buf);
  }
  out.println();
  out.println(
      F("--- rows: °C if >= threshold_105pct; * + °C = head peak in +5% band ---"));
  for (uint8_t y = 0; y < 8; ++y) {
    out.print(F("y"));
    out.print(y);
    out.print(F(" | "));
    for (uint8_t x = 0; x < 8; ++x) {
      const uint8_t idx = static_cast<uint8_t>((y << 3) | x);
      const float t = pixels[(y * 8) + x];
      if ((peakMask >> idx) & 1ULL) {
        snprintf(inner, sizeof(inner), "* %.*f", static_cast<int>(decimals),
                 static_cast<double>(t));
        snprintf(buf, sizeof(buf), "%*s", cellW, inner);
      } else if (t >= printMinC) {
        snprintf(buf, sizeof(buf), "%*.*f", cellW, static_cast<int>(decimals),
                 static_cast<double>(t));
      } else {
        snprintf(buf, sizeof(buf), "%*s", cellW, ".");
      }
      out.print(buf);
    }
    out.println();
  }
  out.println(F("====================================="));
}

} // namespace EyegridHelper

#endif
