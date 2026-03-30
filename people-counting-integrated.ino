#include "ir_beam.h"
#include "src/eyegrid/eyegrid.h"
#include "src/oled/oled_display.h"

namespace {

constexpr unsigned long GRID_LOOKBACK_MS = 700;
constexpr unsigned long GRID_LOOKAHEAD_MS = 900;
constexpr unsigned long HEARTBEAT_MS = 10000;
constexpr size_t MAX_PENDING_EVENTS = 8;

struct PendingBeamEvent {
  int beamDelta;
  unsigned long triggerTime;
  unsigned long readyTime;
};

PendingBeamEvent pendingEvents[MAX_PENDING_EVENTS] = {};
size_t pendingHead = 0;
size_t pendingCount = 0;

long totalEntered = 0;
long totalExited = 0;
long occupancy = 0;
unsigned long lastHeartbeat = 0;
bool gridEyeAvailable = false;

bool sameSign(int a, int b) {
  return (a > 0 && b > 0) || (a < 0 && b < 0);
}

void applyFinalDelta(int finalDelta, int beamDelta, int gridDelta) {
  occupancy += finalDelta;
  if (finalDelta > 0) {
    totalEntered += finalDelta;
  } else if (finalDelta < 0) {
    totalExited += -finalDelta;
  }

  Oled::showCounts(totalEntered, totalExited);

  Serial.printf("Beam=%d Grid-EYE=%d Final=%d Occupancy=%ld\n",
                beamDelta,
                gridDelta,
                finalDelta,
                occupancy);
  Serial.println("-------------------------");
}

void finalizeOldestPending() {
  if (pendingCount == 0) {
    return;
  }

  PendingBeamEvent &event = pendingEvents[pendingHead];
  int gridDelta = 0;

  if (gridEyeAvailable) {
    gridDelta = Eyegrid::consumeWindowDelta(event.triggerTime,
                                            GRID_LOOKBACK_MS,
                                            GRID_LOOKAHEAD_MS);
  }

  int finalDelta = sameSign(gridDelta, event.beamDelta) ? gridDelta : event.beamDelta;
  applyFinalDelta(finalDelta, event.beamDelta, gridDelta);

  pendingHead = (pendingHead + 1) % MAX_PENDING_EVENTS;
  pendingCount--;
}

void enqueueBeamEvent(int beamDelta, unsigned long now) {
  if (pendingCount == MAX_PENDING_EVENTS) {
    Serial.println("Pending beam queue full; finalizing oldest event early.");
    finalizeOldestPending();
  }

  size_t insertIndex = (pendingHead + pendingCount) % MAX_PENDING_EVENTS;
  pendingEvents[insertIndex] = PendingBeamEvent{
    beamDelta,
    now,
    now + GRID_LOOKAHEAD_MS
  };
  pendingCount++;

  Serial.printf("Queued beam event %d at %lu ms\n", beamDelta, now);
}

void finalizeReadyEvents(unsigned long now) {
  while (pendingCount > 0) {
    PendingBeamEvent &event = pendingEvents[pendingHead];
    if (now < event.readyTime) {
      break;
    }
    finalizeOldestPending();
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);

  setupIRSensors();
  Oled::begin();
  Oled::showCounts(totalEntered, totalExited);

  gridEyeAvailable = Eyegrid::start();
  if (gridEyeAvailable) {
    Eyegrid::resetTracker();
    Serial.println("Grid-EYE continuous validation enabled.");
  } else {
    Serial.println("Grid-EYE not found. Running with break-beam only.");
  }

  Serial.println("Integrated people counter started.");
}

void loop() {
  unsigned long now = millis();

  if (gridEyeAvailable) {
    Eyegrid::update(now);
  }

  int beamEvent = getDirectionalCount();
  if (beamEvent != 0) {
    enqueueBeamEvent(beamEvent, now);
  }

  finalizeReadyEvents(now);

  if (now - lastHeartbeat >= HEARTBEAT_MS) {
    Serial.printf("Heartbeat: occupancy=%ld pending=%u\n",
                  occupancy,
                  static_cast<unsigned>(pendingCount));
    lastHeartbeat = now;
  }
}
