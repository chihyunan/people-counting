#line 1 "/Users/richardding/Documents/GitHub/people-counting/src/eyegrid/eyegrid_helper.h"
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

inline void printTemperatureGrid8x8(const float pixels[64], Stream &out = Serial,
                                    uint8_t decimals = 1) {
  out.println(F("8x8 (C):"));
  for (uint8_t y = 0; y < 8; ++y) {
    for (uint8_t x = 0; x < 8; ++x) {
      out.print(pixels[(y * 8) + x], decimals);
      if (x < 7) {
        out.print(' ');
      }
    }
    out.println();
  }
}

} // namespace EyegridHelper

#endif
