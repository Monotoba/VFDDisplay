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

## 2025-09-22
- Initial make/PIO integration work on examples and build scripts.
