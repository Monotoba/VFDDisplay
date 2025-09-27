# VFDUPD16314HAL (NEC/Renesas uPD16314, 20x2 default)

Device HAL for the uPD16314 VFD controller/driver. The controller exposes an HD44780-like instruction set with a Function Set that includes brightness bits `BR1:BR0`.

- Init: Function Set 0x30 | N | BR (0x38 default for 2-line, BR=0), Display On 0x0C, Clear 0x01, Entry Mode 0x06.
- Positioning: Set DDRAM Address 0x80 | addr; bases 0x00 (row 0) and 0x40 (row 1).
- Brightness: via Function Set `BR1:BR0` (4 levels); provided as `setDimming(level)` and `setBrightness(lumens)` mapping.
- Custom chars: CGRAM Address 0x40 | addr; 8 glyphs (5x8 rows).

See `docs/datasheets/UPD16314.PDF` (and OCR sidecar) for full details and timings.
