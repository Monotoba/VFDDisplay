#pragma once
#include <Arduino.h>

// Forward declarations
class ITransport;
class IDisplayCapabilities;


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
virtual bool cursorHome() = 0;
virtual bool setCursorPos(uint8_t row, uint8_t col) = 0;
virtual bool setCursorBlinkRate(uint8_t rate_ms) = 0;


// Writing
virtual bool writeChar(char c) = 0;
virtual bool write(const char* msg) = 0;
virtual bool centerText(const char* str, uint8_t row) = 0;


// Features
virtual bool setBrightness(uint8_t lumens) = 0;
virtual bool saveCustomChar(uint8_t index, const uint8_t* pattern) = 0;

// Escape sequence support - send ESC (0x1B) followed by up to 8 data bytes
// Data bytes are sent until a zero (0x00) is encountered or 8 bytes are sent
virtual bool sendEscapeSequence(const uint8_t* data) = 0;


// Scrolling
virtual bool hScroll(const char* str, int dir, uint8_t row) = 0;
virtual bool vScroll(const char* str, int dir) = 0;


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
};
