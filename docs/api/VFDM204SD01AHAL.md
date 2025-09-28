# VFDM204SD01AHAL (Futaba M204SD01A, 20×4)

HAL for the Futaba M204SD01A 20×4 VFD module using the SD01A command set.

- Positioning: `0x10` + linear address with row bases `0x00, 0x14, 0x28, 0x3C`.
- Clear: `0x0D`
- Home (cursor to top-left): `0x0C`
- Dimming: `0x04` + data (4 levels used); this HAL maps dimming levels to `{00h, 40h, 80h, FFh}`.
- Writes: ASCII character data; `LF` (0x0A), `CR` (0x0D) are supported by the device.

Notes
- User-defined characters are device-specific (DEF) and not exposed in this HAL.
- Serial and parallel modes are supported by the module; this HAL uses the injected transport.

See `docs/datasheets/M204SD01A.pdf` and OCR sidecar for details.
