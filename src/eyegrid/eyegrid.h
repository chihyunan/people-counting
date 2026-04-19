#ifndef EYEGRID_H
#define EYEGRID_H

#include <Adafruit_AMG88xx.h>
#include <Wire.h>
#include "eyegrid_helper.h"
#include "scanner.h"

namespace Eyegrid {

static Adafruit_AMG88xx amg;
static float pixels[64];

struct FrameResult {
  int entered;
  int exited;
  uint8_t hotCells;
  uint8_t blobCount;
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
                                            float thresholdMultiplier = 1.10f,
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

inline FrameResult poll(float thresholdC, bool printDebug = true) {
  amg.readPixels(pixels);

  EyegridHelper::BinaryGrid8x8 mask = EyegridHelper::thresholdMask8x8(pixels, thresholdC);
  Scanner::ScanResult scan = Scanner::scan(mask);

  if (printDebug) {
    Serial.print(F("[eyegrid] hot="));
    Serial.print(scan.hotCells);
    Serial.print(F(" blobs="));
    Serial.print(scan.blobCount);
    Serial.print(F(" top="));
    Serial.print(scan.touchesTop ? 1 : 0);
    Serial.print(F(" bottom="));
    Serial.print(scan.touchesBottom ? 1 : 0);
    Serial.print(F(" entry="));
    Serial.print(scan.entry);
    Serial.print(F(" exit="));
    Serial.println(scan.exit);
  }

  FrameResult out = {scan.entry, scan.exit, scan.hotCells, scan.blobCount};
  return out;
}

} // namespace Eyegrid

#endif
