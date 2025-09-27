# VFD20S401HAL Implementation Documentation
See also
- 20T202 devices: VFD20T202HAL implements a similar IVFDHAL surface for 20x2 modules.
## Overview

The `VFD20S401HAL` class implements the `IVFDHAL` interface for the Futaba VFD20S401 vacuum fluorescent display controller. This 4x20 character display controller supports serial communication and provides a rich set of features including custom characters, scrolling, and various display modes.

## Class Definition

```cpp
class VFD20S401HAL : public IVFDHAL {
public:
    VFD20S401HAL();
    ~VFD20S401HAL() override = default;
    
    // IVFDHAL interface implementation
    void setTransport(ITransport* transport) override;
    bool init() override;
    bool reset() override;
    bool clear() override;
    bool cursorHome() override;
    bool setCursorPos(uint8_t row, uint8_t col) override;
    bool setCursorBlinkRate(uint8_t rate_ms) override;
    bool writeCharAt(uint8_t row, uint8_t column, char c) override;
    bool writeAt(uint8_t row, uint8_t column, const char* text) override;
    bool moveTo(uint8_t row, uint8_t column) override;
    bool backSpace() override;
    bool hTab() override;
    bool lineFeed() override;
    bool carriageReturn() override;
    bool writeChar(char c) override;
    bool write(const char* msg) override;
    bool centerText(const char* str, uint8_t row) override;
    bool setBrightness(uint8_t lumens) override;
    bool saveCustomChar(uint8_t index, const uint8_t* pattern) override;
    bool setDisplayMode(uint8_t mode) override;
    bool setDimming(uint8_t level) override;
    bool cursorBlinkSpeed(uint8_t rate) override;
    bool changeCharSet(uint8_t setId) override;
    bool sendEscapeSequence(const uint8_t* data) override;
    bool hScroll(const char* str, int dir, uint8_t row) override;
    bool vScroll(const char* str, int dir) override;
    bool vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) override;
    bool starWarsScroll(const char* text, uint8_t startRow) override;
    bool flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms) override;
    int getCapabilities() const override;
    const char* getDeviceName() const override;
    const IDisplayCapabilities* getDisplayCapabilities() const override;
    void delayMicroseconds(unsigned int us) const override;
    
    // Additional utility methods
    bool sendEscSequence(const uint8_t* data, size_t len);
    uint8_t countLines(const char* text);
    void centerTextLine(const char* line, char* output, uint8_t maxLen);
    bool formatStarWarsText(const char* input, char* output, size_t outputSize);

private:
    ITransport* _transport;
    DisplayCapabilities* _capabilities;
    
    // Scrolling state tracking
    int16_t _vScrollOffset;
    char _vScrollText[256];
    uint8_t _vScrollTotalLines;
    uint8_t _vScrollStartRow;
};
```

## VFD20S401 Controller Commands

### Basic Commands
- **Clear Display**: `0x09`
- **Cursor Home**: `0x0C`
- **Initialize**: `0x49`
- **Reset**: ESC + `0x49`

### Cursor Positioning (ESC 'H' + Linear Address)
The VFD20S401 uses an ESC-based positioning command:

- Send `ESC` (0x1B), `'H'` (0x48) and then a single linear address byte 0x00–0x4F
- For a 4x20 layout, compute address as `row * 20 + col` (0-based row/col)

### Escape Sequences
Escape sequences start with `0x1B` (ESC) followed by a fixed number of data bytes (no terminator):
- **Display Mode**: ESC + mode byte (0x11-0x17)
- **Dimming Control**: ESC + `0x4C` + level
- **Cursor Blink**: ESC + `0x42` + rate

### Character Sets
- **CT0 (Standard)**: `0x18`
- **CT1 (Extended)**: `0x19`

## Method Implementation Details

### Constructor

```cpp
VFD20S401HAL::VFD20S401HAL() : _transport(nullptr) {
    _capabilities = CapabilitiesRegistry::createVFD20S401Capabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
}
```

**Description:** Initializes the HAL and registers VFD20S401 capabilities with the global registry.

### Transport Management

#### void setTransport(ITransport* transport)

```cpp
void VFD20S401HAL::setTransport(ITransport* transport) {
    _transport = transport;
}
```

