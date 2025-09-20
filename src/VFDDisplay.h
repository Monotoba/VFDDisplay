// File: src/VFDDisplay.h
#ifndef VFD_DISPLAY_H
#define VFD_DISPLAY_H

#include <Arduino.h>
#include "HAL/IVFDHAL.h"
#include "Transports/ITransport.h"
#include "Logger/ILogger.h"
#include "Capabilities/IDisplayCapabilities.h"

class VFDDisplay {
public:
    // Constructor takes raw pointers
    VFDDisplay(IVFDHAL* hal, ITransport* transport)
        : _hal(hal), _transport(transport), _logger(nullptr) {
        // Inject transport into HAL
        if (_hal && _transport) {
            _hal->setTransport(_transport);
        }
    }

    // Basic operations
    bool init() { return _hal && _transport && _hal->init(); }
    bool reset() { return _hal && _transport && _hal->reset(); }
    bool setCursorPos(uint8_t row, uint8_t column) { return _hal->setCursorPos(row, column); }
    bool setCursorBlinkRate(uint8_t rate_ms) { return _hal->setCursorBlinkRate(rate_ms); }
    bool clear() { return _hal->clear(); }
    bool cursorHome() { return _hal->cursorHome(); }
    bool writeChar(char c) { return _hal->writeChar(c); }
    bool write(const char* msg) { return _hal->write(msg); }
    bool centerText(const char* str, uint8_t row) { return _hal->centerText(str, row); }
    bool setBrightness(uint8_t lumens) { return _hal->setBrightness(lumens); }
    bool saveCustomChar(uint8_t index, const uint8_t* pattern) { return _hal->saveCustomChar(index, pattern); }
    bool sendEscapeSequence(const uint8_t* data) { return _hal->sendEscapeSequence(data); }
    bool hScroll(const char* str, int dir, uint8_t row) { return _hal->hScroll(str, dir, row); }
    bool vScroll(const char* str, int dir) { return _hal->vScroll(str, dir); }
    bool flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms) {
        return _hal->flashText(str, row, col, on_ms, off_ms);
    }

    int getCapabilities() { return _hal ? _hal->getCapabilities() : 0; }

    // NEW: Display capabilities access
    const IDisplayCapabilities* getDisplayCapabilities() const { 
        return _hal ? _hal->getDisplayCapabilities() : nullptr; 
    }
    
    // Convenience methods for common capability queries
    uint8_t getTextRows() const {
        const IDisplayCapabilities* caps = getDisplayCapabilities();
        return caps ? caps->getTextRows() : 0;
    }
    
    uint8_t getTextColumns() const {
        const IDisplayCapabilities* caps = getDisplayCapabilities();
        return caps ? caps->getTextColumns() : 0;
    }
    
    bool hasCapability(DisplayCapabilityFlag flag) const {
        const IDisplayCapabilities* caps = getDisplayCapabilities();
        return caps ? caps->hasCapability(flag) : false;
    }
    
    bool supportsCursor() const { return hasCapability(CAP_CURSOR); }
    bool supportsDimming() const { return hasCapability(CAP_DIMMING); }
    bool supportsCursorBlink() const { return hasCapability(CAP_CURSOR_BLINK); }
    bool supportsUserDefinedChars() const { return hasCapability(CAP_USER_DEFINED_CHARS); }
    bool supportsHorizontalScroll() const { return hasCapability(CAP_HORIZONTAL_SCROLL); }
    bool supportsVerticalScroll() const { return hasCapability(CAP_VERTICAL_SCROLL); }

    void attachLogger(ILogger* logger) {
        _logger = logger;
        if (_transport) _transport->attachLogger(logger);
    }

    void detachLogger() {
        _logger = nullptr;
        if (_transport) _transport->detachLogger();
    }

private:
    IVFDHAL* _hal;
    ITransport* _transport;
    ILogger* _logger;
};

#endif // VFD_DISPLAY_H

