# IVFDHAL Interface Documentation

## Overview

The `IVFDHAL` interface defines the contract that all VFD controller implementations must fulfill. It provides a standardized API for hardware-specific operations while abstracting the underlying controller details from the main VFDDisplay class.

## Interface Definition

```cpp
class IVFDHAL {
public:
    virtual ~IVFDHAL() = default;
    
    // Transport injection
    virtual void setTransport(ITransport* transport) = 0;
    
    // Lifecycle
    virtual bool init() = 0;
    virtual bool reset() = 0;
    
    // Screen control
    virtual bool clear() = 0;
    virtual bool cursorHome() = 0;
    virtual bool setCursorPos(uint8_t row, uint8_t col) = 0;
    virtual bool setCursorBlinkRate(uint8_t rate_ms) = 0;
    
    // Enhanced positioning methods
    virtual bool writeCharAt(uint8_t row, uint8_t column, char c) = 0;
    virtual bool writeAt(uint8_t row, uint8_t column, const char* text) = 0;
    virtual bool moveTo(uint8_t row, uint8_t column) = 0;
    
    // Cursor movement convenience methods
    virtual bool backSpace() = 0;
    virtual bool hTab() = 0;
    virtual bool lineFeed() = 0;
    virtual bool carriageReturn() = 0;
    
    // Writing
    virtual bool writeChar(char c) = 0;
    virtual bool write(const char* msg) = 0;
    virtual bool centerText(const char* str, uint8_t row) = 0;
    virtual bool writeCustomChar(uint8_t index) = 0; // write mapped UDF code for index
    virtual bool getCustomCharCode(uint8_t index, uint8_t& codeOut) const = 0; // query mapping
    
    // Features
    virtual bool setBrightness(uint8_t lumens) = 0;
    virtual bool saveCustomChar(uint8_t index, const uint8_t* pattern) = 0;
    virtual bool setDisplayMode(uint8_t mode) = 0;
    virtual bool setDimming(uint8_t level) = 0;
    virtual bool cursorBlinkSpeed(uint8_t rate) = 0;
    virtual bool changeCharSet(uint8_t setId) = 0;
    virtual bool setCustomChar(uint8_t index, const uint8_t* pattern) = 0; // alias; capability-aware
    
    // Escape sequence support
    virtual bool sendEscapeSequence(const uint8_t* data) = 0;
    
    // Scrolling
    virtual bool hScroll(const char* str, int dir, uint8_t row) = 0;
    virtual bool vScroll(const char* str, int dir) = 0;
    virtual bool vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) = 0;
    virtual bool starWarsScroll(const char* text, uint8_t startRow) = 0;
    
    // Flash text
    virtual bool flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms) = 0;
    
    // Capabilities and diagnostics
    virtual int getCapabilities() const = 0;
    virtual const char* getDeviceName() const = 0;
    virtual const IDisplayCapabilities* getDisplayCapabilities() const = 0;
    
    // Timing utility
    virtual void delayMicroseconds(unsigned int us) const = 0;
};
```

## Scroll Direction Enum

```cpp
enum ScrollDirection : uint8_t {
    SCROLL_UP = 0,
    SCROLL_DOWN = 1,
    SCROLL_LEFT = 2,
    SCROLL_RIGHT = 3
};
```

## Method Documentation

### Transport Management

#### void setTransport(ITransport* transport)

Injects the transport layer implementation that will handle low-level communication.

**Parameters:**
- `transport` - Pointer to transport implementation

**Implementation Notes:**
- Store the transport pointer for use in other methods
- Validate that transport is not null before use
- Transport is typically injected by the VFDDisplay constructor

**Example Implementation:**
```cpp
void MyVFDHAL::setTransport(ITransport* transport) {
    _transport = transport;
}
```

### Lifecycle Methods

#### bool init()

Initializes the VFD controller hardware.

**Returns:** `true` if initialization successful, `false` otherwise

**Implementation Notes:**
- Send initialization commands specific to the controller
- Set default display parameters
- Verify communication with the display
- Return false if transport is not available

**Example Implementation (VFD20S401):**
```cpp
bool VFD20S401HAL::init() {
    if (!_transport) return false;
    
    // Send initialization command
    uint8_t cmd = 0x49;
    return _transport->write(&cmd, 1);
}
```

#### bool reset()

Resets the VFD controller to its default state.

