#ifndef EYEGRID_H
#define EYEGRID_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>

namespace Eyegrid {

static Adafruit_AMG88xx amg;
static float pixels[64];

enum class Zone { Out, Mid, In };

static int stage = 0;  // 0 idle, 1 saw OUT, 2 OUT->MID, 3 saw IN, 4 IN->MID
static unsigned long lastPollMs = 0;
static unsigned long lastTrackedMs = 0;

constexpr float HEAT_THRESHOLD_C = 28.0f;  // fallback if uncalibrated
constexpr float OUT_LINE = 2.5f;  // y < this = OUT
constexpr float IN_LINE  = 4.5f;  // y > this = IN
constexpr unsigned long SAMPLE_INTERVAL_MS = 80;
constexpr unsigned long TRACK_TIMEOUT_MS = 1500;

constexpr float BLOB_DELTA_C = 2.0f;
constexpr uint8_t BLOB_MIN_SIZE = 2;
constexpr float AMBIENT_BAND_PCT = 0.10f;
constexpr unsigned long CALIBRATION_MS = 10000;


static bool calibrated = false;
static float ambientMean = 0.0f;
static float ambientUpper = 0.0f;  // mean * (1 + BAND%)

inline Zone zoneOf(float y) {
  if (y < OUT_LINE) return Zone::Out;
  if (y > IN_LINE)  return Zone::In;
  return Zone::Mid;
}

inline bool start(int sda = 21, int scl = 22) {
  Wire.begin(sda, scl);
  return amg.begin(0x69);
}

inline void resetTracker() {
  stage = 0;
  lastTrackedMs = 0;
}

inline void calibrate(unsigned long durationMs = CALIBRATION_MS) {
  double accumulator = 0.0;
  unsigned long frames = 0;
  unsigned long start = millis();

  Serial.printf("Calibrating ambient for %lu ms — keep doorway clear...\n", durationMs);

  while (millis() - start < durationMs) {
    amg.readPixels(pixels);
    for (int i = 0; i < 64; i++) accumulator += pixels[i];
    frames++;
    delay(SAMPLE_INTERVAL_MS);
  }

  ambientMean  = static_cast<float>(accumulator / (frames * 64));
  ambientUpper = ambientMean * (1.0f + AMBIENT_BAND_PCT);
  calibrated   = true;

  Serial.printf("Calibration done: %lu frames, ambient=%.1f C, gate=%.1f\n",
                frames, ambientMean, ambientUpper);
}

inline int processZone(Zone z, float maxT, int y, unsigned long now) {
  int delta = 0;

  switch (stage) {
    case 0:
      if (z == Zone::Out) stage = 1;
      else if (z == Zone::In) stage = 3;
      break;

    case 1:  // saw OUT
      if (z == Zone::Mid) {
        stage = 2;
      } else if (z == Zone::In) {  // OUT->IN quick
        delta = 1;
        stage = 0;
      }
      break;

    case 2:  // OUT->MID
      if (z == Zone::In) {
        delta = 1;
        stage = 0;
      } else if (z == Zone::Out) {
        stage = 1;
      }
      break;

    case 3:  // saw IN
      if (z == Zone::Mid) {
        stage = 4;
      } else if (z == Zone::Out) {
        delta = -1;
        stage = 0;
      }
      break;

    case 4:  // IN->MID
      if (z == Zone::Out) {
        delta = -1;
        stage = 0;
      } else if (z == Zone::In) {
        stage = 3;
      }
      break;
  }

  if (delta != 0) {
    Serial.printf("Grid-EYE event %d (maxT=%.1f at y=%d, t=%lu)\n", delta, maxT, y, now);
  }

  return delta;
}

inline bool update(unsigned long now = millis()) {
  if (now - lastPollMs < SAMPLE_INTERVAL_MS) {
    return false;
  }

  lastPollMs = now;
  amg.readPixels(pixels);

  // find hottest pixel
  float maxT = pixels[0];
  int maxI = 0;
  for (int i = 1; i < 64; i++) {
    if (pixels[i] > maxT) {
      maxT = pixels[i];
      maxI = i;
    }
  }
  int y = maxI / 8;

  float gate = calibrated ? ambientUpper : HEAT_THRESHOLD_C;
  if (maxT < gate) {
    if (stage != 0 && lastTrackedMs != 0 && now - lastTrackedMs > TRACK_TIMEOUT_MS) {
      stage = 0;
    }
    return false;
  }

  lastTrackedMs = now;
  Zone z = zoneOf(static_cast<float>(y));
  int delta = processZone(z, maxT, y, now);
  return delta != 0;
}

inline uint8_t countBlobs(float deltaC = BLOB_DELTA_C,
                          uint8_t minSize = BLOB_MIN_SIZE) {
  float thresh = calibrated ? ambientUpper
                            : ([&]{ float s=0; for(int i=0;i<64;i++) s+=pixels[i]; return s/64.0f; }() + deltaC);

  uint8_t labels[64] = {};
  uint8_t blobCount = 0;
  uint8_t stack[64];

  for (uint8_t seed = 0; seed < 64; seed++) {
    if (labels[seed] || pixels[seed] < thresh) continue;
    blobCount++;
    uint8_t sz = 0;
    uint8_t top = 0;
    stack[top++] = seed;
    while (top > 0) {
      uint8_t idx = stack[--top];
      if (labels[idx]) continue;
      labels[idx] = blobCount;
      sz++;
      uint8_t r = idx / 8, c = idx % 8;
      if (r > 0 && !labels[idx - 8] && pixels[idx - 8] >= thresh)
        stack[top++] = idx - 8;
      if (r < 7 && !labels[idx + 8] && pixels[idx + 8] >= thresh)
        stack[top++] = idx + 8;
      if (c > 0 && !labels[idx - 1] && pixels[idx - 1] >= thresh)
        stack[top++] = idx - 1;
      if (c < 7 && !labels[idx + 1] && pixels[idx + 1] >= thresh)
        stack[top++] = idx + 1;
    }
    if (sz < minSize) blobCount--;
  }

  return blobCount;
}

}  // namespace Eyegrid

#endif
