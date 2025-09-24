#############################################
# Unified Makefile (PIO / Arduino-CLI / AVR) #
#############################################

# Usage:
#   make list                            # list discovered examples
#   make <example> [debug|release]       # build example via default BACKEND=pio
#   make <example>.upload PORT=/dev/ttyACM0 [BAUD=57600]
#   make <example> BACKEND=arduino       # build with Arduino CLI
#   make <example> BACKEND=avr           # build with avr-gcc
#   make clean | deepclean               # clean project artifacts / PIO cache
#
# Flags:
#   make -- --pio <example>              # force PlatformIO backend
#   make -- --arduino <example>          # force Arduino CLI backend
#   make -- --avr <example>              # force avr-gcc backend

########## Global configuration ##########

# Backend selection (pio | arduino | avr)
BACKEND ?= pio

# Optional CLI flags to force backend selection via goal-like switches.
# Put flags after `--` so make doesn't parse them as its own options.
CLI_FLAG_BACKENDS := --pio -pio --arduino -arduino --avr -avr
GOALS_NO_FLAGS := $(filter-out $(CLI_FLAG_BACKENDS),$(MAKECMDGOALS))
ifneq ($(filter --pio -pio,$(MAKECMDGOALS)),)
  BACKEND := pio
endif
ifneq ($(filter --arduino -arduino,$(MAKECMDGOALS)),)
  BACKEND := arduino
endif
ifneq ($(filter --avr -avr,$(MAKECMDGOALS)),)
  BACKEND := avr
endif

# Build type (debug|release). Can be appended as secondary goal.
BUILD_TYPE ?= release
PRIMARY_GOAL := $(firstword $(GOALS_NO_FLAGS))
SECONDARY_GOAL := $(word 2,$(GOALS_NO_FLAGS))
ifneq ($(filter $(SECONDARY_GOAL),debug release),)
  BUILD_TYPE := $(SECONDARY_GOAL)
endif

