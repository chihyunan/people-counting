#include "ir_beam.h"
#include "../wifi/wifi_ap.h"

#define LOG wifiLog()

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
volatile uint8_t pendingEdgeFlags = 0;
volatile unsigned long lastEdgeAtA = 0;
volatile unsigned long lastEdgeAtB = 0;
volatile uint32_t edgeCountA = 0;
volatile uint32_t edgeCountB = 0;

static bool debugEnabled = false;
static const unsigned long LIVE_STUCK_MS = 20000;
static const unsigned long LIVE_WINDOW_MS = 5000;

enum EdgeFlagBits : uint8_t {
  EDGE_A_BREAK = 1 << 0,
  EDGE_A_CLEAR = 1 << 1,
  EDGE_B_BREAK = 1 << 2,
  EDGE_B_CLEAR = 1 << 3,
};

static unsigned long absDiffU(unsigned long a, unsigned long b) {
  return (a > b) ? (a - b) : (b - a);
}

// +1 if beam A broke first, -1 if B broke first (equal time => -1, same as before)
static int breakDirection() { return (timeA < timeB) ? 1 : -1; }

// +1 if A cleared first, -1 if B cleared first (equal => -1, same as before)
static int clearDirection() { return (clearA < clearB) ? 1 : -1; }

static BeamHealth classifyBeam(uint32_t highCount, uint32_t lowCount, uint32_t toggles,
                               uint32_t totalSamples) {
  if (totalSamples == 0) {
    return BEAM_UNKNOWN;
  }
  if (lowCount >= (totalSamples * 98UL) / 100UL) {
    return BEAM_STUCK_LOW;
  }
  if (highCount >= (totalSamples * 98UL) / 100UL && toggles == 0) {
    return BEAM_STUCK_HIGH_OR_OPEN;
  }
  if (toggles > (totalSamples / 8UL)) {
    return BEAM_FLOATING_NOISY;
  }
  return BEAM_OK;
}

const char *beamHealthToString(BeamHealth health) {
  switch (health) {
  case BEAM_OK:
    return "OK";
  case BEAM_STUCK_LOW:
    return "STUCK_LOW";
  case BEAM_STUCK_HIGH_OR_OPEN:
    return "STUCK_HIGH_OR_OPEN";
  case BEAM_FLOATING_NOISY:
    return "FLOATING_NOISY";
  case BEAM_UNKNOWN:
  default:
    return "UNKNOWN";
  }
}

static void IRAM_ATTR updateBeamEdge(int pin, volatile bool *blocked,
                                     volatile unsigned long *breakAt,
                                     volatile unsigned long *clearAt, uint8_t breakFlag,
                                     uint8_t clearFlag, volatile unsigned long *lastEdgeAt,
                                     volatile uint32_t *edgeCount) {
  unsigned long now = millis();
  if (digitalRead(pin) == LOW) {
    *blocked = true;
    *breakAt = now;
    pendingEdgeFlags |= breakFlag;
  } else {
    *blocked = false;
    *clearAt = now;
    pendingEdgeFlags |= clearFlag;
  }
  *lastEdgeAt = now;
  *edgeCount = *edgeCount + 1;
}

static void flushDebugEdges() {
  if (!debugEnabled) {
    return;
  }

  uint8_t flags = 0;
  unsigned long localTimeA = 0;
  unsigned long localClearA = 0;
  unsigned long localTimeB = 0;
  unsigned long localClearB = 0;

  noInterrupts();
  flags = pendingEdgeFlags;
  pendingEdgeFlags = 0;
  localTimeA = timeA;
  localClearA = clearA;
  localTimeB = timeB;
  localClearB = clearB;
  interrupts();

  if (flags & EDGE_A_BREAK) {
    LOG.print(F("[ir] A break t="));
    LOG.println(localTimeA);
  }
  if (flags & EDGE_A_CLEAR) {
    LOG.print(F("[ir] A clear t="));
    LOG.println(localClearA);
  }
  if (flags & EDGE_B_BREAK) {
    LOG.print(F("[ir] B break t="));
    LOG.println(localTimeB);
  }
  if (flags & EDGE_B_CLEAR) {
    LOG.print(F("[ir] B clear t="));
    LOG.println(localClearB);
  }
}

void setupIRSensors() {
  pinMode(PIN_A, INPUT);
  pinMode(PIN_B, INPUT);
  // LOW = beam broken (sensor pulls down). CHANGE = break and clear edges.
  attachInterrupt(digitalPinToInterrupt(PIN_A), handleSensorA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_B), handleSensorB, CHANGE);
}

void IRAM_ATTR handleSensorA() {
  updateBeamEdge(PIN_A, &isBlockedA, &timeA, &clearA, EDGE_A_BREAK, EDGE_A_CLEAR, &lastEdgeAtA,
                 &edgeCountA);
}

void IRAM_ATTR handleSensorB() {
  updateBeamEdge(PIN_B, &isBlockedB, &timeB, &clearB, EDGE_B_BREAK, EDGE_B_CLEAR, &lastEdgeAtB,
                 &edgeCountB);
}