**Description:** Stores the transport pointer for communication operations.

### Lifecycle Methods

#### bool init()

```cpp
bool VFD20S401HAL::init() {
    if (!_transport) return false;
    
    uint8_t cmd = 0x49; // Initialize command
    _transport->write(&cmd, 1);
    return true;
}
```

**Description:** Sends the initialization command to the VFD20S401 controller.

#### bool reset()

```cpp
bool VFD20S401HAL::reset() {
    const uint8_t resetData[] = { 0x49 };
    return sendEscSequence(resetData, sizeof(resetData));
}
```

**Description:** Sends the reset escape sequence to restore default settings.

### Screen Control

#### bool clear()

```cpp
bool VFD20S401HAL::clear() {
    if (!_transport) return false;
    uint8_t cmd = 0x09; // Clear display command
    _transport->write(&cmd, 1);
    return true;
}
```

**Description:** Clears the display screen and returns cursor to home position.

#### bool cursorHome()

```cpp
bool VFD20S401HAL::cursorHome() {
    if (!_transport) return false;
    
    uint8_t cmd = 0x0C; // Cursor home command
    _transport->write(&cmd, 1);
    return true;
}
```

**Description:** Moves the cursor to the home position (top-left corner).

#### bool setCursorPos(uint8_t row, uint8_t col)

```cpp
bool VFD20S401HAL::setCursorPos(uint8_t row, uint8_t col) {
    if (row >= 4 || col >= 20) return false; // Validate 4x20 bounds
    const uint8_t addr = (uint8_t)(row * 20 + col); // 0x00..0x4F
    const uint8_t escData[] = { 0x48 /* 'H' */, addr };
    return sendEscSequence(escData, sizeof(escData));
}
```

**Description:** Sets cursor position using DDRAM addressing for the 4x20 display layout.

### Cursor Movement Methods

#### bool backSpace()

```cpp
bool VFD20S401HAL::backSpace() {
    return writeChar(0x08); // ASCII backspace
}
```

#### bool hTab()

```cpp
bool VFD20S401HAL::hTab() {
    return writeChar(0x09); // ASCII horizontal tab
}
```

#### bool lineFeed()

```cpp
bool VFD20S401HAL::lineFeed() {
    return writeChar(0x0A); // ASCII line feed
}
```

#### bool carriageReturn()

```cpp
bool VFD20S401HAL::carriageReturn() {
    return writeChar(0x0D); // ASCII carriage return
}
```

### Writing Methods

#### bool writeChar(char c)

```cpp
bool VFD20S401HAL::writeChar(char c) {
    if (!_transport) return false;
    return _transport->write(reinterpret_cast<const uint8_t*>(&c), 1);
}
```

**Description:** Writes a single character to the display at the current cursor position.

#### bool write(const char* msg)

```cpp
bool VFD20S401HAL::write(const char* msg) {
    if (!_transport || !msg) return false;
    size_t len = strlen(msg);
    return _transport->write(reinterpret_cast<const uint8_t*>(msg), len);
}
```

**Description:** Writes a null-terminated string to the display.

#### bool centerText(const char* str, uint8_t row)

```cpp
bool VFD20S401HAL::centerText(const char* str, uint8_t row) {
    if (!_transport || !str || !_capabilities) return false;
    
    uint8_t textColumns = _capabilities->getTextColumns();
    uint8_t textRows = _capabilities->getTextRows();
    
    if (row >= textRows) return false;
    
    size_t strLen = strlen(str);
    if (strLen >= textColumns) {
        return setCursorPos(row, 0) && write(str);
    }
    
    uint8_t totalPadding = textColumns - strLen;
    uint8_t leftPadding = totalPadding / 2;
    uint8_t rightPadding = totalPadding - leftPadding;
    
    if (!setCursorPos(row, 0)) return false;
    
    // Write left padding
    for (uint8_t i = 0; i < leftPadding; i++) {
        if (!writeChar(' ')) return false;
    }
    
    if (!write(str)) return false;
    
    // Write right padding
    for (uint8_t i = 0; i < rightPadding; i++) {
        if (!writeChar(' ')) return false;
    }
    
    return true;
}
```

