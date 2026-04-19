FQBN      ?= esp32:esp32:esp32
PORT      ?= /dev/cu.usbserial-0001
SKETCH    ?= .
BUILD_DIR ?= .arduino/build

CLI = arduino-cli

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
	$(CLI) monitor -p $(PORT) -c baudrate=115200

run: upload monitor

clean:
	rm -rf .arduino

.PHONY: compile upload flash monitor run clean