**Returns:** `true` if reset successful, `false` otherwise

**Implementation Notes:**
- Send hardware reset command or sequence
- Clear display and return cursor to home position
- Reset all display parameters to defaults

**Example Implementation (VFD20S401):**
```cpp
bool VFD20S401HAL::reset() {
    const uint8_t resetData[] = { 0x49 }; // ESC 'I'
    return sendEscSequence(resetData, sizeof(resetData));
}
```

### Screen Control Methods

#### bool clear()

Clears the display screen.

**Returns:** `true` if clear successful, `false` otherwise

**Implementation Notes:**
- Send display clear command
- May need to wait for command completion
- Update internal state if tracking cursor position

**Example Implementation (VFD20S401):**
```cpp
bool VFD20S401HAL::clear() {
    if (!_transport) return false;
    
    uint8_t cmd = 0x09; // Clear command
    return _transport->write(&cmd, 1);
}
```

#### bool cursorHome()

Moves cursor to the home position (typically top-left).

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- Send cursor home command
- Update internal cursor position tracking

**Example Implementation (VFD20S401):**
```cpp
bool VFD20S401HAL::cursorHome() {
    if (!_transport) return false;
    
    uint8_t cmd = 0x0C; // Home command
    return _transport->write(&cmd, 1);
}
```

#### bool setCursorPos(uint8_t row, uint8_t col)

Sets cursor to specific row and column position.

**Parameters:**
- `row` - Target row (0-based)
- `col` - Target column (0-based)

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- Validate row/column bounds for the specific display
- Compute linear address for 4x20 as `row*20 + col` (0..79)
- Send ESC 'H' followed by the linear address byte
- Update internal cursor tracking (if tracked)

**Example Implementation (VFD20S401):**
```cpp
bool VFD20S401HAL::setCursorPos(uint8_t row, uint8_t col) {
    if (row >= 4 || col >= 20) return false; // 4x20 display
    const uint8_t addr = (uint8_t)(row * 20 + col); // 0x00..0x4F
    const uint8_t escData[] = { 0x48, addr };
    return sendEscSequence(escData, sizeof(escData));
}
```

#### bool setCursorBlinkRate(uint8_t rate_ms)

Sets the cursor blink rate.

**Parameters:**
- `rate_ms` - Blink rate in milliseconds

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- May not be supported by all controllers
- Rate interpretation depends on controller

### Enhanced Positioning Methods

#### bool writeCharAt(uint8_t row, uint8_t column, char c)

Writes a single character at the specified position.

**Parameters:**
- `row` - Target row (0-based)
- `column` - Target column (0-based)
- `c` - Character to write

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- Typically implemented by moving cursor then writing character
- Must validate position bounds
- Should be atomic operation

#### bool writeAt(uint8_t row, uint8_t column, const char* text)

Writes text starting at the specified position.

**Parameters:**
- `row` - Starting row (0-based)
- `column` - Starting column (0-based)
- `text` - Null-terminated string to write

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- Move cursor to starting position
- Write each character, handling line wrapping if needed
- Validate that text fits within display bounds

#### bool moveTo(uint8_t row, uint8_t column)

Moves cursor to specified position without writing.

**Parameters:**
- `row` - Target row (0-based)
- `column` - Target column (0-based)

**Returns:** `true` if operation successful, `false` otherwise

### Cursor Movement Convenience Methods

These methods implement standard ASCII control character functionality:

#### bool backSpace()
Moves cursor one position left (ASCII 0x08).

#### bool hTab()
Moves cursor to next tab stop (ASCII 0x09).

#### bool lineFeed()
Moves cursor to next line (ASCII 0x0A).

#### bool carriageReturn()
Moves cursor to beginning of current line (ASCII 0x0D).

**Implementation Notes:**
- Typically implemented by writing the corresponding ASCII character
- May need special handling for display-specific behavior

### Writing Methods

#### bool writeChar(char c)

Writes a single character at the current cursor position.

**Parameters:**
- `c` - Character to write

**Returns:** `true` if write successful, `false` otherwise

**Implementation Notes:**
- Send character data to controller
- Handle special characters (control codes)
- Update cursor position automatically

#### bool write(const char* msg)

Writes a null-terminated string at the current cursor position.

**Parameters:**
- `msg` - String to write

**Returns:** `true` if write successful, `false` otherwise

**Implementation Notes:**
- Write each character in sequence
- Handle string length calculation
- Process special characters appropriately

