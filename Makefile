# ESP32 — people-counting.ino + ir_beam (arduino-cli)
# Sketch folder must match the .ino name: people-counting/people-counting.ino

FQBN  := esp32:esp32:esp32
PORT  ?= /dev/cu.usbserial-0001
BAUD  := 115200
SKETCH_DIR ?= .
CLI_LIB  := --library lib/ir_beam --library lib/wifi

.PHONY: help compile upload flash monitor \
	compile-beam-test flash-beam-test \
	compile-wifi-test flash-wifi-test

help:
	@echo "Targets:"
	@echo "  make compile   — build only"
	@echo "  make upload    — build then flash"
	@echo "  make flash     — compile + upload in one step"
	@echo "  make monitor   — USB serial @ $(BAUD) (quit before flashing)"
	@echo "  make compile-beam-test — build test/beam-jumper"
	@echo "  make flash-beam-test   — flash test/beam-jumper"
	@echo "  make compile-wifi-test — build test/wifi-only"
	@echo "  make flash-wifi-test   — flash test/wifi-only"
	@echo "Override port: make flash PORT=/dev/cu.OTHER"
	@echo "Override sketch: make flash SKETCH_DIR=test/wifi-only"

compile:
	arduino-cli compile --fqbn $(FQBN) $(CLI_LIB) $(SKETCH_DIR)

upload: compile
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) $(SKETCH_DIR)

flash:
	arduino-cli compile --upload -p $(PORT) --fqbn $(FQBN) $(CLI_LIB) $(SKETCH_DIR)

compile-beam-test:
	arduino-cli compile --fqbn $(FQBN) $(CLI_LIB) test/beam-jumper

flash-beam-test:
	arduino-cli compile --upload -p $(PORT) --fqbn $(FQBN) $(CLI_LIB) test/beam-jumper

compile-wifi-test:
	arduino-cli compile --fqbn $(FQBN) $(CLI_LIB) test/wifi-only

flash-wifi-test:
	arduino-cli compile --upload -p $(PORT) --fqbn $(FQBN) $(CLI_LIB) test/wifi-only

monitor:
	arduino-cli monitor -p $(PORT) -c baudrate=$(BAUD)
