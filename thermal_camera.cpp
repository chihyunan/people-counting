#include "thermal_camera.h"

// 1. Wrap C includes so the C++ compiler doesn't freak out
extern "C" {
    #include "src/grideye_api_lv1.h"
    #include "src/detect.h"
    #include "src/feature_extraction.h"
}

// 2. Include the C++ tracking headers
#include "src/tracking.hpp"
#include "src/human_object.hpp"

// Global objects (from the GitHub logic)
// Using 'static' keeps these hidden from the rest of the project
static float pixel_data[64]; 

void setupThermalCamera() {
    Serial.println("Initializing Thermal Logic Wrapper...");
    // Future: Add Wire.begin() and amg.begin() here
}

    // This will eventually call the GitHub's frame processing
    // For now, it's a heartbeat to prove the link works
void updateThermalLogic() {
    short mock_pixels[64];
    for(int i=0; i<64; i++) mock_pixels[i] = 20 * 256; // 20 degrees C
    
    // Manually call the Panasonic function you just fixed
    vAMG_PUB_TMP_ConvTemperature64((unsigned char*)mock_pixels, mock_pixels);
    Serial.println("Thermal algorithm processed mock frame.");
}

bool isHumanConfirmed() {
    // This will eventually check the tracking list count
    return true; 
}