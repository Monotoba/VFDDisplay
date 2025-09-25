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
    bool writeCharAt(uint8_t row, uint8_t column, char c) override;
    bool writeAt(uint8_t row, uint8_t column, const char* text) override;
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
    bool writeCustomChar(uint8_t index) override;

    // Features
    bool setBrightness(uint8_t lumens) override;
    bool saveCustomChar(uint8_t index, const uint8_t* pattern) override;
    bool setCustomChar(uint8_t index, const uint8_t* pattern) override;
    bool setDisplayMode(uint8_t mode) override;
    bool setDimming(uint8_t level) override;
    bool cursorBlinkSpeed(uint8_t rate) override;
    bool changeCharSet(uint8_t setId) override;
    
    // Escape sequence support
    bool sendEscapeSequence(const uint8_t* data) override;
    bool sendCmd(int8_t cmd, int8_t* data, int8_t lenData, bool escapeCmd=false);

    // Scrolling
    bool hScroll(const char* str, int dir, uint8_t row) override;
    bool vScroll(const char* str, int dir) override;
    
    // Enhanced scrolling with direction enum and non-blocking operation
    bool vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) override;
    
    // Star Wars style opening crawl - centered text scrolling from bottom to top
    bool starWarsScroll(const char* text, uint8_t startRow);
    
    // Helper methods for text processing
    uint8_t countLines(const char* text);
    void centerTextLine(const char* line, char* output, uint8_t maxLen);
    bool formatStarWarsText(const char* input, char* output, size_t outputSize);

    // Flash text
    bool flashText(const char* str, uint8_t row, uint8_t col,
                   uint8_t on_ms, uint8_t off_ms) override;

    // Capabilities and diagnostics
    int getCapabilities() const override;
    const char* getDeviceName() const override;
    const IDisplayCapabilities* getDisplayCapabilities() const override;

    // Timing utility
    void delayMicroseconds(unsigned int us) const override;

    // Error reporting
    VFDError lastError() const override { return _lastError; }
    void clearError() override { _lastError = VFDError::Ok; }
    
    // =========================================================
    // ==================== Utility Methods ====================
    // =========================================================
    
    // Custom command: send ESC (0x1B) followed by up to 8 bytes
    bool sendEscSequence(const uint8_t* data, size_t len);

    // Pack 5x7 from 8x5 row pattern (bits 0..4 per row) into 5 bytes per datasheet Table 12.1
    static void _pack5x7ToBytes(const uint8_t* rowPattern8x5, uint8_t out5[5]);

    // Validate CHR code safety for this controller (avoid single-byte commands & ESC)
    static bool _isUnsafeCHR(uint8_t chr);


private:
    // ===== NO_TOUCH: Device-specific primitives (single-responsibility helpers) =====
    // Do not edit/refactor methods declared below. These map directly to hardware bytes.
    bool _cmdInit();                       // 0x49
    bool _escReset();                      // ESC 'I'
    bool _cmdClear();                      // 0x09
    bool _cmdHome();                       // 0x0C
    bool _posLinear(uint8_t addr);         // ESC 'H' + addr (0x00..0x4F)
    bool _posRowCol(uint8_t row, uint8_t col); // computes linear and calls _posLinear
    bool _escMode(uint8_t mode);           // ESC + mode (0x11..0x17)
    bool _escDimming(uint8_t level);       // ESC 0x4C + level
    bool _escCursorBlink(uint8_t rate);    // ESC 0x42 + rate
    // ===== NO_TOUCH END =====

    ITransport* _transport;
    DisplayCapabilities* _capabilities;
    VFDError _lastError = VFDError::Ok;
    
    // Scrolling state tracking
    int16_t _vScrollOffset;              // Current vertical scroll offset
    char _vScrollText[256];              // Buffer for scroll text (max 256 chars)
    uint8_t _vScrollTotalLines;          // Total lines in scroll text
    uint8_t _vScrollStartRow;            // Starting row for scrolling

    // Horizontal scroll state
    int16_t _hScrollOffset = 0;          // Current horizontal scroll offset
    uint8_t _hScrollRow = 0;             // Target row for horizontal scroll
    char _hScrollText[160] = {0};        // Buffer for horizontal text (wrapped display)
};
