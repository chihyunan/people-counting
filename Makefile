# ESP32 — people-counting.ino + ir_beam (arduino-cli)
# Sketch folder must match the .ino name: people-counting/people-counting.ino

FQBN  := esp32:esp32:esp32
PORT  ?= /dev/cu.usbserial-0001
BAUD  := 115200
SKETCH_DIR := .
IR_LIB   := lib/ir_beam
CLI_LIB  := --library $(IR_LIB)

.PHONY: help compile upload flash monitor

help:
	@echo "Targets:"
	@echo "  make compile   — build only"
	@echo "  make upload    — build then flash"
	@echo "  make flash     — compile + upload in one step"
	@echo "  make monitor   — USB serial @ $(BAUD) (quit before flashing)"
	@echo "Override port: make flash PORT=/dev/cu.OTHER"

compile:
	arduino-cli compile --fqbn $(FQBN) $(CLI_LIB) $(SKETCH_DIR)

upload: compile
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) $(SKETCH_DIR)

flash:
	arduino-cli compile --upload -p $(PORT) --fqbn $(FQBN) $(CLI_LIB) $(SKETCH_DIR)

monitor:
	arduino-cli monitor -p $(PORT) -c baudrate=$(BAUD)
