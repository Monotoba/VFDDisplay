# VFDCU20025HAL

HAL for Noritake CU20025ECPB-W1J (20×2, 5×7) character VFD implementing `IVFDHAL`.

Datasheet
- docs/datasheets/cu20025ecpb-w1j.pdf (OCR sidecar: cu20025ecpb-w1j.txt)

Capabilities
- 2 rows × 20 columns, 5×7 dot characters
- Interfaces: Parallel (M68/i80 selectable)
- User-defined characters (CGRAM) × 8
- Dimming: 4 levels via Brightness Set

Instruction Mapping
- Clear: 0x01
- Home: 0x02
- Entry Mode: 0x04..0x07
- Display On/Off: 0x08..0x0F (D/C/B bits)
- Cursor/Display Shift: 0x10..0x1F
- Function Set: 0x20..0x3F (lines etc.)
- Set CGRAM: 0x40..0x7F
- Set DDRAM: 0x80 | addr (row0 base 0x00, row1 base 0x40)
- Brightness Set: data write 0x00..0x03 (RS=H)

Unbuffered Usage (IVFDHAL via VFDDisplay)
```cpp
#include "VFDDisplay.h"
#include "HAL/VFDCU20025HAL.h"
#include "Transports/SynchronousSerialTransport.h" // or a parallel transport

VFDCU20025HAL hal;
SynchronousSerialTransport tx(/*stb=*/7, /*sck=*/6, /*sio=*/5); // if using 3-wire bridge
VFDDisplay vfd(&hal, &tx);

void setup() {
  vfd.init();            // 0x38,0x0C,0x01,0x06
  vfd.clear();
  vfd.writeAt(0,0,"CU20025");
  vfd.setCursorPos(1,3);
  vfd.write("!");

  // Dimming levels (0:100%, 1:75%, 2:50%, 3:25%)
  vfd.setDimming(2);

  // Custom glyph
  uint8_t glyph[8] = {0x00,0x0A,0x1F,0x1F,0x0E,0x04,0x00,0x00};
  vfd.setCustomChar(0, glyph);
  vfd.writeCustomChar(0);
}
```

Buffered Usage (BufferedVFD)
```cpp
#include "Buffered/BufferedVFD.h"
BufferedVFD buf(&hal);
void setup(){ vfd.init(); buf.init(); buf.writeAt(0,0,"Buffered"); buf.flush(); }
```

Device-Specific Notes
- Brightness Set uses data writes (00h..03h). A transport that supports control lines (RS) is recommended (`SynchronousSerialTransport` embeds RS into its start byte).

Tests
- Init sequence, clear/home, DDRAM addressing, dimming coding.

