# Changelog

All notable changes to this project will be documented in this file.

## 1.0.1 — 2025-09-28
- PlatformIO: fix CI failure for `leonardo-tests` (and other test envs) that reported "Nothing to build".
  - Replace deprecated/misused `src_filter`/`src_dir` with `build_src_filter` to include both `src/` and `tests/embedded_runner/` in test builds.
  - Remove `lib_deps = VFDDisplay` from test envs to avoid trying to fetch this repo as a registry package during CI.
  - Keep `-Isrc` include path; no changes to library sources or public APIs.
  - Eliminates warnings: unknown `src_dir` in env sections and deprecated `src_filter`.
  - Add `src/EmbeddedTestsMain.cpp` guarded by `VFD_EMBEDDED_TEST_RUNNER` that includes `tests/embedded_runner/main.cpp` to build tests without changing project layout.
  - Define `VFD_TEST_PROFILE_LEONARDO` to run a minimal test set on memory-constrained boards (Leonardo) while keeping full suite on Mega 2560.
  - tests/framework: make `ET_ADD_TEST` variadic to allow lambda bodies with commas/initializers; add `ET_ASSERT_GE` helper.
  - platformio.ini: remove ineffective `build_src_filter` patterns and rely on default `src` discovery; add `-I.` and `-DVFD_EMBEDDED_TEST_RUNNER` (plus profile define for Leonardo).
  - Examples: fix `examples/BlinkExplore/BlinkExplore.ino` incorrect `sprintf` usage; replace with Serial prints.

Release notes: Patch release focusing on CI/build configuration, no API changes. Safe update for users.

## 2025-09-27
- Governance/CI: add MIT LICENSE, CONTRIBUTING.md, CODE_OF_CONDUCT.md, SECURITY.md, .editorconfig, .clang-format, .gitattributes, and GitHub Actions CI (Arduino Lint + PlatformIO builds for tests and examples). Add badges to README.
  - Add badges: CI, Release, Tags, License, Arduino Lint, PlatformIO.
- Docs: add GIF gallery to root `README.md` and to `docs/examples/README.md`; include a representative demo clip in `docs/examples/flappyBirdDemo.md` (images under `docs/images/`).
 - CI: extend workflow to test embedded runner across a board matrix (`megaatmega2560`, `leonardo`, `uno`); add corresponding PlatformIO test envs. Build examples once on Mega 2560.
 - Community: add CODEOWNERS and GitHub issue/PR templates.
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
 - Tools: move `tools/new_hal.py` to `tools/new_hal/new_hal.py`; update Makefile `hal` target accordingly.
 - Tools: add `tools/vfdSender/vfdSender2.py` (PySide6 GUI) for sending ASCII + hex escapes to VFDs during HAL development; documented under docs/README.md.
 
 - HAL: CU40026 (VFDCU40026HAL)
   - Add device-level helpers: `setLuminanceBand(uint8_t)`, `setLuminanceIndex(uint8_t 0..3)`, `setBlinkPeriodMs(uint16_t)`, and `selectFlickerlessMode()` using ESC sequences ('L', 'T', 'S'). No IVFDHAL changes.
   - Tests: extend `tests/device/VFDCU40026HALTests.hpp` with `helper_luminance_index` verifying ESC 'L' 0x80 for idx=2; keep existing init/clear/home/positioning checks.
 - HAL: CU20025 (VFDCU20025HAL)
   - Add `setBrightnessIndex(uint8_t 0..3)` mapping to device Brightness Set (data write 00..03). No IVFDHAL changes.
   - Tests: extend `tests/device/VFDCU20025HALTests.hpp` with `helper_brightness_index` (writes 0x02 after init for idx=2).
 - HAL: HT16514 (VFDHT16514HAL)
   - Add `setBrightnessIndex(uint8_t 0..3)` helper wiring BR1/BR0 via Function Set; keep 2‑line mode. Dimming maps 0:100%, 1:75%, 2:50%, 3:25%.
   - Tests: cover dimming Function Set codes in `tests/device/VFDHT16514HALTests.hpp`.
- HAL: VK202‑25 (VFDVK20225HAL)
  - Add FE‑prefixed helpers: `autoLineWrapOn()` (FE 67), `autoLineWrapOff()` (FE 68), and `saveBrightness(uint8_t)` (FE 145 Byte). Contract + device tests registered.

