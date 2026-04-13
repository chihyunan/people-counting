#include <ir_beam.h>
#include <wifi_ap.h>

static int occupancy = 0;
static unsigned long totalEntered = 0;
static unsigned long totalExited = 0;

void setup() {

    Serial.begin(115200);
    delay(500);
    Serial.println("--- MS1: IR Beam Test Initiated ---");
    setupIRSensors();
    setIRDebug(true);
    BeamDiag bootDiag = runBeamBootDiagnostic(1500, 2);
    Serial.print("[ir] boot health A=");
    Serial.print(beamHealthToString(bootDiag.a));
    Serial.print(" B=");
    Serial.print(beamHealthToString(bootDiag.b));
    Serial.print(" togglesA=");
    Serial.print(bootDiag.togglesA);
    Serial.print(" togglesB=");
    Serial.print(bootDiag.togglesB);
    Serial.print(" sampledMs=");
    Serial.println(bootDiag.sampledMs);
    Serial.println("System Live. Use phone IR or physical break to test.");
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

        // Optional: Heartbeat to prove code isn't frozen
        static unsigned long lastHeartbeat = 0;
        if (millis() - lastHeartbeat > 10000) {
        Serial.println("System Status: Monitoring...");
        BeamDiag liveDiag = getBeamHealthLive();
        Serial.print("[ir] live health A=");
        Serial.print(beamHealthToString(liveDiag.a));
        Serial.print(" B=");
        Serial.print(beamHealthToString(liveDiag.b));
        Serial.print(" edgeWindowA=");
        Serial.print(liveDiag.togglesA);
        Serial.print(" edgeWindowB=");
        Serial.print(liveDiag.togglesB);
        Serial.print(" sampledMs=");
        Serial.println(liveDiag.sampledMs);
        lastHeartbeat = millis();
        }

  wifiLoop();
}
