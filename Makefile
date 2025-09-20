# Makefile for Arduino Mega2560 (avr-gcc build)
# Place in: ~/Arduino/libraries/VFDDisplay/Makefile

# ==== CONFIGURATION ====
ARDUINO_BASE    = /usr/share/arduino/hardware/arduino/avr
CORE_PATH       = $(ARDUINO_BASE)/cores/arduino
VARIANT_PATH    = $(ARDUINO_BASE)/variants/mega
MCU             = atmega2560
F_CPU           = 16000000UL
PORT            = /dev/ttyACM0    # adjust if needed
BAUD            = 57600
CC              = avr-gcc
CXX             = avr-g++
OBJCOPY         = avr-objcopy
CFLAGS_COMMON   = -mmcu=$(MCU) -DF_CPU=$(F_CPU) \
                  -I$(CORE_PATH) -I$(VARIANT_PATH) \
                  -Isrc -Wall
LDFLAGS         = -mmcu=$(MCU)

# ==== BUILD CONFIGURATION ====
BUILD_DIR       = build
LIB_SRC_DIR     = src
LIB_SOURCES     = $(wildcard $(LIB_SRC_DIR)/*.cpp)
CORE_SOURCES    = $(wildcard $(CORE_PATH)/*.c) $(wildcard $(CORE_PATH)/*.cpp)

# ==== EXAMPLE CONFIGURATION ====
# Default demo: SimpleDemo
DEMO ?= SimpleDemo
EXAMPLE_SRC = examples/$(DEMO)/main.cpp
SKETCH_DBG_ELF = $(BUILD_DIR)/$(DEMO).debug.elf
SKETCH_REL_ELF = $(BUILD_DIR)/$(DEMO).release.elf

# ==== JOBS ====

# 1. Compile sketch + library with debug info
sketch-debug: $(SKETCH_DBG_ELF)
$(SKETCH_DBG_ELF): $(EXAMPLE_SRC) $(LIB_SOURCES)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CFLAGS_COMMON) -Og -g -o $@ $^ $(CORE_SOURCES)

# 2. Compile sketch + library optimized (no debug)
sketch-release: $(SKETCH_REL_ELF)
$(SKETCH_REL_ELF): $(EXAMPLE_SRC) $(LIB_SOURCES)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CFLAGS_COMMON) -Os -o $@ $^ $(CORE_SOURCES)

# 3. Convert ELF → HEX
$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# 4. Upload release build
upload: $(BUILD_DIR)/$(DEMO).release.hex
	avrdude -v -p $(MCU) -c wiring -P $(PORT) -b $(BAUD) -D -U flash:w:$<:i

# 5. Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

