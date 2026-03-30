#ifndef EYEGRID_H
#define EYEGRID_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AMG88xx.h>

namespace Eyegrid {

static Adafruit_AMG88xx amg;
static float pixels[64];

struct SignedEvent {
  int delta;
  unsigned long timestamp;
  bool consumed;
};

enum class Zone { None, Out, Mid, In };

static int stage = 0;  // 0 idle, 1 saw OUT, 2 OUT->MID, 3 saw IN, 4 IN->MID
static unsigned long lastPollMs = 0;
static unsigned long lastTrackedMs = 0;

constexpr float HEAT_THRESHOLD_C = 28.0f;
constexpr float OUT_LINE = 2.5f;  // y < this = OUT
constexpr float IN_LINE  = 4.5f;  // y > this = IN
constexpr unsigned long SAMPLE_INTERVAL_MS = 80;
constexpr unsigned long TRACK_TIMEOUT_MS = 1500;
constexpr size_t EVENT_BUFFER_SIZE = 12;

static SignedEvent eventBuffer[EVENT_BUFFER_SIZE] = {};
static size_t nextEventIndex = 0;

inline Zone zoneOf(float y) {
  if (y < OUT_LINE) return Zone::Out;
  if (y > IN_LINE)  return Zone::In;
  return Zone::Mid;
}

inline bool start(int sda = 21, int scl = 22) {
  Wire.begin(sda, scl);
  return amg.begin(0x69);
}

inline void resetTracker() {
  stage = 0;
  lastTrackedMs = 0;
}

inline void recordEvent(int delta, unsigned long timestamp) {
  eventBuffer[nextEventIndex] = SignedEvent{delta, timestamp, false};
  nextEventIndex = (nextEventIndex + 1) % EVENT_BUFFER_SIZE;
}

inline int processZone(Zone z, float maxT, int y, unsigned long now) {
  int delta = 0;

  switch (stage) {
    case 0:
      if (z == Zone::Out) stage = 1;
      else if (z == Zone::In) stage = 3;
      break;

    case 1:  // saw OUT
      if (z == Zone::Mid) {
        stage = 2;
      } else if (z == Zone::In) {  // OUT->IN quick
        delta = 1;
        stage = 0;
      }
      break;

    case 2:  // OUT->MID
      if (z == Zone::In) {
        delta = 1;
        stage = 0;
      } else if (z == Zone::Out) {
        stage = 1;
      }
      break;

    case 3:  // saw IN
      if (z == Zone::Mid) {
        stage = 4;
      } else if (z == Zone::Out) {
        delta = -1;
        stage = 0;
      }
      break;

    case 4:  // IN->MID
      if (z == Zone::Out) {
        delta = -1;
        stage = 0;
      } else if (z == Zone::In) {
        stage = 3;
      }
      break;
  }

  if (delta != 0) {
    Serial.printf("Grid-EYE event %d (maxT=%.1f at y=%d, t=%lu)\n", delta, maxT, y, now);
  }

  return delta;
}

inline bool update(unsigned long now = millis()) {
  if (now - lastPollMs < SAMPLE_INTERVAL_MS) {
    return false;
  }

  lastPollMs = now;
  amg.readPixels(pixels);

  // find hottest pixel
  float maxT = pixels[0];
  int maxI = 0;
  for (int i = 1; i < 64; i++) {
    if (pixels[i] > maxT) {
      maxT = pixels[i];
      maxI = i;
    }
  }
  int y = maxI / 8;

  if (maxT < HEAT_THRESHOLD_C) {
    if (stage != 0 && lastTrackedMs != 0 && now - lastTrackedMs > TRACK_TIMEOUT_MS) {
      stage = 0;
    }
    return false;
  }

  lastTrackedMs = now;
  Zone z = zoneOf(static_cast<float>(y));
  int delta = processZone(z, maxT, y, now);
  if (delta != 0) {
    recordEvent(delta, now);
    return true;
  }

  return false;
}

inline int consumeWindowDelta(unsigned long triggerTime,
                              unsigned long lookbackMs,
                              unsigned long lookaheadMs) {
  unsigned long windowStart = (triggerTime > lookbackMs) ? (triggerTime - lookbackMs) : 0;
  unsigned long windowEnd = triggerTime + lookaheadMs;
  int total = 0;

  for (size_t i = 0; i < EVENT_BUFFER_SIZE; ++i) {
    SignedEvent &event = eventBuffer[i];
    if (event.delta == 0 || event.consumed) {
      continue;
    }

    if (event.timestamp >= windowStart && event.timestamp <= windowEnd) {
      total += event.delta;
      event.consumed = true;
    }
  }

  return total;
}

}  // namespace Eyegrid

#endif
