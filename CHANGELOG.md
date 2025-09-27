# Changelog

All notable changes to this project will be documented in this file.

## 2025-09-27
- Tests: add a lightweight, embedded-friendly unit test framework under `tests/framework/EmbeddedTest.h` that runs on Arduino IDE, PlatformIO, and AVR targets (no external deps).
- Tests: add `tests/mocks/MockTransport.h` to capture HAL writes for verification.
- Tests: add reusable IVFDHAL contract tests in `tests/common/IVFDHALContractTests.hpp` (transport injection, invalid args, positioning + write, capabilities presence).
- Tests: add device-specific tests for VFD20S401HAL in `tests/device/VFD20S401HALTests.hpp` (init/clear/home opcodes, ESC 'H' addressing, null-terminated ESC, custom-char mapping and packing).
- Tests: add Arduino test sketch `tests/arduino/IVFDHAL_And_Device_Tests/IVFDHAL_And_Device_Tests.ino` to run all tests via Serial.
- PlatformIO: add `[env:megaatmega2560-tests]` that builds and runs the embedded test runner at `tests/embedded_runner/main.cpp`; links this repo as a library using `library.json`.
- Docs: add `docs/tests/README.md` describing the test framework, how to run in Arduino IDE and PlatformIO, and how to add tests for new HALs.
- Makefile: add `tests/%` pattern targets and `tests/all` to build (and optionally upload) tests via PlatformIO, Arduino CLI, or avr-gcc without altering existing example flow.
- Makefile: add optional `ARDUINO_LIB_DIRS` variable to pass one or more `--libraries` paths to Arduino CLI, keeping PlatformIO flow unchanged.
- Docs: remove `HD44780.pdf` from datasheets index and root docs since it’s not needed in-repo; keep external reference context in prose if required.
- Docs: document additional datasheets in `docs/README.md` and `docs/datasheets/README.md` (CU40026-TW200A, CU20025ECPB-W1J, M204SD01A, 20T202DA2JA, M0216MD-162MDBR2-J, VK202-25 manual, HT16514, PT6302, PT6314, uPD16314, STV7710).
- HAL: add `VFD20T202HAL` (20x2) using HD44780-style instructions (Function Set 0x38, Display On 0x0C, Clear 0x01, Entry Mode 0x06, Set DDRAM 0x80|addr); exposes IVFDHAL surface without modifying locked interfaces.
- Tests: add contract + device tests for `VFD20T202HAL` (init sequence, clear/home, DDRAM addressing). Runners updated.
- Docs: expand `docs/api/VFD20S401HAL.md` and `docs/api/VFD20T202HAL.md` with IVFDHAL method references, unbuffered and buffered code examples, and device-specific notes.
- Docs: add `docs/api/HAL_Authoring_Guide.md` and `docs/api/HAL_Doc_Template.md` for consistent HAL documentation and authoring.
- Tools/Make: add `tools/new_hal.py` scaffold and `make hal` to generate a new HAL skeleton (code, tests, docs) with next steps guidance.

## 2025-09-23
- Restore full repository from archive and preserve `.git/`, `src/`, `examples/`, `docs/`, and `utility/`.
- Add `AGENTS.md` directives to codify safety rules and commit policy.
- PlatformIO: add per‑example `platformio.ini` files for:
  - `examples/BasicTest`, `MinimalVFDDemo`, `ModeSpecificTest`, `SimpleDemo`, `SimpleVFDTest`, `CorrectCodesDemo`.
- Arduino IDE: add `.ino` wrappers for all examples listed above.
- New example: `examples/CorrectCodesDemo` demonstrating corrected VFD control/escape codes.
- Build system: replace Makefile with unified multi‑backend (PIO / Arduino‑CLI / avr‑gcc) and add backend flags (`--pio`, `--arduino`, `--avr`).
- Library config: update `library.json` with `build.srcFilter` to exclude non‑C++ folders from PIO builds (`utility/`, `docs/`, tests, `.pio/`, `.build/`).
- Docs: update `build_cmd.txt` to a concise build command cheat sheet including backend flags.
- Arduino IDE: add `library.properties` so the library and examples appear in the IDE again.
- Cleanup: remove `src/examples` from the library tree to prevent duplicate `setup()`/`loop()` conflicts; all examples live under top-level `examples/`.
- Git ignore: add `*.ino.cpp` to avoid committing Arduino preprocessed sketch artifacts.
- PlatformIO (megaatmega2560): set `upload_protocol = wiring` and default `upload_port = /dev/ttyACM0` in all example `platformio.ini` files to match working Arduino IDE 1.8.x settings.
- PlatformIO (megaatmega2560): set `upload_speed = 57600` (hardware-fixed) in examples and mirror wiring/port/speed in top-level `platformio.ini` for consistency.
 - PlatformIO examples: fix `lib_extra_dirs` to `../../..` so PIO resolves local `VFDDisplay` from `~/Arduino/libraries` instead of trying to install from registry.
 - PlatformIO: standardize env name to `[env:megaatmega2560]` and enforce `upload_protocol = stk500`, `upload_speed = 57600` across library root and all examples.