**Description:** Centers text horizontally on the specified row with proper padding.

### Display Features

#### bool setDisplayMode(uint8_t mode)

```cpp
bool VFD20S401HAL::setDisplayMode(uint8_t mode) {
    if (mode < 0x11 || mode > 0x17) return false;
    
    uint8_t escData[] = {mode, 0x00};
    return sendEscapeSequence(escData);
}
```

**Description:** Sets display mode using escape sequences (valid range: 0x11-0x17).

#### bool setDimming(uint8_t level)

```cpp
bool VFD20S401HAL::setDimming(uint8_t level) {
    const uint8_t dimmingData[] = { 0x4C, level }; // ESC + 0x4C for dimming
    return sendEscSequence(dimmingData, sizeof(dimmingData));
}
```

**Description:** Controls display dimming using the dimming escape sequence.

#### bool cursorBlinkSpeed(uint8_t rate)

```cpp
bool VFD20S401HAL::cursorBlinkSpeed(uint8_t rate) {
    const uint8_t blinkData[] = { 0x42, rate }; // ESC + cursor control
    return sendEscSequence(blinkData, sizeof(blinkData));
}
```

**Description:** Sets cursor blink speed (0 = no blink, 1-255 = blink rates).

#### bool changeCharSet(uint8_t setId)

```cpp
bool VFD20S401HAL::changeCharSet(uint8_t setId) {
    if (setId == 0) {
        return writeChar(0x18); // CT0 - Standard ASCII
    } else if (setId == 1) {
        return writeChar(0x19); // CT1 - Extended character set
    }
    return false;
}
```

**Description:** Switches between character sets (0 = CT0 standard, 1 = CT1 extended).

#### bool saveCustomChar(uint8_t index, const uint8_t* pattern)

Implements User Definable Font (UDF) per datasheet section 5.2.16 [1]. Packs a 5x7 pattern from an 8x5 row format into 5 bytes and sends ESC 'C' + CHR + PT1..PT5.

```cpp
bool VFD20S401HAL::saveCustomChar(uint8_t index, const uint8_t* pattern) {
    return setCustomChar(index, pattern);
}

bool VFD20S401HAL::setCustomChar(uint8_t index, const uint8_t* pattern) {
    if (!_capabilities || !_transport || !pattern) return false;
    if (!_capabilities->hasCapability(CAP_USER_DEFINED_CHARS)) return false;
    if (index >= _capabilities->getMaxUserDefinedCharacters()) return false;

    uint8_t bytes[5];
    _pack5x7ToBytes(pattern, bytes);
    const uint8_t data[] = { 0x43, index, bytes[0], bytes[1], bytes[2], bytes[3], bytes[4] };
    return sendEscSequence(data, sizeof(data));
}
```

Pattern format expected by the API is 8 rows with 5 bits per row (bits 0..4, LSB=leftmost). Only the top 7 rows are used by the device (5x7 characters).

Index-to-code mapping (16 UDFs):
- Indices 0–7 map to codes 0x00–0x07 (legacy-compatible).
- Indices 8–15 map to codes 0x80–0x87 to avoid collisions with control codes.
Use `writeCustomChar(index)` to display a custom character by its logical index; on this device, indices 0–7 can also be shown by `writeChar(index)`.

### Escape Sequence Handling

#### bool sendEscapeSequence(const uint8_t* data)

```cpp
bool VFD20S401HAL::sendEscapeSequence(const uint8_t* data) {
    if (!_transport || !data) return false;
    
    uint8_t esc = 0x1B;
    if (!_transport->write(&esc, 1)) return false;
    
    uint8_t byteCount = 0;
    while (byteCount < 8 && data[byteCount] != 0) {
        if (!_transport->write(&data[byteCount], 1)) return false;
        byteCount++;
    }
    
    return true;
}
```

**Description:** Sends escape sequences with ESC (0x1B) prefix followed by data bytes. For sequences that may contain 0x00 in parameters, prefer the length-aware `sendEscSequence(data, len)`.

### Enhanced Positioning

#### bool writeCharAt(uint8_t row, uint8_t column, char c)

```cpp
bool VFD20S401HAL::writeCharAt(uint8_t row, uint8_t column, char c) {
    if (!moveTo(row, column)) return false;
    return writeChar(c);
}
```

