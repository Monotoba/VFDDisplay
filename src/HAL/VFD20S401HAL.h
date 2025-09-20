# pragma once
#include "IVFDHAL.h"
#include "Transports/ITransport.h"
#include "../Capabilities/IDisplayCapabilities.h"
#include "../Capabilities/DisplayCapabilities.h"
#include <Arduino.h>



// VFD20S401HAL: Skeleton HAL for the VFD20S401 controller.
// Implements the IVFDHAL interface with stub methods.
// Replace stub logic with actual command sequences for the controller.
class VFD20S401HAL : public IVFDHAL {
public:
    VFD20S401HAL();
    ~VFD20S401HAL() override = default;
    
    // ==========================================================
    // ==================== Required Methods ====================
    // ==========================================================

    // Transport injection
    void setTransport(ITransport* transport) override;

    // Lifecycle
    bool init() override;
    bool reset() override;

    // Screen control
    bool clear() override;
    bool cursorHome() override;
    bool setCursorPos(uint8_t row, uint8_t col) override;
    bool setCursorBlinkRate(uint8_t rate_ms) override;
    
    // Enhanced positioning methods for 4x20 display
    bool writeAt(uint8_t row, uint8_t column, char c) override;
    bool moveTo(uint8_t row, uint8_t column) override;
    
    // Cursor movement convenience methods (wrapper around writeChar)
    bool backSpace() override;
    bool hTab() override;
    bool lineFeed() override;
    bool carriageReturn() override;

    // Writing
    bool writeChar(char c) override;
    bool write(const char* msg) override;
    bool centerText(const char* str, uint8_t row) override;

    // Features
    bool setBrightness(uint8_t lumens) override;
    bool saveCustomChar(uint8_t index, const uint8_t* pattern) override;
    bool setDisplayMode(uint8_t mode) override;
    bool setDimming(uint8_t level) override;
    bool cursorBlinkSpeed(uint8_t rate) override;
    bool changeCharSet(uint8_t setId) override;
    
    // Escape sequence support
    bool sendEscapeSequence(const uint8_t* data) override;

    // Scrolling
    bool hScroll(const char* str, int dir, uint8_t row) override;
    bool vScroll(const char* str, int dir) override;
    
    // Enhanced scrolling with direction enum and non-blocking operation
    bool vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) override;

    // Flash text
    bool flashText(const char* str, uint8_t row, uint8_t col,
                   uint8_t on_ms, uint8_t off_ms) override;

    // Capabilities and diagnostics
    int getCapabilities() const override;
    const char* getDeviceName() const override;
    const IDisplayCapabilities* getDisplayCapabilities() const override;

    // Timing utility
    void delayMicroseconds(unsigned int us) const override;
    
    // =========================================================
    // ==================== Utility Methods ====================
    // =========================================================
    
    // Custom command: send ESC (0x1B) followed by up to 8 bytes
    bool sendEscSequence(const uint8_t* data, size_t len);


private:
    ITransport* _transport;
    DisplayCapabilities* _capabilities;
    
    // Scrolling state tracking
    int16_t _vScrollOffset;              // Current vertical scroll offset
    char _vScrollText[256];              // Buffer for scroll text (max 256 chars)
    uint8_t _vScrollTotalLines;          // Total lines in scroll text
    uint8_t _vScrollStartRow;            // Starting row for scrolling
};
