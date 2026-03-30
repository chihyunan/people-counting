#ifndef EYEGRID_H
#define EYEGRID_H

#include <Wire.h>
#include <Adafruit_AMG88xx.h>

namespace Eyegrid {

static Adafruit_AMG88xx amg;
static float pixels[64];

struct PeopleDelta {
  int entered;
  int exited;
};

enum class Zone { Out, Mid, In };

static int stage = 0;  // 0 idle, 1 saw OUT, 2 OUT->MID, 3 saw IN, 4 IN->MID

constexpr float HEAT_THRESHOLD_C = 28.0f;
constexpr float OUT_LINE = 2.5f;  // y < this = OUT
constexpr float IN_LINE  = 4.5f;  // y > this = IN

inline Zone zoneOf(float y) {
  if (y < OUT_LINE) return Zone::Out;
  if (y > IN_LINE)  return Zone::In;
  return Zone::Mid;
}

inline bool start(int sda = 21, int scl = 22) {
  Wire.begin(sda, scl);
  return amg.begin(0x69);
}

inline PeopleDelta poll() {
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
  int x = maxI % 8;
  int y = maxI / 8;

  Zone z = zoneOf(static_cast<float>(y));

  PeopleDelta d{0, 0};

  // Simple zone-crossing FSM (OUT->MID->IN = enter, IN->MID->OUT = exit)
  switch (stage) {
    case 0:
      if (z == Zone::Out) stage = 1;
      else if (z == Zone::In) stage = 3;
      break;

    case 1:  // saw OUT
      if (z == Zone::Mid) {
        stage = 2;
      } else if (z == Zone::In) {  // OUT->IN quick
        d.entered++;
        Serial.printf("ENTER  (maxT=%.1f at y=%d)\n", maxT, y);
        stage = 0;
      }
      break;

    case 2:  // OUT->MID
      if (z == Zone::In) {
        d.entered++;
        Serial.printf("ENTER  (maxT=%.1f at y=%d)\n", maxT, y);
        stage = 0;
      } else if (z == Zone::Out) {
        stage = 1;
      }
      break;

    case 3:  // saw IN
      if (z == Zone::Mid) {
        stage = 4;
      } else if (z == Zone::Out) {
        d.exited++;
        Serial.printf("EXIT   (maxT=%.1f at y=%d)\n", maxT, y);
        stage = 0;
      }
      break;

    case 4:  // IN->MID
      if (z == Zone::Out) {
        d.exited++;
        Serial.printf("EXIT   (maxT=%.1f at y=%d)\n", maxT, y);
        stage = 0;
      } else if (z == Zone::In) {
        stage = 3;
      }
      break;
  }

  // still print full 8x8 grid for debugging
  Serial.println("8x8 (C):");
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      float t = pixels[row * 8 + col];
      Serial.print(t, 1);
      if (col < 7) Serial.print(" ");
    }
    Serial.println();
  }

  return d;
}

}  // namespace Eyegrid

#endif
