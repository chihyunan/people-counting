FQBN  = esp32:esp32:esp32
PORT ?= /dev/cu.usbserial-0001

compile:
	arduino-cli compile --fqbn $(FQBN) .

upload: compile
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) .

flash: upload

monitor:
	arduino-cli monitor -p $(PORT) -c baudrate=115200

run: upload monitor

.PHONY: compile upload flash monitor run
