#include "ir_beam.h"

// Hardware Pins (Use ESP32 pins that support interrupts)
const int PIN_A = 16; 
const int PIN_B = 17;

// Internal State Variables
volatile bool triggeredA = false;
volatile bool triggeredB = false;
volatile unsigned long timeA = 0;
volatile unsigned long timeB = 0;

// Function to initialize pins and interrupts
void setupIRSensors() {
    pinMode(PIN_A, INPUT_PULLUP);
    pinMode(PIN_B, INPUT_PULLUP);
    
    // Attach interrupts: Trigger on falling edge (NPN sensor break)
    attachInterrupt(digitalPinToInterrupt(PIN_A), handleSensorA, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_B), handleSensorB, FALLING);
}

// ISR for Sensor A
void IRAM_ATTR handleSensorA() {
    if (!triggeredA && !triggeredB) { // A is the first one hit
        triggeredA = true;
        timeA = millis();
    }
}

// ISR for Sensor B
void IRAM_ATTR handleSensorB() {
    if (!triggeredB && !triggeredA) { // B is the first one hit
        triggeredB = true;
        timeB = millis();
    }
}
void resetBeams() {
    triggeredA = false;
    triggeredB = false;
    timeA = 0;
    timeB = 0;
}

// Main logic to determine if a full "pass" occurred
int getDirectionalCount() {
    int result = 0;
    unsigned long now = millis();
    const unsigned long timeout = 1500; // 1.5s to complete a walk-through

    // Case 1: Started with A, waiting for B
    if (triggeredA && digitalRead(PIN_B) == LOW) {
        result = 1; // Entry
        resetBeams();
    }
    // Case 2: Started with B, waiting for A
    else if (triggeredB && digitalRead(PIN_A) == LOW) {
        result = -1; // Exit
        resetBeams();
    }
    // Case 3: Timeout (someone stood in the way or swiped only one beam)
    else if ((triggeredA || triggeredB) && (now - (triggeredA ? timeA : timeB) > timeout)) {
        resetBeams();
    }

    return result;
}
