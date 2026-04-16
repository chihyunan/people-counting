#include "src/eyegrid/eyegrid.h"

namespace {

constexpr unsigned long HEARTBEAT_MS = 5000;

unsigned long lastHeartbeat = 0;
uint8_t lastBlobCount = 0;

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);

  if (!Eyegrid::start()) {
    Serial.println("Grid-EYE not found — halting.");
    for (;;) delay(1000);
  }

  Eyegrid::resetTracker();
  Serial.printf("Grid-EYE ready. Blob tuning: deltaC=%.1f minSize=%u\n",
                Eyegrid::BLOB_DELTA_C, Eyegrid::BLOB_MIN_SIZE);
}

void loop() {
  unsigned long now = millis();

  Eyegrid::update(now);
  Eyegrid::BlobResult blobs = Eyegrid::countBlobs();

  if (blobs.count != lastBlobCount) {
    Serial.printf("Blobs=%u  baseline=%.1f  thresh=%.1f\n",
                  blobs.count, blobs.baseline, blobs.threshold);
    lastBlobCount = blobs.count;
  }

  if (now - lastHeartbeat >= HEARTBEAT_MS) {
    Serial.printf("Blobs=%u  baseline=%.1f  thresh=%.1f\n",
                  blobs.count, blobs.baseline, blobs.threshold);
    Serial.println("8x8 (C):");
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        Serial.printf("%5.1f", Eyegrid::pixels[row * 8 + col]);
      }
      Serial.println();
    }
    lastHeartbeat = now;
  }
}
