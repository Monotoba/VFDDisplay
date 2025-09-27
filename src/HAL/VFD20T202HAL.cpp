#include "VFD20T202HAL.h"
#include "../Transports/ITransport.h"
#include "../Capabilities/CapabilitiesRegistry.h"
#include <string.h>

static constexpr uint8_t ESC_CHAR = 0x1B;

VFD20T202HAL::VFD20T202HAL() {
    _capabilities = CapabilitiesRegistry::createVFD20T202Capabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
}

bool VFD20T202HAL::init() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    bool ok = _cmdInit();
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20T202HAL::reset() {
    bool ok = _escReset();
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20T202HAL::clear() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    bool ok = _cmdClear();
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20T202HAL::cursorHome() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    bool ok = _cmdHome();
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20T202HAL::setCursorPos(uint8_t row, uint8_t col) {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    if (!_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    if (row >= _capabilities->getTextRows() || col >= _capabilities->getTextColumns()) {
        _lastError = VFDError::InvalidArgs; return false;
    }
    bool ok = _posRowCol(row, col);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20T202HAL::setCursorBlinkRate(uint8_t rate_ms) {
    (void)rate_ms;
    _lastError = VFDError::NotSupported;
    return false;
}

bool VFD20T202HAL::writeCharAt(uint8_t row, uint8_t column, char c) {
    if (!moveTo(row, column)) return false;
    return writeChar(c);
}

bool VFD20T202HAL::writeAt(uint8_t row, uint8_t column, const char* text) {
    if (!text) { _lastError = VFDError::InvalidArgs; return false; }
    if (!moveTo(row, column)) return false;
    return write(text);
}

bool VFD20T202HAL::moveTo(uint8_t row, uint8_t column) {
    return setCursorPos(row, column);
}

bool VFD20T202HAL::backSpace() { bool ok = writeChar(0x08); _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFD20T202HAL::hTab()      { bool ok = writeChar(0x09); _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFD20T202HAL::lineFeed()  { bool ok = writeChar(0x0A); _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok; }
bool VFD20T202HAL::carriageReturn(){ bool ok = writeChar(0x0D); _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok; }

bool VFD20T202HAL::writeChar(char c) {
    if (!_transport) return false;
    bool ok = _transport->write(reinterpret_cast<const uint8_t*>(&c), 1);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20T202HAL::write(const char* msg) {
    if (!_transport || !msg) { _lastError = VFDError::InvalidArgs; return false; }
    size_t len = strlen(msg);
    bool ok = _transport->write(reinterpret_cast<const uint8_t*>(msg), len);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20T202HAL::centerText(const char* str, uint8_t row) {
    if (!_transport || !str || !_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    uint8_t cols = _capabilities->getTextColumns();
    size_t len = strlen(str);
    if (len > cols) len = cols;
    uint8_t pad = (uint8_t)((cols - len)/2);
    if (!setCursorPos(row, 0)) return false;
    for (uint8_t i=0;i<pad;i++){ if(!writeChar(' ')) return false; }
    if (!write(str)) return false;
    _lastError = VFDError::Ok;
    return true;
}

bool VFD20T202HAL::writeCustomChar(uint8_t index) {
    uint8_t code=0; if (!getCustomCharCode(index, code)) { _lastError = VFDError::InvalidArgs; return false; }
    return writeChar((char)code);
}

bool VFD20T202HAL::getCustomCharCode(uint8_t index, uint8_t& codeOut) const {
    if (!_capabilities) return false;
    uint8_t maxUdf = _capabilities->getMaxUserDefinedCharacters();
    if (index >= maxUdf) return false;
    // Conservative mapping: 0..7 -> 0x00..0x07
    if (index < 8) { codeOut = index; return true; }
    return false;
}

bool VFD20T202HAL::setBrightness(uint8_t lumens) {
    (void)lumens; _lastError = VFDError::NotSupported; return false;
}

bool VFD20T202HAL::saveCustomChar(uint8_t index, const uint8_t* pattern) {
    return setCustomChar(index, pattern);
}

bool VFD20T202HAL::setCustomChar(uint8_t index, const uint8_t* pattern) {
    if (!_transport || !_capabilities || !pattern) { _lastError = VFDError::InvalidArgs; return false; }
    if (!_capabilities->hasCapability(CAP_USER_DEFINED_CHARS)) { _lastError = VFDError::NotSupported; return false; }
    if (index >= _capabilities->getMaxUserDefinedCharacters()) { _lastError = VFDError::InvalidArgs; return false; }
    // CGRAM address: index * 8
    uint8_t cgramAddr = (uint8_t)(index * 8) & 0x3F;
    if (!_writeCmd((uint8_t)(0x40 | cgramAddr))) { _lastError = VFDError::TransportFail; return false; }
    // Write 8 rows (5 LSBs used)
    for (uint8_t r=0; r<8; ++r) {
        uint8_t row = pattern[r] & 0x1F;
        if (!_writeData(&row, 1)) { _lastError = VFDError::TransportFail; return false; }
    }
    _lastError = VFDError::Ok;
    return true;
}

bool VFD20T202HAL::setDisplayMode(uint8_t mode) {
    // Not applicable; return false to indicate unsupported
    (void)mode; _lastError = VFDError::NotSupported; return false;
}

bool VFD20T202HAL::setDimming(uint8_t level) {
    (void)level; _lastError = VFDError::NotSupported; return false;
}

bool VFD20T202HAL::cursorBlinkSpeed(uint8_t rate) {
    // Map: any non-zero rate enables blink, zero disables
    bool blink = (rate != 0);
    // Display on, cursor on, blink per flag (0x0C base: display on, cursor off, blink off)
    uint8_t cmd = (uint8_t)(0x08 | 0x04 | 0x02 | (blink ? 0x01 : 0x00));
    bool ok = _writeCmd(cmd);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20T202HAL::changeCharSet(uint8_t setId) {
    (void)setId; _lastError = VFDError::NotSupported; return false;
}

bool VFD20T202HAL::sendEscapeSequence(const uint8_t* data) {
    (void)data; _lastError = VFDError::NotSupported; return false;
}

bool VFD20T202HAL::hScroll(const char* str, int dir, uint8_t row) {
    if (!_transport || !_capabilities || !str) { _lastError = VFDError::InvalidArgs; return false; }
    uint8_t cols = _capabilities->getTextColumns();
    if (row >= _capabilities->getTextRows()) { _lastError = VFDError::InvalidArgs; return false; }
    if (strncmp(str, _hScrollText, sizeof(_hScrollText)) != 0 || _hScrollRow != row) {
        size_t len = strlen(str); if (len >= sizeof(_hScrollText)) len = sizeof(_hScrollText)-1; memcpy(_hScrollText, str, len); _hScrollText[len]='\0'; _hScrollOffset=0; _hScrollRow=row;
    }
    int textLen = (int)strlen(_hScrollText);
    if (textLen==0) { bool ok = setCursorPos(row,0) && write(""); _lastError = ok?VFDError::Ok:lastError(); return ok; }
    if (dir>0) _hScrollOffset = (_hScrollOffset+1) % (textLen + cols);
    else if (dir<0) { _hScrollOffset = (_hScrollOffset-1); if (_hScrollOffset<0) _hScrollOffset = textLen + cols - 1; }
    char window[40]; if (cols > sizeof(window)-1) cols = sizeof(window)-1;
    for (uint8_t i=0;i<cols;++i){ int idx = _hScrollOffset + i; char c=' '; if (idx<textLen) c=_hScrollText[idx]; else if (idx < textLen+cols) c=' '; else { int w=idx-(textLen+cols); if (w>=0 && w<textLen) c=_hScrollText[w]; } window[i]=c; }
    window[cols]='\0';
    bool ok = writeAt(row, 0, window); _lastError = ok?VFDError::Ok:lastError(); return ok;
}

bool VFD20T202HAL::vScroll(const char* str, int dir) {
    (void)str; (void)dir; _lastError = VFDError::NotSupported; return false;
}

bool VFD20T202HAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) {
    (void)text; (void)startRow; (void)direction; _lastError = VFDError::NotSupported; return false;
}

bool VFD20T202HAL::starWarsScroll(const char* text, uint8_t startRow) { (void)text; (void)startRow; _lastError = VFDError::NotSupported; return false; }

bool VFD20T202HAL::flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms) {
    (void)str; (void)row; (void)col; (void)on_ms; (void)off_ms; _lastError = VFDError::NotSupported; return false;
}

int VFD20T202HAL::getCapabilities() const {
    return _capabilities ? _capabilities->getAllCapabilities() : 0;
}

const char* VFD20T202HAL::getDeviceName() const { return _capabilities ? _capabilities->getDeviceName() : "VFD20T202"; }

// ===== Device-specific primitives (NO_TOUCH) =====
bool VFD20T202HAL::_cmdInit() {
    // Function set: 8-bit, 2-line, 5x8 (0x38)
    if (!_writeCmd(0x38)) return false;
    // Display on, cursor off, blink off (0x0C)
    if (!_writeCmd(0x0C)) return false;
    // Clear display (0x01)
    if (!_cmdClear()) return false;
    // Entry mode: increment, no shift (0x06)
    if (!_writeCmd(0x06)) return false;
    return true;
}

bool VFD20T202HAL::_escReset() {
    return _cmdInit();
}

bool VFD20T202HAL::_cmdClear() {
    return _writeCmd(0x01);
}

bool VFD20T202HAL::_cmdHome() {
    return _writeCmd(0x02);
}

bool VFD20T202HAL::_posLinear(uint8_t addr) {
    return _writeCmd((uint8_t)(0x80 | (addr & 0x7F)));
}

bool VFD20T202HAL::_posRowCol(uint8_t row, uint8_t col) {
    const uint8_t base[] = { 0x00, 0x40 };
    if (row >= sizeof(base)) return false;
    const uint8_t addr = (uint8_t)(base[row] + col);
    return _posLinear(addr);
}

bool VFD20T202HAL::_escMode(uint8_t mode) {
    (void)mode; return false;
}

bool VFD20T202HAL::_escDimming(uint8_t level) { (void)level; return false; }

bool VFD20T202HAL::_escCursorBlink(uint8_t rate) { (void)rate; return false; }

// ===== NO_TOUCH: Bus helpers =====
bool VFD20T202HAL::_writeCmd(uint8_t cmd) {
    // If the transport supports control lines, drive RS=0 then pulse E, else just write the byte
    if (_transport && _transport->supportsControlLines()) {
        (void)_transport->setControlLine("RS", false);
        if (!_transport->write(&cmd, 1)) return false;
        (void)_transport->pulseControlLine("E", 1);
        return true;
    }
    return _transport ? _transport->write(&cmd, 1) : false;
}

bool VFD20T202HAL::_writeData(const uint8_t* data, size_t len) {
    if (!_transport || !data || len == 0) return false;
    if (_transport->supportsControlLines()) {
        (void)_transport->setControlLine("RS", true);
        bool ok = _transport->write(data, len);
        (void)_transport->pulseControlLine("E", 1);
        return ok;
    }
    return _transport->write(data, len);
}
