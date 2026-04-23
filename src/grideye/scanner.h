#ifndef GRIDEYE_SCANNER_H
#define GRIDEYE_SCANNER_H

#include <stdint.h>

namespace Scanner {

static constexpr uint8_t kMaxScanBlobs = 8;

struct ScanResult {
  uint8_t hotCells;
  uint8_t blobCount;
  float frameMaxC;
  float peakFloorC;
  /** Bit set for each orth-filtered peak cell (for grid *). */
  uint64_t peakMask;
  /** Per 8-connected blob of peaks: bitmask and centroid in grid cells. */
  float blobCx[kMaxScanBlobs];
  float blobCy[kMaxScanBlobs];
  uint64_t blobMask[kMaxScanBlobs];
};

/** Orthogonal local maximum: center >= each existing N/E/S/W neighbor (no diagonals). */
inline bool orthLocalPeak4(const float pixels[64], uint8_t x, uint8_t y) {
  const float c = pixels[(static_cast<uint16_t>(y) << 3) + x];
  if (y > 0 && c < pixels[(static_cast<uint16_t>(y - 1u) << 3) + x]) {
    return false;
  }
  if (y < 7 && c < pixels[(static_cast<uint16_t>(y + 1u) << 3) + x]) {
    return false;
  }
  if (x > 0 && c < pixels[(static_cast<uint16_t>(y) << 3) + (x - 1u)]) {
    return false;
  }
  if (x < 7 && c < pixels[(static_cast<uint16_t>(y) << 3) + (x + 1u)]) {
    return false;
  }
  return true;
}

/** Minimum temperature over existing N/E/S/W neighbors. */
inline float orthNeighborMin4(const float pixels[64], uint8_t x, uint8_t y) {
  float nMin = 9999.0f;
  if (y > 0) {
    const float v = pixels[(static_cast<uint16_t>(y - 1u) << 3) + x];
    if (v < nMin) {
      nMin = v;
    }
  }
  if (y < 7) {
    const float v = pixels[(static_cast<uint16_t>(y + 1u) << 3) + x];
    if (v < nMin) {
      nMin = v;
    }
  }
  if (x > 0) {
    const float v = pixels[(static_cast<uint16_t>(y) << 3) + (x - 1u)];
    if (v < nMin) {
      nMin = v;
    }
  }
  if (x < 7) {
    const float v = pixels[(static_cast<uint16_t>(y) << 3) + (x + 1u)];
    if (v < nMin) {
      nMin = v;
    }
  }
  return nMin;
}

/**
 * hotCells: T >= thresholdC (calibration warm mask).
 * Peaks only inside high band: T >= highBandMinC (e.g. threshold * 1.05).
 * Peak rules: T >= peakFloor, orth local max, (T - min orth N) >= minProminenceC,
 *             peakFloor = max(thresholdC, frameMax - headBandC).
 * blobCount: 8-connected components among peak cells (includes diagonals so
 *             two * cells touching only on a corner count as one blob).
 * peakMask: bits set for each peak cell.
 * No qualifying local peak → blobCount 0 (no synthetic blob for flat hot regions).
 */
inline ScanResult scan(const float pixels[64], float thresholdC, float headBandC,
                       float minProminenceC, float highBandMinC) {
  ScanResult out{};
  out.frameMaxC = pixels[0];

  for (uint8_t i = 1; i < 64; ++i) {
    if (pixels[i] > out.frameMaxC) {
      out.frameMaxC = pixels[i];
    }
  }

  out.peakFloorC = out.frameMaxC - headBandC;
  if (out.peakFloorC < thresholdC) {
    out.peakFloorC = thresholdC;
  }

  bool isPeak[8][8] = {};

  for (uint8_t y = 0; y < 8; ++y) {
    for (uint8_t x = 0; x < 8; ++x) {
      const float t = pixels[(static_cast<uint16_t>(y) << 3) + x];
      if (t >= thresholdC) {
        out.hotCells++;
      }
      if (t < highBandMinC) {
        continue;
      }
      if (t < out.peakFloorC) {
        continue;
      }
      if (!orthLocalPeak4(pixels, x, y)) {
        continue;
      }
      const float nMin = orthNeighborMin4(pixels, x, y);
      if ((t - nMin) < minProminenceC) {
        continue;
      }
      isPeak[y][x] = true;
    }
  }

  const int8_t d8[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1},
                           {1, -1},  {1, 0},  {1, 1}};
  bool visited[8][8] = {};
  int qx[64];
  int qy[64];

  uint8_t nb = 0;
  for (uint8_t y = 0; y < 8; ++y) {
    for (uint8_t x = 0; x < 8; ++x) {
      if (!isPeak[y][x] || visited[y][x]) {
        continue;
      }
      uint32_t sumX = 0;
      uint32_t sumY = 0;
      uint32_t cnt = 0;
      uint64_t compMask = 0ULL;

      int head = 0;
      int tail = 0;
      qx[tail] = x;
      qy[tail] = y;
      tail++;
      visited[y][x] = true;

      while (head < tail) {
        const int cx = qx[head];
        const int cy = qy[head];
        head++;
        sumX += static_cast<uint32_t>(cx);
        sumY += static_cast<uint32_t>(cy);
        cnt++;
        compMask |= (1ULL << static_cast<uint8_t>((static_cast<uint8_t>(cy) << 3) |
                                                   static_cast<uint8_t>(cx)));

        for (uint8_t i = 0; i < 8; ++i) {
          const int nx = cx + d8[i][0];
          const int ny = cy + d8[i][1];
          if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8) {
            continue;
          }
          if (visited[ny][nx] || !isPeak[ny][nx]) {
            continue;
          }
          visited[ny][nx] = true;
          qx[tail] = nx;
          qy[tail] = ny;
          tail++;
        }
      }

      if (nb < kMaxScanBlobs) {
        const float inv = 1.0f / static_cast<float>(cnt);
        out.blobCx[nb] = static_cast<float>(sumX) * inv;
        out.blobCy[nb] = static_cast<float>(sumY) * inv;
        out.blobMask[nb] = compMask;
        nb++;
      }
    }
  }
  out.blobCount = nb;
  for (uint8_t b = nb; b < kMaxScanBlobs; ++b) {
    out.blobCx[b] = 0.0f;
    out.blobCy[b] = 0.0f;
    out.blobMask[b] = 0ULL;
  }

  for (uint8_t y = 0; y < 8; ++y) {
    for (uint8_t x = 0; x < 8; ++x) {
      if (isPeak[y][x]) {
        const uint8_t bit = static_cast<uint8_t>((y << 3) | x);
        out.peakMask |= (1ULL << bit);
      }
    }
  }

  return out;
}

} // namespace Scanner

#endif
