#pragma once
#include "IVFDHAL.h"
#include "Transports/ITransport.h"
#include "../Capabilities/IDisplayCapabilities.h"
#include "../Capabilities/DisplayCapabilities.h"
#include <Arduino.h>

// VFDCU20025HAL: HAL for Noritake CU20025ECPB-W1J (20x2, 5x7) character VFD
// Implements HD44780-like instruction set with a device-specific Brightness Set.
class VFDCU20025HAL : public IVFDHAL {
public:
    VFDCU20025HAL();
    ~VFDCU20025HAL() override = default;

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

    // Device-specific helpers
    bool setBrightnessIndex(uint8_t idx0to3); // maps to Brightness Set (00..03)

    // ===== NO_TOUCH: device primitives =====
    bool _cmdInit();                 // 0x38, 0x0C, 0x01, 0x06
    bool _cmdClear();                // 0x01
    bool _cmdHome();                 // 0x02
    bool _posLinear(uint8_t addr);   // 0x80 | addr
    bool _posRowCol(uint8_t row, uint8_t col);
    bool _displayControl(bool displayOn, bool cursorOn, bool blinkOn); // 0x08 | D<<2 | C<<1 | B
    bool _writeCGRAM(uint8_t startAddr, const uint8_t* rows8);
    bool _writeCmd(uint8_t cmd);
    bool _writeData(const uint8_t* data, size_t len);
    bool _brightnessSet(uint8_t idx); // data write 0x00..0x03 (per datasheet table)
    // ===== NO_TOUCH END =====

private:
    ITransport* _transport = nullptr;
    DisplayCapabilities* _capabilities = nullptr;
    VFDError _lastError = VFDError::Ok;
};
