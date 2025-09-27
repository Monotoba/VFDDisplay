#include "VFDPT6302HAL.h"
#include "../Capabilities/CapabilitiesRegistry.h"
#include <string.h>

// Helper for clamping
static inline uint8_t clampu8(uint8_t v, uint8_t lo, uint8_t hi){ return v<lo?lo:(v>hi?hi:v); }

VFDPT6302HAL::VFDPT6302HAL() {
    _capabilities = CapabilitiesRegistry::createVFDPT6302Capabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
}

bool VFDPT6302HAL::init() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    // Recommended: set number of digits, duty, and normal light state
    if (!_cmdNumberOfDigits(16)) { _lastError = VFDError::TransportFail; return false; }
    if (!_cmdDisplayDuty(7)) { _lastError = VFDError::TransportFail; return false; } // 15/16
    if (!_cmdAllLights(0,0)) { _lastError = VFDError::TransportFail; return false; } // normal
    _row = 0; _col = 0; _lastError = VFDError::Ok; return true;
}

bool VFDPT6302HAL::reset() { return init(); }

bool VFDPT6302HAL::clear() {
    if (!_transport || !_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    // Write spaces across all digits
    const uint8_t cols = _capabilities->getTextColumns();
    uint8_t buf[32]; if (cols > sizeof(buf)) {
        // chunked writes
        uint8_t remaining = cols; uint8_t addr = 0; while (remaining) {
            uint8_t n = remaining > sizeof(buf) ? sizeof(buf) : remaining;
            memset(buf, ' ', n);
            if (!_dcramWriteChars(addr, buf, n)) { _lastError = VFDError::TransportFail; return false; }
            addr += n; remaining -= n;
        }
    } else {
        memset(buf, ' ', cols);
        if (!_dcramWriteChars(0, buf, cols)) { _lastError = VFDError::TransportFail; return false; }
    }
    _row = 0; _col = 0; _lastError = VFDError::Ok; return true;
}

bool VFDPT6302HAL::cursorHome() { _row = 0; _col = 0; _lastError = VFDError::Ok; return true; }

bool VFDPT6302HAL::setCursorPos(uint8_t row, uint8_t col) {
    if (!_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    if (row >= _capabilities->getTextRows() || col >= _capabilities->getTextColumns()) { _lastError = VFDError::InvalidArgs; return false; }
    _row = row; _col = col; _lastError = VFDError::Ok; return true;
}

bool VFDPT6302HAL::setCursorBlinkRate(uint8_t rate_ms) { (void)rate_ms; _lastError = VFDError::NotSupported; return false; }

bool VFDPT6302HAL::writeCharAt(uint8_t row, uint8_t column, char c) { return moveTo(row,column) && writeChar(c); }
bool VFDPT6302HAL::writeAt(uint8_t row, uint8_t column, const char* text) { return moveTo(row,column) && write(text); }
bool VFDPT6302HAL::moveTo(uint8_t row, uint8_t column) { return setCursorPos(row,column); }

bool VFDPT6302HAL::backSpace() { if (_col==0) return true; _col--; return writeChar(' '); }
bool VFDPT6302HAL::hTab() { _col = (uint8_t)((_col + 4) & ~3u); return true; }
bool VFDPT6302HAL::lineFeed() { _lastError=VFDError::NotSupported; return false; }
bool VFDPT6302HAL::carriageReturn() { _col = 0; return true; }

bool VFDPT6302HAL::writeChar(char c) {
    if (!_transport || !_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    uint8_t cols = _capabilities->getTextColumns();
    uint8_t addr = clampu8(_col, 0, cols?cols-1:0);
    if (!_dcramWriteChars(addr, reinterpret_cast<uint8_t*>(&c), 1)) { _lastError=VFDError::TransportFail; return false; }
    if (++_col >= cols) _col = (uint8_t)(cols-1); // clamp at end
    _lastError = VFDError::Ok; return true;
}

bool VFDPT6302HAL::write(const char* msg) {
    if (!_transport || !msg || !_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    size_t len = strlen(msg); if (len == 0) { _lastError = VFDError::Ok; return true; }
    uint8_t cols = _capabilities->getTextColumns(); uint8_t addr = clampu8(_col, 0, cols?cols-1:0);
    // Trim to available columns
    size_t avail = cols - addr; if (len > avail) len = avail;
    if (len == 0) { _lastError = VFDError::Ok; return true; }
    if (!_dcramWriteChars(addr, reinterpret_cast<const uint8_t*>(msg), len)) { _lastError=VFDError::TransportFail; return false; }
    _col = (uint8_t)(addr + len); if (_col >= cols) _col = (uint8_t)(cols-1);
    _lastError = VFDError::Ok; return true;
}

bool VFDPT6302HAL::centerText(const char* str, uint8_t row) {
    if (!_capabilities || !str) { _lastError = VFDError::InvalidArgs; return false; }
    uint8_t cols=_capabilities->getTextColumns(); size_t len=strlen(str); if(len>cols) len=cols; uint8_t pad=(uint8_t)((cols-len)/2);
    if (!setCursorPos(row,0)) return false; for(uint8_t i=0;i<pad;++i){ if(!writeChar(' ')) return false; }
    return write(str);
}

bool VFDPT6302HAL::writeCustomChar(uint8_t index) { uint8_t code; if(!getCustomCharCode(index, code)){ _lastError=VFDError::InvalidArgs; return false;} return writeChar((char)code); }
bool VFDPT6302HAL::getCustomCharCode(uint8_t index, uint8_t& codeOut) const { if(!_capabilities) return false; if(index>=_capabilities->getMaxUserDefinedCharacters()) return false; codeOut=(uint8_t)index; return true; }

bool VFDPT6302HAL::setBrightness(uint8_t lumens) {
    // Map 0..255 to 8 duty levels (0..7)
    uint8_t idx = (lumens < 32) ? 0 : (lumens < 64) ? 1 : (lumens < 96) ? 2 : (lumens < 128) ? 3 : (lumens < 160) ? 4 : (lumens < 192) ? 5 : (lumens < 224) ? 6 : 7;
    bool ok = _cmdDisplayDuty(idx); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFDPT6302HAL::saveCustomChar(uint8_t index, const uint8_t* pattern) { return setCustomChar(index, pattern); }

bool VFDPT6302HAL::setCustomChar(uint8_t index, const uint8_t* pattern) {
    // For simplicity, not implemented. Device supports CGRAM with 35-bit patterns.
    (void)index; (void)pattern; _lastError=VFDError::NotSupported; return false;
}

bool VFDPT6302HAL::setDisplayMode(uint8_t mode) { (void)mode; _lastError=VFDError::NotSupported; return false; }
bool VFDPT6302HAL::setDimming(uint8_t level) { uint8_t idx = (uint8_t)(level & 0x07); bool ok=_cmdDisplayDuty(idx); _lastError=ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFDPT6302HAL::cursorBlinkSpeed(uint8_t rate) { return setCursorBlinkRate(rate); }
bool VFDPT6302HAL::changeCharSet(uint8_t setId) { (void)setId; _lastError=VFDError::NotSupported; return false; }

bool VFDPT6302HAL::sendEscapeSequence(const uint8_t* data) { (void)data; _lastError=VFDError::NotSupported; return false; }
bool VFDPT6302HAL::hScroll(const char* str, int dir, uint8_t row){ (void)str;(void)dir;(void)row; _lastError=VFDError::NotSupported; return false; }
bool VFDPT6302HAL::vScroll(const char* str, int dir){ (void)str;(void)dir; _lastError=VFDError::NotSupported; return false; }
bool VFDPT6302HAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction){ (void)text;(void)startRow;(void)direction; _lastError=VFDError::NotSupported; return false; }
bool VFDPT6302HAL::starWarsScroll(const char* text, uint8_t startRow){ (void)text;(void)startRow; _lastError=VFDError::NotSupported; return false; }

bool VFDPT6302HAL::flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms){ (void)str;(void)row;(void)col;(void)on_ms;(void)off_ms; _lastError=VFDError::NotSupported; return false; }

int VFDPT6302HAL::getCapabilities() const { return _capabilities?_capabilities->getAllCapabilities():0; }
const char* VFDPT6302HAL::getDeviceName() const { return _capabilities?_capabilities->getDeviceName():"PT6302"; }

// ===== NO_TOUCH primitives =====
bool VFDPT6302HAL::_cmdDisplayDuty(uint8_t dutyIdx) {
    dutyIdx &= 0x07; // 0..7
    uint8_t b = (uint8_t)(0x50 | (dutyIdx & 0x07));
    return _writeByte(b);
}

bool VFDPT6302HAL::_cmdNumberOfDigits(uint8_t digits) {
    // datasheet: supports 9..16; map to K = digits-9 (0..7)
    if (digits < 9) digits = 9; if (digits > 16) digits = 16;
    uint8_t k = (uint8_t)(digits - 9) & 0x07; // K0..K2 in bits 0..2; bit3=0
    uint8_t b = (uint8_t)(0x60 | k);
    return _writeByte(b);
}

bool VFDPT6302HAL::_cmdAllLights(uint8_t L, uint8_t H) {
    L = L ? 1 : 0; H = H ? 1 : 0;
    uint8_t low = (uint8_t)((H<<1) | L); // bits 1..0
    uint8_t b = (uint8_t)(0x70 | (low & 0x03));
    return _writeByte(b);
}

bool VFDPT6302HAL::_cmdDCRAMAddr(uint8_t addr4) {
    addr4 &= 0x0F; // 0..15
    uint8_t b = (uint8_t)(0x10 | addr4);
    return _writeByte(b);
}

bool VFDPT6302HAL::_writeByte(uint8_t b) { if(!_transport) return false; return _transport->write(&b,1); }
bool VFDPT6302HAL::_writeData(const uint8_t* p, size_t n) { if(!_transport||!p||n==0) return false; return _transport->write(p,n); }

bool VFDPT6302HAL::_dcramWriteChars(uint8_t addr, const uint8_t* data, size_t n) {
    if (!_cmdDCRAMAddr(addr)) return false;
    return _writeData(data, n);
}

