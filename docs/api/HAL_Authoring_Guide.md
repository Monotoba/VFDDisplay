# HAL Authoring Guide (IVFDHAL)

This guide describes the minimal steps to add a new device HAL that implements `IVFDHAL` without changing locked components.

Locked Components
- Do not modify: `IVFDHAL`, existing device HALs (e.g., `VFD20S401HAL`), and `BufferedVFD`.
- All changes for a new device must stay within the new HAL, its docs, and tests.

1) Gather and OCR the Datasheet
- Place PDF in `docs/datasheets/`.
- If scanned, OCR and keep the sidecar text:
  - `ocrmypdf --sidecar <file>.txt <file>.pdf <file>_ocr.pdf`
- Identify: init sequence, clear/home codes, positioning method (DDRAM/ESC), CGRAM/UDF format, display on/off/blink, brightness/dimming, timing.

2) Implement the HAL
- Create `src/HAL/<YourHAL>.h/.cpp` implementing all `IVFDHAL` methods.
- Define device NO_TOUCH primitives for raw commands (e.g., `_cmdClear()`, `_posRowCol()`, etc.).
- Map `setCursorPos()` correctly (e.g., HD44780: DDRAM 0x80|addr; row bases 0x00/0x40; 4×20 devices: linear address or device‑specific mapping).
- If brightness/dimming is device‑specific (e.g., function set bits), expose it through `setDimming()`/`setBrightness()`.

Quick scaffold (optional)
- `make hal NAME=20X2ABC CLASS=VFD20X2ABCHAL ROWS=2 COLS=20 DATASHEET=docs/datasheets/20X2ABC.pdf FAMILY=hd44780 TRANSPORT=sync3`
- This generates HAL headers/sources, a device test stub, and a docs page based on the template.

3) Capabilities
- Add a factory in `CapabilitiesRegistry` (e.g., `create<Your>Capabilities()`): rows/cols, user chars, flags (DIMMING, H/V scroll, interfaces), timing.

4) Transports
- Reuse existing transports; for 3‑wire synchronous serial, use `SynchronousSerialTransport`.
- If a new bus is required, implement an `ITransport` subclass (separate from HAL) and add docs.

5) Tests
- Reuse contract tests: `register_IVFDHAL_contract_tests<YourHAL>("Name")`.
- Add device‑specific tests under `tests/device/`: init, clear/home, positioning, brightness/dimming, custom char.
- Use `MockTransport` to verify exact bytes.
- Plug into runners: `tests/embedded_runner/main.cpp` and Arduino runner.

6) Documentation
- Copy `docs/api/HAL_Doc_Template.md` to `docs/api/<YourHAL>.md`.
- Fill in method list, instruction mapping, unbuffered and buffered examples, and device‑specific notes.

7) CHANGELOG and Commits
- Update `CHANGELOG.md` describing the added HAL, tests, any transport/doc additions.
- Make small, focused commits; do not include unrelated files.

Checklist (copy into PRs)
- [ ] Datasheet added (and OCR `.txt` if scanned)
- [ ] HAL implements all `IVFDHAL` methods
- [ ] NO_TOUCH primitives cover raw commands
- [ ] Capabilities registered
- [ ] Contract + device tests added and included in runners
- [ ] Per‑HAL doc page created from template
- [ ] CHANGELOG updated
- [ ] No changes to locked components

Notes
- For DDRAM addressing: verify row bases and width. Common patterns:
  - 20×2 (HD44780): row0 base 0x00, row1 base 0x40 → command `0x80 | base + col`
  - 4×20 linear (e.g., 20S401): ESC 'H' + linear address `row*20 + col`
- For brightness: confirm bit positions and valid levels from the datasheet.

## Operational Flow Used (step‑by‑step)

When adding new HALs or mapping devices to existing HALs, follow this pragmatic flow:

1) Scan + OCR new datasheets
   - Drop PDFs under `docs/datasheets/`.
   - OCR sidecars to `docs/datasheets/text/` and `docs/datasheets/ocr/`.
   - Triage: keep character‑type (5×7, DDRAM/CGRAM or ESC addressing); list/remove non‑character items.

2) Decide reuse vs. new HAL
   - HD44780‑like 16×2 → reuse `VFDM0216MDHAL`.
   - HD44780‑like 20×2 → reuse `VFD20T202HAL`.
   - HD44780‑like 20×4 → reuse `VFD20T204HAL` (row bases 00/40/14/54).
   - SD01/SD01A family (e.g., M202SD01, NA204SD01, M204SD01A) → device‑specific HALs using module command bytes (0x10+addr, 0x0D clear, etc.).

3) If reusing an existing HAL
   - Add a Compatibility note to the HAL doc page (e.g., `docs/api/VFDM0216MDHAL.md`).
   - No code changes required unless the device needs extra helpers.

4) If implementing a new HAL
   - Create `src/HAL/<DeviceHAL>.h/.cpp` implementing `IVFDHAL`.
   - Add a capabilities factory to `CapabilitiesRegistry`.
   - Add device tests under `tests/device/` (init, clear/home, positioning, brightness if any).
   - Register in both test runners.
   - Create `docs/api/<DeviceHAL>.md` from the template and document command mappings.
   - Update `CHANGELOG.md`.

5) Validate + commit
   - Ensure tests compile in embedded runners (Arduino/PIO environment).
   - Make small, focused commits per HAL/docs/tests.

Tip: Prefer documenting compatibility over duplicating HALs when the instruction set matches an existing implementation.
