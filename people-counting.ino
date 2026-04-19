#include "src/eyegrid/eyegrid.h"
#include "src/eyegrid/eyegrid_helper.h"
#include "src/oled/oled_display.h"

long enterCount = 0, exitCount = 0;
static float heatThresholdC = 28.0f;
static const unsigned long TEMP_PRINT_INTERVAL_MS = 5000;
static unsigned long lastTempPrintAt = 0;

void setup() {
  Serial.begin(115200);
  Oled::begin();
  Oled::showCounts(enterCount, exitCount);
  if (!Eyegrid::start()) {
    Serial.println("Grid-EYE not found.");
    for (;;) delay(1000);
  }
  Serial.println("EyeGrid calibration started (10s)...");
  Eyegrid::CalibrationResult calibration = Eyegrid::calibrateThreshold(10000, 100, 1.10f, true);
  heatThresholdC = calibration.thresholdC;
  Serial.print("EyeGrid threshold active: ");
  Serial.println(heatThresholdC, 2);
  Serial.println("Grid-EYE started.");
}

void loop() {
  Eyegrid::FrameResult frame = Eyegrid::poll(heatThresholdC, true);
  enterCount += frame.entered;
  exitCount += frame.exited;

  unsigned long now = millis();
  if (now - lastTempPrintAt >= TEMP_PRINT_INTERVAL_MS) {
    EyegridHelper::printTemperatureGrid8x8(Eyegrid::pixels);
    lastTempPrintAt = now;
  }

  Oled::showCounts(enterCount, exitCount);
  delay(200);
}
