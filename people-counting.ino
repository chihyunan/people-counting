#include "src/eyegrid/eyegrid.h"
#include "src/eyegrid/eyegrid_helper.h"
#include "src/oled/oled_display.h"

long enterCount = 0, exitCount = 0;
static float heatThresholdC = 28.0f;
static const unsigned long TEMP_PRINT_INTERVAL_MS = 5000;
static unsigned long lastTempPrintAt = 0;
static const int I2C_SDA_PIN = 21;
static const int I2C_SCL_PIN = 22;

void setup() {
  Serial.begin(115200);
  Oled::begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Oled::showCounts(enterCount, exitCount);
  if (!Eyegrid::start(I2C_SDA_PIN, I2C_SCL_PIN)) {
    for (;;) delay(1000);
  }
  Eyegrid::CalibrationResult calibration = Eyegrid::calibrateThreshold(10000, 100, 1.10f, false);
  heatThresholdC = calibration.thresholdC;
}

void loop() {
  Eyegrid::FrameResult frame = Eyegrid::poll(heatThresholdC, false);
  enterCount += frame.entered;
  exitCount += frame.exited;

  unsigned long now = millis();
  if (now - lastTempPrintAt >= TEMP_PRINT_INTERVAL_MS) {
    const float printMinC =
        heatThresholdC * EyegridHelper::PRINT_TEMP_ABOVE_THRESHOLD_RATIO;
    if (EyegridHelper::anyCellAtLeast(Eyegrid::pixels, printMinC)) {
      EyegridHelper::printTemperatureGrid8x8(Eyegrid::pixels, Serial, 1, now,
                                               printMinC);
    }
    lastTempPrintAt = now;
  }

  Oled::showCounts(enterCount, exitCount);
  delay(200);
}
