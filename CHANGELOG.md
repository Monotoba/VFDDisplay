# Changelog

All notable changes to this project will be documented in this file.

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
- Docs: update VFD20S401HAL reference to document hScroll/vScroll/flashText semantics and corrected positioning.
 - Directives: add NO_TOUCH markers policy to AGENTS.md; methods marked NO_TOUCH in VFD20S401HAL are hardware primitives and must not be edited.
 - HAL: add VFDError enum and lastError()/clearError(); set errors across public methods without modifying NO_TOUCH primitives.
 - Buffered top-layer: add `src/Buffered/BufferedVFD.h` with screen buffer, diff flush, and non-blocking hScroll/vScroll/flash scaffolding.
- Example: add `examples/AnimationsDemo` demonstrating BufferedVFD animations.
- Example: add `examples/FlappyBirdDemo` (no input) using BufferedVFD and CT1; bird auto-navigates obstacles.
- Example: add `examples/BargraphDemo` (no input) using BufferedVFD; rows labeled CH1–CH4 with simulated bar levels.
- Example: add `examples/MatrixRainDemo` (no input) using BufferedVFD and CT1 to simulate digital rain.
 - Example: add `examples/ClockDemo` (no input) buffered HH:MM:SS clock with blinking colon.

## 2025-09-22
- Initial make/PIO integration work on examples and build scripts.
