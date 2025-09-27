# VFD20T202HAL

Device HAL for Futaba 20T202 20x2 VFD modules implementing `IVFDHAL`.

Status
- Initial implementation mirrors the 20S401-style ESC addressing (ESC 'H' + linear address) and common control codes where applicable.
- Review command bytes against the module’s datasheet and adjust NO_TOUCH primitives as needed.

Key Methods
- Positioning: `setCursorPos(row,col)` → ESC 'H' + linear address (row*20 + col)
- Clear/Home: single-byte `0x09` (clear), `0x0C` (home)
- Write: `write(const char*)`, `writeChar(char)`
- Center: `centerText(const char*, row)`
- Custom chars: conservative 0..7 → codes 0x00..0x07 (set/save returns NotSupported until confirmed)

Capabilities
- 2 rows x 20 columns, typical 5x8 characters
- Serial interface advertised

NO_TOUCH Primitives
- `_cmdInit()`, `_escReset()`, `_cmdClear()`, `_cmdHome()`, `_posLinear()`, `_escMode()`, `_escDimming()`, `_escCursorBlink()`
- Only modify these after validating against the 20T202 datasheet.

Integration
- Works with `BufferedVFD` via the `IVFDHAL` methods (writeAt/moveTo/write).

