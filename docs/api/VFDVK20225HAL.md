# VFDVK20225HAL

HAL for Matrix Orbital VK202-25 20×2 VFD module implementing `IVFDHAL`.

Datasheet
- docs/datasheets/Manual for VK202-25 Rev 3.0.pdf (OCR sidecar retained)

Capabilities
- 2 × 20 characters, 5×7 dots
- Interface: Serial (RS-232/TTL/I2C options; this HAL targets 0xFE serial command prefix)
- User-defined characters via banks (not implemented here)
- Dimming/Brightness: 0..255 via command

Instruction Mapping (VK commands)
- Prefix: 254 (0xFE)
- Clear Screen: FE 58 (decimal 254 88)
- Set Cursor Position: FE 71 Column Row (1-based)
- Auto Line Wrap On: FE 67; Off: FE 68
- Set VFD Brightness: FE 89 Byte (0..255)

Unbuffered Usage
```cpp
#include "VFDDisplay.h"
#include "HAL/VFDVK20225HAL.h"
#include "Transports/SerialTransport.h"

VFDVK20225HAL hal; SerialTransport tx(&Serial1); VFDDisplay vfd(&hal,&tx);
void setup(){ Serial1.begin(19200); vfd.init(); vfd.clear(); vfd.writeAt(0,0,"VK202-25"); vfd.setDimming(128); }
```

Buffered Usage
```cpp
#include "Buffered/BufferedVFD.h"
BufferedVFD buf(&hal);
void setup(){ vfd.init(); buf.init(); buf.centerText(0,"Buffered 20x2"); buf.flush(); }
```

Notes
- Cursor blink rate is not programmable; this HAL reports NotSupported for exact rate control.
- Custom character banks and advanced drawing are not covered here; this HAL focuses on core text operations and brightness.

Device-Specific Helpers
- `autoLineWrapOn()` → FE 67
- `autoLineWrapOff()` → FE 68
- `saveBrightness(uint8_t)` → FE 145 Byte (persists to NVM)

Tests
- Clear: FE 58; SetCursorPos row/col → FE 71 C R; Brightness → FE 89 Byte.
