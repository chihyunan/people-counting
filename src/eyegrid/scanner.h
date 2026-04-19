#ifndef EYEGRID_SCANNER_H
#define EYEGRID_SCANNER_H

#include <stdint.h>
#include "eyegrid_helper.h"

namespace Scanner {

struct CountTuple {
  int entry;
  int exit;
};

static bool sawTop = false;
static bool sawBottom = false;
static int8_t firstEdge = 0; // 0 none, +1 top, -1 bottom
static uint8_t missingFrames = 0;
static const uint8_t LOST_TRACK_RESET_FRAMES = 3;

inline CountTuple scan(const EyegridHelper::BinaryGrid8x8 &grid) {
  CountTuple out = {0, 0};
  bool anyHot = false;
  bool top = false;
  bool bottom = false;

  for (uint8_t y = 0; y < 8; ++y) {
    for (uint8_t x = 0; x < 8; ++x) {
      if (grid.cell[y][x] == 0) {
        continue;
      }
      anyHot = true;
      if (y == 0) {
        top = true;
      } else if (y == 7) {
        bottom = true;
      }
    }
  }

  if (!anyHot) {
    missingFrames++;
    if (missingFrames >= LOST_TRACK_RESET_FRAMES) {
      sawTop = false;
      sawBottom = false;
      firstEdge = 0;
      missingFrames = 0;
    }
    return out;
  }

  missingFrames = 0;
  sawTop = sawTop || top;
  sawBottom = sawBottom || bottom;

  if (firstEdge == 0) {
    if (top && !bottom) {
      firstEdge = 1;
    } else if (bottom && !top) {
      firstEdge = -1;
    }
  }

  if (sawTop && sawBottom && firstEdge != 0) {
    if (firstEdge > 0) {
      out.entry = 1;
    } else {
      out.exit = 1;
    }
    sawTop = false;
    sawBottom = false;
    firstEdge = 0;
    missingFrames = 0;
  }

  return out;
}

} // namespace Scanner

#endif
