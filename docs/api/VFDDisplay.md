# VFDDisplay Class Documentation

## Overview

The `VFDDisplay` class is the main entry point for the VFD Display library. It provides a unified, high-level interface for controlling Vacuum Fluorescent Displays (VFDs) through a layered architecture with Hardware Abstraction Layer (HAL) and transport abstraction.

## Class Definition

```cpp
class VFDDisplay {
public:
    VFDDisplay(IVFDHAL* hal, ITransport* transport);
    
    // Basic operations
    bool init();
    bool reset();
    bool clear();
    bool cursorHome();
    
    // Text operations
    bool write(const char* msg);
    bool writeChar(char c);
    bool writeAt(uint8_t row, uint8_t column, const char* text);
    bool writeCharAt(uint8_t row, uint8_t column, char c);
    bool centerText(const char* str, uint8_t row);
    
    // Cursor control
    bool setCursorPos(uint8_t row, uint8_t column);
    bool setCursorBlinkRate(uint8_t rate_ms);
    bool backSpace();
    bool hTab();
    bool lineFeed();
    bool carriageReturn();
    
    // Display features
    bool setBrightness(uint8_t lumens);
    bool setDimming(uint8_t level);
    bool setDisplayMode(uint8_t mode);
    bool setCursorBlinkRate(uint8_t rate_ms);
    bool cursorBlinkSpeed(uint8_t rate); // legacy/compat wrapper on some HALs
    bool changeCharSet(uint8_t setId);
    
    // Scrolling effects
    bool hScroll(const char* str, int dir, uint8_t row);
    bool vScroll(const char* str, int dir);
    bool vScrollText(const char* text, uint8_t startRow, ScrollDirection direction);
    bool starWarsScroll(const char* text, uint8_t startRow);
    
    // Custom characters
    bool saveCustomChar(uint8_t index, const uint8_t* pattern);
    
    // Special effects
    bool flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms);
    bool sendEscapeSequence(const uint8_t* data);
    
    // Capabilities
    int getCapabilities();
    const IDisplayCapabilities* getDisplayCapabilities() const;
    uint8_t getTextRows() const;
    uint8_t getTextColumns() const;
    bool hasCapability(DisplayCapabilityFlag flag) const;
    bool supportsCursor() const;
    bool supportsDimming() const;
    bool supportsCursorBlink() const;
    bool supportsUserDefinedChars() const;
    bool supportsHorizontalScroll() const;
    bool supportsVerticalScroll() const;
    
    // Logging
    void attachLogger(ILogger* logger);
    void detachLogger();
};
```

## Constructor

### VFDDisplay(IVFDHAL* hal, ITransport* transport)

Creates a new VFDDisplay instance with the specified hardware abstraction layer and transport.

**Parameters:**
- `hal` - Pointer to a HAL implementation (e.g., VFD20S401HAL)
- `transport` - Pointer to a transport implementation (e.g., SerialTransport)

**Example:**
```cpp
VFD20S401HAL* hal = new VFD20S401HAL();
SerialTransport* transport = new SerialTransport(&Serial1);
VFDDisplay* vfd = new VFDDisplay(hal, transport);
```

## Basic Operations

### bool init()

Initializes the VFD display. Must be called before any other operations.

**Returns:** `true` if initialization successful, `false` otherwise

**Example:**
```cpp
if (!vfd->init()) {
    Serial.println("VFD init failed!");
    return;
}
```

### bool reset()

Resets the VFD display to its default state.

**Returns:** `true` if reset successful, `false` otherwise

**Example:**
```cpp
vfd->reset();
```

### bool clear()

Clears the display screen.

**Returns:** `true` if clear successful, `false` otherwise

**Example:**
```cpp
vfd->clear();
```

### bool cursorHome()

