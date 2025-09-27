# VFDNA204SD01HAL (Noritake NA204SD01, 20x4)

Device HAL for the Noritake NA204SD01 20x4 character VFD module using the SD01 command set.

- Positioning: `0x10` + linear address. Row bases (0‑based): `0x00, 0x14, 0x28, 0x3C`; address = base[row] + col.
- Reset: `0x1F`
- Clear: `0x0D`
- Home: `0x0C`
- Dimming: `0x04` + code, where levels map to `{00,20,40,60,80,FF}` for 6 levels.
- Cursor modes: `0x17` + data (0x00 off, 0x88 blink, 0xFF solid)

IVFDHAL surface is fully implemented without interface changes. Custom characters are not supported on this device family in this HAL.

See `docs/datasheets/NA204SD01CC_revC_12Mar2013.pdf` for command details.
