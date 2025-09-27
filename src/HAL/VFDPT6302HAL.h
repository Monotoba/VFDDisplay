#pragma once
#include "IVFDHAL.h"
#include "Transports/ITransport.h"
#include "../Capabilities/IDisplayCapabilities.h"
#include "../Capabilities/DisplayCapabilities.h"
#include <Arduino.h>

// VFDPT6302HAL: HAL for Princeton PT6302 VFD Controller/Driver (16-digit, 5x7 chars)
// Implements PT6302 serial command set: DCRAM (character) write, ADRAM (symbols),
// CGRAM (user chars), display duty, number-of-digits, and light control.
class VFDPT6302HAL : public IVFDHAL {
public:
    VFDPT6302HAL();
    ~VFDPT6302HAL() override = default;

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

private:
    // ===== NO_TOUCH: PT6302 serial primitives =====
    bool _cmdDisplayDuty(uint8_t dutyIdx);          // 0..7 -> 8/16..15/16
    bool _cmdNumberOfDigits(uint8_t digits);        // 9..16
    bool _cmdAllLights(uint8_t L, uint8_t H);       // 0/1 bits -> normal/off/on
    bool _cmdDCRAMAddr(uint8_t addr4);              // 0..15, header 0x10 | addr
    bool _writeByte(uint8_t b);
    bool _writeData(const uint8_t* p, size_t n);
    bool _dcramWriteChars(uint8_t addr, const uint8_t* data, size_t n);
    // ===== NO_TOUCH END =====

    // Local state for cursor emulation
    uint8_t _row = 0;
    uint8_t _col = 0;

    ITransport* _transport = nullptr;
    DisplayCapabilities* _capabilities = nullptr;
    VFDError _lastError = VFDError::Ok;
};

