#include "src/grideye/grideye.h"
#include "src/grideye/grideye_helper.h"
static float heatThresholdC = 28.0f;
/** Only pixels within this many °C of frame max can be peak seeds (head band). */
static constexpr float HEAD_BAND_C = 1.6f;
/** Min (cell − coolest orth neighbor) °C to count as a peak (noise rejection). */
static constexpr float MIN_PEAK_PROMINENCE_C = 0.35f;
static const unsigned long TEMP_PRINT_INTERVAL_MS = 3000;
static unsigned long lastTempPrintAt = 0;
static const int I2C_SDA_PIN = 21;
static const int I2C_SCL_PIN = 22;
static int occupancy = 0;
static unsigned long totalEntered = 0;
static unsigned long totalExited = 0;

void setup() {
  Serial.begin(115200);
  if (!Grideye::start(I2C_SDA_PIN, I2C_SCL_PIN)) {
    for (;;) delay(1000);
  }
  Grideye::CalibrationResult calibration = Grideye::calibrateThreshold(10000, 100, 1.05f, false);
  heatThresholdC = calibration.thresholdC;
}

void loop() {
  Grideye::FrameResult frame =
      Grideye::poll(heatThresholdC, HEAD_BAND_C, MIN_PEAK_PROMINENCE_C, false);

  for (uint8_t i = 0; i < frame.exitsThisFrame; i++) {
    if (occupancy > 0) occupancy--;
    totalExited++;
    Serial.println(F("<<< EXIT"));
  }
  for (uint8_t i = 0; i < frame.entriesThisFrame; i++) {
    occupancy++;
    totalEntered++;
    Serial.println(F(">>> ENTRY"));
  }

  unsigned long now = millis();
  if (now - lastTempPrintAt >= TEMP_PRINT_INTERVAL_MS) {
    const float printMinC =
        heatThresholdC * GrideyeHelper::PRINT_TEMP_ABOVE_THRESHOLD_RATIO;
    if (GrideyeHelper::anyCellAtLeast(Grideye::pixels, printMinC)) {
      Serial.print(F("[grideye] hot="));
      Serial.print(frame.hotCells);
      Serial.print(F(" blobs="));
      Serial.print(frame.blobCount);
      Serial.print(F(" active="));
      Serial.print(frame.activeBlobCount);
      Serial.print(F(" entries="));
      Serial.print(frame.entriesThisFrame);
      Serial.print(F(" exits="));
      Serial.print(frame.exitsThisFrame);
      Serial.print(F(" inRoom="));
      Serial.print(occupancy);
      Serial.print(F(" entered="));
      Serial.print(totalEntered);
      Serial.print(F(" exited="));
      Serial.print(totalExited);
      Serial.print(F(" frameMaxC="));
      Serial.print(frame.frameMaxC, 2);
      Serial.print(F(" peakFloorC="));
      Serial.println(frame.peakFloorC, 2);
      GrideyeHelper::printTemperatureGrid8x8(Grideye::pixels, Serial, 1, now,
                                               printMinC, frame.peakMask);
    }
    lastTempPrintAt = now;
  }

  delay(200);
}
