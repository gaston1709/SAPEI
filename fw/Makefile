# Define the FQBN (Fully Qualified Board Name) for your Arduino board
FQBN = arduino:avr:uno

# Define the output directory
BUILD_DIR = ./build

SRC_DIR = ./src/SAPEI

PORT ?= /dev/ttyUSB1

# Default targct: Compile the sketch
all: compile

# Rule to compile the sketch
compile:
	arduino-cli compile --fqbn $(FQBN) --no-color --output-dir $(BUILD_DIR) $(SRC_DIR)

upload:
	arduino-cli compile -u -b $(FQBN) -p $(PORT) --no-color $(SRC_DIR)

dependencies:
	arduino-cli lib install "MFRC522"

# Rule to clean the build directory
clean:
	rm -rf $(BUILD_DIR)
