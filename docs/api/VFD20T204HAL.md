# VFD20T204HAL (Generic 20x4 HD44780‑like)

Generic HAL for 20×4 character VFD modules that expose an HD44780‑style instruction set.

- Init: Function Set 0x38 (2‑line mode used for 4‑line mapping), Display On 0x0C, Clear 0x01, Entry Mode 0x06.
- Positioning: Set DDRAM 0x80 | addr with row bases: 0x00, 0x40, 0x14, 0x54.
- Custom chars: CGRAM 0x40 | addr, 8 glyphs (5×8 rows).
- Dimming: not exposed generically.

Compatibility
- Noritake CU20045SCPB‑U2J (20×4) — HD44780‑like.
- Matrix Orbital MOP‑AV204A (20×4) — HD44780‑compatible.

See device datasheets in docs/datasheets for dimensions and timings.
