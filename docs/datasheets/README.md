# Datasheets Index

This folder contains controller and module datasheets referenced by the VFDDisplay library. They inform the HAL command mappings, timing, and capabilities.

Included PDFs
- 20s401da1.pdf — VFD20S401 controller datasheet used by `VFD20S401HAL` (init, clear, home, ESC 'H' addressing, dimming, blink).
- M202MD15AJ.pdf — Futaba 20x2 VFD module details (mechanical, electrical).
- M202MD15FA-Futaba.pdf — Futaba 20x2 VFD module variant.
- M202SD01HA.pdf — Futaba 20x2 VFD module; timing and power references.
- NA204SD01CC_revC_12Mar2013.pdf — 20x4 VFD module datasheet (NA204SD01CC); dimensions, pinout, and timing.

Notes
- The VFD20S401 HAL implements ESC-based positioning (`ESC 'H' + linear address`) per 20s401da1.pdf.
- For module power and timing constraints, prefer the specific module’s datasheet when connecting hardware.
- When adding a new HAL, cite its controller or module datasheet link here and in the corresponding API doc.

Removed
- The HD44780.pdf reference was removed as it is not directly used in this library; it can be consulted externally if needed.
