#include "ir_beam.h"
#include "thermal_camera.h"

void setup() {
  Serial.begin(115200);
  setupIRSensors(); // Defined in your other file
}

void loop() {
  int event = getDirectionalCount();
  if (event != 0) {
    // This is where you would "toggle" the GridEye
    Serial.println(event == 1 ? "Entry Detected" : "Exit Detected");
  }
  updateThermalLogic();
}