#include "src/ir_beam/ir_beam.h"
#include "src/wifi/wifi_ap.h"

#define LOG wifiLog()

static int occupancy = 0;
static unsigned long totalEntered = 0;
static unsigned long totalExited = 0;

void setup() {

    Serial.begin(115200);
    delay(500);
    LOG.println("--- People Counter ---");
    setupIRSensors();
    setIRDebug(true);
    BeamDiag bootDiag = runBeamBootDiagnostic(1500, 2);
    LOG.print("[ir] boot health A=");
    LOG.print(beamHealthToString(bootDiag.a));
    LOG.print(" B=");
    LOG.print(beamHealthToString(bootDiag.b));
    LOG.print(" togglesA=");
    LOG.print(bootDiag.togglesA);
    LOG.print(" togglesB=");
    LOG.print(bootDiag.togglesB);
    LOG.print(" sampledMs=");
    LOG.println(bootDiag.sampledMs);
    wifiBegin("esp32-people", "people123", &occupancy, &totalEntered, &totalExited);
    LOG.println("System ready.");
}

void loop() {
  int event = getDirectionalCount();
  if (event != 0) {
    if (event == 1) {
      if (occupancy > 0)
        occupancy--;
      totalExited++;
      LOG.println(F("<<< EXIT (-1)"));
    } else {
      occupancy++;
      totalEntered++;
      LOG.println(F(">>> ENTRY (+1)"));
    }
    LOG.print(F("inRoom="));
    LOG.print(occupancy);
    LOG.print(F(" entered="));
    LOG.print(totalEntered);
    LOG.print(F(" exited="));
    LOG.println(totalExited);
    LOG.println(F("-------------------------"));
  }

        // Optional: Heartbeat to prove code isn't frozen
        static unsigned long lastHeartbeat = 0;
        if (millis() - lastHeartbeat > 10000) {
        LOG.print(F("System Status: Monitoring... inRoom="));
        LOG.print(occupancy);
        LOG.print(F(" entered="));
        LOG.print(totalEntered);
        LOG.print(F(" exited="));
        LOG.println(totalExited);
        BeamDiag liveDiag = getBeamHealthLive();
        LOG.print("[ir] live health A=");
        LOG.print(beamHealthToString(liveDiag.a));
        LOG.print(" B=");
        LOG.print(beamHealthToString(liveDiag.b));
        LOG.print(" edgeWindowA=");
        LOG.print(liveDiag.togglesA);
        LOG.print(" edgeWindowB=");
        LOG.print(liveDiag.togglesB);
        LOG.print(" sampledMs=");
        LOG.println(liveDiag.sampledMs);
        lastHeartbeat = millis();
        }

  wifiLoop();
}
