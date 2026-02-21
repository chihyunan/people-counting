#ifndef THERMAL_CAMERA_H
#define THERMAL_CAMERA_H

#include <Arduino.h>
#include <Adafruit_AMG88xx.h>
#include <U8g2lib.h>

// Export variables so main.ino can read the counts
extern long enterCount;
extern long exitCount;

void setupThermalCamera();
void updateThermalLogic();
void displayCounts(); // OLED update

#endif