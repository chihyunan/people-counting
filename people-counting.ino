#include "ir_beam.h"
#include "thermal_camera.h"

// If your repo's ir_beam.h uses different names, keep yours and adjust these.
// In the original repo, it uses setupIRSensors() and getDirectionalCount().

static long totalCount = 0;   // official occupancy count (from IR)

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("\n--- People Counting Start ---");

  // 1) IR Break-beam setup (repo)
  setupIRSensors();

  // 2) Thermal + OLED setup (your integrated thermal_camera.cpp)
  setupThermalCamera();

  Serial.println("Setup complete.");
}

void loop() {
  // ----- IR BEAM LOGIC (official entry/exit events) -----
  // getDirectionalCount():  +1 = entry, -1 = exit, 0 = none
  int ev = getDirectionalCount();
  if (ev == 1) {
    totalCount++;
    Serial.printf("IR: ENTRY  | Occupancy=%ld\n", totalCount);
  } else if (ev == -1) {
    if (totalCount > 0) totalCount--;   // prevent going negative
    Serial.printf("IR: EXIT   | Occupancy=%ld\n", totalCount);
  }

  // ----- THERMAL + OLED (your zone test) -----
  // This updates the OLED every ~200ms inside the function
  updateThermalLogic();

  // Optional: print when thermal "confirms" activity
  if (isHumanConfirmed()) {
    Serial.println("Thermal: activity detected");
  }

  delay(20);
}