Moves the cursor to the home position (top-left corner).

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
vfd->cursorHome();
```

## Text Operations

### bool write(const char* msg)

Writes a string to the display at the current cursor position.

**Parameters:**
- `msg` - Null-terminated string to write

**Returns:** `true` if write successful, `false` otherwise

**Example:**
```cpp
vfd->write("Hello, VFD!");
```

### bool writeChar(char c)

Writes a single character to the display at the current cursor position.

**Parameters:**
- `c` - Character to write

**Returns:** `true` if write successful, `false` otherwise

**Example:**
```cpp
vfd->writeChar('A');
```

### bool writeAt(uint8_t row, uint8_t column, const char* text)

Writes text at a specific position on the display.

**Parameters:**
- `row` - Row number (0-based)
- `column` - Column number (0-based)
- `text` - Null-terminated string to write

**Returns:** `true` if write successful, `false` otherwise

**Example:**
```cpp
vfd->writeAt(0, 0, "Top Left");
vfd->writeAt(1, 10, "Middle");
```

### bool writeCharAt(uint8_t row, uint8_t column, char c)

Writes a single character at a specific position on the display.

**Parameters:**
- `row` - Row number (0-based)
- `column` - Column number (0-based)
- `c` - Character to write

**Returns:** `true` if write successful, `false` otherwise

**Example:**
```cpp
vfd->writeCharAt(2, 5, 'X');
```

### bool centerText(const char* str, uint8_t row)

Centers text horizontally on the specified row.

**Parameters:**
- `str` - Text to center
- `row` - Row number (0-based)

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
vfd->centerText("Centered Title", 0);
```

## Cursor Control

### bool setCursorPos(uint8_t row, uint8_t column)

Sets the cursor position to the specified row and column.

**Parameters:**
- `row` - Row number (0-based)
- `column` - Column number (0-based)

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
vfd->setCursorPos(2, 5);
```

### bool setCursorBlinkRate(uint8_t rate_ms)

Sets the cursor blink rate.

**Parameters:**
- `rate_ms` - Blink rate in milliseconds

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
vfd->setCursorBlinkRate(500); // Blink every 500ms
```

### Cursor Movement Methods

#### bool backSpace()
Moves cursor one position to the left (ASCII backspace).

#### bool hTab()
Moves cursor to next horizontal tab stop (ASCII horizontal tab).

#### bool lineFeed()
Moves cursor to next line (ASCII line feed).

#### bool carriageReturn()
Moves cursor to beginning of current line (ASCII carriage return).

**Example:**
```cpp
vfd->backSpace();        // Move left
vfd->hTab();            // Tab right
vfd->lineFeed();        // Next line
vfd->carriageReturn();  // Start of line
```

## Display Features

### bool setBrightness(uint8_t lumens)

Sets the display brightness level.

**Parameters:**
- `lumens` - Brightness level (0-255)

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
vfd->setBrightness(128); // Medium brightness
```

### bool setDimming(uint8_t level)

Sets the display dimming level.

**Parameters:**
- `level` - Dimming level

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
vfd->setDimming(4); // Dimming level 4
```

### bool setDisplayMode(uint8_t mode)

Sets the display mode.

**Parameters:**
- `mode` - Display mode (VFD20S401: 0x11–0x13 affect display)

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
vfd->setDisplayMode(0x11); // Normal mode
```

### bool setCursorBlinkRate(uint8_t rate_ms)

Sets the cursor blink rate.

Notes (VFD20S401): Uses ESC 'T' + rate. 0x00 disables blink; non‑zero selects device‑defined period.

**Parameters:**
- `rate_ms` - Device‑specific blink control byte

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
vfd->setCursorMode(1);            // ensure cursor is visible (VFD20S401: DC5)
vfd->setCursorBlinkRate(0x20);    // enable blink
vfd->setCursorBlinkRate(0x00);    // disable blink
```

### bool changeCharSet(uint8_t setId)

Changes the character set.

**Parameters:**
- `setId` - Character set ID (0 = standard ASCII, 1 = extended/custom)

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
vfd->changeCharSet(1); // Switch to extended character set
```

## Scrolling Effects

### bool hScroll(const char* str, int dir, uint8_t row)

Performs horizontal scrolling of text on the specified row.

**Parameters:**
- `str` - Text to scroll
- `dir` - Scroll direction (negative = left, positive = right)
- `row` - Row number to scroll on

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
vfd->hScroll("Scrolling text...", -1, 0); // Scroll left on row 0
```

