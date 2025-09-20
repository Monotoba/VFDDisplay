#include "VFD20S401HAL.h"
#include "../Transports/ITransport.h"
#include "../Capabilities/CapabilitiesRegistry.h"
#include <Arduino.h>
#include <string.h>

// Constructor
VFD20S401HAL::VFD20S401HAL() : _transport(nullptr) {
    // Create and register capabilities
    _capabilities = CapabilitiesRegistry::createVFD20S401Capabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
}


// ==========================================================
// ==================== Required Methods ====================
// ==========================================================

// --- Transport injection ---
void VFD20S401HAL::setTransport(ITransport* transport) {
    _transport = transport;
}

// --- Lifecycle ---
bool VFD20S401HAL::init() {
    if (!_transport) return false;
    
    uint8_t cmd = 0x49;
    _transport->write(&cmd, 1);
    return true;
}

bool VFD20S401HAL::reset() {
    // Send escape sequence for reset: ESC (0x1B) followed by 0x49
    uint8_t resetData[] = {0x49, 0x00}; // 0x49 is the reset command
    return sendEscapeSequence(resetData);
}

// --- Screen control ---
bool VFD20S401HAL::clear() {
    if (!_transport) return false;
    
    uint8_t cmd = 0x09;
    _transport->write(&cmd, 1);
    return true;
}

bool VFD20S401HAL::cursorHome() {
    if (!_transport) return false;
    
    uint8_t cmd = 0x0C;
    _transport->write(&cmd, 1);
    return true;
}

bool VFD20S401HAL::setCursorPos(uint8_t row, uint8_t col) {
    // TODO: set DDRAM address based on row/col
    (void)row; (void)col;
    return false;
}

bool VFD20S401HAL::setCursorBlinkRate(uint8_t rate_ms) {
    // TODO: implement blink-rate command
    (void)rate_ms;
    return false;
}

// --- Writing ---
bool VFD20S401HAL::writeChar(char c) {
    if (!_transport) return false;
    return _transport->write(reinterpret_cast<const uint8_t*>(&c), 1);
}

bool VFD20S401HAL::write(const char* msg) {
    if (!_transport || !msg) return false;
    size_t len = strlen(msg);
    return _transport->write(reinterpret_cast<const uint8_t*>(msg), len);
}

bool VFD20S401HAL::centerText(const char* str, uint8_t row) {
    if (!_transport || !str || !_capabilities) return false;
    
    // Get display dimensions from capabilities
    uint8_t textColumns = _capabilities->getTextColumns();
    uint8_t textRows = _capabilities->getTextRows();
    
    // Validate row parameter
    if (row >= textRows) return false;
    
    // Calculate string length
    size_t strLen = strlen(str);
    
    // If string is longer than display width, just write it normally
    if (strLen >= textColumns) {
        return setCursorPos(row, 0) && write(str);
    }
    
    // Calculate padding for centering
    uint8_t totalPadding = textColumns - strLen;
    uint8_t leftPadding = totalPadding / 2;
    uint8_t rightPadding = totalPadding - leftPadding;
    
    // Set cursor to beginning of specified row
    if (!setCursorPos(row, 0)) return false;
    
    // Write left padding (spaces)
    for (uint8_t i = 0; i < leftPadding; i++) {
        if (!writeChar(' ')) return false;
    }
    
    // Write the centered text
    if (!write(str)) return false;
    
    // Write right padding (spaces) - optional, for completeness
    for (uint8_t i = 0; i < rightPadding; i++) {
        if (!writeChar(' ')) return false;
    }
    
    return true;
}

// --- Features ---
bool VFD20S401HAL::setBrightness(uint8_t lumens) {
    // TODO: map lumens -> brightness command
    (void)lumens;
    return false;
}

bool VFD20S401HAL::saveCustomChar(uint8_t index, const uint8_t* pattern) {
    // TODO: implement custom char save
    (void)index; (void)pattern;
    return false;
}

bool VFD20S401HAL::sendEscapeSequence(const uint8_t* data) {
    if (!_transport || !data) return false;
    
    // Send ESC character (0x1B)
    uint8_t esc = 0x1B;
    if (!_transport->write(&esc, 1)) return false;
    
    // Send data bytes until we find a zero or reach 8 bytes
    uint8_t byteCount = 0;
    while (byteCount < 8 && data[byteCount] != 0) {
        if (!_transport->write(&data[byteCount], 1)) return false;
        byteCount++;
    }
    
    return true;
}

bool VFD20S401HAL::setDisplayMode(uint8_t mode) {
    // Validate mode range (0x11-0x17)
    if (mode < 0x11 || mode > 0x17) return false;
    
    // Create escape sequence: ESC followed by mode byte and terminator
    uint8_t escData[] = {mode, 0x00};
    
    return sendEscapeSequence(escData);
}

// --- Scrolling ---
bool VFD20S401HAL::hScroll(const char* str, int dir, uint8_t row) {
    // TODO: implement horizontal scroll
    (void)str; (void)dir; (void)row;
    return false;
}

bool VFD20S401HAL::vScroll(const char* str, int dir) {
    // TODO: implement vertical scroll
    (void)str; (void)dir;
    return false;
}

// --- Flash text ---
bool VFD20S401HAL::flashText(const char* str, uint8_t row, uint8_t col,
                             uint8_t on_ms, uint8_t off_ms) {
    // TODO: implement flash text
    (void)str; (void)row; (void)col; (void)on_ms; (void)off_ms;
    return false;
}

// --- Capabilities and diagnostics ---
int VFD20S401HAL::getCapabilities() const {
    // TODO: return capability flags (bitmask)
    return 0;
}

const char* VFD20S401HAL::getDeviceName() const {
    return "VFD20S401";
}

const IDisplayCapabilities* VFD20S401HAL::getDisplayCapabilities() const {
    return _capabilities;
}

// --- Timing utility ---
void VFD20S401HAL::delayMicroseconds(unsigned int us) const {
    // Forward to global Arduino delay if available.
    ::delayMicroseconds(us);
}

// ==========================================================
// ===================== Utility Methods ====================
// ==========================================================

// Custom command: send ESC (0x1B) followed by up to 8 bytes
bool VFD20S401HAL::sendEscSequence(const uint8_t* data, size_t len) {
    // check that the class transport is valid and input is sane
    if (!_transport || !data || len == 0 || len > 8) return false;

    uint8_t esc = 0x1B;

    // send the ESC byte
    if (!_transport->write(&esc, 1)) return false;

    // send the data bytes
    if (!_transport->write(data, len)) return false;

    return true;
}


