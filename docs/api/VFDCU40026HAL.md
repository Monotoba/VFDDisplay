# VFDCU40026HAL

HAL for Noritake CU40026-TW200A (40×2, 5×7) character VFD implementing `IVFDHAL`.

Datasheet
- docs/datasheets/CU40026-TW200A_e00-f7.pdf (OCR sidecar: CU40026-TW200A_e00-f7.txt)

Capabilities
- 2 rows × 40 columns, 5×7 characters
- Interfaces: Serial and Parallel (per datasheet)
- User-defined characters (UDF) up to 16 via ESC ‘C’
- Luminance control via ESC ‘L’ (four bands)

Instruction Mapping
- Control codes:
  - BS 0x08, HT 0x09, LF 0x0A, FF (home top-left) 0x0C, CR 0x0D, CLR 0x0E, CAN 0x0F
- Cursor Addressing: ESC ‘H’ + addr (top row: 0x00..0x27, bottom row: 0x28..0x4F)
- Initialize: ESC ‘I’
- Luminance: ESC ‘L’ + data (00..3F≈25%, 40..7F≈50%, 80..BF≈75%, C0..FF=100%)
- Blink Speed: ESC ‘T’ + data (period ≈ data × 30ms)
- Char Tables: 0x18 (CT0), 0x19 (CT1)
- UDF: ESC ‘C’ + CHR + PT1..PT5 (5 bytes packed column data)

Unbuffered Usage (IVFDHAL via VFDDisplay)
```cpp
#include "VFDDisplay.h"
#include "HAL/VFDCU40026HAL.h"
#include "Transports/SynchronousSerialTransport.h" // or another suitable transport

VFDCU40026HAL hal;
SynchronousSerialTransport tx(/*stb=*/7, /*sck=*/6, /*sio=*/5);
VFDDisplay vfd(&hal, &tx);

void setup() {
  vfd.init();               // ESC 'I'
  vfd.clear();              // 0x0E
  vfd.setCursorPos(1, 3);   // ESC 'H' + (40+3)
  vfd.write("Hello 40x2");

  // Luminance (bands): 25%, 50%, 75%, 100%
  vfd.setDimming(3); // 100%

  // Blink speed ~ 600ms
  vfd.cursorBlinkSpeed(600);
}
```

Buffered Usage (BufferedVFD)
```cpp
#include "Buffered/BufferedVFD.h"
BufferedVFD buf(&hal);
void setup(){ vfd.init(); buf.init(); buf.writeAt(0,0,"Buffered"); buf.flush(); }
```

Device-Specific Notes
- Cursor and screen control include additional DCx modes (DC1..DC7) and line-control semantics — refer to datasheet §7.2 for behavior.
- ESC ‘L’ data bands can be set to 00, 40, 80, C0 hex to hit nominal 25/50/75/100%.

Tests
- Init (ESC ‘I’), clear/home codes, ESC ‘H’ addressing, ESC ‘L’ luminance levels.

