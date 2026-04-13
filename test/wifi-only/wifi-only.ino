#include <wifi_ap.h>

// WiFi-only harness.
// No IR dependency: this sketch mutates counters on a timer so /json and /state
// can be validated independently from sensor hardware.
static int occupancy = 0;
static unsigned long totalEntered = 0;
static unsigned long totalExited = 0;

constexpr unsigned long STEP_MS = 1500;

void setup() {
  Serial.begin(115200);
  delay(400);
  Serial.println(F("--- WiFi-only harness ---"));
  wifiBegin("esp32-people-test", "people123", &occupancy, &totalEntered, &totalExited);
}

void loop() {
  static unsigned long lastStepAt = 0;
  static bool doEntry = true;
  static unsigned long lastHeartbeatAt = 0;

  if (millis() - lastStepAt >= STEP_MS) {
    if (doEntry) {
      occupancy++;
      totalEntered++;
      Serial.println(F("SIM >>> ENTRY (+1)"));
    } else {
      if (occupancy > 0) {
        occupancy--;
      }
      totalExited++;
      Serial.println(F("SIM <<< EXIT (-1)"));
    }
    doEntry = !doEntry;
    lastStepAt = millis();

    Serial.print(F("inRoom="));
    Serial.print(occupancy);
    Serial.print(F(" entered="));
    Serial.print(totalEntered);
    Serial.print(F(" exited="));
    Serial.println(totalExited);
  }

  if (millis() - lastHeartbeatAt > 10000) {
    Serial.println(F("[wifi-only] running"));
    lastHeartbeatAt = millis();
  }

  wifiLoop();
}
