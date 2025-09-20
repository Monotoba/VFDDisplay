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

    // Writing
    bool writeChar(char c) override;
    bool write(const char* msg) override;
    bool centerText(const char* str, uint8_t row) override;

    // Features
    bool setBrightness(uint8_t lumens) override;
    bool saveCustomChar(uint8_t index, const uint8_t* pattern) override;
    bool setDisplayMode(uint8_t mode) override;
    
    // Escape sequence support
    bool sendEscapeSequence(const uint8_t* data) override;

    // Scrolling
    bool hScroll(const char* str, int dir, uint8_t row) override;
    bool vScroll(const char* str, int dir) override;

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
};
