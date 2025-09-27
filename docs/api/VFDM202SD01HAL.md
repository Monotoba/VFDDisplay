# VFDM202SD01HAL

HAL for Futaba M202SD01HA 20×2 VFD module implementing `IVFDHAL`.

Datasheet
- docs/datasheets/M202SD01HA.pdf (OCR sidecar: M202SD01HA.txt)

Capabilities
- 2 × 20 characters, 5×7 dots
- Interfaces: Parallel (M68/i80) and Serial (selectable baud)
- Dimming: 6 levels via DIM command (04h + data)

Instruction Mapping (Module-specific)
- DIM: 04h then data (00h=0%, 20h=20%, 40h=40%, 60h=60%, 80h=80%, FFh=100%)
- Back Space: 08h, Horizontal Tab: 09h
- Clear: 0Dh (clears DDRAM, keeps dimming/cursor)
- Display Position: 10h + addr (row0 00h..13h, row1 14h..27h)
- Cursor Mode: 17h + data (FFh lighting, 88h blinking, 00h off)
- Reset: 1Fh

Unbuffered Usage (IVFDHAL via VFDDisplay)
```cpp
#include "VFDDisplay.h"
#include "HAL/VFDM202SD01HAL.h"
#include "Transports/SynchronousSerialTransport.h"

VFDM202SD01HAL hal;
SynchronousSerialTransport tx(/*stb=*/7, /*sck=*/6, /*sio=*/5);
VFDDisplay vfd(&hal, &tx);

void setup(){
  vfd.init();               // 1Fh
  vfd.clear();              // 0Dh
  vfd.setCursorPos(1,3);    // 10h + 17h
  vfd.write("20x2");
  vfd.setDimming(3);        // 04h + 60h
}
```

Buffered Usage (BufferedVFD)
```cpp
#include "Buffered/BufferedVFD.h"
BufferedVFD buf(&hal);
void setup(){ vfd.init(); buf.init(); buf.centerText(0,"Buffered"); buf.flush(); }
```

Notes
- The module does not expose CGRAM programming in the manual; custom glyphs are NotSupported in this HAL.
- Cursor blink control is via 17h + 88h; rate is not programmable.

Tests
- Reset (1Fh), clear/home codes, addressing via 10h+addr, dimming level mapping.

