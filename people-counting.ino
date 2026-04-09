#include <ir_beam.h>
#include <wifi_ap.h>

static int occupancy = 0;
static unsigned long totalEntered = 0;
static unsigned long totalExited = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("--- IR beam + WiFi ---"));
  setupIRSensors();
  wifiBegin("esp32-people", "people123", &occupancy, &totalEntered, &totalExited);
}

void loop() {
  int event = getDirectionalCount();
  if (event != 0) {
    if (event == 1) {
      occupancy++;
      totalEntered++;
      Serial.println(F(">>> ENTRY (+1)"));
    } else {
      if (occupancy > 0)
        occupancy--;
      totalExited++;
      Serial.println(F("<<< EXIT (-1)"));
    }
    Serial.print(F("inRoom="));
    Serial.print(occupancy);
    Serial.print(F(" entered="));
    Serial.print(totalEntered);
    Serial.print(F(" exited="));
    Serial.println(totalExited);
    Serial.println(F("-------------------------"));
  }

  static unsigned long lastHb = 0;
  if (millis() - lastHb > 10000) {
    Serial.println(F("System Status: Monitoring..."));
    lastHb = millis();
  }

  wifiLoop();
}