# Example discovery (folders under examples/)
EXAMPLES := $(notdir $(wildcard examples/*))

# Common serial defaults
PORT ?=
BAUD ?= 57600

# Bootloader protocol for upload; override via PROTOCOL=stk500 or CLI flag `--protocol=stk500|wiring`
PIO_UPLOAD_PROTOCOL ?= stk500

# Parse `--protocol=...` or `-protocol=...` flags from MAKECMDGOALS
PROTO_FLAG := $(filter --protocol=%,$(MAKECMDGOALS))
ifeq ($(strip $(PROTO_FLAG)),)
  PROTO_FLAG := $(filter -protocol=%,$(MAKECMDGOALS))
endif
ifneq ($(strip $(PROTO_FLAG)),)
  PIO_UPLOAD_PROTOCOL := $(patsubst --protocol=%,%,$(patsubst -protocol=%,%,$(PROTO_FLAG)))
endif

# Also accept PROTOCOL=... env/var to override
ifdef PROTOCOL
  PIO_UPLOAD_PROTOCOL := $(PROTOCOL)
endif

# Build output root
BUILD_ROOT := .build

########## PlatformIO backend ##########

PIO_ENV ?= mega2560_oldbootloader
PIO_BOARD ?= megaatmega2560
PIO_BUILD_TYPE := $(if $(filter $(BUILD_TYPE),debug),debug,release)
PIO_BUILD_FLAGS := -std=gnu++11 $(if $(filter $(BUILD_TYPE),debug),-D VFD_BUILD_DEBUG,-D VFD_BUILD_RELEASE)

_pio_build:
	@if [ -z "$(NAME)" ]; then echo "Set NAME=<name> or call via: make <example>"; exit 1; fi
	$(eval SRC_PATH := examples/$(NAME))
	@if [ ! -d "$(SRC_PATH)" ]; then echo "[PIO] Source path not found: $(SRC_PATH)"; exit 1; fi
	$(eval WORK_PIO_DIR := $(BUILD_ROOT)/pio/$(NAME))
	@mkdir -p $(WORK_PIO_DIR)
	@echo "[PIO] Preparing ephemeral project at $(WORK_PIO_DIR)"
	@printf "[platformio]\nsrc_dir = $(abspath $(SRC_PATH))\n\n[env:$(PIO_ENV)]\nplatform = atmelavr\nboard = $(PIO_BOARD)\nframework = arduino\nlib_ldf_mode = deep+\nbuild_type = $(PIO_BUILD_TYPE)\nbuild_flags = $(PIO_BUILD_FLAGS)\nmonitor_speed = $(BAUD)\nupload_protocol = $(PIO_UPLOAD_PROTOCOL)\n" > $(WORK_PIO_DIR)/platformio.ini
	@mkdir -p $(WORK_PIO_DIR)/lib/VFDDisplay
	@cp -r src/* $(WORK_PIO_DIR)/lib/VFDDisplay/
	@echo "[PIO] Building $(BUILD_TYPE) target $(NAME) in env $(PIO_ENV)"
	pio run -d $(WORK_PIO_DIR) -e $(PIO_ENV)

_pio_upload:
	@if [ -z "$(NAME)" ]; then echo "Set NAME=<name> or call via: make <name>.upload"; exit 1; fi
	@if [ -z "$(PORT)" ]; then echo "Set PORT=/dev/ttyACM0 (or your port)"; exit 1; fi
	$(eval SRC_PATH := examples/$(NAME))
	@if [ ! -d "$(SRC_PATH)" ]; then echo "[PIO] Source path not found: $(SRC_PATH)"; exit 1; fi
	$(eval WORK_PIO_DIR := $(BUILD_ROOT)/pio/$(NAME))
	@mkdir -p $(WORK_PIO_DIR)
	@printf "[platformio]\nsrc_dir = $(abspath $(SRC_PATH))\n\n[env:$(PIO_ENV)]\nplatform = atmelavr\nboard = $(PIO_BOARD)\nframework = arduino\nlib_ldf_mode = deep+\nbuild_type = $(PIO_BUILD_TYPE)\nbuild_flags = $(PIO_BUILD_FLAGS)\nmonitor_speed = $(BAUD)\nupload_protocol = $(PIO_UPLOAD_PROTOCOL)\nupload_speed = $(BAUD)\n" > $(WORK_PIO_DIR)/platformio.ini
	@mkdir -p $(WORK_PIO_DIR)/lib/VFDDisplay
	@cp -r src/* $(WORK_PIO_DIR)/lib/VFDDisplay/
	@echo "[PIO] Uploading $(BUILD_TYPE) target $(NAME) to $(PORT)"
	pio run -d $(WORK_PIO_DIR) -e $(PIO_ENV) -t upload --upload-port $(PORT)

_pio_clean:
	@echo "[PIO] Cleaning PIO build artifacts"
	- rm -rf $(BUILD_ROOT)/pio

########## Arduino CLI backend ##########

FQBN ?= arduino:avr:mega

define _arduino_detect_sketch
$(firstword $(wildcard examples/$(1)/*.ino) $(wildcard examples/$(1)/main.cpp))
endef

_arduino_build:
	@if [ -z "$(NAME)" ]; then echo "Set NAME=<name> or call via: make <example>"; exit 1; fi
	$(eval SKETCH := $(call _arduino_detect_sketch,$(NAME)))
	@if [ -z "$(SKETCH)" ]; then echo "[Arduino-CLI] No .ino or main.cpp in examples/$(NAME)"; exit 1; fi
	@echo "[Arduino-CLI] Building $(BUILD_TYPE) $(SKETCH) (FQBN=$(FQBN))"
	@mkdir -p $(BUILD_ROOT)/arduino/$(NAME)
	$(eval ARD_FLAGS := $(if $(filter $(BUILD_TYPE),debug),--build-property compiler.cpp.extra_flags=\"-Og -g -D VFD_BUILD_DEBUG\" --build-property compiler.c.extra_flags=\"-Og -g -D VFD_BUILD_DEBUG\",--build-property compiler.cpp.extra_flags=\"-Os -DNDEBUG -D VFD_BUILD_RELEASE\" --build-property compiler.c.extra_flags=\"-Os -DNDEBUG -D VFD_BUILD_RELEASE\"))
	arduino-cli compile --fqbn $(FQBN) --build-path $(BUILD_ROOT)/arduino/$(NAME) --libraries . $(ARD_FLAGS) $(SKETCH)

_arduino_upload:
	@if [ -z "$(NAME)" ]; then echo "Set NAME=<name> or call via: make <name>.upload"; exit 1; fi
	@if [ -z "$(PORT)" ]; then echo "Set PORT=/dev/ttyACM0 (or your port)"; exit 1; fi
	$(eval SKETCH := $(call _arduino_detect_sketch,$(NAME)))
	@echo "[Arduino-CLI] Uploading $(BUILD_TYPE) $(SKETCH) to $(PORT) (FQBN=$(FQBN))"
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) $(SKETCH)

_arduino_clean:
	@echo "[Arduino-CLI] Cleaning build artifacts"
	- rm -rf $(BUILD_ROOT)/arduino

########## avr-gcc backend ##########

AVR_CC ?= avr-gcc
AVR_CXX ?= avr-g++
AVR_OBJCOPY ?= avr-objcopy
AVRDUDE ?= avrdude
MCU ?= atmega2560
F_CPU ?= 16000000UL
ARDUINO_AVR_DIR ?= /usr/share/arduino/hardware/arduino/avr
CORE_DIR := $(ARDUINO_AVR_DIR)/cores/arduino
VARIANT_DIR := $(ARDUINO_AVR_DIR)/variants/mega
AVR_CFLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -ffunction-sections -fdata-sections -I$(CORE_DIR) -I$(VARIANT_DIR) -Isrc
AVR_CXXFLAGS := $(AVR_CFLAGS) -std=gnu++11 -fno-exceptions -fno-threadsafe-statics
AVR_LDFLAGS := -mmcu=$(MCU) -Wl,--gc-sections

ifeq ($(BUILD_TYPE),debug)
  AVR_CFLAGS += -Og -g -D VFD_BUILD_DEBUG
  AVR_CXXFLAGS += -Og -g -D VFD_BUILD_DEBUG
else
  AVR_CFLAGS += -Os -DNDEBUG -D VFD_BUILD_RELEASE
  AVR_CXXFLAGS += -Os -DNDEBUG -D VFD_BUILD_RELEASE
endif

# Programmer is configurable; default wiring (stk500v2). Override for old bootloaders: AVR_PROGRAMMER=stk500
AVR_PROGRAMMER ?= wiring
# Sync AVR programmer to chosen protocol when provided via PROTOCOL/--protocol
ifneq ($(strip $(PROTO_FLAG)$(PROTOCOL)),)
  AVR_PROGRAMMER := $(PIO_UPLOAD_PROTOCOL)
endif

_avr_build:
	@if [ -z "$(NAME)" ]; then echo "Set NAME=<name> or call via: make <example>"; exit 1; fi
	$(eval AVR_BUILD := $(BUILD_ROOT)/avr/$(NAME))
	$(eval SRC_DIR := examples/$(NAME))
	$(eval EXAMPLE_SRC := $(firstword $(wildcard $(SRC_DIR)/main.cpp) $(wildcard $(SRC_DIR)/*.cpp)))
	@if [ -z "$(EXAMPLE_SRC)" ]; then echo "[avr-gcc] No .cpp found in $(SRC_DIR). Add main.cpp."; exit 1; fi
	@echo "[avr-gcc] Building $(BUILD_TYPE) $(EXAMPLE_SRC) (MCU=$(MCU))"
	@mkdir -p $(AVR_BUILD)/core $(AVR_BUILD)/src $(AVR_BUILD)/example
	@for f in $(wildcard $(CORE_DIR)/*.c); do \
	  $(AVR_CC) $(AVR_CFLAGS) -c $$f -o $(AVR_BUILD)/core/$$(basename $$f .c).o; \
	done
	@for f in $(wildcard $(CORE_DIR)/*.cpp); do \
	  $(AVR_CXX) $(AVR_CXXFLAGS) -c $$f -o $(AVR_BUILD)/core/$$(basename $$f .cpp).o; \
	done
	@for f in $(shell find src -maxdepth 2 -name '*.cpp'); do \
  oname=$$(echo $$f | sed 's#^src/##; s#/#_#g; s#\\.cpp$$#.o#'); \
  $(AVR_CXX) $(AVR_CXXFLAGS) -c $$f -o $(AVR_BUILD)/src/$$oname; \
done
	$(AVR_CXX) $(AVR_CXXFLAGS) -c $(EXAMPLE_SRC) -o $(AVR_BUILD)/example/$(NAME).o
	$(AVR_CXX) $(AVR_LDFLAGS) $(AVR_BUILD)/example/$(NAME).o $(AVR_BUILD)/src/*.o $(AVR_BUILD)/core/*.o -o $(AVR_BUILD)/$(NAME).elf
	$(AVR_OBJCOPY) -O ihex -R .eeprom $(AVR_BUILD)/$(NAME).elf $(AVR_BUILD)/$(NAME).hex
	@echo "[avr-gcc] Built: $(AVR_BUILD)/$(NAME).hex"

_avr_upload:
	@if [ -z "$(PORT)" ]; then echo "Set PORT=/dev/ttyACM0 (or your port)"; exit 1; fi
	$(eval AVR_BUILD := $(BUILD_ROOT)/avr/$(NAME))
	@if [ ! -f "$(AVR_BUILD)/$(NAME).hex" ]; then echo "Build first: make $(NAME) BACKEND=avr"; exit 1; fi
	@echo "[avr-gcc] Uploading to $(PORT) at $(BAUD) (programmer=$(AVR_PROGRAMMER))"
	$(AVRDUDE) -v -p $(MCU) -c $(AVR_PROGRAMMER) -P $(PORT) -b $(BAUD) -D -U flash:w:$(AVR_BUILD)/$(NAME).hex:i

_avr_clean:
	@echo "[avr-gcc] Cleaning avr build artifacts"
	- rm -rf $(BUILD_ROOT)/avr

########## Front-end dispatch ##########

BUILD_DISPATCH  = _$(BACKEND)_build
UPLOAD_DISPATCH = _$(BACKEND)_upload
CLEAN_DISPATCH  = _$(BACKEND)_clean

.PHONY: help list clean deepclean $(EXAMPLES) %.upload pio arduino avr --pio -pio --arduino -arduino --avr -avr debug release

help:
	@echo "Unified Makefile for VFDDisplay"
	@echo "--------------------------------"
	@echo "Examples: $(EXAMPLES)"
	@echo ""
	@echo "Usage:"
	@echo "  make list"
	@echo "  make <example> [debug|release] [BACKEND=pio|arduino|avr]"
	@echo "  make -- --pio <example>  (force PlatformIO)"
	@echo "  make -- --arduino <example>  (force Arduino CLI)"
	@echo "  make -- --avr <example>  (force avr-gcc)"
	@echo "  make <example>.upload PORT=/dev/ttyACM0 [BAUD=$(BAUD)]"
	@echo ""
	@echo "Defaults: BACKEND=pio, PIO_ENV=mega2560_oldbootloader, FQBN=arduino:avr:mega, MCU=atmega2560, PIO_UPLOAD_PROTOCOL=$(PIO_UPLOAD_PROTOCOL)"
	@echo "Protocol override: pass PROTOCOL=stk500 or add --protocol=stk500 (or wiring)"

list:
	@echo "Discovered examples:"; \
	for e in $(EXAMPLES); do echo "  - $$e"; done

# Build any example: make <ExampleName>
$(EXAMPLES):
	@$(MAKE) $(BUILD_DISPATCH) NAME=$@ BUILD_TYPE=$(BUILD_TYPE)

# Upload any example: make <ExampleName>.upload PORT=/dev/ttyACM0
%.upload:
	@nm=$*; $(MAKE) $(UPLOAD_DISPATCH) NAME=$$nm BUILD_TYPE=$(BUILD_TYPE) PORT=$(PORT) BAUD=$(BAUD)

# Convenience wrappers
pio:
	@$(MAKE) $(BUILD_DISPATCH) BACKEND=pio EXAMPLE=$(EXAMPLE)

arduino:
	@$(MAKE) $(BUILD_DISPATCH) BACKEND=arduino EXAMPLE=$(EXAMPLE)

avr:
	@$(MAKE) $(BUILD_DISPATCH) BACKEND=avr EXAMPLE=$(EXAMPLE)

# Clean project-local artifacts for current backend
clean:
	@$(MAKE) $(CLEAN_DISPATCH)
	- rm -rf $(BUILD_ROOT)

# Hard clean: also drop PlatformIO cache in this project
deepclean: clean
	- rm -rf .pio .pioenvs .piolibdeps

# No-op targets to swallow backend flags and build-type words when passed as goals
--pio -pio --arduino -arduino --avr -avr:
	@true

debug release:
	@true
