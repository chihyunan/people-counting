#include "ir_beam.h"

const int PIN_A = 36;
const int PIN_B = 39;
const unsigned long NOISE_FLOOR = 5;     // ms: ignore near-simultaneous breaks (EMI / noise)
const unsigned long MAX_BLOCK = 3000;    // ms: max time between A and B break; also armed timeout
const unsigned long MIN_EVENT_GAP = 300; // ms: minimum time between accepted counts

volatile bool isBlockedA = false;
volatile bool isBlockedB = false;
volatile unsigned long timeA = 0;
volatile unsigned long timeB = 0;

volatile unsigned long clearA = 0;
volatile unsigned long clearB = 0;

static unsigned long absDiffU(unsigned long a, unsigned long b) {
  return (a > b) ? (a - b) : (b - a);
}

// +1 if beam A broke first, -1 if B broke first (equal time => -1, same as before)
static int breakDirection() { return (timeA < timeB) ? 1 : -1; }

// +1 if A cleared first, -1 if B cleared first (equal => -1, same as before)
static int clearDirection() { return (clearA < clearB) ? 1 : -1; }

static void IRAM_ATTR updateBeamEdge(int pin, volatile bool *blocked,
                                     volatile unsigned long *breakAt,
                                     volatile unsigned long *clearAt) {
  if (digitalRead(pin) == LOW) {
    *blocked = true;
    *breakAt = millis();
  } else {
    *blocked = false;
    *clearAt = millis();
  }
}

void setupIRSensors() {
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  // LOW = beam broken (sensor pulls down). CHANGE = break and clear edges.
  attachInterrupt(digitalPinToInterrupt(PIN_A), handleSensorA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_B), handleSensorB, CHANGE);
}

void IRAM_ATTR handleSensorA() {
  updateBeamEdge(PIN_A, &isBlockedA, &timeA, &clearA);
}

void IRAM_ATTR handleSensorB() {
  updateBeamEdge(PIN_B, &isBlockedB, &timeB, &clearB);
}

/*
 * FSM (one call = one pass through, order matters):
 * 1) Arm: both beams blocked, break times differ by (NOISE_FLOOR, MAX_BLOCK).
 * 2) Commit: both clear; exit order must match break direction; MIN_EVENT_GAP.
 * 3) Timeout: still armed but no valid completion within MAX_BLOCK of last break.
 */
int getDirectionalCount() {
  unsigned long now = millis();
  static bool passInProgress = false;
  static int detectedDir = 0;
  static unsigned long lastCountReturnAt = 0;

  if (isBlockedA && isBlockedB && !passInProgress) {
    unsigned long diff = absDiffU(timeA, timeB);
    if (diff > NOISE_FLOOR && diff < MAX_BLOCK) {
      detectedDir = breakDirection();
      passInProgress = true;
    }
  }

  if (passInProgress && !isBlockedA && !isBlockedB) {
    passInProgress = false;

    if (clearDirection() != detectedDir) {
      detectedDir = 0;
      return 0;
    }

    if (lastCountReturnAt != 0 && (now - lastCountReturnAt) < MIN_EVENT_GAP) {
      detectedDir = 0;
      return 0;
    }

    int finalResult = detectedDir;
    lastCountReturnAt = now;
    detectedDir = 0;
    return finalResult;
  }

  if (passInProgress && (now - max(timeA, timeB) > MAX_BLOCK)) {
    passInProgress = false;
    detectedDir = 0;
  }

  return 0;
}
