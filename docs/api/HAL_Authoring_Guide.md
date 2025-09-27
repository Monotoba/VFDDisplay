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

