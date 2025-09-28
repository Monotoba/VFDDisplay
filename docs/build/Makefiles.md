# Makefile Documentation (Unified: PlatformIO / Arduino CLI / avr-gcc)

## Overview

The root `Makefile` is a unified frontend that can build and upload any example using one of three backends:
- PlatformIO (`BACKEND=pio`, default)
- Arduino CLI (`BACKEND=arduino`)
- avr-gcc + avrdude (`BACKEND=avr`)

Examples are discovered dynamically from `examples/*`. Each example name becomes a make target. Upload targets are `<Example>.upload`.

Key features
- Debug or release builds via a simple secondary goal: `make <Example> debug|release` (default: `release`).
- Per-backend overrides: `make -- --pio <Example>`, `make -- --arduino <Example>`, `make -- --avr <Example>`.
- Upload with `PORT`, `BAUD`, and `PROTOCOL` control across backends.
- Ephemeral, per-example PlatformIO projects ensure consistent upload protocol/baud.
- Central build output root: `.build/`.

Quick start
- List examples: `make list`
- Build (default backend PIO): `make ClockDemo`
- Upload: `make ClockDemo.upload PORT=/dev/ttyACM0`
- Force backend: `make -- --avr MinimalVFDDemo`

## Common Variables

- `BACKEND` (pio|arduino|avr). Default `pio`.
- `BUILD_TYPE` (debug|release). Default `release`.
- `PORT` — serial upload port, e.g. `/dev/ttyACM0` (required for upload).
- `BAUD` — upload baud (default `57600`).
- `PROTOCOL` — upload protocol for PIO/AVR backends: `stk500` (default) or `wiring`. Also accepted as CLI `--protocol=...`.
- `BUILD_ROOT` — output root (default `.build`).

## Core Targets

- `make list` — print discovered examples.
- `make <Example>` — build example with selected backend and `BUILD_TYPE`.
- `make <Example> debug` — debug build.
- `make <Example>.upload PORT=/dev/ttyACM0 [BAUD=57600] [PROTOCOL=stk500|wiring]` — upload with chosen backend.
- `make clean` — clean backend artifacts and `.build/` for the current backend.
- `make deepclean` — `clean` + remove local PlatformIO caches (`.pio`, `.pioenvs`, `.piolibdeps`).
- `make help` — show usage summary and defaults.

Shortcut backend flags (pass after `--` so `make` ignores them)
- Force PlatformIO: `make -- --pio <Example>`
- Force Arduino CLI: `make -- --arduino <Example>`
- Force avr-gcc: `make -- --avr <Example>`

Protocol override
- CLI flag: `make -- --protocol=wiring <Example>.upload PORT=/dev/ttyACM0`
- Variable: `make <Example>.upload PROTOCOL=wiring PORT=/dev/ttyACM0`

## PlatformIO Backend

Defaults
- `PIO_ENV=megaatmega2560`, `PIO_BOARD=megaatmega2560`
- `PIO_BUILD_TYPE` mirrors `BUILD_TYPE` (debug/release)
- `PIO_BUILD_FLAGS` include `-std=gnu++11` and `-D VFD_BUILD_DEBUG|VFD_BUILD_RELEASE`
- `PIO_UPLOAD_PROTOCOL` default `stk500` (override via `PROTOCOL` or `--protocol=...`)

Behavior
- Creates an ephemeral PIO project at `.build/pio/<Example>` pointing `src_dir` at `examples/<Example>` and vendoring the library under `lib/VFDDisplay/`.
- Build: `pio run -d .build/pio/<Example> -e <env>`
- Upload: `pio run -d .build/pio/<Example> -e <env> -t upload --upload-port $(PORT)` with `upload_protocol=$(PROTOCOL)` and `upload_speed=$(BAUD)`.

Customizing
- Override env/board: `make -- --pio <Example> PIO_ENV=megaatmega2560 PIO_BOARD=megaatmega2560`
- Change upload protocol: `PROTOCOL=wiring` (or CLI `--protocol=wiring`)
- Baud and monitor speed are set from `BAUD`.

Cleaning
- `make clean` removes `.build/pio/` and the ephemeral projects.
- Clean a single example manually: `pio run -d .build/pio/<Example> -t clean` or `rm -rf .build/pio/<Example>`.

## Arduino CLI Backend

Defaults
- `FQBN=arduino:avr:mega`
- `BUILD_TYPE` controls extra flags:
  - `debug`: `-Og -g -D VFD_BUILD_DEBUG`
  - `release`: `-Os -DNDEBUG -D VFD_BUILD_RELEASE`
- Output path: `.build/arduino/<Example>`

Build
- Auto-detects sketch: prefers `examples/<Example>/*.ino`, otherwise `main.cpp`.
- Command used: `arduino-cli compile --fqbn $(FQBN) --build-path .build/arduino/<Example> --libraries . <flags> <sketch>`

