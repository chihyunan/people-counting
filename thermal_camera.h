#ifndef THERMAL_CAMERA_H
#define THERMAL_CAMERA_H

#include <Arduino.h>

// Initialize the AMG8833 and the tracking algorithm
void setupThermalCamera();

// The main verification function
// Returns true if the algorithm currently tracks a human blob
bool isHumanVerified();

// Optional: Get the count from the internal tracking algorithm
// to compare against your IR beam count
int getThermalRegistryCount();

void updateThermalLogic();

#endif