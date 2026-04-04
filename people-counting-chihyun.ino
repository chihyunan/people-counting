#include "ir_beam.h"



static int occupancy = 0;

void setup() {

    Serial.begin(115200);
    delay(500);
    Serial.println("--- MS1: IR Beam Test Initiated ---");
    setupIRSensors();
    Serial.println("System Live. Use phone IR or physical break to test.");
}

void loop() {
    //if interrupt triggered, update count 
    int event = getDirectionalCount();
    if (event != 0) {
        //or, call GridEye to validate before update 
        if (event == 1) {
            occupancy++;
            Serial.println(">>> [EVENT] ENTRY (+1)");
            } else {
                if (occupancy > 0) occupancy--;
                    Serial.println("<<< [EVENT] EXIT (-1)");
            }
            Serial.print("Current Occupancy: ");
            Serial.println(occupancy);
            Serial.println("-------------------------");
    }

        // Optional: Heartbeat to prove code isn't frozen
        static unsigned long lastHeartbeat = 0;
        if (millis() - lastHeartbeat > 10000) {
        Serial.println("System Status: Monitoring...");
        lastHeartbeat = millis();
        }

    }