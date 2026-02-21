#include "thermal_camera.h"

// Pull in the Panasonic Engine
extern "C" {
    #include "src/grideye_api_lv1.h"
    #include "src/detect.h"
}

// Global Objects from Erick's code
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
Adafruit_AMG88xx amg;

float pixels[64];
short raw_pixels_for_engine[64]; // The Engine wants 'short'
long enterCount = 0;
long exitCount = 0;

void setupThermalCamera() {
    Wire.begin(21, 22);
    u8g2.setI2CAddress(0x3C << 1);
    u8g2.begin();

    if (!amg.begin(0x69)) {
        Serial.println("AMG8833 NOT FOUND");
    }
}

void updateThermalLogic() {
    amg.readPixels(pixels);

    // --- BRIDGE TO ENGINE ---
    // Convert floats to short (Multiplied by 256 per Panasonic specs)
    for(int i=0; i<64; i++) {
        raw_pixels_for_engine[i] = (short)(pixels[i] * 256);
    }

    // Call the Panasonic detection function from /src
    // For MS1, we just ensure this call doesn't crash the ESP32
    // vAMG_PUB_TMP_ConvTemperature64((unsigned char*)raw_pixels_for_engine, raw_pixels_for_engine);
    
    // --- ERICK'S FSM LOGIC (Simplified) ---
    // You can keep Erick's hottest-pixel logic here as a fallback
    // while we refine the multi-blob tracking in /src.
}

void displayCounts() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setCursor(0, 15);
    u8g2.printf("IN: %ld  OUT: %ld", enterCount, exitCount);
    u8g2.sendBuffer();
}