- Makefile: add `--protocol=<wiring|stk500>` (or `PROTOCOL=...`) to override upload protocol for PIO/AVR backends; default remains `stk500` @ 57600.
 - HAL: cursor positioning now uses ESC 'H' + linear address (0x00..0x4F). Address is computed row*20+col for 4x20.
- HAL: fix escape-sequence sender to avoid zero-terminated truncation by switching all callers to the length-aware `sendEscSequence(data,len)` (row/col/levels can be 0x00).
- Examples: increase inter-test delays to ~2.5s for better visual inspection (BasicTest, CorrectCodesDemo, ModeSpecificTest).
- CorrectCodesDemo: call cursorHome() after clear() before headings ("Reset OK", "Blink rates:") to eliminate leading space and ensure column 0 start.
- Docs: add a concise "Command Reference (VFD20S401)" section to docs/README.md reflecting HAL commands (init, reset, clear, home, positioning, mode, dimming, blink, charset).
 - Docs: add "Gotchas (VFD20S401)" section summarizing practical tips (clear doesn’t home; ESC 'H'+linear address; fixed-length ESC; avoid raw DDRAM bytes; 0-based API).
- HAL: implement higher-level helpers: hScroll (stateful left/right scroll on a row), vScroll (wrapper for vScrollText), fix vScrollText to render full-width lines, and add blocking flashText().
- HAL: implement custom characters for VFD20S401 – add setCustomChar() (capability-aware) and wire saveCustomChar() to it; pack 5x7 patterns and send ESC 'C' + CHR + PT1..PT5 per datasheet.
- HAL: add length-aware sendEscSequence(data,len) used by NO_TOUCH primitives and UDF sender.
- API: add writeCustomChar(index) and getCustomCharCode(index,&code) to abstract device code mapping.
- VFD20S401: support 16 UDFs; indices 0..7 -> 0x00..0x07, 8..15 -> 0x80..0x87 to avoid control code collisions.
- Examples: add CustomCharsSimple and CustomCharsAdvanced; showcase setCustomChar(), writeCustomChar(), and getCustomCharCode().
- Tooling: run_all_demos.sh includes the new custom-char examples in the curated order.
- Examples: add CustomCharsAnimation (runner sprite) demonstrating dynamic reprogramming of a glyph index for animation.
- Examples: add CustomCharsTetris – mini auto-drop Tetris using custom glyphs; spawns, drops, rotates, places, and clears lines on a 10x4 field.
- README: add top-level Custom Characters section with API quickstart, mapping notes, and example links.
- Docs: update VFD20S401HAL reference to document hScroll/vScroll/flashText semantics and corrected positioning.
 - Directives: add NO_TOUCH markers policy to AGENTS.md; methods marked NO_TOUCH in VFD20S401HAL are hardware primitives and must not be edited.
 - HAL: add VFDError enum and lastError()/clearError(); set errors across public methods without modifying NO_TOUCH primitives.
 - Buffered top-layer: add `src/Buffered/BufferedVFD.h` with screen buffer, diff flush, and non-blocking hScroll/vScroll/flash scaffolding.
- Example: add `examples/AnimationsDemo` demonstrating BufferedVFD animations.
- Example: add `examples/FlappyBirdDemo` (no input) using BufferedVFD and CT1; bird auto-navigates obstacles.
- Example: add `examples/BargraphDemo` (no input) using BufferedVFD; rows labeled CH1–CH4 with simulated bar levels.
- Example: add `examples/MatrixRainDemo` (no input) using BufferedVFD and CT1 to simulate digital rain.
- Example: add `examples/ClockDemo` (no input) buffered HH:MM:SS clock with blinking colon.
- Tooling: add `run_all_demos.sh` to build/upload each example sequentially with a per-demo wait period.
- Tooling: `run_all_demos.sh` now uses a curated demo order (SimpleDemo → Clock → Bargraph → Animations → MatrixRain → FlappyBird → MinimalVFDDemo → SimpleVFDTest → BasicTest → CorrectCodesDemo → ModeSpecificTest), then any remaining.
- Example: add `examples/AdDemo` (buffered, no-input) – animated ad for BIG SKY VCR REPAIR with fades, marquee, and callout.
 - Docs: add root README.md with run_all_demos.sh usage; update docs/README.md with a "Running All Demos" section and an expanded guide for adding new device HALs (NO_TOUCH policy, capabilities, error handling, positioning).

## 2025-09-22
- Initial make/PIO integration work on examples and build scripts.
