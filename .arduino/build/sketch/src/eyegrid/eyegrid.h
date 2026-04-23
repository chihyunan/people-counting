#line 1 "/Users/richardding/Documents/GitHub/people-counting/src/eyegrid/eyegrid.h"
#ifndef EYEGRID_H
#define EYEGRID_H

#include <Adafruit_AMG88xx.h>
#include <Wire.h>
#include "blob_motion.h"
#include "eyegrid_helper.h"
#include "scanner.h"

namespace Eyegrid {

static Adafruit_AMG88xx amg;
static float pixels[64];

struct FrameResult {
  uint8_t hotCells;
  /** Raw 8-connected peak blobs this frame. */
  uint8_t blobCount;
  /** Blobs still counted after motion filter (5s stationary excluded). */
  uint8_t activeBlobCount;
  /** +1 = exit, -1 = entry, 0 = none. */
  int8_t directionalEvent;
  float frameMaxC;
  float peakFloorC;
  uint64_t peakMask;
};

struct CalibrationResult {
  float averageC;
  float thresholdC;
  uint16_t frames;
};

inline bool start(int sda = 21, int scl = 22) {
  Wire.begin(sda, scl);
  return amg.begin(0x69);
}

inline CalibrationResult calibrateThreshold(unsigned long durationMs = 10000,
                                            unsigned long sampleIntervalMs = 100,
                                            float thresholdMultiplier = 1.05f,
                                            bool printDebug = true) {
  if (sampleIntervalMs == 0) {
    sampleIntervalMs = 1;
  }

  unsigned long startMs = millis();
  uint16_t frames = 0;
  float sumTempC = 0.0f;

  while ((millis() - startMs) < durationMs) {
    amg.readPixels(pixels);
    for (uint8_t i = 0; i < 64; ++i) {
      sumTempC += pixels[i];
    }
    frames++;
    delay(sampleIntervalMs);
  }

  if (frames == 0) {
    amg.readPixels(pixels);
    for (uint8_t i = 0; i < 64; ++i) {
      sumTempC += pixels[i];
    }
    frames = 1;
  }

  float averageC = sumTempC / (64.0f * static_cast<float>(frames));
  float thresholdC = averageC * thresholdMultiplier;

  if (printDebug) {
    Serial.print(F("[eyegrid] calibration avgC="));
    Serial.print(averageC, 2);
    Serial.print(F(" thresholdC="));
    Serial.print(thresholdC, 2);
    Serial.print(F(" frames="));
    Serial.println(frames);
  }

  CalibrationResult out = {averageC, thresholdC, frames};
  return out;
}

inline FrameResult poll(float thresholdC, float headBandC, float minProminenceC,
                        bool printDebug = true) {
  amg.readPixels(pixels);

  const float highBandMinC =
      thresholdC * EyegridHelper::PRINT_TEMP_ABOVE_THRESHOLD_RATIO;
  Scanner::ScanResult scan = Scanner::scan(pixels, thresholdC, headBandC, minProminenceC,
                                           highBandMinC);

  const BlobMotion::UpdateResult motion = BlobMotion::update(scan, millis());

  if (printDebug) {
    Serial.print(F("[eyegrid] hot="));
    Serial.print(scan.hotCells);
    Serial.print(F(" blobs="));
    Serial.print(scan.blobCount);
    Serial.print(F(" active="));
    Serial.print(motion.activeBlobCount);
    Serial.print(F(" ev="));
    Serial.print(motion.directionalEvent);
    Serial.print(F(" frameMaxC="));
    Serial.print(scan.frameMaxC, 2);
    Serial.print(F(" peakFloorC="));
    Serial.println(scan.peakFloorC, 2);
  }

  FrameResult out = {scan.hotCells, scan.blobCount, motion.activeBlobCount, motion.directionalEvent,
                     scan.frameMaxC,
                     scan.peakFloorC, scan.peakMask};
  return out;
}

} // namespace Eyegrid

#endif
