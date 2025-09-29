#include "VFDM204SD01AHAL.h"
#include "../Capabilities/CapabilitiesRegistry.h"
#include <string.h>

VFDM204SD01AHAL::VFDM204SD01AHAL() {
    _capabilities = CapabilitiesRegistry::createVFDM204SD01ACapabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
}

bool VFDM204SD01AHAL::init() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    bool ok = _cmdInit(); _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDM204SD01AHAL::reset() { return init(); }

bool VFDM204SD01AHAL::clear() { bool ok=_cmdClear(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDM204SD01AHAL::cursorHome() { bool ok=_cmdHomeTopLeft(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }

bool VFDM204SD01AHAL::setCursorPos(uint8_t row, uint8_t col) {
    if (!_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    if (row >= _capabilities->getTextRows() || col >= _capabilities->getTextColumns()) { _lastError = VFDError::InvalidArgs; return false; }
    bool ok = _posRowCol(row,col); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDM204SD01AHAL::setCursorBlinkRate(uint8_t rate_ms) { (void)rate_ms; _lastError=VFDError::NotSupported; return false; }
bool VFDM204SD01AHAL::setCursorMode(uint8_t mode) { (void)mode; _lastError=VFDError::NotSupported; return false; }

bool VFDM204SD01AHAL::writeCharAt(uint8_t row, uint8_t column, char c) { return moveTo(row,column) && writeChar(c); }
bool VFDM204SD01AHAL::writeAt(uint8_t row, uint8_t column, const char* text) { return moveTo(row,column) && write(text); }
bool VFDM204SD01AHAL::moveTo(uint8_t row, uint8_t column) { return _posRowCol(row,column); }

bool VFDM204SD01AHAL::backSpace() { return writeChar(0x08); }
bool VFDM204SD01AHAL::hTab() { return writeChar(0x09); }
bool VFDM204SD01AHAL::lineFeed() { return writeChar(0x0A); }
bool VFDM204SD01AHAL::carriageReturn() { return writeChar(0x0D); }

bool VFDM204SD01AHAL::writeChar(char c) { if(!_transport) return false; return _writeData(reinterpret_cast<const uint8_t*>(&c),1); }
bool VFDM204SD01AHAL::write(const char* msg) { if(!_transport||!msg){ _lastError=VFDError::InvalidArgs; return false;} return _writeData(reinterpret_cast<const uint8_t*>(msg), strlen(msg)); }

bool VFDM204SD01AHAL::centerText(const char* str, uint8_t row){ if(!_capabilities||!str){ _lastError=VFDError::InvalidArgs; return false;} uint8_t cols=_capabilities->getTextColumns(); size_t len=strlen(str); if(len>cols) len=cols; uint8_t pad=(uint8_t)((cols-len)/2); if(!setCursorPos(row,0)) return false; for(uint8_t i=0;i<pad;++i) if(!_writeData((const uint8_t*)" ",1)) return false; return write(str);} 

bool VFDM204SD01AHAL::writeCustomChar(uint8_t index) { (void)index; _lastError=VFDError::NotSupported; return false; }
bool VFDM204SD01AHAL::getCustomCharCode(uint8_t index, uint8_t& codeOut) const { (void)index; (void)codeOut; return false; }

bool VFDM204SD01AHAL::setBrightness(uint8_t lumens) {
    // 4 levels: approximate mapping
    uint8_t code = (lumens<64)?0x00 : (lumens<128)?0x40 : (lumens<192)?0x80 : 0xFF;
    bool ok = _cmdDimming(code); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDM204SD01AHAL::saveCustomChar(uint8_t index, const uint8_t* pattern) { (void)index;(void)pattern; _lastError=VFDError::NotSupported; return false; }
bool VFDM204SD01AHAL::setCustomChar(uint8_t index, const uint8_t* pattern) { (void)index;(void)pattern; _lastError=VFDError::NotSupported; return false; }
bool VFDM204SD01AHAL::setDisplayMode(uint8_t mode) { (void)mode; _lastError=VFDError::NotSupported; return false; }
bool VFDM204SD01AHAL::setDimming(uint8_t level) { uint8_t map4[]={0x00,0x40,0x80,0xFF}; if(level>3) level=3; bool ok=_cmdDimming(map4[level]); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDM204SD01AHAL::cursorBlinkSpeed(uint8_t rate) { return setCursorBlinkRate(rate); }
bool VFDM204SD01AHAL::changeCharSet(uint8_t setId) { (void)setId; _lastError=VFDError::NotSupported; return false; }

bool VFDM204SD01AHAL::sendEscapeSequence(const uint8_t* data) { (void)data; _lastError=VFDError::NotSupported; return false; }
bool VFDM204SD01AHAL::hScroll(const char* str, int dir, uint8_t row){ (void)str;(void)dir;(void)row; _lastError=VFDError::NotSupported; return false; }
bool VFDM204SD01AHAL::vScroll(const char* str, int dir){ (void)str;(void)dir; _lastError=VFDError::NotSupported; return false; }
bool VFDM204SD01AHAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction){ (void)text;(void)startRow;(void)direction; _lastError=VFDError::NotSupported; return false; }
bool VFDM204SD01AHAL::starWarsScroll(const char* text, uint8_t startRow){ (void)text;(void)startRow; _lastError=VFDError::NotSupported; return false; }

bool VFDM204SD01AHAL::flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms){ (void)str;(void)row;(void)col;(void)on_ms;(void)off_ms; _lastError=VFDError::NotSupported; return false; }

int VFDM204SD01AHAL::getCapabilities() const { return _capabilities?_capabilities->getAllCapabilities():0; }
const char* VFDM204SD01AHAL::getDeviceName() const { return _capabilities?_capabilities->getDeviceName():"M204SD01A"; }

// ===== NO_TOUCH primitives =====
bool VFDM204SD01AHAL::_cmdInit() {
    // Clear and home
    if (!_cmdClear()) return false;
    if (!_cmdHomeTopLeft()) return false;
    return true;
}

bool VFDM204SD01AHAL::_cmdReset() { return _writeByte(0x1F); }
bool VFDM204SD01AHAL::_cmdClear() { return _writeByte(0x0D); }
bool VFDM204SD01AHAL::_cmdHomeTopLeft() { return _writeByte(0x0C); }
bool VFDM204SD01AHAL::_posLinear(uint8_t addr) { uint8_t b[2]={0x10, addr}; return _writeData(b,2); }
bool VFDM204SD01AHAL::_posRowCol(uint8_t row, uint8_t col) { const uint8_t base[4]={0x00,0x14,0x28,0x3C}; if(row>=4) return false; return _posLinear((uint8_t)(base[row]+col)); }
bool VFDM204SD01AHAL::_cmdDimming(uint8_t code) { uint8_t b[2]={0x04, code}; return _writeData(b,2); }
bool VFDM204SD01AHAL::_writeByte(uint8_t b) { if(!_transport) return false; return _transport->write(&b,1); }
bool VFDM204SD01AHAL::_writeData(const uint8_t* p, size_t n) { if(!_transport||!p||n==0) return false; return _transport->write(p,n); }
