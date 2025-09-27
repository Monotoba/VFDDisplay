# HAL Documentation Template

Use this template to document a new device HAL. Copy to `docs/api/<YourHAL>.md` and fill in each section.

## <HAL Name>

Overview
- Device family/model, character geometry, controller type.
- Primary interface(s): Serial/Parallel/3‑wire synchronous.
- One‑sentence summary of how this HAL maps the device’s command set to `IVFDHAL`.

Datasheet Links
- <link to docs/datasheets/...pdf>
- If OCR’d: keep `<file>.txt` sidecar in the same folder.

Capabilities Summary
- Rows × Columns, 5×8 or 5×7 characters
- User‑defined chars support (count)
- Dimming/brightness support
- Interfaces supported

Transports
- Recommended transport(s) (e.g., `SerialTransport`, `SynchronousSerialTransport`), pin notes if needed.

Instruction Set Mapping (brief)
- Positioning: how `setCursorPos()` maps to device command (e.g., `0x80|addr`, row bases `0x00/0x40`)
- Clear/Home codes
- Display On/Off and blink bit
- Custom char programming (CGRAM/UDF specifics)
- Any device‑specific brightness/dimming mapping

Implemented IVFDHAL Methods
- List the relevant methods and one‑line semantics.

Unbuffered Usage (IVFDHAL via VFDDisplay)
```cpp
#include "VFDDisplay.h"
#include "HAL/<YourHAL>.h"
#include "Transports/<Recommended>.h"

<YourHAL> hal;
<RecommendedTransport> tx(...);
VFDDisplay vfd(&hal, &tx);

void setup() {
  vfd.init();
  vfd.clear();
  vfd.writeAt(0,0, "Hello");
}
void loop() {}
```

Buffered Usage (BufferedVFD)
```cpp
#include "Buffered/BufferedVFD.h"

BufferedVFD buf(&hal);
void setup(){ vfd.init(); buf.init(); buf.writeAt(0,0,"Buffered"); buf.flush(); }
```

Device‑Specific Methods (non‑IVFDHAL)
- Document any additional methods (or state) provided by the HAL.

Tests
- Contract tests included (IVFDHALContractTests)
- Device‑specific tests (list main cases: init, clear/home, addressing, brightness, CGRAM)

Known Limitations
- Enumerate anything not supported (e.g., readback, exact blink rate control).

Revision History
- Date – short description.

