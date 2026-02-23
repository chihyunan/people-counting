#include "src/eyegrid/eyegrid.h"
#include "src/oled/oled_display.h"

long enterCount = 0, exitCount = 0;

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
  Eyegrid::PeopleDelta delta = Eyegrid::poll();
  enterCount += delta.entered;
  exitCount += delta.exited;
  Oled::showCounts(enterCount, exitCount);
  delay(2000);
}
