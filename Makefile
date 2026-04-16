# ESP32 — people-counting-integrated (arduino-cli)
# Sketch dir = repo root (people-counting.ino)

FQBN  := esp32:esp32:esp32
PORT  ?= /dev/cu.usbserial-0001
BAUD  := 115200

.PHONY: help compile flash monitor

help:
	@echo "Targets:"
	@echo "  make compile  — build only (no flash)"
	@echo "  make flash    — compile + flash"
	@echo "  make monitor  — serial monitor @ $(BAUD)"
	@echo ""
	@echo "Override port:  make flash PORT=/dev/cu.OTHER"

compile:
	arduino-cli compile --fqbn $(FQBN) .

flash:
	arduino-cli compile --upload -p $(PORT) --fqbn $(FQBN) .

monitor:
	arduino-cli monitor -p $(PORT) -c baudrate=$(BAUD)
