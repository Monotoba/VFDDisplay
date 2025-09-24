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
- HAL: fix cursor positioning for VFD20S401 by using ESC-based positioning (ESC 'H', row, col) instead of raw DDRAM `0x80|addr`. This prevents high-bit glyphs (e.g., 'Ë'/0xCB) at start-of-line.
- HAL: fix escape-sequence sender to avoid zero-terminated truncation by switching all callers to the length-aware `sendEscSequence(data,len)` (row/col/levels can be 0x00).
 - Examples: increase inter-test delays to ~2.5s for better visual inspection (BasicTest, CorrectCodesDemo, ModeSpecificTest).

## 2025-09-22
- Initial make/PIO integration work on examples and build scripts.
