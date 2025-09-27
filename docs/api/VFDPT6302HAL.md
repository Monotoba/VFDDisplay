# VFDPT6302HAL (Princeton PT6302, 1x16)

Device HAL for the Princeton PT6302 VFD Controller/Driver. Provides a 1x16 character display abstraction using the PT6302’s CGROM (5x7) and DCRAM addressing.

- DCRAM write: `0x10 | addr` then character codes; addresses auto-increment on continuous writes.
- CGRAM write: `0x20 | addr` followed by 35-bit pattern data (not implemented in this HAL).
- ADRAM write: `0x30 | addr` to set 2-bit symbol data per digit (optional cursor usage; not implemented).
- Number of digits: `0x60 | K` (K=0..7 for 9..16 digits). HAL uses 16 digits (K=7).
- Duty (dimming): `0x50 | D` (D=0..7 for 8/16..15/16 duty). Exposed via `setDimming(level)` and `setBrightness(lumens)` mapping.
- Lights: `0x70 | (H<<1|L)` where `L=0,H=0` = normal display, other combinations force all outputs low/high.

Notes
- IVFDHAL surface is implemented without interface changes; cursor/blink and user chars are not implemented.
- See `docs/datasheets/PT6302.PDF` (and OCR sidecar) for the full command reference and timing.
