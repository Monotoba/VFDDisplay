# VFD20T202HAL

Device HAL for Futaba 20T202 20x2 VFD modules implementing `IVFDHAL`.

Status
- Implements HD44780-style instruction set per datasheet (Function Set with brightness BR1/BR0, DDRAM/CGRAM addressing, Display On/Off, Clear/Home, Entry Mode Set).
- 3‑wire synchronous serial supported via `SynchronousSerialTransport` (/STB,SCK,SIO).

Key Methods
- Positioning: `setCursorPos(row,col)` → Set DDRAM (0x80 | addr), row bases 0x00 and 0x40
- Clear/Home: 0x01 (clear), 0x02 (home)
- Write: `write(const char*)`, `writeChar(char)`
- Center: `centerText(const char*, row)`
- Custom chars: `setCustomChar(index, pattern[8])` writes CGRAM rows; indices 0..7 map to 0x00..0x07

Capabilities
- 2 rows x 20 columns, typical 5x8 characters
- Serial (3‑wire) and parallel interfaces
- Dimming supported via Function Set (BR1/BR0 → 100/75/50/25%)

Compatibility
- Matrix Orbital MOP‑AV202C (20×2) — HD44780‑compatible; works with this HAL.

NO_TOUCH Primitives
- `_cmdInit()`, `_escReset()`, `_cmdClear()`, `_cmdHome()`, `_posLinear()`, `_escMode()`, `_escDimming()`, `_escCursorBlink()`
- Only modify these after validating against the 20T202 datasheet.

Integration
- Works with `BufferedVFD` via the `IVFDHAL` methods (writeAt/moveTo/write).

## Unbuffered Usage (IVFDHAL via VFDDisplay)

```cpp
#include "VFDDisplay.h"
#include "HAL/VFD20T202HAL.h"
#include "Transports/SynchronousSerialTransport.h"

VFD20T202HAL hal;
SynchronousSerialTransport tx(/*stb=*/7, /*sck=*/6, /*sio=*/5);
VFDDisplay vfd(&hal, &tx);

void setup() {
  vfd.init();              // Function Set, Display On, Clear, Entry Mode
  vfd.clear();
  vfd.writeAt(0,0,"Hello 20x2");
  vfd.centerText("Centered", 1);

  // Positioning: row base 0x00/0x40
  vfd.setCursorPos(1,3);   // internally sends 0x80 | (0x40+3) = 0xC3
  vfd.write("!");

  // Brightness via Function Set
  vfd.setDimming(0);       // 100%
  vfd.setDimming(1);       // 75%
  vfd.setDimming(2);       // 50%
  vfd.setDimming(3);       // 25%

  // Custom char (CGRAM)
  uint8_t heart[8] = {0x00,0x0A,0x1F,0x1F,0x0E,0x04,0x00,0x00};
  vfd.setCustomChar(0, heart);
  vfd.writeCustomChar(0);
}

void loop() {}
```

### Method-by-Method Examples (Unbuffered)

Lifecycle
```cpp
vfd.init();
vfd.reset();        // re-initializes function set + defaults
```

Screen control
```cpp
vfd.clear();        // 0x01
vfd.cursorHome();   // 0x02
vfd.setCursorPos(0,0);
```

Writing
```cpp
vfd.write("ABC");
vfd.writeChar('X');
vfd.writeAt(1, 10, "RIGHT");
vfd.centerText("TITLE", 0);
```

Custom characters
```cpp
uint8_t glyph[8] = {/* 8 rows, bits 0..4 used */};
vfd.setCustomChar(3, glyph);
vfd.writeCustomChar(3);
```

Display control
```cpp
vfd.setCursorBlinkRate(1);  // toggles blink bit in Display On/Off
vfd.setDimming(2);        // map to Function Set BR bits (50%)
```

## Buffered Usage (BufferedVFD)

```cpp
#include "Buffered/BufferedVFD.h"

BufferedVFD buf(&hal);

void setup() {
  vfd.init();
  buf.init();
  buf.clearBuffer();
  buf.writeAt(0,0,"Buffered");
  buf.centerText(1, "20x2");
  buf.flush();
}
```

## Device-Specific (non‑IVFDHAL) Methods

- None exposed beyond the `IVFDHAL` surface in this HAL. All device semantics are implemented via `IVFDHAL` methods.
