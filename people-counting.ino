#include "src/eyegrid/eyegrid_old.h"
#include "src/eyegrid/eyegrid_helper.h"
#include "src/eyegrid/scanner.h"
#include "src/oled/oled_display.h"

long enterCount = 0, exitCount = 0;
static const float HEAT_THRESHOLD_C = 28.0f;
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
  Serial.println("Grid-EYE started.");
}

void loop() {
  Eyegrid::amg.readPixels(Eyegrid::pixels);

  EyegridHelper::BinaryGrid8x8 mask =
      EyegridHelper::thresholdMask8x8(Eyegrid::pixels, HEAT_THRESHOLD_C);
  Scanner::CountTuple delta = Scanner::scan(mask);
  enterCount += delta.entry;
  exitCount += delta.exit;

  unsigned long now = millis();
  if (now - lastTempPrintAt >= TEMP_PRINT_INTERVAL_MS) {
    EyegridHelper::printTemperatureGrid8x8(Eyegrid::pixels);
    lastTempPrintAt = now;
  }

  Oled::showCounts(enterCount, exitCount);
  delay(200);
}
