# VFDSTV7710HAL (STV7710 Graphics VFD Driver)

This HAL is a placeholder for the STV7710, a graphics-oriented VFD driver that does not expose character DDRAM/CGRAM like HD44780-compatible parts. Consequently, the text-centric IVFDHAL methods are not supported by this HAL yet.

- Purpose: provide a type-safe hook and capabilities entry for future graphics APIs.
- Text methods: return NotSupported.
- Capabilities: no text geometry; marks serial/parallel as supported transport types.

Datasheet: `docs/datasheets/stv7710.pdf` (OCR sidecar added).
