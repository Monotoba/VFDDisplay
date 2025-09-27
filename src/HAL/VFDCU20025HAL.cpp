#include "VFDCU20025HAL.h"
#include "../Capabilities/CapabilitiesRegistry.h"
#include <string.h>

VFDCU20025HAL::VFDCU20025HAL() {
    _capabilities = CapabilitiesRegistry::createVFDCU20025Capabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
}

bool VFDCU20025HAL::init() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    bool ok = _cmdInit();
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFDCU20025HAL::reset() { return init(); }

bool VFDCU20025HAL::clear() { bool ok=_cmdClear(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDCU20025HAL::cursorHome() { bool ok=_cmdHome(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }

bool VFDCU20025HAL::setCursorPos(uint8_t row, uint8_t col) {
    if (!_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    if (row >= _capabilities->getTextRows() || col >= _capabilities->getTextColumns()) { _lastError = VFDError::InvalidArgs; return false; }
    bool ok = _posRowCol(row, col);
    _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDCU20025HAL::setCursorBlinkRate(uint8_t rate_ms) {
    bool ok = _displayControl(true, false, rate_ms!=0);
    _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDCU20025HAL::writeCharAt(uint8_t row, uint8_t column, char c) { return moveTo(row,column) && writeChar(c); }
bool VFDCU20025HAL::writeAt(uint8_t row, uint8_t column, const char* text) { return moveTo(row,column) && write(text); }
bool VFDCU20025HAL::moveTo(uint8_t row, uint8_t column) { return _posRowCol(row,column); }

bool VFDCU20025HAL::backSpace() { return writeChar(0x08); }
bool VFDCU20025HAL::hTab() { return writeChar(0x09); }
bool VFDCU20025HAL::lineFeed() { return writeChar(0x0A); }
bool VFDCU20025HAL::carriageReturn() { return writeChar(0x0D); }

bool VFDCU20025HAL::writeChar(char c) {
    if (!_transport) return false;
    // RS=1 for data on parallel-like transports
    if (_transport->supportsControlLines()) (void)_transport->setControlLine("RS", true);
    return _transport->write(reinterpret_cast<const uint8_t*>(&c), 1);
}

bool VFDCU20025HAL::write(const char* msg) {
    if (!_transport || !msg) { _lastError = VFDError::InvalidArgs; return false; }
    size_t len = strlen(msg);
    if (_transport->supportsControlLines()) (void)_transport->setControlLine("RS", true);
    bool ok = _transport->write(reinterpret_cast<const uint8_t*>(msg), len);
    _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDCU20025HAL::centerText(const char* str, uint8_t row) {
    if (!_capabilities || !str) { _lastError = VFDError::InvalidArgs; return false; }
    uint8_t cols = _capabilities->getTextColumns();
    size_t len = strlen(str); if (len>cols) len=cols;
    uint8_t pad = (uint8_t)((cols - len)/2);
    if (!setCursorPos(row,0)) return false;
    for (uint8_t i=0;i<pad;++i) { if (!writeChar(' ')) return false; }
    return write(str);
}

bool VFDCU20025HAL::writeCustomChar(uint8_t index) {
    uint8_t code=0; if (!getCustomCharCode(index, code)) { _lastError = VFDError::InvalidArgs; return false; }
    return writeChar((char)code);
}

bool VFDCU20025HAL::getCustomCharCode(uint8_t index, uint8_t& codeOut) const {
    if (!_capabilities) return false;
    if (index >= _capabilities->getMaxUserDefinedCharacters()) return false;
    codeOut = index; return true;
}

bool VFDCU20025HAL::setBrightness(uint8_t lumens) {
    // Map 0..255 to 4 levels 0..3 per datasheet (Brightness Set 00h..03h with RS=H)
    uint8_t idx = (lumens < 64) ? 3 : (lumens < 128) ? 2 : (lumens < 192) ? 1 : 0;
    bool ok = _brightnessSet(idx);
    _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDCU20025HAL::saveCustomChar(uint8_t index, const uint8_t* pattern) { return setCustomChar(index, pattern); }

bool VFDCU20025HAL::setCustomChar(uint8_t index, const uint8_t* pattern) {
    if (!_capabilities || !_transport || !pattern) { _lastError = VFDError::InvalidArgs; return false; }
    if (index >= _capabilities->getMaxUserDefinedCharacters()) { _lastError = VFDError::InvalidArgs; return false; }
    uint8_t addr = (uint8_t)((index & 0x07) * 8);
    if (!_writeCmd((uint8_t)(0x40 | (addr & 0x3F)))) { _lastError = VFDError::TransportFail; return false; }
    for (uint8_t r=0;r<8;++r) { uint8_t row = pattern[r] & 0x1F; if (!_writeData(&row,1)) { _lastError = VFDError::TransportFail; return false; } }
    _lastError = VFDError::Ok; return true;
}

bool VFDCU20025HAL::setDisplayMode(uint8_t mode) { (void)mode; _lastError=VFDError::NotSupported; return false; }

bool VFDCU20025HAL::setDimming(uint8_t level) {
    uint8_t idx = (uint8_t)(level & 0x03); bool ok = _brightnessSet(idx);
    _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDCU20025HAL::cursorBlinkSpeed(uint8_t rate) { return setCursorBlinkRate(rate); }

bool VFDCU20025HAL::changeCharSet(uint8_t setId) { (void)setId; _lastError=VFDError::NotSupported; return false; }

bool VFDCU20025HAL::sendEscapeSequence(const uint8_t* data) { (void)data; _lastError=VFDError::NotSupported; return false; }

bool VFDCU20025HAL::hScroll(const char* str, int dir, uint8_t row) {
    // Simple writeAt window; for brevity, reuse 20T202 style if needed later
    (void)str; (void)dir; (void)row; _lastError=VFDError::NotSupported; return false;
}
bool VFDCU20025HAL::vScroll(const char* str, int dir) { (void)str; (void)dir; _lastError=VFDError::NotSupported; return false; }
bool VFDCU20025HAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) { (void)text;(void)startRow;(void)direction; _lastError=VFDError::NotSupported; return false; }
bool VFDCU20025HAL::starWarsScroll(const char* text, uint8_t startRow) { (void)text;(void)startRow; _lastError=VFDError::NotSupported; return false; }

bool VFDCU20025HAL::flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms) { (void)str;(void)row;(void)col;(void)on_ms;(void)off_ms; _lastError=VFDError::NotSupported; return false; }

int VFDCU20025HAL::getCapabilities() const { return _capabilities ? _capabilities->getAllCapabilities() : 0; }
const char* VFDCU20025HAL::getDeviceName() const { return _capabilities ? _capabilities->getDeviceName() : "VFDCU20025"; }

// ===== NO_TOUCH primitives =====
bool VFDCU20025HAL::_cmdInit() {
    uint8_t seq[] = { 0x38, 0x0C, 0x01, 0x06 };
    for (uint8_t b : seq) { if (!_writeCmd(b)) return false; }
    return true;
}

bool VFDCU20025HAL::_cmdClear() { return _writeCmd(0x01); }
bool VFDCU20025HAL::_cmdHome() { return _writeCmd(0x02); }

bool VFDCU20025HAL::_posLinear(uint8_t addr) { return _writeCmd((uint8_t)(0x80 | (addr & 0x7F))); }

bool VFDCU20025HAL::_posRowCol(uint8_t row, uint8_t col) {
    const uint8_t base[] = { 0x00, 0x40 };
    if (row >= 2) return false;
    return _posLinear((uint8_t)(base[row] + col));
}

bool VFDCU20025HAL::_displayControl(bool displayOn, bool cursorOn, bool blinkOn) {
    uint8_t cmd = 0x08 | (displayOn ? 0x04 : 0x00) | (cursorOn ? 0x02 : 0x00) | (blinkOn ? 0x01 : 0x00);
    return _writeCmd(cmd);
}

bool VFDCU20025HAL::_writeCGRAM(uint8_t startAddr, const uint8_t* rows8) {
    if (!_writeCmd((uint8_t)(0x40 | (startAddr & 0x3F)))) return false;
    for (uint8_t i=0;i<8;++i) { uint8_t row = rows8[i] & 0x1F; if (!_writeData(&row,1)) return false; }
    return true;
}

bool VFDCU20025HAL::_writeCmd(uint8_t cmd) {
    if (!_transport) return false;
    if (_transport->supportsControlLines()) (void)_transport->setControlLine("RS", false);
    return _transport->write(&cmd, 1);
}

bool VFDCU20025HAL::_writeData(const uint8_t* data, size_t len) {
    if (!_transport || !data || len==0) return false;
    if (_transport->supportsControlLines()) (void)_transport->setControlLine("RS", true);
    return _transport->write(data, len);
}

bool VFDCU20025HAL::_brightnessSet(uint8_t idx) {
    // Datasheet table: Brightness Set uses data writes 00h..03h; send as data with RS=H
    uint8_t val = (uint8_t)(idx & 0x03);
    return _writeData(&val, 1);
}

