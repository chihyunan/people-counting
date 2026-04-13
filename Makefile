# ESP32 — people-counting (arduino-cli)

FQBN  := esp32:esp32:esp32
PORT  ?= /dev/cu.usbserial-0001
BAUD  := 115200
LIBS  := --library lib/ir_beam --library lib/wifi

.PHONY: help compile flash monitor

help:
	@echo "Targets:"
	@echo "  make compile  — build only (no flash)"
	@echo "  make flash    — compile + flash production sketch"
	@echo "  make monitor  — serial monitor @ $(BAUD)"
	@echo ""
	@echo "Override port:  make flash PORT=/dev/cu.OTHER"

compile:
	arduino-cli compile --fqbn $(FQBN) $(LIBS) .

flash:
	arduino-cli compile --upload -p $(PORT) --fqbn $(FQBN) $(LIBS) .

monitor:
	arduino-cli monitor -p $(PORT) -c baudrate=$(BAUD)
