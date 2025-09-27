# VFDPT6314HAL (Princeton PT6314, 20x2 default)

Device HAL for the Princeton PT6314 VFD controller/driver. The controller provides an HD44780-like instruction set (Function Set, Display Control, Clear, Entry Mode, Set DDRAM/CGRAM Address) with both parallel (RS/RW/E) and serial options. This HAL targets a default 20x2 text geometry.

- Init sequence: Function Set 0x38 (2-line), Display On 0x0C, Clear 0x01, Entry Mode 0x06.
- Positioning: Set DDRAM Address 0x80 | addr; bases 0x00 (row 0), 0x40 (row 1).
- Custom chars: CGRAM Address 0x40 | addr; 8 glyphs (5x8 rows).
- Dimming: not exposed via this HAL.

See `docs/datasheets/PT6314.PDF` (and OCR sidecar) for the complete instruction set and timings.
