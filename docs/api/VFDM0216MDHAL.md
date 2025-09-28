# VFDM0216MDHAL

HAL for M0216MD 16×2 VFD module implementing `IVFDHAL` (HD44780-like).

Datasheet
- docs/datasheets/M0216MD-162MDBR2-J.pdf (OCR: M0216MD-162MDBR2-J.txt)

Capabilities
- 2 × 16 characters, 5×8 dots
- Interfaces: Parallel (M68/i80) and Serial (synchronous)
- User-defined characters (CGRAM) × 8
- Dimming: 4 levels via Function Set BR1/BR0

Instruction Mapping
- Function Set: 0x30 | N<<3 | BR1..BR0
- Display On/Off: 0x08 | D<<2 | C<<1 | B
- Clear: 0x01, Home: 0x02; Entry Mode: 0x04..0x07
- Set CGRAM: 0x40 | addr; Set DDRAM: 0x80 | addr (row0 base 0x00, row1 base 0x40)

Unbuffered Usage
```cpp
#include "VFDDisplay.h"
#include "HAL/VFDM0216MDHAL.h"
#include "Transports/SynchronousSerialTransport.h"

VFDM0216MDHAL hal; SynchronousSerialTransport tx(7,6,5); VFDDisplay vfd(&hal,&tx);
void setup(){ vfd.init(); vfd.clear(); vfd.writeAt(0,0,"16x2"); vfd.setDimming(1); }
```

Buffered Usage
```cpp
#include "Buffered/BufferedVFD.h"
BufferedVFD buf(&hal);
void setup(){ vfd.init(); buf.init(); buf.centerText(0,"Buffered 16x2"); buf.flush(); }
```

Notes
- Brightness via Function Set: 0..3 map to 100/75/50/25%.
- Blink is on/off via Display On/Off (B bit); rate not programmable.

Compatibility
- Noritake CU16025ECPB‑W6J (16×2)
- Noritake CU16029ECPB‑W1J / CU16029‑UW1J (16×2)
These modules implement an HD44780‑like instruction set and work with this HAL.

Tests
- Init (0x38,0x0C,0x01,0x06); clear/home; setCursorPos uses 0x80 | base+col; dimming mapping.
