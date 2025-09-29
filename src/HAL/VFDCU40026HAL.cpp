#include "VFDCU40026HAL.h"
#include "../Capabilities/CapabilitiesRegistry.h"
#include <string.h>

static constexpr uint8_t ESC_CH = 0x1B;

VFDCU40026HAL::VFDCU40026HAL() {
    _capabilities = CapabilitiesRegistry::createVFDCU40026Capabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
}

bool VFDCU40026HAL::init() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    bool ok = _escInit();
    _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDCU40026HAL::reset() { return init(); }

bool VFDCU40026HAL::clear() { bool ok=_cmdClear(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDCU40026HAL::cursorHome() { bool ok=_cmdHomeTopLeft(); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }

bool VFDCU40026HAL::setCursorPos(uint8_t row, uint8_t col) {
    if (!_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    if (row >= _capabilities->getTextRows() || col >= _capabilities->getTextColumns()) { _lastError = VFDError::InvalidArgs; return false; }
    bool ok = _posRowCol(row, col); _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDCU40026HAL::setCursorBlinkRate(uint8_t rate_ms) {
    // period = data * ~30ms; choose data ~ rate_ms/30 (clamped)
    uint16_t d = rate_ms / 30; if (d==0) d=1; if (d>255) d=255;
    bool ok = _escBlinkPeriod((uint8_t)d); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDCU40026HAL::setCursorMode(uint8_t mode) {
    (void)mode; _lastError = VFDError::NotSupported; return false;
}

bool VFDCU40026HAL::writeCharAt(uint8_t row, uint8_t column, char c) { return moveTo(row,column) && writeChar(c); }
bool VFDCU40026HAL::writeAt(uint8_t row, uint8_t column, const char* text) { return moveTo(row,column) && write(text); }
bool VFDCU40026HAL::moveTo(uint8_t row, uint8_t column) { return _posRowCol(row,column); }

bool VFDCU40026HAL::backSpace() { return writeChar(0x08); }
bool VFDCU40026HAL::hTab() { return writeChar(0x09); }
bool VFDCU40026HAL::lineFeed() { return writeChar(0x0A); }
bool VFDCU40026HAL::carriageReturn() { return writeChar(0x0D); }

bool VFDCU40026HAL::writeChar(char c) {
    if (!_transport) return false; return _writeData(reinterpret_cast<const uint8_t*>(&c), 1);
}

bool VFDCU40026HAL::write(const char* msg) {
    if (!_transport || !msg) { _lastError = VFDError::InvalidArgs; return false; }
    return _writeData(reinterpret_cast<const uint8_t*>(msg), strlen(msg));
}

bool VFDCU40026HAL::centerText(const char* str, uint8_t row) {
    if (!_capabilities || !str) { _lastError = VFDError::InvalidArgs; return false; }
    uint8_t cols=_capabilities->getTextColumns(); size_t len=strlen(str); if (len>cols) len=cols; uint8_t pad=(uint8_t)((cols-len)/2);
    if (!setCursorPos(row,0)) return false; for (uint8_t i=0;i<pad;++i) if(!_writeData((const uint8_t*)" ",1)) return false; return write(str);
}

bool VFDCU40026HAL::writeCustomChar(uint8_t index) { uint8_t code; if (!getCustomCharCode(index, code)) { _lastError = VFDError::InvalidArgs; return false; } return writeChar((char)code); }
bool VFDCU40026HAL::getCustomCharCode(uint8_t index, uint8_t& codeOut) const { if (!_capabilities) return false; if (index>=_capabilities->getMaxUserDefinedCharacters()) return false; codeOut=index; return true; }

bool VFDCU40026HAL::setBrightness(uint8_t lumens) {
    // Map to 4 bands: 25,50,75,100% via ESC 'L' + code in range
    uint8_t code = (lumens<64)?0x00 : (lumens<128)?0x40 : (lumens<192)?0x80 : 0xC0;
    bool ok = _escLuminance(code); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDCU40026HAL::saveCustomChar(uint8_t index, const uint8_t* pattern) { return setCustomChar(index, pattern); }

bool VFDCU40026HAL::setCustomChar(uint8_t index, const uint8_t* pattern) {
    if (!_transport || !_capabilities || !pattern) { _lastError = VFDError::InvalidArgs; return false; }
    if (index >= 16) { _lastError = VFDError::InvalidArgs; return false; } // datasheet supports 16 UDFs
    // Pack 5x7 rows (8 rows provided; row7 may be cursor overlay)
    uint8_t rows5[5]={0,0,0,0,0};
    // pattern is 8 rows with bits 0..4 used; pack column-wise per device (same as 20S401)
    // simple row-wise to 5 bytes: each byte accumulates vertical bits (PT1..PT5)
    for (uint8_t r=0;r<7;++r){ uint8_t row = pattern[r] & 0x1F; for (uint8_t c=0;c<5;++c){ if ((row>>c)&1){ rows5[c] |= (1u<<r); } } }
    bool ok = _escUDF(index, rows5); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDCU40026HAL::setDisplayMode(uint8_t mode) { (void)mode; _lastError=VFDError::NotSupported; return false; }
bool VFDCU40026HAL::setDimming(uint8_t level) { uint8_t code = (level&3)<<6; bool ok=_escLuminance(code); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDCU40026HAL::cursorBlinkSpeed(uint8_t rate) { return setCursorBlinkRate(rate); }
bool VFDCU40026HAL::changeCharSet(uint8_t setId) { if (setId==0) return writeChar(0x18); if (setId==1) return writeChar(0x19); return false; }

bool VFDCU40026HAL::sendEscapeSequence(const uint8_t* data) { if (!_transport||!data) return false; uint8_t esc=ESC_CH; if(!_transport->write(&esc,1)) return false; uint8_t n=0; while (n<8 && data[n]!=0){ if(!_transport->write(&data[n],1)) return false; ++n; } return true; }

bool VFDCU40026HAL::hScroll(const char* str, int dir, uint8_t row) { (void)str;(void)dir;(void)row; _lastError=VFDError::NotSupported; return false; }
bool VFDCU40026HAL::vScroll(const char* str, int dir) { (void)str;(void)dir; _lastError=VFDError::NotSupported; return false; }
bool VFDCU40026HAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) { (void)text;(void)startRow;(void)direction; _lastError=VFDError::NotSupported; return false; }
bool VFDCU40026HAL::starWarsScroll(const char* text, uint8_t startRow) { (void)text;(void)startRow; _lastError=VFDError::NotSupported; return false; }

bool VFDCU40026HAL::flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms) { (void)str;(void)row;(void)col;(void)on_ms;(void)off_ms; _lastError=VFDError::NotSupported; return false; }

int VFDCU40026HAL::getCapabilities() const { return _capabilities?_capabilities->getAllCapabilities():0; }
const char* VFDCU40026HAL::getDeviceName() const { return _capabilities?_capabilities->getDeviceName():"CU40026"; }

// ===== NO_TOUCH primitives =====
bool VFDCU40026HAL::_escInit() { uint8_t esc=ESC_CH, I='I'; if(!_transport->write(&esc,1)) return false; return _transport->write((uint8_t*)&I,1); }
bool VFDCU40026HAL::_cmdClear() { uint8_t b=0x0E; return _writeCmd(b); }
bool VFDCU40026HAL::_cmdHomeTopLeft() { uint8_t b=0x0C; return _writeCmd(b); }
bool VFDCU40026HAL::_posLinear(uint8_t addr) { uint8_t esc=ESC_CH, H='H'; if(!_transport->write(&esc,1)) return false; if(!_transport->write((uint8_t*)&H,1)) return false; return _transport->write(&addr,1); }
bool VFDCU40026HAL::_posRowCol(uint8_t row, uint8_t col) { uint8_t addr = (uint8_t)(row*40 + col); return _posLinear(addr); }
bool VFDCU40026HAL::_escLuminance(uint8_t code) { uint8_t esc=ESC_CH, L='L'; if(!_transport->write(&esc,1)) return false; if(!_transport->write((uint8_t*)&L,1)) return false; return _transport->write(&code,1); }
bool VFDCU40026HAL::_escBlinkPeriod(uint8_t data) { uint8_t esc=ESC_CH, T='T'; if(!_transport->write(&esc,1)) return false; if(!_transport->write((uint8_t*)&T,1)) return false; return _transport->write(&data,1); }
bool VFDCU40026HAL::_escUDF(uint8_t chr, const uint8_t rows5[5]) { uint8_t esc=ESC_CH,C='C'; if(!_transport->write(&esc,1)) return false; if(!_transport->write((uint8_t*)&C,1)) return false; if(!_transport->write(&chr,1)) return false; return _transport->write(rows5,5); }
bool VFDCU40026HAL::_writeCmd(uint8_t b) { if (!_transport) return false; return _transport->write(&b,1); }
bool VFDCU40026HAL::_writeData(const uint8_t* p, size_t n) { if (!_transport||!p||n==0) return false; return _transport->write(p,n); }

// Device-specific helpers
bool VFDCU40026HAL::setLuminanceBand(uint8_t code) { bool ok=_escLuminance(code); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDCU40026HAL::setLuminanceIndex(uint8_t idx0to3) {
    uint8_t code = (uint8_t)((idx0to3 & 0x03) << 6); // 0->00,1->40,2->80,3->C0
    return setLuminanceBand(code);
}
bool VFDCU40026HAL::setBlinkPeriodMs(uint16_t periodMs) {
    uint16_t d = periodMs/30; if (d==0) d=1; if (d>255) d=255; return _escBlinkPeriod((uint8_t)d);
}
bool VFDCU40026HAL::selectFlickerlessMode() { uint8_t esc=ESC_CH, S='S'; if(!_transport->write(&esc,1)) return false; return _transport->write((uint8_t*)&S,1); }
