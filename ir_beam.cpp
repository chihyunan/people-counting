#include "ir_beam.h"

const int PIN_A =36; 
const int PIN_B =39;
const unsigned long NOISE_FLOOR = 5;    // 5ms: Human bodies aren't faster than this
const unsigned long MAX_BLOCK = 3000;   // 3s: If blocked longer, ignore as obstacle
const unsigned long WINDOW = 1000;      // 1s to finish the A->B sequence
const unsigned long MIN_EVENT_GAP = 200; // Ignore counts that occur too soon after the last valid event

volatile bool isBlockedA = false;
volatile bool isBlockedB = false;
volatile unsigned long timeA = 0;
volatile unsigned long timeB = 0;

volatile unsigned long clearA = 0;
volatile unsigned long clearB = 0;

void setupIRSensors() {
    pinMode(PIN_A, INPUT_PULLUP); //INPUT if using FALLING / RISING 
    pinMode(PIN_B, INPUT_PULLUP);

    // Trigger on any voltage change
    // Sensor FALLS from 3.3 to 0 when BROKEN 
    //Noise (phone) flashes break for ~30us
    attachInterrupt(digitalPinToInterrupt(PIN_A), handleSensorA, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_B), handleSensorB, CHANGE);
}

//use IRAM_ATTR to ensure interrupt stores in fastest memory, shorter pause 
void IRAM_ATTR handleSensorA() {
    bool state = digitalRead(PIN_A);
    //when interrupt triggered by CHANGE, already looking at Pin_A so digitalRead speed is not a concern
    if (state == LOW) { // FALLING
        isBlockedA = true;
        timeA = millis();
    } else { // RISING
        isBlockedA = false;
        clearA = millis(); // record when restored
    }
}
void IRAM_ATTR handleSensorB() {
    bool state = digitalRead(PIN_B);
    if (state == LOW){
        isBlockedB = true;
        timeB = millis();
    }   else{
        isBlockedB = false;
        clearB = millis(); //
    }
}

//this is called by loop, sees if interrupt has updated triggerA, B
int getDirectionalCount() {
    unsigned long now = millis();
    static bool passInProgress = false;
    static int detectedDir = 0;
    static unsigned long lastCountReturnAt = 0;

    // 1. SENSOR BREAKS (The Entry/Exit Start)
    if (isBlockedA && isBlockedB && !passInProgress) {
        unsigned long diff = (timeA > timeB) ? (timeA - timeB) : (timeB - timeA);
        
        //NOISE FILTER: If A and B triggered within < 5ms of each other, 
        // it's likely an electrical spike/EMI, not a human moving 3-6cm.
        //OBSTACLE FILTER: If A - B > 3s, someone lingered.
        if (diff > NOISE_FLOOR && diff < MAX_BLOCK) { //if diff within Plausible  
            detectedDir = (timeA < timeB) ? 1 : -1; //direction
            passInProgress = true; // Lock the logic until beams clear
        }
    }

    // 2. RETURN DIRECTION: Only return the count once the "Zone" is empty
    // Stander problem solution (B3)
    if (passInProgress && !isBlockedA && !isBlockedB) {
        passInProgress = false; 
        
        // Check if the order of clearing matches the order of entering
        // For +1 (A then B), they should clear A then B (clearA < clearB)
        int exitDir = (clearA < clearB) ? 1 : -1;

        if (exitDir == detectedDir) {
            if (lastCountReturnAt != 0 && (now - lastCountReturnAt) < MIN_EVENT_GAP) {
                detectedDir = 0;
                return 0;
            }

            int finalResult = detectedDir;
            lastCountReturnAt = now;
            detectedDir = 0;
            return finalResult; // Valid pass confirmed, in future can use this to call GridEYE
        } else {
            detectedDir = 0;
            return 0; // Person backed out; ignore count
        }
    }

    // 3. TIMEOUT: If they stand there for 3s, reset without counting
    if (passInProgress && (now - max(timeA, timeB) > MAX_BLOCK)) {
        passInProgress = false;
        detectedDir = 0;
    }

    return 0;
}