void setIRDebug(bool enabled) {
  debugEnabled = enabled;
  if (debugEnabled) {
    LOG.println(F("[ir] debug enabled"));
  } else {
    LOG.println(F("[ir] debug disabled"));
  }
}

BeamDiag runBeamBootDiagnostic(unsigned long windowMs, unsigned long sampleMs) {
  if (sampleMs == 0) {
    sampleMs = 1;
  }

  unsigned long start = millis();
  int prevA = digitalRead(PIN_A);
  int prevB = digitalRead(PIN_B);
  uint32_t highA = (prevA == HIGH) ? 1 : 0;
  uint32_t lowA = (prevA == LOW) ? 1 : 0;
  uint32_t highB = (prevB == HIGH) ? 1 : 0;
  uint32_t lowB = (prevB == LOW) ? 1 : 0;
  uint16_t togglesA = 0;
  uint16_t togglesB = 0;
  uint32_t samples = 1;

  while ((millis() - start) < windowMs) {
    delay(sampleMs);
    int currA = digitalRead(PIN_A);
    int currB = digitalRead(PIN_B);
    if (currA != prevA) {
      togglesA++;
      prevA = currA;
    }
    if (currB != prevB) {
      togglesB++;
      prevB = currB;
    }
    if (currA == HIGH) {
      highA++;
    } else {
      lowA++;
    }
    if (currB == HIGH) {
      highB++;
    } else {
      lowB++;
    }
    samples++;
  }

  BeamDiag out;
  out.a = classifyBeam(highA, lowA, togglesA, samples);
  out.b = classifyBeam(highB, lowB, togglesB, samples);
  out.sampledMs = millis() - start;
  out.togglesA = togglesA;
  out.togglesB = togglesB;
  return out;
}

BeamDiag getBeamHealthLive() {
  static unsigned long prevCheckMs = 0;
  static uint32_t prevEdgeCountA = 0;
  static uint32_t prevEdgeCountB = 0;

  unsigned long now = millis();
  bool blockedA = false;
  bool blockedB = false;
  unsigned long localLastEdgeAtA = 0;
  unsigned long localLastEdgeAtB = 0;
  uint32_t localEdgeCountA = 0;
  uint32_t localEdgeCountB = 0;

  noInterrupts();
  blockedA = isBlockedA;
  blockedB = isBlockedB;
  localLastEdgeAtA = lastEdgeAtA;
  localLastEdgeAtB = lastEdgeAtB;
  localEdgeCountA = edgeCountA;
  localEdgeCountB = edgeCountB;
  interrupts();

  unsigned long elapsed = (prevCheckMs == 0) ? 0 : (now - prevCheckMs);
  uint32_t edgesInWindowA = localEdgeCountA - prevEdgeCountA;
  uint32_t edgesInWindowB = localEdgeCountB - prevEdgeCountB;

  BeamDiag out;
  out.sampledMs = elapsed;
  out.togglesA = (edgesInWindowA > 65535UL) ? 65535 : (uint16_t)edgesInWindowA;
  out.togglesB = (edgesInWindowB > 65535UL) ? 65535 : (uint16_t)edgesInWindowB;
  out.a = BEAM_OK;
  out.b = BEAM_OK;

  if (elapsed > 0 && elapsed <= LIVE_WINDOW_MS) {
    if (edgesInWindowA > (elapsed / 8UL)) {
      out.a = BEAM_FLOATING_NOISY;
    }
    if (edgesInWindowB > (elapsed / 8UL)) {
      out.b = BEAM_FLOATING_NOISY;
    }
  }

  if (out.a == BEAM_OK && blockedA && localLastEdgeAtA != 0 && (now - localLastEdgeAtA) > LIVE_STUCK_MS) {
    out.a = BEAM_STUCK_LOW;
  }
  if (out.b == BEAM_OK && blockedB && localLastEdgeAtB != 0 && (now - localLastEdgeAtB) > LIVE_STUCK_MS) {
    out.b = BEAM_STUCK_LOW;
  }

  if (out.a == BEAM_OK && !blockedA && localLastEdgeAtA == 0 && now > LIVE_STUCK_MS) {
    out.a = BEAM_STUCK_HIGH_OR_OPEN;
  }
  if (out.b == BEAM_OK && !blockedB && localLastEdgeAtB == 0 && now > LIVE_STUCK_MS) {
    out.b = BEAM_STUCK_HIGH_OR_OPEN;
  }

  prevCheckMs = now;
  prevEdgeCountA = localEdgeCountA;
  prevEdgeCountB = localEdgeCountB;
  return out;
}

/*
 * FSM (one call = one pass through, order matters):
 * 1) Arm: both beams blocked, break times differ by (NOISE_FLOOR, MAX_BLOCK).
 * 2) Commit: both clear; exit order must match break direction; MIN_EVENT_GAP.
 * 3) Timeout: still armed but no valid completion within MAX_BLOCK of last break.
 */
int getDirectionalCount() {
  flushDebugEdges();
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