### bool vScroll(const char* str, int dir)

Performs vertical scrolling of text.

**Parameters:**
- `str` - Text to scroll
- `dir` - Scroll direction (negative = up, positive = down)

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
vfd->vScroll("Line 1\nLine 2\nLine 3", 1); // Scroll down
```

### bool vScrollText(const char* text, uint8_t startRow, ScrollDirection direction)

Performs enhanced vertical text scrolling with direction control.

**Parameters:**
- `text` - Multi-line text to scroll
- `startRow` - Starting row for scrolling
- `direction` - Scroll direction (`SCROLL_UP` or `SCROLL_DOWN`)

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
const char* story = "Line 1\nLine 2\nLine 3\nLine 4\nLine 5";
vfd->vScrollText(story, 0, SCROLL_UP);
```

### bool starWarsScroll(const char* text, uint8_t startRow)

Creates a Star Wars-style opening crawl effect with centered text scrolling from bottom to top.

**Parameters:**
- `text` - Multi-line text to scroll
- `startRow` - Starting row for the effect

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
const char* opening = "A long time ago\nin a galaxy far,\nfar away...";
vfd->starWarsScroll(opening, 0);
```

## Custom Characters

### bool saveCustomChar(uint8_t index, const uint8_t* pattern)

Saves a custom character pattern to the specified index. Alias of `setCustomChar()`.

Pattern format (row-major): pass one byte per row; bits 0..(W-1) encode columns left→right. The number of rows is defined by the device’s character height capability (e.g., 7 for 5x7, 8 for 5x8). Extra rows (if provided) are ignored by device HALs that don’t use them.

**Parameters:**
- `index` - Custom character index (0..N-1 where `N = getMaxUserDefinedCharacters()`).
- `pattern` - Pointer to `H` row bytes; `H = getCharacterPixelHeight()`.

**Returns:** `true` if operation successful, `false` otherwise

**Example (5x7/5x8):**
```cpp
uint8_t customPattern[8] = {
    0b00000, // Row 0
    0b01010, // Row 1
    0b11111, // Row 2
    0b01010, // Row 3
    0b01010, // Row 4
    0b11111, // Row 5
    0b01010, // Row 6
    0b00000  // Row 7 (ignored on 5x7 devices)
};
vfd->setCustomChar(0, customPattern);
vfd->writeCustomChar(0); // Display custom character (mapping handled by HAL)

// For indices 8..15 on devices that support 16 UDFs,
// use writeCustomChar(index) instead of writeChar(index) to avoid device-specific code mappings.
```

### bool setCustomChar(uint8_t index, const uint8_t* pattern)

Capability-aware method to define a custom character; validates support and index bounds using the display capabilities. See `saveCustomChar()` for usage and pattern format.

### bool writeCustomChar(uint8_t index)

Writes a previously-defined custom character by logical index. This abstracts any device-specific mapping between the logical index and the actual character code to output.

### bool getCustomCharCode(uint8_t index, uint8_t& codeOut) const

Returns the device-specific code corresponding to the logical custom character index. Useful if you need to embed the raw byte in a buffer or stream.

## Special Effects

### bool flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms)

Flashes text at the specified position.

**Parameters:**
- `str` - Text to flash
- `row` - Row position
- `col` - Column position
- `on_ms` - Time in milliseconds for text to be visible
- `off_ms` - Time in milliseconds for text to be hidden

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
vfd->flashText("ALERT!", 1, 5, 500, 250); // Flash every 750ms
```

### bool sendEscapeSequence(const uint8_t* data)

Sends an escape sequence to the display.

**Parameters:**
- `data` - Array of bytes to send after ESC (0x1B), terminated with 0x00

**Returns:** `true` if operation successful, `false` otherwise

**Example:**
```cpp
uint8_t escData[] = {0x42, 0x01, 0x00}; // Custom escape sequence
vfd->sendEscapeSequence(escData);
```

## Capabilities

### int getCapabilities()

Returns capability flags for the display.

**Returns:** Bitmask of capability flags

