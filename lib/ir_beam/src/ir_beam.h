#ifndef IR_BEAM_H
#define IR_BEAM_H

#include <Arduino.h>

enum BeamHealth : uint8_t {
  BEAM_OK = 0,
  BEAM_STUCK_LOW = 1,
  BEAM_STUCK_HIGH_OR_OPEN = 2,
  BEAM_FLOATING_NOISY = 3,
  BEAM_UNKNOWN = 4
};

struct BeamDiag {
  BeamHealth a;
  BeamHealth b;
  unsigned long sampledMs;
  uint16_t togglesA;
  uint16_t togglesB;
};

// Function Declarations
void setupIRSensors();
void IRAM_ATTR handleSensorA();
void IRAM_ATTR handleSensorB();
int getDirectionalCount(); // Returns 1 for Entry, -1 for Exit, 0 for None
void setIRDebug(bool enabled);
BeamDiag runBeamBootDiagnostic(unsigned long windowMs = 1500, unsigned long sampleMs = 2);
BeamDiag getBeamHealthLive();
const char *beamHealthToString(BeamHealth health);

#endif
