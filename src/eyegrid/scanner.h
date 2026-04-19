#ifndef EYEGRID_SCANNER_H
#define EYEGRID_SCANNER_H

#include <stdint.h>
#include "eyegrid_helper.h"

namespace Scanner {

struct ScanResult {
  int entry;
  int exit;
  uint8_t hotCells;
  uint8_t blobCount;
  bool touchesTop;
  bool touchesBottom;
};

static bool sawTop = false;
static bool sawBottom = false;
static int8_t firstEdge = 0; // 0 none, +1 top, -1 bottom
static uint8_t missingFrames = 0;
static const uint8_t LOST_TRACK_RESET_FRAMES = 3;

inline ScanResult scan(const EyegridHelper::BinaryGrid8x8 &grid) {
  ScanResult out = {0, 0, 0, 0, false, false};
  bool top = false;
  bool bottom = false;

  bool visited[8][8] = {};
  const int8_t dirs[8][2] = {
      {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
      {0, 1},   {1, -1}, {1, 0},  {1, 1},
  };
  int qx[64];
  int qy[64];

  for (uint8_t y = 0; y < 8; ++y) {
    for (uint8_t x = 0; x < 8; ++x) {
      if (grid.cell[y][x] == 0) {
        continue;
      }

      out.hotCells++;
      if (y == 0) {
        top = true;
      } else if (y == 7) {
        bottom = true;
      }

      if (visited[y][x]) {
        continue;
      }

      out.blobCount++;
      int head = 0;
      int tail = 0;
      qx[tail] = x;
      qy[tail] = y;
      tail++;
      visited[y][x] = true;

      while (head < tail) {
        int cx = qx[head];
        int cy = qy[head];
        head++;

        for (uint8_t i = 0; i < 8; ++i) {
          int nx = cx + dirs[i][0];
          int ny = cy + dirs[i][1];
          if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8) {
            continue;
          }
          if (visited[ny][nx] || grid.cell[ny][nx] == 0) {
            continue;
          }
          visited[ny][nx] = true;
          qx[tail] = nx;
          qy[tail] = ny;
          tail++;
        }
      }
    }
  }

  out.touchesTop = top;
  out.touchesBottom = bottom;

  if (out.hotCells == 0) {
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
