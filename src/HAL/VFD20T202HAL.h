#pragma once
#include "IVFDHAL.h"
#include "Transports/ITransport.h"
#include "../Capabilities/IDisplayCapabilities.h"
#include "../Capabilities/DisplayCapabilities.h"
#include <Arduino.h>

// VFD20T202HAL: HAL for Futaba 20T202 family (20x2 VFD modules).
// NOTE: Initial implementation mirrors the 20S401 ESC addressing model
// (ESC 'H' + linear address) and common single-byte commands where applicable.
// Review and adjust command bytes against the 20T202 datasheet(s).
class VFD20T202HAL : public IVFDHAL {
public:
    VFD20T202HAL();
    ~VFD20T202HAL() override = default;

    // Transport injection
    void setTransport(ITransport* transport) override { _transport = transport; }

    // Lifecycle
    bool init() override;
    bool reset() override;

    // Screen control
    bool clear() override;
    bool cursorHome() override;
    bool setCursorPos(uint8_t row, uint8_t col) override;
    bool setCursorBlinkRate(uint8_t rate_ms) override;

    // Enhanced positioning helpers
    bool writeCharAt(uint8_t row, uint8_t column, char c) override;
    bool writeAt(uint8_t row, uint8_t column, const char* text) override;
    bool moveTo(uint8_t row, uint8_t column) override;

    // Cursor movement convenience
    bool backSpace() override;
    bool hTab() override;
    bool lineFeed() override;
    bool carriageReturn() override;

    // Writing
    bool writeChar(char c) override;
    bool write(const char* msg) override;
    bool centerText(const char* str, uint8_t row) override;
    bool writeCustomChar(uint8_t index) override;
    bool getCustomCharCode(uint8_t index, uint8_t& codeOut) const override;

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

    // Scrolling
    bool hScroll(const char* str, int dir, uint8_t row) override;
    bool vScroll(const char* str, int dir) override;
    bool vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) override;
    bool starWarsScroll(const char* text, uint8_t startRow) override;

    // Flash text
    bool flashText(const char* str, uint8_t row, uint8_t col,
                   uint8_t on_ms, uint8_t off_ms) override;

    // Capabilities and diagnostics
    int getCapabilities() const override;
    const char* getDeviceName() const override;
    const IDisplayCapabilities* getDisplayCapabilities() const override { return _capabilities; }

    // Timing utility
    void delayMicroseconds(unsigned int us) const override { ::delayMicroseconds(us); }

    // Error reporting
    VFDError lastError() const override { return _lastError; }
    void clearError() override { _lastError = VFDError::Ok; }

    // ===== NO_TOUCH: Device-specific primitives =====
    // Validate and adjust bytes per 20T202 datasheet before modifying.
    // HD44780-like primitives (verify against 20T202 datasheet)
    bool _cmdInit();                 // Function set, display on, clear, entry mode
    bool _escReset();                // Soft re-init sequence
    bool _cmdClear();                // 0x01
    bool _cmdHome();                 // 0x02
    bool _posLinear(uint8_t addr);   // Set DDRAM: 0x80 | addr
    bool _posRowCol(uint8_t row, uint8_t col);
    bool _escMode(uint8_t mode);     // NotSupported (return false)
    bool _escDimming(uint8_t level); // NotSupported (return false)
    bool _escCursorBlink(uint8_t rate); // Map to display on/off blink bit
    // ===== NO_TOUCH END =====

private:
    ITransport* _transport = nullptr;
    DisplayCapabilities* _capabilities = nullptr;
    VFDError _lastError = VFDError::Ok;

    // h/v scroll state (minimal reuse)
    int16_t _hScrollOffset = 0;
    uint8_t _hScrollRow = 0;
    char _hScrollText[80] = {0};

    // ===== NO_TOUCH: Bus write helpers =====
    bool _writeCmd(uint8_t cmd);
    bool _writeData(const uint8_t* data, size_t len);

    // Function-set composition (brightness + lines)
    bool _writeFunctionSet(uint8_t brightnessIndex);
    bool _twoLine = true;
    uint8_t _brightnessIndex = 0; // 0:100%, 1:75%, 2:50%, 3:25%
};
