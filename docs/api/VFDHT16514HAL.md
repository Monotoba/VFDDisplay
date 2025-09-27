# VFDHT16514HAL

HAL for Holtek HT16514 VFD controller/driver (supports 16/20/24 × 2), implementing `IVFDHAL`.

Datasheet
- docs/datasheets/Holtek-HT16514-001-datasheet.pdf (OCR sidecar: Holtek-HT16514-001-datasheet.txt)

Capabilities
- Default 2 × 20 characters (chip supports 40-column DDRAM per row; modules vary)
- Interfaces: Serial (SI/SO/SCK/ST) and Parallel (M68/i80)
- User-defined characters (CGRAM) × 8
- Dimming: 4 levels via Function Set BR1/BR0

Instruction Mapping (HD44780-like)
- Function Set: 0x30 | N<<3 | BR1..BR0 (data length fixed 8-bit here)
- Display On/Off: 0x08 | D<<2 | C<<1 | B
- Clear: 0x01, Home: 0x02
- Entry Mode: 0x04..0x07 (I/D, S)
- Set CGRAM: 0x40 | addr (6-bit)
- Set DDRAM: 0x80 | addr (row0 base 0x00, row1 base 0x40)

Unbuffered Usage (IVFDHAL via VFDDisplay)
```cpp
#include "VFDDisplay.h"
#include "HAL/VFDHT16514HAL.h"
#include "Transports/SynchronousSerialTransport.h"

VFDHT16514HAL hal;
SynchronousSerialTransport tx(/*stb=*/7, /*sck=*/6, /*sio=*/5);
VFDDisplay vfd(&hal, &tx);

void setup(){ vfd.init(); vfd.clear(); vfd.writeAt(0,0,"HT16514"); vfd.setDimming(1); }
```

Buffered Usage (BufferedVFD)
```cpp
#include "Buffered/BufferedVFD.h"
BufferedVFD buf(&hal);
void setup(){ vfd.init(); buf.init(); buf.centerText(0,"Buffered"); buf.flush(); }
```

Notes
- Brightness via Function Set: 0..3 map to 100/75/50/25% respectively.
- Blink is on/off (Display On/Off control B bit); there is no programmable blink period.

Tests
- Init sequence (0x38, 0x0C, 0x01, 0x06), clear/home, DDRAM addressing, brightness mapping.