#### bool centerText(const char* str, uint8_t row)

Centers text horizontally on the specified row.

**Parameters:**
- `str` - Text to center
- `row` - Row number (0-based)

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- Calculate text length
- Determine display width from capabilities
- Calculate left padding needed
- Write padding spaces, then text

**Example Implementation:**
```cpp
bool VFD20S401HAL::centerText(const char* str, uint8_t row) {
    if (!_transport || !str || !_capabilities) return false;
    
    uint8_t textColumns = _capabilities->getTextColumns();
    if (row >= _capabilities->getTextRows()) return false;
    
    size_t strLen = strlen(str);
    if (strLen >= textColumns) {
        return setCursorPos(row, 0) && write(str);
    }
    
    uint8_t totalPadding = textColumns - strLen;
    uint8_t leftPadding = totalPadding / 2;
    
    if (!setCursorPos(row, 0)) return false;
    
    // Write left padding
    for (uint8_t i = 0; i < leftPadding; i++) {
        if (!writeChar(' ')) return false;
    }
    
    return write(str);
}
```

### Feature Methods

#### bool setBrightness(uint8_t lumens)

Sets display brightness level.

**Parameters:**
- `lumens` - Brightness level (interpretation varies by controller)

**Returns:** `true` if operation successful, `false` otherwise

#### bool saveCustomChar(uint8_t index, const uint8_t* pattern)

Saves a custom character pattern.

**Parameters:**
- `index` - Character index (typically 0-7)
- `pattern` - Array of bytes representing character bitmap

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- Validate index range for the controller
- Send character generation command sequence
- Pattern format depends on controller (typically 5x8 or 5x10)

#### bool setCustomChar(uint8_t index, const uint8_t* pattern)

Alias for saveCustomChar with capability-aware validation and documentation.

Parameters:
- `index` - Custom character slot index (0..N-1 from capabilities)
- `pattern` - 8-byte array; bits 0..4 used per row; row 7 ignored on 5x7 devices

Returns: `true` on success, `false` otherwise

Implementation Notes:
- Implementations should verify `CAP_USER_DEFINED_CHARS` and use `getMaxUserDefinedCharacters()` from `getDisplayCapabilities()`.
- Controllers with 5x7 dot cells must pack 35 bits into device format before sending.

#### bool setDisplayMode(uint8_t mode)

Sets the display mode.

**Parameters:**
- `mode` - Mode identifier (controller-specific)

**Returns:** `true` if operation successful, `false` otherwise

#### bool setDimming(uint8_t level)

Sets display dimming level.

**Parameters:**
- `level` - Dimming level (controller-specific range)

**Returns:** `true` if operation successful, `false` otherwise

#### bool cursorBlinkSpeed(uint8_t rate)

Sets cursor blink speed.

**Parameters:**
- `rate` - Blink rate (0 = no blink, 1-255 = various rates)

**Returns:** `true` if operation successful, `false` otherwise

#### bool changeCharSet(uint8_t setId)

Changes the active character set.

**Parameters:**
- `setId` - Character set identifier (0 = standard, 1 = extended)

**Returns:** `true` if operation successful, `false` otherwise

### Escape Sequence Support

#### bool sendEscapeSequence(const uint8_t* data)

Sends an escape sequence to the controller.

**Parameters:**
- `data` - Array of bytes to send after ESC (0x1B), terminated with 0x00

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- Send ESC character (0x1B) first
- Send data bytes until 0x00 terminator or max length reached
- Many controllers support up to 8 data bytes

**Example Implementation:**
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

### Scrolling Methods

#### bool hScroll(const char* str, int dir, uint8_t row)

Performs horizontal scrolling on specified row.

**Parameters:**
- `str` - Text to scroll
- `dir` - Scroll direction (negative = left, positive = right)
- `row` - Row to scroll on

**Returns:** `true` if operation successful, `false` otherwise

#### bool vScroll(const char* str, int dir)

Performs vertical scrolling.

**Parameters:**
- `str` - Text to scroll
- `dir` - Scroll direction (negative = up, positive = down)

**Returns:** `true` if operation successful, `false` otherwise

#### bool vScrollText(const char* text, uint8_t startRow, ScrollDirection direction)

Enhanced vertical scrolling with direction control.

