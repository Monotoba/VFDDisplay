#pragma once
#include "IVFDHAL.h"
#include "Transports/ITransport.h"
#include "../Capabilities/IDisplayCapabilities.h"
#include "../Capabilities/DisplayCapabilities.h"
#include <Arduino.h>

// VFDM202SD01HAL: HAL for Futaba M202SD01HA (20x2) VFD module
// Implements module-specific control codes and addressing per manual.
class VFDM202SD01HAL : public IVFDHAL {
public:
    VFDM202SD01HAL();
    ~VFDM202SD01HAL() override = default;

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
    // ===== NO_TOUCH: device-specific primitives =====
    bool _cmdInit();                   // Reset + defaults
    bool _cmdReset();                  // 0x1F
    bool _cmdClear();                  // 0x0D
    bool _cmdHomeTopLeft();            // 0x0C (per common, if applicable)
    bool _posLinear(uint8_t addr);     // 0x10 + addr (00..27)
    bool _posRowCol(uint8_t row, uint8_t col);
    bool _cmdBackSpace();              // 0x08
    bool _cmdHtab();                   // 0x09
    bool _cmdCR();                     // 0x0D behaves as clear? Use 0x0D only for clear; CR via 0x0D not present -> emulate
    bool _cmdDimming(uint8_t code);    // 0x04 + data (00,20,40,60,80,FF)
    bool _cmdCursorMode(uint8_t mode); // 0x17 + data (FF lighting, 88 blinking, 00 off)
    bool _writeByte(uint8_t b);
    bool _writeData(const uint8_t* p, size_t n);
    // ===== NO_TOUCH END =====

    ITransport* _transport = nullptr;
    DisplayCapabilities* _capabilities = nullptr;
    VFDError _lastError = VFDError::Ok;
};

