#include "VFDPT6314HAL.h"
#include "../Capabilities/CapabilitiesRegistry.h"
#include <string.h>

VFDPT6314HAL::VFDPT6314HAL() {
    _capabilities = CapabilitiesRegistry::createVFDPT6314Capabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
}

bool VFDPT6314HAL::init() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    bool ok = _cmdInit();
    _lastError = ok?VFDError::Ok:VFDError::TransportFail;
    return ok;
}

bool VFDPT6314HAL::reset() { return init(); }

bool VFDPT6314HAL::clear() { bool ok=_cmdClear(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDPT6314HAL::cursorHome() { bool ok=_cmdHome(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }

bool VFDPT6314HAL::setCursorPos(uint8_t row, uint8_t col) {
    if (!_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    if (row >= _capabilities->getTextRows() || col >= _capabilities->getTextColumns()) { _lastError = VFDError::InvalidArgs; return false; }
    bool ok = _posRowCol(row, col);
    _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDPT6314HAL::setCursorBlinkRate(uint8_t rate_ms) {
    bool blink = (rate_ms!=0);
    bool ok = _displayControl(true, false, blink);
    _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDPT6314HAL::writeCharAt(uint8_t row, uint8_t column, char c) { return moveTo(row,column) && writeChar(c); }
bool VFDPT6314HAL::writeAt(uint8_t row, uint8_t column, const char* text) { return moveTo(row,column) && write(text); }
bool VFDPT6314HAL::moveTo(uint8_t row, uint8_t column) { return _posRowCol(row,column); }

bool VFDPT6314HAL::backSpace() { return writeChar(0x08); }
bool VFDPT6314HAL::hTab() { return writeChar(0x09); }
bool VFDPT6314HAL::lineFeed() { return writeChar(0x0A); }
bool VFDPT6314HAL::carriageReturn() { return writeChar(0x0D); }

bool VFDPT6314HAL::writeChar(char c) {
    if (!_transport) return false;
    return _writeData(reinterpret_cast<const uint8_t*>(&c), 1);
}

bool VFDPT6314HAL::write(const char* msg) {
    if (!_transport || !msg) { _lastError = VFDError::InvalidArgs; return false; }
    return _writeData(reinterpret_cast<const uint8_t*>(msg), strlen(msg));
}

bool VFDPT6314HAL::centerText(const char* str, uint8_t row) {
    if (!_capabilities || !str) { _lastError = VFDError::InvalidArgs; return false; }
    uint8_t cols = _capabilities->getTextColumns();
    size_t len = strlen(str); if (len>cols) len=cols;
    uint8_t pad = (uint8_t)((cols - len)/2);
    if (!setCursorPos(row,0)) return false;
    for (uint8_t i=0;i<pad;++i) { if (!_writeData((const uint8_t*)" ",1)) return false; }
    return write(str);
}

bool VFDPT6314HAL::writeCustomChar(uint8_t index) { uint8_t code; if (!getCustomCharCode(index, code)) { _lastError = VFDError::InvalidArgs; return false; } return writeChar((char)code); }
bool VFDPT6314HAL::getCustomCharCode(uint8_t index, uint8_t& codeOut) const { if (!_capabilities) return false; if (index>=_capabilities->getMaxUserDefinedCharacters()) return false; codeOut=index; return true; }

bool VFDPT6314HAL::setBrightness(uint8_t lumens) { (void)lumens; _lastError=VFDError::NotSupported; return false; }
bool VFDPT6314HAL::saveCustomChar(uint8_t index, const uint8_t* pattern) { return setCustomChar(index, pattern); }
bool VFDPT6314HAL::setCustomChar(uint8_t index, const uint8_t* pattern) {
    if (!_transport || !_capabilities || !pattern) { _lastError = VFDError::InvalidArgs; return false; }
    if (index >= 8) { _lastError = VFDError::InvalidArgs; return false; }
    uint8_t addr = (uint8_t)((index & 0x07) * 8);
    if (!_writeCmd((uint8_t)(0x40 | (addr & 0x3F)))) { _lastError = VFDError::TransportFail; return false; }
    for (uint8_t r=0;r<8;++r) { uint8_t row = pattern[r] & 0x1F; if(!_writeData(&row,1)) { _lastError = VFDError::TransportFail; return false; } }
    _lastError = VFDError::Ok; return true;
}

bool VFDPT6314HAL::setDisplayMode(uint8_t mode) { (void)mode; _lastError=VFDError::NotSupported; return false; }
bool VFDPT6314HAL::setDimming(uint8_t level) { (void)level; _lastError=VFDError::NotSupported; return false; }
bool VFDPT6314HAL::cursorBlinkSpeed(uint8_t rate) { return setCursorBlinkRate(rate); }
bool VFDPT6314HAL::changeCharSet(uint8_t setId) { (void)setId; _lastError=VFDError::NotSupported; return false; }

bool VFDPT6314HAL::sendEscapeSequence(const uint8_t* data) { (void)data; _lastError=VFDError::NotSupported; return false; }
bool VFDPT6314HAL::hScroll(const char* str, int dir, uint8_t row){ (void)str;(void)dir;(void)row; _lastError=VFDError::NotSupported; return false; }
bool VFDPT6314HAL::vScroll(const char* str, int dir){ (void)str;(void)dir; _lastError=VFDError::NotSupported; return false; }
bool VFDPT6314HAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction){ (void)text;(void)startRow;(void)direction; _lastError=VFDError::NotSupported; return false; }
bool VFDPT6314HAL::starWarsScroll(const char* text, uint8_t startRow){ (void)text;(void)startRow; _lastError=VFDError::NotSupported; return false; }

bool VFDPT6314HAL::flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms){ (void)str;(void)row;(void)col;(void)on_ms;(void)off_ms; _lastError=VFDError::NotSupported; return false; }

int VFDPT6314HAL::getCapabilities() const { return _capabilities?_capabilities->getAllCapabilities():0; }
const char* VFDPT6314HAL::getDeviceName() const { return _capabilities?_capabilities->getDeviceName():"PT6314"; }

// ===== NO_TOUCH primitives =====
bool VFDPT6314HAL::_functionSet(bool twoLine) {
    // 0x30 base; DB3=N (2-line)
    uint8_t cmd = 0x30;
    if (twoLine) cmd |= 0x08;
    return _writeCmd(cmd);
}

bool VFDPT6314HAL::_cmdInit() {
    if (!_functionSet(_twoLine)) return false;
    if (!_displayControl(true,false,false)) return false;
    if (!_cmdClear()) return false;
    if (!_writeCmd(0x06)) return false; // entry mode: increment, no shift
    return true;
}

bool VFDPT6314HAL::_cmdClear() { return _writeCmd(0x01); }
bool VFDPT6314HAL::_cmdHome() { return _writeCmd(0x02); }

bool VFDPT6314HAL::_posLinear(uint8_t addr) { return _writeCmd((uint8_t)(0x80 | (addr & 0x7F))); }
bool VFDPT6314HAL::_posRowCol(uint8_t row, uint8_t col) { uint8_t base[] = {0x00, 0x40}; if (row>=2) return false; return _posLinear((uint8_t)(base[row]+col)); }
bool VFDPT6314HAL::_displayControl(bool d, bool c, bool b) { uint8_t cmd = 0x08 | (d?0x04:0) | (c?0x02:0) | (b?0x01:0); return _writeCmd(cmd); }

bool VFDPT6314HAL::_writeCmd(uint8_t cmd) {
    if (!_transport) return false;
    if (_transport->supportsControlLines()) {
        (void)_transport->setControlLine("RS", false);
        return _transport->write(&cmd,1);
    } else {
        return _serialWriteFrame(false, false, &cmd, 1);
    }
}

bool VFDPT6314HAL::_writeData(const uint8_t* data, size_t len) {
    if(!_transport||!data||len==0) return false;
    if (_transport->supportsControlLines()) {
        (void)_transport->setControlLine("RS", true);
        return _transport->write(data,len);
    } else {
        return _serialWriteFrame(true, false, data, len);
    }
}

bool VFDPT6314HAL::_serialWriteFrame(bool rsData, bool rwRead, const uint8_t* payload, size_t len) {
    // PT6314 serial: Start Byte = [sync bits][R/W][RS][0]; use 0b11111 as sync (bits7..3)
    // Construct a start byte 0xF8 | (rw<<2) | (rs<<1)
    uint8_t start = (uint8_t)(0xF8 | ((rwRead?1:0) << 2) | ((rsData?1:0) << 1));
    if (!_transport->write(&start,1)) return false;
    return _transport->write(payload, len);
}

// Device-specific helper
bool VFDPT6314HAL::setBrightnessIndex(uint8_t idx0to3) { (void)idx0to3; _lastError=VFDError::NotSupported; return false; }