**Parameters:**
- `text` - Multi-line text to scroll
- `startRow` - Starting row for scrolling
- `direction` - Scroll direction (SCROLL_UP or SCROLL_DOWN)

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- Parse text into lines
- Track scroll offset between calls
- Display visible portion based on offset
- Handle line wrapping and display dimensions

#### bool starWarsScroll(const char* text, uint8_t startRow)

Creates Star Wars-style opening crawl effect.

**Parameters:**
- `text` - Multi-line text to scroll
- `startRow` - Starting row for effect

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- Center each line of text
- Use vScrollText with SCROLL_UP direction
- Create bottom-to-top scrolling effect

### Special Effects

#### bool flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms)

Flashes text at specified position.

**Parameters:**
- `str` - Text to flash
- `row` - Row position
- `col` - Column position
- `on_ms` - Time visible (milliseconds)
- `off_ms` - Time hidden (milliseconds)

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- This method may be blocking depending on implementation
- Consider using non-blocking approach with timers
- Write text, delay, clear, delay, repeat

### Capabilities and Diagnostics

#### int getCapabilities() const

Returns capability flags for the controller.

**Returns:** Bitmask of capability flags (controller-specific)

**Implementation Notes:**
- Return predefined capability flags for the controller
- Used by higher-level code to determine feature availability

#### const char* getDeviceName() const

Returns the device/controller name.

**Returns:** Static string identifying the controller

**Implementation Notes:**
- Return a descriptive name (e.g., "VFD20S401", "HD44780")
- String should be static/constant

#### const IDisplayCapabilities* getDisplayCapabilities() const

Returns detailed display capabilities object.

**Returns:** Pointer to capabilities interface

**Implementation Notes:**
- Return pointer to capabilities object for this controller
- Capabilities should include display dimensions, features, etc.

### Timing Utility

#### void delayMicroseconds(unsigned int us) const

Provides microsecond delay functionality.

**Parameters:**
- `us` - Microseconds to delay

**Implementation Notes:**
- Typically delegates to Arduino's delayMicroseconds()
- Can be overridden for custom timing requirements
- Used for controller-specific timing requirements

## Implementation Guidelines

### Error Handling
- Always validate input parameters
- Check transport availability before operations
- Return false for any failure condition
- Provide meaningful error handling where possible

### State Management
- Track cursor position if needed for optimization
- Maintain display state for efficient updates
- Handle display-specific addressing schemes

### Performance Considerations
- Minimize transport operations where possible
- Batch operations when feasible
- Use display-specific features for efficiency

### Thread Safety
- Implementations are not required to be thread-safe
- Assume single-threaded access from main loop
- Document any threading requirements

## Example Implementation Structure

```cpp
class MyVFDHAL : public IVFDHAL {
private:
    ITransport* _transport;
    IDisplayCapabilities* _capabilities;
    uint8_t _currentRow;
    uint8_t _currentCol;
    
public:
    MyVFDHAL() : _transport(nullptr), _currentRow(0), _currentCol(0) {
        _capabilities = createCapabilities();
    }
    
    // Implement all IVFDHAL methods...
    
private:
    IDisplayCapabilities* createCapabilities() {
        // Create and return capabilities object
    }
};
```

## See Also

- [VFDDisplay Class](VFDDisplay.md)
- [VFD20S401HAL Implementation](VFD20S401HAL.md)
- [ITransport Interface](ITransport.md)
- [IDisplayCapabilities Interface](IDisplayCapabilities.md)
#### bool writeCustomChar(uint8_t index)

Writes a previously-defined custom character by logical index. Implementations map the index to the correct device-specific code and write it using `writeChar()`.

Parameters:
- `index` - Logical custom character index (0..N-1 from capabilities)

Returns: `true` if operation successful, `false` otherwise

Implementation Notes:
- Validate CAP_USER_DEFINED_CHARS and index bounds.
- Map index→code for the controller (for VFD20S401 this is the identity mapping, with unsafe codes filtered).

#### bool getCustomCharCode(uint8_t index, uint8_t& codeOut) const

Returns the device-specific code that will render the given logical custom character index.

Parameters:
- `index` - Logical custom index (0..N-1)
- `codeOut` - Output parameter set to the device byte to write

Returns: `true` if mapping available, `false` otherwise

Implementation Notes:
- Same validation as `writeCustomChar()`.
- Should use the exact same mapping function as `setCustomChar()`/`writeCustomChar()` to guarantee consistency.
