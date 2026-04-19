FQBN      ?= esp32:esp32:esp32
PORT      ?= /dev/cu.usbserial-0001
BAUD      ?= 115200
SKETCH    ?= .
BUILD_DIR ?= .arduino/build

CLI = arduino-cli

default: help

help:
	@echo "Targets:"
	@echo "  make compile            Build sketch only"
	@echo "  make upload             Build and upload firmware"
	@echo "  make flash              Alias for upload"
	@echo "  make monitor            Serial monitor"
	@echo "  make run                Upload then monitor"
	@echo "  make clean              Remove local build artifacts"
	@echo ""
	@echo "Overrides:"
	@echo "  make upload PORT=/dev/cu.usbserial-0001"
	@echo "  make compile FQBN=esp32:esp32:esp32"
	@echo "  make monitor BAUD=115200"

compile:
	mkdir -p "$(BUILD_DIR)"
	$(CLI) compile \
		--fqbn $(FQBN) \
		--build-path "$(BUILD_DIR)" \
		$(SKETCH)

upload: compile
	$(CLI) upload \
		-p $(PORT) \
		--fqbn $(FQBN) \
		--input-dir "$(BUILD_DIR)" \
		$(SKETCH)

flash: upload

monitor:
	$(CLI) monitor -p $(PORT) -c baudrate=$(BAUD)

run: upload monitor

clean:
	rm -rf .arduino

.PHONY: default help compile upload flash monitor run clean