Upload
- `make <Example>.upload PORT=/dev/ttyACM0`
- Runs `arduino-cli upload -p $(PORT) --fqbn $(FQBN) <sketch>` (Arduino CLI recompiles as needed).

Cleaning
- `make clean` removes `.build/arduino/`.

Tips
- To include additional library search paths when building tests: set `ARDUINO_LIB_DIRS="~/Arduino/libraries /opt/arduino/libs"` (used by test targets).

## avr-gcc Backend

Defaults
- `MCU=atmega2560`, `F_CPU=16000000UL`
- `ARDUINO_AVR_DIR=/usr/share/arduino/hardware/arduino/avr`
- `CORE_DIR=$(ARDUINO_AVR_DIR)/cores/arduino`, `VARIANT_DIR=$(ARDUINO_AVR_DIR)/variants/mega`
- C/C++ flags include `-std=gnu++11`, dead-code stripping, and family includes.
- `BUILD_TYPE` controls flags:
  - `debug`: `-Og -g -D VFD_BUILD_DEBUG`
  - `release`: `-Os -DNDEBUG -D VFD_BUILD_RELEASE`
- Output path: `.build/avr/<Example>` (generates `.elf` and `.hex`).

Build
- Compiles Arduino core sources, library sources under `src/`, and the example’s `main.cpp` (or first `*.cpp`).
- Links to produce `<Example>.elf` and converts to `<Example>.hex`.

Upload
- `make <Example>.upload PORT=/dev/ttyACM0 [BAUD=57600] [PROTOCOL=stk500|wiring]`
- Runs: `avrdude -v -p $(MCU) -c $(AVR_PROGRAMMER) -P $(PORT) -b $(BAUD) -D -U flash:w:.build/avr/<Example>/<Example>.hex:i`
- `AVR_PROGRAMMER` defaults to `wiring` and syncs to `PROTOCOL` when provided.
 - Note: build first (`make -- --avr <Example>`) so the HEX exists.

Cleaning
- `make clean` removes `.build/avr/`.

Board tweaks
- For UNO: `MCU=atmega328p`, `VARIANT_DIR=$(ARDUINO_AVR_DIR)/variants/standard`
- For Leonardo: `MCU=atmega32u4`, `VARIANT_DIR=$(ARDUINO_AVR_DIR)/variants/leonardo`

## Build/Upload Examples

Basic
- List examples: `make list`
- Build release (default): `make SimpleDemo`
- Build debug: `make SimpleDemo debug`
- Upload (set your port): `make SimpleDemo.upload PORT=/dev/ttyACM0`

Force backend
- PlatformIO: `make -- --pio ClockDemo`
- Arduino CLI: `make -- --arduino BasicTest`
- avr-gcc: `make -- --avr MinimalVFDDemo`

Protocol and baud
- `make ClockDemo.upload PORT=/dev/ttyACM0 PROTOCOL=wiring BAUD=115200`
- Or via flag: `make -- --pio --protocol=wiring ClockDemo.upload PORT=/dev/ttyUSB0`

Clean
- Clean current backend artifacts: `make clean`
- Deep clean (also PIO caches): `make deepclean`
- Clean a specific example folder manually under `.build/<backend>/<Example>` if needed.

## Tests and Tools

Tests
- Discover: `make tests`
- Build all: `make tests/all BACKEND=pio|arduino|avr`
- Build one: `make tests/<path-to-test> BACKEND=avr`
- Upload a test: add `UPLOAD=1 PORT=/dev/ttyACM0`.

HAL scaffold
- Generate skeleton: `make hal NAME=20T202 CLASS=VFD20T202HAL ROWS=2 COLS=20 DATASHEET=docs/datasheets/20T202DA2JA.pdf FAMILY=hd44780 TRANSPORT=sync3`

GUI sender
- Launch VFDSender GUI: `make vfd-sender` (requires `PySide6` and `pyserial`).

## Serial Monitoring

- PlatformIO: `pio device monitor -b 57600 -p /dev/ttyACM0`
- Screen: `screen /dev/ttyACM0 57600` (Ctrl‑A K to exit)

## Troubleshooting

- Upload permission denied: add your user to `dialout` (Linux) or use `sudo`.
- Wrong port: `ls /dev/tty*` (Linux) or check device manager; pass `PORT=/dev/ttyUSB0`.
- Protocol mismatch: try `PROTOCOL=wiring` (common for older bootloaders) or `stk500` (default here).
- Arduino CLI not found: install from Arduino and ensure `arduino-cli` is in PATH.
- PlatformIO not found: `pip install platformio` or use the VS Code extension.

## See Also

- Quick commands: `docs/build/build_cmd.txt`
- PlatformIO guide: `docs/build/PlatformIO.md`
- Root README “Makefile Quickstart” section
- `run_all_demos.sh` — build+upload many examples in sequence