- Docs: datasheets
  - Reorganize OCR outputs into `docs/datasheets/ocr/` and text sidecars into `docs/datasheets/text/`; stage removals of old top-level sidecar files.
  - Add/refresh OCR and `.txt` for CU40026, CU20025, HT16514, 20T202DA2JA, M202MD15AJ, and M202SD01HA; include VK202‑25 manual OCR and sidecar.

- HAL: NA204SD01 (VFDNA204SD01HAL)
  - New HAL for Noritake NA204SD01 (20x4) using SD01 command set (Reset 0x1F, Clear 0x0D, Home 0x0C, Pos 0x10+addr with 00/14/28/3C bases, Dimming 0x04 + 6 levels).
  - Capabilities: 4x20, dimming, cursor/blink; added factory to CapabilitiesRegistry.
  - Tests: device tests for reset, clear/home, 4th-row positioning, and dimming; registered in Arduino and embedded runners along with IVFDHAL contract tests.
  - Docs: `docs/api/VFDNA204SD01HAL.md` short reference and link to datasheet. Added OCR+text sidecar for `NA204SD01CC_revC_12Mar2013.pdf`.

- HAL: PT6302 (VFDPT6302HAL)
  - New HAL for Princeton PT6302 providing a 1x16 character abstraction using CGROM (5x7) and DCRAM writes; supports number-of-digits (K=7, 16 digits), duty/dimming (8 levels), and lights control.
  - Capabilities: 1x16, serial interface, 8 CGRAM chars, 8 dimming levels; factory added to CapabilitiesRegistry.
  - Tests: device tests for init bytes, DCRAM header + chars on writeAt, and dimming codes; registered in both runners alongside contract tests.
  - Docs: `docs/api/VFDPT6302HAL.md` command summary; OCR + text sidecar added for `PT6302.PDF`.

- HAL: PT6314 (VFDPT6314HAL)
  - New HAL for PT6314 (HD44780-like) with 20x2 default geometry; init 0x38, 0x0C, 0x01, 0x06; DDRAM/CGRAM addressing and basic write helpers.
  - Capabilities: 20x2, serial/parallel, 8 CGRAM chars; factory added.
  - Tests: init sequence and DDRAM addressing; registered in Arduino and embedded runners.
  - Docs: `docs/api/VFDPT6314HAL.md`. OCR + text sidecar added for `PT6314.PDF`.

- HAL: uPD16314 (VFDUPD16314HAL)
  - New HAL for NEC/Renesas uPD16314 (HD44780-like) with Function Set brightness control (BR1/BR0) and default 20x2 geometry.
  - Capabilities: 20x2, serial/parallel, 8 CGRAM chars, 4 brightness levels; factory added.
  - Tests: init sequence, clear/home/pos, and dimming via Function Set; added to both runners with contract tests.
  - Docs: `docs/api/VFDUPD16314HAL.md`. OCR + text sidecar added for `UPD16314.PDF`.

- Enhancements
  - PT6314: add serial-mode write framing (Start Byte 0xF8 | R/W<<2 | RS<<1) when control lines are unavailable.
  - PT6302: implement CGRAM `setCustomChar()` (0x20 | addr, 7 row bytes) with unit test.


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

- Cleanup
  - Remove legacy `Makefile.simple` and `Makefile.enhanced` in favor of the unified root `Makefile` (PIO/Arduino/AVR backends).
  - Delete stray editor backup files in `src/HAL` (`*~`).

## 2025-09-22
- Initial make/PIO integration work on examples and build scripts.
- HAL: VFD20T204HAL (generic 20x4 HD44780-like)
  - Add new 20x4 HAL with DDRAM bases 0x00/0x40/0x14/0x54; Function Set, Display On, Clear, Entry Mode, and CGRAM writes.
  - Capabilities: 4x20, serial/parallel, 8 CGRAM chars.
  - Tests: init sequence and DDRAM base addressing; registered in Arduino and embedded runners.
  - Docs: `docs/api/VFD20T204HAL.md`. Notes compatibility for Noritake CU20045SCPB‑U2J and MOP‑AV204A.

- Docs: compatibility notes
  - VFDM0216MDHAL: add compatibility for Noritake CU16025ECPB‑W6J and CU16029 (ECPB‑W1J/UW1J).
  - VFD20T202HAL: add compatibility for Matrix Orbital MOP‑AV202C.
