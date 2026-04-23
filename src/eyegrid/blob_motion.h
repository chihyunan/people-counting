#ifndef EYEGRID_BLOB_MOTION_H
#define EYEGRID_BLOB_MOTION_H

#include <Arduino.h>
#include <stdint.h>
#include "scanner.h"

namespace BlobMotion {

static constexpr unsigned long STATIONARY_EXCLUDE_MS = 5000UL;
/** Centroid shift (grid cells) above this counts as movement. */
static constexpr float MOVE_THRESH_CELL = 0.35f;
/** Debounced directional crossing emit gap. */
static constexpr unsigned long DIRECTION_EVENT_GAP_MS = 700UL;
/** Virtual doorway centerline (0..7 grid row space). */
static constexpr float CROSS_LINE_Y = 3.5f;
/** Hysteresis around centerline; inside band means "no side". */
static constexpr float CROSS_BAND_Y = 0.6f;

enum Side : uint8_t {
  SIDE_UNKNOWN = 0,
  SIDE_TOP = 1,
  SIDE_BOTTOM = 2
};

inline Side classifySide(float cy) {
  if (cy <= (CROSS_LINE_Y - CROSS_BAND_Y)) {
    return SIDE_TOP;
  }
  if (cy >= (CROSS_LINE_Y + CROSS_BAND_Y)) {
    return SIDE_BOTTOM;
  }
  return SIDE_UNKNOWN;
}

struct UpdateResult {
  uint8_t activeBlobCount;
  uint8_t entriesThisFrame;
  uint8_t exitsThisFrame;
};

struct Slot {
  bool active;
  uint64_t mask;
  float cx;
  float cy;
  unsigned long lastMovedMs;
  unsigned long lastDirectionEmitMs;
  uint32_t id;
  bool stationaryAnnounced;
  Side lastSide;
};

static Slot slots[8];
static uint32_t nextTrackId = 1;

inline int popcnt64(uint64_t v) { return static_cast<int>(__builtin_popcountll(v)); }

inline UpdateResult update(const Scanner::ScanResult &scan, unsigned long nowMs) {
  UpdateResult out{};
  uint8_t n = scan.blobCount;
  if (n > 8) {
    n = 8;
  }

  if (n == 0) {
    for (uint8_t j = 0; j < 8; ++j) {
      slots[j].active = false;
    }
    return out;
  }

  bool slotPaired[8] = {};

  for (uint8_t i = 0; i < n; ++i) {
    int bestJ = -1;
    int bestOv = -1;
    for (uint8_t j = 0; j < 8; ++j) {
      if (!slots[j].active || slotPaired[j]) {
        continue;
      }
      const int ov = popcnt64(scan.blobMask[i] & slots[j].mask);
      if (ov > bestOv) {
        bestOv = ov;
        bestJ = static_cast<int>(j);
      }
    }
    if (bestJ >= 0 && bestOv < 1) {
      bestJ = -1;
    }

    if (bestJ < 0) {
      float bestD2 = 9999.0f;
      int bestJ2 = -1;
      for (uint8_t j = 0; j < 8; ++j) {
        if (!slots[j].active || slotPaired[j]) {
          continue;
        }
        const float dx = scan.blobCx[i] - slots[j].cx;
        const float dy = scan.blobCy[i] - slots[j].cy;
        const float d2 = dx * dx + dy * dy;
        if (d2 < bestD2) {
          bestD2 = d2;
          bestJ2 = static_cast<int>(j);
        }
      }
      const float maxAttach2 = 2.5f * 2.5f;
      if (bestJ2 >= 0 && bestD2 <= maxAttach2) {
        bestJ = bestJ2;
      }
    }

    if (bestJ < 0) {
      for (uint8_t j = 0; j < 8; ++j) {
        if (slots[j].active) {
          continue;
        }
        slots[j].active = true;
        slots[j].mask = scan.blobMask[i];
        slots[j].cx = scan.blobCx[i];
        slots[j].cy = scan.blobCy[i];
        slots[j].lastMovedMs = nowMs;
        slots[j].lastDirectionEmitMs = 0;
        slots[j].id = nextTrackId++;
        slots[j].stationaryAnnounced = false;
        slots[j].lastSide = classifySide(slots[j].cy);
        slotPaired[j] = true;
        break;
      }
    } else {
      Slot &s = slots[static_cast<uint8_t>(bestJ)];
      const float dx = scan.blobCx[i] - s.cx;
      const float dy = scan.blobCy[i] - s.cy;
      const float thr2 = MOVE_THRESH_CELL * MOVE_THRESH_CELL;
      if (dx * dx + dy * dy > thr2) {
        s.lastMovedMs = nowMs;
        if (s.stationaryAnnounced) {
          Serial.print(F("[motion] blob id="));
          Serial.print(s.id);
          Serial.println(F(" moved again — counting resumed"));
        }
        s.stationaryAnnounced = false;
      }
      s.mask = scan.blobMask[i];
      s.cx = scan.blobCx[i];
      s.cy = scan.blobCy[i];

      const Side sideNow = classifySide(s.cy);
      const bool recentlyMoving = (nowMs - s.lastMovedMs) < STATIONARY_EXCLUDE_MS;
      const bool emitCooldownOk =
          (s.lastDirectionEmitMs == 0) || ((nowMs - s.lastDirectionEmitMs) >= DIRECTION_EVENT_GAP_MS);
      if (sideNow != SIDE_UNKNOWN && s.lastSide != SIDE_UNKNOWN && sideNow != s.lastSide &&
          recentlyMoving && emitCooldownOk) {
        if (s.lastSide == SIDE_TOP && sideNow == SIDE_BOTTOM) {
          out.exitsThisFrame++;
        } else {
          out.entriesThisFrame++;
        }
        s.lastDirectionEmitMs = nowMs;
      }
      if (sideNow != SIDE_UNKNOWN) {
        s.lastSide = sideNow;
      }
      slotPaired[static_cast<uint8_t>(bestJ)] = true;
    }
  }

  for (uint8_t j = 0; j < 8; ++j) {
    if (slots[j].active && !slotPaired[j]) {
      slots[j].active = false;
    }
  }

  for (uint8_t j = 0; j < 8; ++j) {
    if (!slots[j].active) {
      continue;
    }
    const unsigned long idle = nowMs - slots[j].lastMovedMs;
    if (idle < STATIONARY_EXCLUDE_MS) {
      out.activeBlobCount++;
    } else {
      if (!slots[j].stationaryAnnounced) {
        Serial.print(F("[motion] *** blob id="));
        Serial.print(slots[j].id);
        Serial.print(F(" stationary "));
        Serial.print(STATIONARY_EXCLUDE_MS / 1000UL);
        Serial.println(F("s+ — excluded from people count ***"));
        slots[j].stationaryAnnounced = true;
      }
    }
  }

  return out;
}

} // namespace BlobMotion

#endif