**Description:** Moves to the specified position and writes a single character.

#### bool writeAt(uint8_t row, uint8_t column, const char* text)

```cpp
bool VFD20S401HAL::writeAt(uint8_t row, uint8_t column, const char* text) {
    if (!text) return false;
    if (!moveTo(row, column)) return false;
    return write(text);
}
```

**Description:** Writes text at a specific position by moving the cursor and sending the string.

#### bool moveTo(uint8_t row, uint8_t column)

```cpp
bool VFD20S401HAL::moveTo(uint8_t row, uint8_t column) {
    return setCursorPos(row, column);
}
```

**Description:** Moves cursor to specified position without writing (uses ESC 'H' + linear address).

### Scrolling Implementation

#### bool vScrollText(const char* text, uint8_t startRow, ScrollDirection direction)

```cpp
bool VFD20S401HAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) {
    if (!_transport || !text || !_capabilities) return false;
    
    uint8_t textRows = _capabilities->getTextRows();
    if (startRow >= textRows) return false;
    
    // Store new text if different from current
    if (strcmp(text, _vScrollText) != 0) {
        size_t textLen = strlen(text);
        if (textLen >= sizeof(_vScrollText) - 1) textLen = sizeof(_vScrollText) - 1;
        memcpy(_vScrollText, text, textLen);
        _vScrollText[textLen] = '\0';
        _vScrollOffset = 0;
        _vScrollStartRow = startRow;
        
        // Count total lines
        _vScrollTotalLines = 1;
        for (size_t i = 0; i < textLen; i++) {
            if (_vScrollText[i] == '\n') _vScrollTotalLines++;
        }
    }
    
    // Update scroll offset based on direction
    if (direction == SCROLL_DOWN) {
        _vScrollOffset++;
        if (_vScrollOffset >= _vScrollTotalLines) _vScrollOffset = 0;
    } else if (direction == SCROLL_UP) {
        _vScrollOffset--;
        if (_vScrollOffset < 0) _vScrollOffset = _vScrollTotalLines - 1;
    } else {
        return false;
    }
    
    // Display visible portion
    uint8_t visibleRows = textRows - startRow;
    if (visibleRows == 0) return false;
    
    for (uint8_t row = 0; row < visibleRows; row++) {
        uint8_t displayLine = (_vScrollOffset + row) % _vScrollTotalLines;
        
        // Find line start
        const char* displayLineStart = _vScrollText;
        uint8_t lineCount = 0;
        while (lineCount < displayLine && *displayLineStart) {
            if (*displayLineStart == '\n') lineCount++;
            displayLineStart++;
        }
        
        // Display the line
        uint8_t writeRow = startRow + row;
        for (uint8_t col = 0; col < 20; col++) {
            char c = ' ';
            if (col < (lineEnd - displayLineStart) && *displayLineStart != '\n') {
                c = displayLineStart[col];
            }
            char buf[2] = {c, '\0'};
            if (!writeAt(writeRow, col, buf)) return false;
        }
    }
    
    return true;
}
```

**Description:** Implements vertical text scrolling with state tracking and multi-line support. Call repeatedly with `SCROLL_UP`/`SCROLL_DOWN` to animate.

#### bool vScroll(const char* str, int dir)

Convenience wrapper that scrolls starting at row 0:

```cpp
// Scroll up one line
vfd->vScroll("Line1\nLine2\nLine3", -1);
```

#### bool hScroll(const char* str, int dir, uint8_t row)

Scrolls a single line of text horizontally across a row; maintains internal offset. Call repeatedly:

```cpp
// Scroll left on row 1
vfd->hScroll("HELLO WORLD", +1, 1);
```

#### bool starWarsScroll(const char* text, uint8_t startRow)

```cpp
bool VFD20S401HAL::starWarsScroll(const char* text, uint8_t startRow) {
    if (!text || !_capabilities) return false;
    
    uint8_t textRows = _capabilities->getTextRows();
    if (startRow >= textRows) return false;
    
    char centeredText[256];
    if (!formatStarWarsText(text, centeredText, sizeof(centeredText))) {
        return false;
    }
    
    return vScrollText(centeredText, startRow, SCROLL_UP);
}
```

