#pragma once
#include "IVFDHAL.h"
#include "Transports/ITransport.h"
#include "../Capabilities/IDisplayCapabilities.h"
#include "../Capabilities/DisplayCapabilities.h"
#include <Arduino.h>

// VFDCU40026HAL: HAL for Noritake CU40026 (40x2) VFD modules with controller commands
// using single-byte control codes and ESC sequences per datasheet DS-1028.
class VFDCU40026HAL : public IVFDHAL {
public:
    VFDCU40026HAL();
    ~VFDCU40026HAL() override = default;

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

    // ESC helper (null-terminated variant per interface)
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

    // Device-specific helpers (not in IVFDHAL)
    bool setLuminanceBand(uint8_t code);         // ESC 'L' + code (00..FF)
    bool setLuminanceIndex(uint8_t idx0to3);     // 0:25%,1:50%,2:75%,3:100%
    bool setBlinkPeriodMs(uint16_t periodMs);    // ESC 'T' + (period/30ms, 1..255)
    bool selectFlickerlessMode();                // ESC 'S' (per datasheet)

private:
    // ===== NO_TOUCH: Device-specific primitives =====
    bool _escInit();                   // ESC 'I'
    bool _cmdClear();                  // 0x0E
    bool _cmdHomeTopLeft();            // 0x0C (FF)
    bool _posLinear(uint8_t addr);     // ESC 'H' + addr (0x00..0x4F for 40x2)
    bool _posRowCol(uint8_t row, uint8_t col);
    bool _escLuminance(uint8_t code);  // ESC 'L' + code (00..FF mapped 25..100%)
    bool _escBlinkPeriod(uint8_t data);// ESC 'T' + data (period=data*~30ms)
    bool _escUDF(uint8_t chr, const uint8_t rows5[5]); // ESC 'C' + CHR + PT1..PT5
    bool _writeCmd(uint8_t b);         // raw single byte command
    bool _writeData(const uint8_t* p, size_t n);
    // ===== NO_TOUCH END =====

    ITransport* _transport = nullptr;
    DisplayCapabilities* _capabilities = nullptr;
    VFDError _lastError = VFDError::Ok;
};
