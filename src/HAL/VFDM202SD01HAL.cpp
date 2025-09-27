#include "VFDM202SD01HAL.h"
#include "../Capabilities/CapabilitiesRegistry.h"
#include <string.h>

VFDM202SD01HAL::VFDM202SD01HAL() {
    _capabilities = CapabilitiesRegistry::createVFDM202SD01Capabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
}

bool VFDM202SD01HAL::init() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    bool ok = _cmdInit(); _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDM202SD01HAL::reset() { return init(); }

bool VFDM202SD01HAL::clear() { bool ok=_cmdClear(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDM202SD01HAL::cursorHome() { bool ok=_cmdHomeTopLeft(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }

bool VFDM202SD01HAL::setCursorPos(uint8_t row, uint8_t col) {
    if (!_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    if (row >= _capabilities->getTextRows() || col >= _capabilities->getTextColumns()) { _lastError = VFDError::InvalidArgs; return false; }
    bool ok = _posRowCol(row,col); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDM202SD01HAL::setCursorBlinkRate(uint8_t rate_ms) {
    // Use cursor mode: blink if non-zero, else lighting off
    uint8_t mode = (rate_ms!=0) ? 0x88 : 0x00;
    bool ok = _cmdCursorMode(mode); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDM202SD01HAL::writeCharAt(uint8_t row, uint8_t column, char c) { return moveTo(row,column) && writeChar(c); }
bool VFDM202SD01HAL::writeAt(uint8_t row, uint8_t column, const char* text) { return moveTo(row,column) && write(text); }
bool VFDM202SD01HAL::moveTo(uint8_t row, uint8_t column) { return _posRowCol(row,column); }

bool VFDM202SD01HAL::backSpace() { return _cmdBackSpace(); }
bool VFDM202SD01HAL::hTab() { return _cmdHtab(); }
bool VFDM202SD01HAL::lineFeed() { return writeChar(0x0A); }
bool VFDM202SD01HAL::carriageReturn() { return writeChar(0x0D); }

bool VFDM202SD01HAL::writeChar(char c) { if(!_transport) return false; return _writeData(reinterpret_cast<const uint8_t*>(&c),1); }
bool VFDM202SD01HAL::write(const char* msg) { if(!_transport||!msg){ _lastError=VFDError::InvalidArgs; return false;} return _writeData(reinterpret_cast<const uint8_t*>(msg), strlen(msg)); }

bool VFDM202SD01HAL::centerText(const char* str, uint8_t row) {
    if(!_capabilities||!str){ _lastError=VFDError::InvalidArgs; return false;} uint8_t cols=_capabilities->getTextColumns(); size_t len=strlen(str); if(len>cols) len=cols; uint8_t pad=(uint8_t)((cols-len)/2);
    if(!setCursorPos(row,0)) return false; for(uint8_t i=0;i<pad;++i) if(!_writeData((const uint8_t*)" ",1)) return false; return write(str);
}

bool VFDM202SD01HAL::writeCustomChar(uint8_t index) { uint8_t code; if(!getCustomCharCode(index, code)){ _lastError=VFDError::InvalidArgs; return false;} return writeChar((char)code); }
bool VFDM202SD01HAL::getCustomCharCode(uint8_t index, uint8_t& codeOut) const { if(!_capabilities) return false; if(index>=_capabilities->getMaxUserDefinedCharacters()) return false; codeOut=index; return true; }

bool VFDM202SD01HAL::setBrightness(uint8_t lumens) {
    // 6 levels: 0,20,40,60,80,100 -> 00,20,40,60,80,FF
    uint8_t code = (lumens<17)?0x00 : (lumens<33)?0x20 : (lumens<50)?0x40 : (lumens<67)?0x60 : (lumens<84)?0x80 : 0xFF;
    bool ok = _cmdDimming(code); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDM202SD01HAL::saveCustomChar(uint8_t index, const uint8_t* pattern) { (void)index;(void)pattern; _lastError=VFDError::NotSupported; return false; }
bool VFDM202SD01HAL::setCustomChar(uint8_t index, const uint8_t* pattern) { (void)index;(void)pattern; _lastError=VFDError::NotSupported; return false; }
bool VFDM202SD01HAL::setDisplayMode(uint8_t mode) { (void)mode; _lastError=VFDError::NotSupported; return false; }
bool VFDM202SD01HAL::setDimming(uint8_t level) { uint8_t map[]={0x00,0x20,0x40,0x60,0x80,0xFF}; if(level>5) level=5; bool ok=_cmdDimming(map[level]); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDM202SD01HAL::cursorBlinkSpeed(uint8_t rate) { return setCursorBlinkRate(rate); }
bool VFDM202SD01HAL::changeCharSet(uint8_t setId) { (void)setId; _lastError=VFDError::NotSupported; return false; }

bool VFDM202SD01HAL::sendEscapeSequence(const uint8_t* data) { (void)data; _lastError=VFDError::NotSupported; return false; }
bool VFDM202SD01HAL::hScroll(const char* str, int dir, uint8_t row){ (void)str;(void)dir;(void)row; _lastError=VFDError::NotSupported; return false; }
bool VFDM202SD01HAL::vScroll(const char* str, int dir){ (void)str;(void)dir; _lastError=VFDError::NotSupported; return false; }
bool VFDM202SD01HAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction){ (void)text;(void)startRow;(void)direction; _lastError=VFDError::NotSupported; return false; }
bool VFDM202SD01HAL::starWarsScroll(const char* text, uint8_t startRow){ (void)text;(void)startRow; _lastError=VFDError::NotSupported; return false; }

bool VFDM202SD01HAL::flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms){ (void)str;(void)row;(void)col;(void)on_ms;(void)off_ms; _lastError=VFDError::NotSupported; return false; }

int VFDM202SD01HAL::getCapabilities() const { return _capabilities?_capabilities->getAllCapabilities():0; }
const char* VFDM202SD01HAL::getDeviceName() const { return _capabilities?_capabilities->getDeviceName():"M202SD01"; }

// ===== NO_TOUCH primitives =====
bool VFDM202SD01HAL::_cmdInit() { if(!_cmdReset()) return false; // default dimming set by device
    return true; }
bool VFDM202SD01HAL::_cmdReset() { return _writeByte(0x1F); }
bool VFDM202SD01HAL::_cmdClear() { return _writeByte(0x0D); }
bool VFDM202SD01HAL::_cmdHomeTopLeft() { return _writeByte(0x0C); }
bool VFDM202SD01HAL::_posLinear(uint8_t addr) { uint8_t b[2]={0x10, addr}; return _writeData(b,2); }
bool VFDM202SD01HAL::_posRowCol(uint8_t row, uint8_t col) { uint8_t base[]={0x00,0x14}; if(row>=2) return false; return _posLinear((uint8_t)(base[row]+col)); }
bool VFDM202SD01HAL::_cmdBackSpace() { return _writeByte(0x08); }
bool VFDM202SD01HAL::_cmdHtab() { return _writeByte(0x09); }
bool VFDM202SD01HAL::_cmdCR() { return _writeByte(0x0D); }
bool VFDM202SD01HAL::_cmdDimming(uint8_t code) { uint8_t b[2]={0x04, code}; return _writeData(b,2); }
bool VFDM202SD01HAL::_cmdCursorMode(uint8_t mode) { uint8_t b[2]={0x17, mode}; return _writeData(b,2); }
bool VFDM202SD01HAL::_writeByte(uint8_t b) { if(!_transport) return false; return _transport->write(&b,1); }
bool VFDM202SD01HAL::_writeData(const uint8_t* p, size_t n) { if(!_transport||!p||n==0) return false; return _transport->write(p,n); }

