# VFDM202MD15HAL

HAL for Futaba M202MD15 20×2 VFD module implementing `IVFDHAL`.

Datasheet
- docs/datasheets/M202MD15AJ.pdf (OCR sidecar: M202MD15AJ.txt)

Capabilities
- 2 × 20 characters, 5×8 dots
- Interfaces: Parallel (M68/i80) and Synchronous Serial
- User-defined characters (CGRAM) × 8
- Dimming: 4 levels via Function Set BR1/BR0

Instruction Mapping (HD44780-like)
- Function Set: 0x30 | N<<3 | BR1..BR0
- Display On/Off: 0x08 | D<<2 | C<<1 | B
- Clear: 0x01, Home: 0x02
- Entry Mode: 0x04..0x07
- Set CGRAM: 0x40 | addr (6-bit)
- Set DDRAM: 0x80 | addr (row0 base 0x00, row1 base 0x40)

Unbuffered Usage (IVFDHAL via VFDDisplay)
```cpp
#include "VFDDisplay.h"
#include "HAL/VFDM202MD15HAL.h"
#include "Transports/SynchronousSerialTransport.h"

VFDM202MD15HAL hal;
SynchronousSerialTransport tx(/*stb=*/7, /*sck=*/6, /*sio=*/5);
VFDDisplay vfd(&hal, &tx);

void setup(){ vfd.init(); vfd.clear(); vfd.writeAt(0,0,"M202MD15"); vfd.setDimming(1); }
```

Buffered Usage (BufferedVFD)
```cpp
#include "Buffered/BufferedVFD.h"
BufferedVFD buf(&hal);
void setup(){ vfd.init(); buf.init(); buf.centerText(0,"Buffered"); buf.flush(); }
```

Notes
- Brightness via Function Set: 0..3 → 100/75/50/25%.
- Blink is on/off via Display On/Off (B bit).

Tests
- Init sequence (0x38, 0x0C, 0x01, 0x06), clear/home, DDRAM addressing, dimming mapping.

