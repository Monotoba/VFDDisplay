#include "VFDVK20225HAL.h"
#include "../Capabilities/CapabilitiesRegistry.h"
#include <string.h>

static constexpr uint8_t VK_CMD_PREFIX = 254; // 0xFE

VFDVK20225HAL::VFDVK20225HAL() {
    _capabilities = CapabilitiesRegistry::createVFDVK20225Capabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
}

bool VFDVK20225HAL::init() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    // No mandatory init; assume defaults
    _lastError = VFDError::Ok; return true;
}

bool VFDVK20225HAL::reset() {
    // No documented simple reset; emulate by clearing
    return clear();
}

bool VFDVK20225HAL::clear() {
    bool ok = _cmd(88); // Clear Screen: FE 88
    _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDVK20225HAL::cursorHome() {
    // Use setCursorPos(0,0)
    return setCursorPos(0,0);
}

bool VFDVK20225HAL::setCursorPos(uint8_t row, uint8_t col) {
    if (!_capabilities) { _lastError=VFDError::InvalidArgs; return false; }
    if (row >= _capabilities->getTextRows() || col >= _capabilities->getTextColumns()) { _lastError=VFDError::InvalidArgs; return false; }
    // VK uses 1-based Column, Row
    uint8_t C = (uint8_t)(col + 1);
    uint8_t R = (uint8_t)(row + 1);
    bool ok = _cmd3(71, C, R); // FE 71 C R
    _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDVK20225HAL::setCursorBlinkRate(uint8_t rate_ms) {
    // Use block cursor on/off approximating blink state if non-zero; NotSupported for exact rate
    (void)rate_ms; _lastError=VFDError::NotSupported; return false;
}

bool VFDVK20225HAL::writeCharAt(uint8_t row, uint8_t column, char c) { return moveTo(row,column) && writeChar(c); }
bool VFDVK20225HAL::writeAt(uint8_t row, uint8_t column, const char* text) { return moveTo(row,column) && write(text); }
bool VFDVK20225HAL::moveTo(uint8_t row, uint8_t column) { return setCursorPos(row,column); }

bool VFDVK20225HAL::backSpace() { return writeChar(0x08); }
bool VFDVK20225HAL::hTab() { return writeChar(0x09); }
bool VFDVK20225HAL::lineFeed() { return writeChar(0x0A); }
bool VFDVK20225HAL::carriageReturn() { return writeChar(0x0D); }

bool VFDVK20225HAL::writeChar(char c) { if(!_transport) return false; return _transport->write(reinterpret_cast<const uint8_t*>(&c),1); }
bool VFDVK20225HAL::write(const char* msg) { if(!_transport||!msg){ _lastError=VFDError::InvalidArgs; return false;} return _transport->write(reinterpret_cast<const uint8_t*>(msg), strlen(msg)); }

bool VFDVK20225HAL::centerText(const char* str, uint8_t row) {
    if(!_capabilities||!str){ _lastError=VFDError::InvalidArgs; return false;} uint8_t cols=_capabilities->getTextColumns(); size_t len=strlen(str); if(len>cols) len=cols; uint8_t pad=(uint8_t)((cols-len)/2); if(!setCursorPos(row,0)) return false; for(uint8_t i=0;i<pad;++i) if(!writeChar(' ')) return false; return write(str);
}

bool VFDVK20225HAL::writeCustomChar(uint8_t index) { (void)index; _lastError=VFDError::NotSupported; return false; }
bool VFDVK20225HAL::getCustomCharCode(uint8_t index, uint8_t& codeOut) const { (void)index; (void)codeOut; return false; }

bool VFDVK20225HAL::setBrightness(uint8_t lumens) {
    // VK: Set VFD Brightness FE 89 Byte (0..255)
    bool ok = _cmd2(89, lumens);
    _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDVK20225HAL::saveCustomChar(uint8_t index, const uint8_t* pattern) { (void)index;(void)pattern; _lastError=VFDError::NotSupported; return false; }
bool VFDVK20225HAL::setCustomChar(uint8_t index, const uint8_t* pattern) { (void)index;(void)pattern; _lastError=VFDError::NotSupported; return false; }
bool VFDVK20225HAL::setDisplayMode(uint8_t mode) { (void)mode; _lastError=VFDError::NotSupported; return false; }
bool VFDVK20225HAL::setDimming(uint8_t level) { return setBrightness((uint8_t)((level & 0xFF))); }
bool VFDVK20225HAL::cursorBlinkSpeed(uint8_t rate) { return setCursorBlinkRate(rate); }
bool VFDVK20225HAL::changeCharSet(uint8_t setId) { (void)setId; _lastError=VFDError::NotSupported; return false; }

bool VFDVK20225HAL::sendEscapeSequence(const uint8_t* data) { (void)data; _lastError=VFDError::NotSupported; return false; }
bool VFDVK20225HAL::hScroll(const char* str, int dir, uint8_t row){ (void)str;(void)dir;(void)row; _lastError=VFDError::NotSupported; return false; }
bool VFDVK20225HAL::vScroll(const char* str, int dir){ (void)str;(void)dir; _lastError=VFDError::NotSupported; return false; }
bool VFDVK20225HAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction){ (void)text;(void)startRow;(void)direction; _lastError=VFDError::NotSupported; return false; }
bool VFDVK20225HAL::starWarsScroll(const char* text, uint8_t startRow){ (void)text;(void)startRow; _lastError=VFDError::NotSupported; return false; }

bool VFDVK20225HAL::flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms){ (void)str;(void)row;(void)col;(void)on_ms;(void)off_ms; _lastError=VFDError::NotSupported; return false; }

int VFDVK20225HAL::getCapabilities() const { return _capabilities?_capabilities->getAllCapabilities():0; }
const char* VFDVK20225HAL::getDeviceName() const { return _capabilities?_capabilities->getDeviceName():"VK202-25"; }

// ===== NO_TOUCH: VK command helpers =====
bool VFDVK20225HAL::_cmd(uint8_t code) {
    if(!_transport) return false; uint8_t p[2]={VK_CMD_PREFIX, code}; return _transport->write(p,2);
}
bool VFDVK20225HAL::_cmd2(uint8_t code, uint8_t a) {
    if(!_transport) return false; uint8_t p[3]={VK_CMD_PREFIX, code, a}; return _transport->write(p,3);
}
bool VFDVK20225HAL::_cmd3(uint8_t code, uint8_t a, uint8_t b) {
    if(!_transport) return false; uint8_t p[4]={VK_CMD_PREFIX, code, a, b}; return _transport->write(p,4);
}

