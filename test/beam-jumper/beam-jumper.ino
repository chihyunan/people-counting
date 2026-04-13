#include <ir_beam.h>
#include <wifi_ap.h>

// Beam jumper harness.
// Wire GPIO25 -> GPIO36 (A) and GPIO26 -> GPIO39 (B), with common GND.
// The harness drives LOW pulses (beam break) and HIGH restore (beam clear)
// so you can validate directional counting without optical sensors.
constexpr int DRIVE_A = 25;
constexpr int DRIVE_B = 26;
constexpr unsigned long STEP_MS = 35;
constexpr unsigned long CYCLE_PAUSE_MS = 800;

static int occupancy = 0;
static unsigned long totalEntered = 0;
static unsigned long totalExited = 0;

enum SimMode {
  SIM_ENTRY,
  SIM_EXIT
};

static void driveIdle() {
  digitalWrite(DRIVE_A, HIGH);
  digitalWrite(DRIVE_B, HIGH);
}

static void pulseEntry() {
  digitalWrite(DRIVE_A, LOW);
  delay(STEP_MS);
  digitalWrite(DRIVE_B, LOW);
  delay(STEP_MS);
  digitalWrite(DRIVE_A, HIGH);
  delay(STEP_MS);
  digitalWrite(DRIVE_B, HIGH);
}

static void pulseExit() {
  digitalWrite(DRIVE_B, LOW);
  delay(STEP_MS);
  digitalWrite(DRIVE_A, LOW);
  delay(STEP_MS);
  digitalWrite(DRIVE_B, HIGH);
  delay(STEP_MS);
  digitalWrite(DRIVE_A, HIGH);
}

static void runSimulation(SimMode mode) {
  if (mode == SIM_ENTRY) {
    pulseEntry();
  } else {
    pulseExit();
  }
}

void setup() {
  Serial.begin(115200);
  delay(400);
  Serial.println(F("--- Beam jumper harness ---"));
  Serial.println(F("Wiring: GPIO25->GPIO36, GPIO26->GPIO39"));

  pinMode(DRIVE_A, OUTPUT);
  pinMode(DRIVE_B, OUTPUT);
  driveIdle();

  setupIRSensors();
  wifiBegin("esp32-people-test", "people123", &occupancy, &totalEntered, &totalExited);
}

void loop() {
  static SimMode nextMode = SIM_ENTRY;
  static unsigned long lastPulseAt = 0;
  static unsigned long lastHeartbeatAt = 0;

  if (millis() - lastPulseAt >= CYCLE_PAUSE_MS) {
    runSimulation(nextMode);
    nextMode = (nextMode == SIM_ENTRY) ? SIM_EXIT : SIM_ENTRY;
    lastPulseAt = millis();
  }

  int event = getDirectionalCount();
  if (event != 0) {
    if (event == 1) {
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

    Serial.print(F("inRoom="));
    Serial.print(occupancy);
    Serial.print(F(" entered="));
    Serial.print(totalEntered);
    Serial.print(F(" exited="));
    Serial.println(totalExited);
  }

  if (millis() - lastHeartbeatAt > 10000) {
    Serial.println(F("[beam-jumper] running"));
    lastHeartbeatAt = millis();
  }

  wifiLoop();
}
