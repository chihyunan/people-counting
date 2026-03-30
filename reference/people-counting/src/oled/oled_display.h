#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Wire.h>
#include <U8g2lib.h>

namespace Oled {

static U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

inline void begin(int sda = 21, int scl = 22, uint8_t i2cAddr = 0x3C) {
  Wire.begin(sda, scl);
  u8g2.setI2CAddress(i2cAddr << 1);
  u8g2.begin();
}

inline void showCounts(long enter, long ex) {
  long net = enter - ex;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 12, "Team Banana");
  u8g2.drawStr(0, 24, "People Counter");
  char buf[24];
  snprintf(buf, sizeof(buf), "Enter: %ld", enter);
  u8g2.drawStr(0, 40, buf);
  snprintf(buf, sizeof(buf), "Exit:  %ld", ex);
  u8g2.drawStr(0, 52, buf);
  snprintf(buf, sizeof(buf), "Net:   %ld", net);
  u8g2.drawStr(0, 64, buf);
  u8g2.sendBuffer();
}

}  // namespace Oled

#endif
