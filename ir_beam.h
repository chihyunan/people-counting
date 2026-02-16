#ifndef IR_BEAM_H
#define IR_BEAM_H

#include <Arduino.h>

// Function Declarations
void setupIRSensors();
void IRAM_ATTR handleSensorA();
void IRAM_ATTR handleSensorB();
int getDirectionalCount(); // Returns 1 for Entry, -1 for Exit, 0 for None

#endif