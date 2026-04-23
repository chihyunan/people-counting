#include <Arduino.h>
#line 1 "/Users/richardding/Documents/GitHub/people-counting/people-counting.ino"
#include "src/eyegrid/eyegrid.h"
#include "src/eyegrid/eyegrid_helper.h"
#include "src/oled/oled_display.h"

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

#line 18 "/Users/richardding/Documents/GitHub/people-counting/people-counting.ino"
void setup();
#line 29 "/Users/richardding/Documents/GitHub/people-counting/people-counting.ino"
void loop();
#line 18 "/Users/richardding/Documents/GitHub/people-counting/people-counting.ino"
void setup() {
  Serial.begin(115200);
  Oled::begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Oled::showBlobCount(0);
  if (!Eyegrid::start(I2C_SDA_PIN, I2C_SCL_PIN)) {
    for (;;) delay(1000);
  }
  Eyegrid::CalibrationResult calibration = Eyegrid::calibrateThreshold(10000, 100, 1.05f, false);
  heatThresholdC = calibration.thresholdC;
}

void loop() {
  Eyegrid::FrameResult frame =
      Eyegrid::poll(heatThresholdC, HEAD_BAND_C, MIN_PEAK_PROMINENCE_C, false);

  if (frame.directionalEvent != 0) {
    if (frame.directionalEvent == 1) {
      if (occupancy > 0) {
        occupancy--;
      }
      totalExited++;
      Serial.println(F("<<< EXIT (event=+1)"));
    } else {
      occupancy++;
      totalEntered++;
      Serial.println(F(">>> ENTRY (event=-1)"));
    }
  }

  unsigned long now = millis();
  if (now - lastTempPrintAt >= TEMP_PRINT_INTERVAL_MS) {
    const float printMinC =
        heatThresholdC * EyegridHelper::PRINT_TEMP_ABOVE_THRESHOLD_RATIO;
    if (EyegridHelper::anyCellAtLeast(Eyegrid::pixels, printMinC)) {
      Serial.print(F("[eyegrid] hot="));
      Serial.print(frame.hotCells);
      Serial.print(F(" blobs="));
      Serial.print(frame.blobCount);
      Serial.print(F(" active="));
      Serial.print(frame.activeBlobCount);
      Serial.print(F(" ev="));
      Serial.print(frame.directionalEvent);
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
      EyegridHelper::printTemperatureGrid8x8(Eyegrid::pixels, Serial, 1, now,
                                               printMinC, frame.peakMask);
    }
    lastTempPrintAt = now;
  }

  Oled::showBlobCount(frame.activeBlobCount);
  delay(200);
}

