#include "src/ir_beam/ir_beam.h"
#include "src/wifi/wifi_ap.h"
#include "src/grideye/grideye.h"
#include "src/grideye/grideye_helper.h"

#define LOG wifiLog()

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

static inline bool beamHealthy(BeamHealth h) { return h == BEAM_OK; }

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

  if (!Grideye::start(I2C_SDA_PIN, I2C_SCL_PIN)) {
    LOG.println("[grideye] FAILED to start AMG88xx — continuing without it");
  } else {
    Grideye::CalibrationResult cal =
        Grideye::calibrateThreshold(10000, 100, 1.05f, false);
    heatThresholdC = cal.thresholdC;
    LOG.print("[grideye] thresholdC=");
    LOG.println(heatThresholdC, 2);
  }

  wifiBegin("esp32-people", "people123", &occupancy, &totalEntered, &totalExited);
  LOG.println("System ready.");
}

void loop() {
  // ── sensor reads ──────────────────────────────────────────────────────────
  BeamDiag liveDiag = getBeamHealthLive();
  bool beamsOk = beamHealthy(liveDiag.a) && beamHealthy(liveDiag.b);

  Grideye::FrameResult frame =
      Grideye::poll(heatThresholdC, HEAD_BAND_C, MIN_PEAK_PROMINENCE_C, false);

  // ── fusion logic ──────────────────────────────────────────────────────────
  int deltaEntered = 0;
  int deltaExited  = 0;

  if (!beamsOk) {
    // At least one beam stuck/open/noisy — fall back to GridEye exclusively
    deltaEntered = frame.entriesThisFrame;
    deltaExited  = frame.exitsThisFrame;
  } else {
    int irEvent = getDirectionalCount(); // 1=entry, -1=exit, 0=none
    int geNet   = (int)frame.entriesThisFrame - (int)frame.exitsThisFrame;

    bool agree = (irEvent > 0 && geNet > 0) ||
                 (irEvent < 0 && geNet < 0) ||
                 (irEvent == 0 && geNet == 0);

    if (agree) {
      // Sensors agree on direction → trust GridEye's count (more accurate)
      deltaEntered = frame.entriesThisFrame;
      deltaExited  = frame.exitsThisFrame;
    } else {
      // Sensors disagree → trust breakbeam direction, count 1 event
      if (irEvent > 0)      deltaEntered = 1;
      else if (irEvent < 0) deltaExited  = 1;
      // irEvent == 0 but grideye fired: no confident event, do nothing
    }
  }

  // ── apply deltas ──────────────────────────────────────────────────────────
  for (int i = 0; i < deltaExited; i++) {
    if (occupancy > 0) occupancy--;
    totalExited++;
    LOG.println(F("<<< EXIT (-1)"));
  }
  for (int i = 0; i < deltaEntered; i++) {
    occupancy++;
    totalEntered++;
    LOG.println(F(">>> ENTRY (+1)"));
  }
  if (deltaEntered > 0 || deltaExited > 0) {
    LOG.print(F("inRoom="));
    LOG.print(occupancy);
    LOG.print(F(" entered="));
    LOG.print(totalEntered);
    LOG.print(F(" exited="));
    LOG.println(totalExited);
    LOG.println(F("-------------------------"));
  }

  // ── periodic GridEye temperature grid ────────────────────────────────────
  unsigned long now = millis();
  if (now - lastTempPrintAt >= TEMP_PRINT_INTERVAL_MS) {
    const float printMinC =
        heatThresholdC * GrideyeHelper::PRINT_TEMP_ABOVE_THRESHOLD_RATIO;
    if (GrideyeHelper::anyCellAtLeast(Grideye::pixels, printMinC)) {
      LOG.print(F("[grideye] hot="));
      LOG.print(frame.hotCells);
      LOG.print(F(" blobs="));
      LOG.print(frame.blobCount);
      LOG.print(F(" active="));
      LOG.print(frame.activeBlobCount);
      LOG.print(F(" entries="));
      LOG.print(frame.entriesThisFrame);
      LOG.print(F(" exits="));
      LOG.print(frame.exitsThisFrame);
      LOG.print(F(" inRoom="));
      LOG.print(occupancy);
      LOG.print(F(" entered="));
      LOG.print(totalEntered);
      LOG.print(F(" exited="));
      LOG.print(totalExited);
      LOG.print(F(" frameMaxC="));
      LOG.print(frame.frameMaxC, 2);
      LOG.print(F(" peakFloorC="));
      LOG.println(frame.peakFloorC, 2);
      GrideyeHelper::printTemperatureGrid8x8(Grideye::pixels, Serial, 1, now,
                                             printMinC, frame.peakMask);
    }
    lastTempPrintAt = now;
  }

  // ── heartbeat ─────────────────────────────────────────────────────────────
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 10000) {
    LOG.print(F("System Status: Monitoring... inRoom="));
    LOG.print(occupancy);
    LOG.print(F(" entered="));
    LOG.print(totalEntered);
    LOG.print(F(" exited="));
    LOG.println(totalExited);
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
  delay(200);
}