**Description:** Creates Star Wars-style opening crawl by centering text and scrolling upward.

### Utility Methods

#### bool formatStarWarsText(const char* input, char* output, size_t outputSize)

```cpp
bool VFD20S401HAL::formatStarWarsText(const char* input, char* output, size_t outputSize) {
    if (!input || !output || outputSize == 0) return false;
    
    output[0] = '\0';
    size_t outputPos = 0;
    
    const char* lineStart = input;
    const char* lineEnd = input;
    
    while (*lineStart != '\0' && outputPos < outputSize - 1) {
        // Find end of current line
        lineEnd = lineStart;
        while (*lineEnd != '\0' && *lineEnd != '\n') {
            lineEnd++;
        }
        
        // Extract this line
        uint8_t lineLen = lineEnd - lineStart;
        char line[64];
        if (lineLen >= sizeof(line)) lineLen = sizeof(line) - 1;
        memcpy(line, lineStart, lineLen);
        line[lineLen] = '\0';
        
        // Center this line
        char centeredLine[64];
        centerTextLine(line, centeredLine, sizeof(centeredLine));
        
        // Add to output
        if (outputPos > 0) {
            if (outputPos < outputSize - 1) {
                output[outputPos++] = '\n';
            }
        }
        
        size_t centeredLen = strlen(centeredLine);
        if (outputPos + centeredLen < outputSize - 1) {
            strcpy(output + outputPos, centeredLine);
            outputPos += centeredLen;
        }
        
        // Move to next line
        if (*lineEnd == '\n') lineEnd++;
        lineStart = lineEnd;
    }
    
    output[outputPos] = '\0';
    return true;
}
```

**Description:** Formats text for Star Wars scroll by centering each line.

#### void centerTextLine(const char* line, char* output, uint8_t maxLen)

```cpp
void VFD20S401HAL::centerTextLine(const char* line, char* output, uint8_t maxLen) {
    if (!line || !output || maxLen == 0) return;
    
    uint8_t lineLen = strlen(line);
    uint8_t textColumns = _capabilities->getTextColumns();
    
    if (lineLen >= textColumns) {
        strcpy(output, line);
        return;
    }
    
    uint8_t totalPadding = textColumns - lineLen;
    uint8_t leftPadding = totalPadding / 2;
    uint8_t rightPadding = totalPadding - leftPadding;
    
    uint8_t pos = 0;
    
    // Left padding
    for (uint8_t i = 0; i < leftPadding && pos < maxLen - 1; i++) {
        output[pos++] = ' ';
    }
    
    // Original text
    for (uint8_t i = 0; i < lineLen && pos < maxLen - 1; i++) {
        output[pos++] = line[i];
    }
    
    // Right padding
    for (uint8_t i = 0; i < rightPadding && pos < maxLen - 1; i++) {
        output[pos++] = ' ';
    }
    
    output[pos] = '\0';
}
```

**Description:** Centers a single line of text with appropriate padding.

## Display Specifications

### Physical Characteristics
- **Display Type**: Vacuum Fluorescent Display (VFD)
- **Character Format**: 5x8 dot matrix
- **Display Size**: 4 rows × 20 characters
- **Character Size**: 3.5mm × 6.0mm (typical)
- **Overall Dimensions**: 116mm × 32mm

### Electrical Specifications
- **Supply Voltage**: +5V DC
- **Communication**: Serial (TTL level)
- **Baud Rate**: 19200 bps (default)
- **Data Format**: 8 data bits, no parity, 2 stop bits (8N2)
- **Power Consumption**: 800mW typical, 1500mW maximum

### Timing Specifications
- **Command Delay**: 10μs minimum, 100μs maximum
- **Reset Delay**: 100ms
- **Clear Display Time**: 1.64ms maximum

## Unbuffered Usage (IVFDHAL via VFDDisplay)

