#pragma once
#include <Arduino.h>

// Forward declarations
class ITransport;
class IDisplayCapabilities;

// Scroll directions for text scrolling
enum ScrollDirection : uint8_t {
    SCROLL_UP = 0,
    SCROLL_DOWN = 1,
    SCROLL_LEFT = 2,
    SCROLL_RIGHT = 3
};

// Error codes for HAL operations
enum class VFDError : uint8_t {
    Ok = 0,
    NotSupported = 1,
    InvalidArgs = 2,
    TransportFail = 3,
    Timeout = 4,
    Unknown = 255
};


// IVFDHAL: Interface for all device-specific VFD controller HALs.
// Each controller chip implementation must implement this interface.
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
virtual bool setCursorMode(uint8_t mode) = 0;
virtual bool cursorHome() = 0;
virtual bool setCursorPos(uint8_t row, uint8_t col) = 0;
virtual bool setCursorBlinkRate(uint8_t rate_ms) = 0;

// Enhanced positioning methods for 4x20 display
virtual bool writeCharAt(uint8_t row, uint8_t column, char c) = 0;
virtual bool writeAt(uint8_t row, uint8_t column, const char* text) = 0;
virtual bool moveTo(uint8_t row, uint8_t column) = 0;

// Cursor movement convenience methods (wrapper around writeChar)
virtual bool backSpace() = 0;
virtual bool hTab() = 0;
virtual bool lineFeed() = 0;
virtual bool carriageReturn() = 0;


// Writing
virtual bool writeChar(char c) = 0;
virtual bool write(const char* msg) = 0;
virtual bool centerText(const char* str, uint8_t row) = 0;
// High-level: write previously-defined custom char by index (capability-aware mapping)
virtual bool writeCustomChar(uint8_t index) = 0;


// Features
virtual bool setBrightness(uint8_t lumens) = 0;
// Save a custom character in a device-specific slot/code.
// Pattern is 8 rows of 5-bit values (bits 0..4 used per row). Row 7 is ignored on 5x7 devices.
virtual bool saveCustomChar(uint8_t index, const uint8_t* pattern) = 0;

// New: Set a custom character (alias for saveCustomChar) with capability-aware validation.
// Implementations should validate CAP_USER_DEFINED_CHARS and index range via getDisplayCapabilities().
virtual bool setCustomChar(uint8_t index, const uint8_t* pattern) = 0;
virtual bool setDisplayMode(uint8_t mode) = 0;
virtual bool setDimming(uint8_t level) = 0;
virtual bool cursorBlinkSpeed(uint8_t rate) = 0;
virtual bool changeCharSet(uint8_t setId) = 0;

// Escape sequence support - send ESC (0x1B) followed by up to 8 data bytes
// Data bytes are sent until a zero (0x00) is encountered or 8 bytes are sent
virtual bool sendEscapeSequence(const uint8_t* data) = 0;


// Scrolling
virtual bool hScroll(const char* str, int dir, uint8_t row) = 0;
virtual bool vScroll(const char* str, int dir) = 0;

// Enhanced scrolling with direction enum and non-blocking operation
virtual bool vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) = 0;

// Star Wars style opening crawl - centered text scrolling from bottom to top
virtual bool starWarsScroll(const char* text, uint8_t startRow) = 0;


// Flash text
virtual bool flashText(const char* str, uint8_t row, uint8_t col,
uint8_t on_ms, uint8_t off_ms) = 0;


// Capabilities and diagnostics
virtual int getCapabilities() const = 0;
virtual const char* getDeviceName() const = 0;


// NEW: Display capabilities access
virtual const IDisplayCapabilities* getDisplayCapabilities() const = 0;


// Timing utility
    virtual void delayMicroseconds(unsigned int us) const = 0;

    // Error inspection (non-throwing): methods still return bool for compatibility.
    virtual VFDError lastError() const = 0;
    virtual void clearError() = 0;

    // Query the device-specific code used to render a logical custom char index.
    // Returns false if unsupported or index out of range.
    virtual bool getCustomCharCode(uint8_t index, uint8_t& codeOut) const = 0;
};