**Example:**
```cpp
int caps = vfd->getCapabilities();
if (caps & CAP_BRIGHTNESS_CONTROL) {
    Serial.println("Brightness control supported");
}
```

### const IDisplayCapabilities* getDisplayCapabilities() const

Returns the display capabilities object.

**Returns:** Pointer to IDisplayCapabilities interface

**Example:**
```cpp
const IDisplayCapabilities* caps = vfd->getDisplayCapabilities();
Serial.print("Display: ");
Serial.println(caps->getDeviceName());
```

### uint8_t getTextRows() const

Returns the number of text rows supported by the display.

**Returns:** Number of rows (e.g., 4 for 4x20 display)

**Example:**
```cpp
uint8_t rows = vfd->getTextRows(); // Returns 4 for VFD20S401
```

### uint8_t getTextColumns() const

Returns the number of text columns supported by the display.

**Returns:** Number of columns (e.g., 20 for 4x20 display)

**Example:**
```cpp
uint8_t cols = vfd->getTextColumns(); // Returns 20 for VFD20S401
```

### bool hasCapability(DisplayCapabilityFlag flag) const

Checks if the display supports a specific capability.

**Parameters:**
- `flag` - Capability flag to check

**Returns:** `true` if capability is supported, `false` otherwise

**Example:**
```cpp
if (vfd->hasCapability(CAP_VERTICAL_SCROLL)) {
    vfd->vScrollText("Scrolling text", 0, SCROLL_UP);
}
```

### Convenience Capability Methods

#### bool supportsCursor() const
Returns `true` if cursor control is supported.

#### bool supportsDimming() const
Returns `true` if dimming control is supported.

#### bool supportsCursorBlink() const
Returns `true` if cursor blinking is supported.

#### bool supportsUserDefinedChars() const
Returns `true` if custom characters are supported.

#### bool supportsHorizontalScroll() const
Returns `true` if horizontal scrolling is supported.

#### bool supportsVerticalScroll() const
Returns `true` if vertical scrolling is supported.

## Logging

### void attachLogger(ILogger* logger)

Attaches a logger for debugging transport operations.

**Parameters:**
- `logger` - Pointer to logger implementation

**Example:**
```cpp
SerialLogger* logger = new SerialLogger(&Serial);
vfd->attachLogger(logger);
```

### void detachLogger()

Detaches the current logger.

**Example:**
```cpp
vfd->detachLogger();
```

## Usage Example

```cpp
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

// Create VFD system
VFD20S401HAL* hal = new VFD20S401HAL();
SerialTransport* transport = new SerialTransport(&Serial1);
VFDDisplay* vfd = new VFDDisplay(hal, transport);

void setup() {
    // Initialize serial for VFD
    Serial1.begin(19200, SERIAL_8N2);
    
    // Initialize VFD
    if (!vfd->init()) {
        Serial.println("VFD init failed!");
        return;
    }
    
    // Clear display and write message
    vfd->clear();
    vfd->write("Hello, VFD World!");
    
    // Center text on row 1
    vfd->centerText("Centered Text", 1);
    
    // Create Star Wars scroll effect
    const char* story = "A long time ago\nin a galaxy far,\nfar away...";
    vfd->starWarsScroll(story, 2);
}

void loop() {
    // Your application code here
}
```

## Thread Safety

The VFDDisplay class is not thread-safe. All operations should be called from the same thread/context, typically the main Arduino loop.

## Error Handling

Most methods return `bool` to indicate success or failure. Always check return values for critical operations:

```cpp
if (!vfd->init()) {
    // Handle initialization failure
    return;
}

if (!vfd->write("Important message")) {
    // Handle write failure
}
```

## Dependencies

- `IVFDHAL` - Hardware abstraction layer interface
- `ITransport` - Transport layer interface
- `IDisplayCapabilities` - Display capabilities interface
- `Arduino.h` - Arduino framework

## See Also

- [IVFDHAL Interface](IVFDHAL.md)
- [VFD20S401HAL Implementation](VFD20S401HAL.md)
- [ITransport Interface](ITransport.md)
- [IDisplayCapabilities Interface](IDisplayCapabilities.md)