```cpp
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

// Create VFD system
VFD20S401HAL* hal = new VFD20S401HAL();
SerialTransport* transport = new SerialTransport(&Serial1);
VFDDisplay* vfd = new VFDDisplay(hal, transport);

void setup() {
    // Initialize serial communication
    Serial1.begin(19200, SERIAL_8N2);
    
    // Initialize VFD
    if (!vfd->init()) {
        Serial.println("VFD initialization failed!");
        return;
    }
    
    // Clear display
    vfd->clear();
    
    // Write centered title
    vfd->centerText("VFD20S401 Display", 0);
    
    // Set display mode
    vfd->setDisplayMode(0x11); // Normal mode
    
    // Create Star Wars scroll effect
    const char* story = "A long time ago\nin a galaxy far,\nfar away...";
    vfd->starWarsScroll(story, 2);
}

void loop() {
    // Your application code
}
```

### Method-by-Method Examples (Unbuffered)

Lifecycle
- init/reset
```cpp
VFDDisplay vfd(new VFD20S401HAL(), new SerialTransport(&Serial1));
Serial1.begin(19200, SERIAL_8N2);
vfd.init();
vfd.reset();
```

Screen control
```cpp
vfd.clear();
vfd.cursorHome();
vfd.setCursorPos(2, 5);
```

Writing
```cpp
vfd.write("Hello");
vfd.writeChar('!');
vfd.writeAt(3, 0, "Bottom row");
vfd.centerText("Centered", 1);
```

Cursor movement
```cpp
vfd.backSpace();
vfd.hTab();
vfd.lineFeed();
vfd.carriageReturn();
```

Custom characters
```cpp
uint8_t smile[8] = {0x00,0x0A,0x00,0x11,0x0E,0x00,0x0A,0x00};
vfd.setCustomChar(0, smile);
vfd.writeCustomChar(0);
```

Display features
```cpp
vfd.setDisplayMode(0x11);     // Mode per datasheet
vfd.setDimming(0x80);         // Example level
vfd.cursorBlinkSpeed(3);      // Device-specific rate
vfd.changeCharSet(0);         // CT0
```

Scrolling and effects
```cpp
vfd.hScroll("SCROLL", +1, 0);
vfd.vScroll("Line1\nLine2\nLine3", +1);
vfd.vScrollText("A\nB\nC", 0, SCROLL_DOWN);
vfd.starWarsScroll("A long time ago...", 2);
vfd.flashText("ALERT", 0, 10, 200, 200);
```

Escape sequences
```cpp
const uint8_t esc[] = { 0x4C, 0x80, 0x00 }; // DIM to example level
vfd.sendEscapeSequence(esc);
```

## Buffered Usage (BufferedVFD)

```cpp
#include "Buffered/BufferedVFD.h"

VFD20S401HAL hal;
SerialTransport tx(&Serial1);
VFDDisplay vfd(&hal, &tx);
BufferedVFD buf(&hal);

void setup() {
  Serial1.begin(19200, SERIAL_8N2);
  vfd.init();
  buf.init();

  buf.clearBuffer();
  buf.writeAt(0, 0, "Hello");
  buf.centerText(1, "Centered");
  buf.flush();

  // Diff flush after updating small region
  buf.writeAt(0, 6, "!");
  buf.flushDiff();

  // Non-blocking animations (call step from loop)
  buf.hScrollBegin(0, "MARQUEE TEXT", 100);
}

void loop() {
  static uint32_t t=0; uint32_t now=millis();
  buf.hScrollStep(now);
  if (now - t > 50) { buf.flushDiff(); t = now; }
}
```

## Error Handling

The VFD20S401HAL implementation includes comprehensive error checking:

- **Parameter Validation**: All methods validate input parameters
- **Bounds Checking**: Row/column positions are validated against 4x20 display limits
- **Transport Validation**: Transport pointer is checked before use
- **Return Values**: Methods return `false` on any error condition

## Performance Characteristics

- **Command Processing**: Single byte commands execute in ~10-100μs
- **Text Writing**: Character writing is limited by serial baud rate (19200 bps)
- **Scrolling**: Software-based scrolling with state tracking
- **Memory Usage**: ~256 bytes for scroll text buffer

## Limitations

- **Brightness Control**: setBrightness() not implemented

## See Also

- [IVFDHAL Interface](IVFDHAL.md)
- [VFDDisplay Class](VFDDisplay.md)
- [ITransport Interface](ITransport.md)
- [IDisplayCapabilities Interface](IDisplayCapabilities.md)
