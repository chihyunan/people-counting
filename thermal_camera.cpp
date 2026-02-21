#include "thermal_camera.h"

// 1) Keep the repo’s C includes (for future use)
extern "C" {
  #include "src/grideye_api_lv1.h"
  #include "src/detect.h"
  #include "src/feature_extraction.h"
}

// 2) Keep the repo’s C++ tracking headers (for future use)
#include "src/tracking.hpp"
#include "src/human_object.hpp"

// 3) Add your hardware libs
#include <Wire.h>
#include <Adafruit_AMG88xx.h>
#include <U8g2lib.h>

// -------------------- OLED (CH1116 -> SH1106 compatible in U8g2) --------------------
// If your OLED is 128x32, swap to the 128x32 constructor below.
static U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// static U8G2_SH1106_128X32_VISIONOX_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// -------------------- AMG8833 --------------------
static Adafruit_AMG88xx amg;
static float p[64];

// Keep this buffer so it still “fits” the repo’s pipeline later
static float pixel_data[64];

// -------------------- Your zone test logic --------------------
static const float OUT_LINE = 2.5;  // y < this = OUT
static const float IN_LINE  = 4.5;  // y > this = IN

enum Zone { Z_OUT, Z_MID, Z_IN };

static Zone zoneOf(float y) {
  if (y < OUT_LINE) return Z_OUT;
  if (y > IN_LINE)  return Z_IN;
  return Z_MID;
}

static const char* zoneName(Zone z) {
  if (z == Z_OUT) return "OUT";
  if (z == Z_IN)  return "IN";
  return "MID";
}

// FSM stage and counts
static int stage = 0;  // 0 idle, 1 saw OUT, 2 OUT->MID, 3 saw IN, 4 IN->MID
static long enterCount = 0, exitCount = 0;

// OLED update (5 Hz)
static void drawOLED(float maxT, int x, int y, Zone z) {
  static unsigned long last = 0;
  unsigned long now = millis();
  if (now - last < 200) return;
  last = now;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.drawStr(0, 10, "Zone Test (AMG8833)");

  char line[32];
  snprintf(line, sizeof(line), "ENTER: %ld", enterCount);
  u8g2.drawStr(0, 24, line);

  snprintf(line, sizeof(line), "EXIT : %ld", exitCount);
  u8g2.drawStr(0, 36, line);

  snprintf(line, sizeof(line), "MaxT: %.1fC  y=%d", maxT, y);
  u8g2.drawStr(0, 48, line);

  snprintf(line, sizeof(line), "Hot: x=%d  Zone:%s", x, zoneName(z));
  u8g2.drawStr(0, 60, line);

  u8g2.sendBuffer();
}

// -------------------- REQUIRED functions from thermal_camera.h --------------------
void setupThermalCamera() {
  Serial.println("Initializing Thermal Logic Wrapper + OLED...");

  // I2C: SDA=21, SCL=22
  Wire.begin(21, 22);

  // OLED at 0x3C (U8g2 expects 8-bit address => shift left)
  u8g2.setI2CAddress(0x3C << 1);
  u8g2.begin();

  // AMG8833 at 0x69
  if (!amg.begin(0x69)) {
    Serial.println("AMG8833 not found.");
    while (1) delay(1000);
  }

  Serial.println("Thermal + OLED ready.");
}

void updateThermalLogic() {
  // Read real sensor data
  amg.readPixels(p);

  // Copy into pixel_data buffer (so later you can feed repo algorithm)
  for (int i = 0; i < 64; i++) pixel_data[i] = p[i];

  // Find hottest pixel
  float maxT = -999;
  int maxI = 0;
  for (int i = 0; i < 64; i++) {
    if (p[i] > maxT) { maxT = p[i]; maxI = i; }
  }
  int x = maxI % 8;
  int y = maxI / 8;

  Zone z = zoneOf((float)y);

  // Your zone-crossing FSM
  switch (stage) {
    case 0:
      if (z == Z_OUT) stage = 1;
      else if (z == Z_IN) stage = 3;
      break;

    case 1: // saw OUT
      if (z == Z_MID) stage = 2;
      else if (z == Z_IN) { // OUT->IN quick
        enterCount++;
        Serial.printf("ENTER  (maxT=%.1f at y=%d)  E=%ld X=%ld\n", maxT, y, enterCount, exitCount);
        stage = 0;
      }
      break;

    case 2: // OUT->MID
      if (z == Z_IN) {
        enterCount++;
        Serial.printf("ENTER  (maxT=%.1f at y=%d)  E=%ld X=%ld\n", maxT, y, enterCount, exitCount);
        stage = 0;
      } else if (z == Z_OUT) stage = 1;
      break;

    case 3: // saw IN
      if (z == Z_MID) stage = 4;
      else if (z == Z_OUT) {
        exitCount++;
        Serial.printf("EXIT   (maxT=%.1f at y=%d)  E=%ld X=%ld\n", maxT, y, enterCount, exitCount);
        stage = 0;
      }
      break;

    case 4: // IN->MID
      if (z == Z_OUT) {
        exitCount++;
        Serial.printf("EXIT   (maxT=%.1f at y=%d)  E=%ld X=%ld\n", maxT, y, enterCount, exitCount);
        stage = 0;
      } else if (z == Z_IN) stage = 3;
      break;
  }

  // OLED display
  drawOLED(maxT, x, y, z);
}

// Simple stub: true if we have “any activity” (you can make it smarter later)
bool isHumanConfirmed() {
  // For now: treat a count change as confirmation
  static long lastE = 0, lastX = 0;
  bool changed = (enterCount != lastE) || (exitCount != lastX);
  lastE = enterCount; lastX = exitCount;
  return changed;
}
bool isHumanConfirmed() {
    // This will eventually check the tracking list count
    return true; 
}
