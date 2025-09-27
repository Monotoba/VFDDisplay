#pragma once
#include "IVFDHAL.h"
#include "Transports/ITransport.h"
#include "../Capabilities/IDisplayCapabilities.h"
#include "../Capabilities/DisplayCapabilities.h"
#include <Arduino.h>

// VFDHT16514HAL: HAL for Holtek HT16514 VFD controller/driver (supports 16/20/24 x 2)
// Implements an HD44780-like instruction set with Function Set brightness bits and
// DDRAM/CGRAM addressing.
class VFDHT16514HAL : public IVFDHAL {
public:
    VFDHT16514HAL();
    ~VFDHT16514HAL() override = default;

    void setTransport(ITransport* transport) override { _transport = transport; }

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
    bool writeCustomChar(uint8_t index) override;
    bool getCustomCharCode(uint8_t index, uint8_t& codeOut) const override;

    bool setBrightness(uint8_t lumens) override;
    bool saveCustomChar(uint8_t index, const uint8_t* pattern) override;
    bool setCustomChar(uint8_t index, const uint8_t* pattern) override;
    bool setDisplayMode(uint8_t mode) override;
    bool setDimming(uint8_t level) override;
    bool cursorBlinkSpeed(uint8_t rate) override;
    bool changeCharSet(uint8_t setId) override;

    bool sendEscapeSequence(const uint8_t* data) override;

    bool hScroll(const char* str, int dir, uint8_t row) override;
    bool vScroll(const char* str, int dir) override;
    bool vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) override;
    bool starWarsScroll(const char* text, uint8_t startRow) override;

    bool flashText(const char* str, uint8_t row, uint8_t col,
                   uint8_t on_ms, uint8_t off_ms) override;

    int getCapabilities() const override;
    const char* getDeviceName() const override;
    const IDisplayCapabilities* getDisplayCapabilities() const override { return _capabilities; }

    void delayMicroseconds(unsigned int us) const override { ::delayMicroseconds(us); }

    VFDError lastError() const override { return _lastError; }
    void clearError() override { _lastError = VFDError::Ok; }

    // ===== NO_TOUCH: HD44780-like primitives =====
    bool _functionSet(uint8_t brightnessIndex);
    bool _cmdInit();                 // function set + display on + clear + entry mode
    bool _cmdClear();                // 0x01
    bool _cmdHome();                 // 0x02
    bool _posLinear(uint8_t addr);   // 0x80 | addr
    bool _posRowCol(uint8_t row, uint8_t col);
    bool _displayControl(bool displayOn, bool cursorOn, bool blinkOn); // 0x08 | D<<2 | C<<1 | B
    bool _writeCmd(uint8_t cmd);
    bool _writeData(const uint8_t* data, size_t len);
    // ===== NO_TOUCH END =====

private:
    ITransport* _transport = nullptr;
    DisplayCapabilities* _capabilities = nullptr;
    VFDError _lastError = VFDError::Ok;
    bool _twoLine = true;
    uint8_t _brightnessIndex = 0; // 0..3 => 100/75/50/25
};

