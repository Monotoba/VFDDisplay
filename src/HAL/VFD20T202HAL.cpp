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
    // If supported by module: ESC 'C' + CHR + PT1..PT5 similar to 20S401; otherwise NotSupported.
    (void)index; (void)pattern; _lastError = VFDError::NotSupported; return false;
}

bool VFD20T202HAL::setDisplayMode(uint8_t mode) {
    bool ok = _escMode(mode); _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFD20T202HAL::setDimming(uint8_t level) {
    bool ok = _escDimming(level); _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFD20T202HAL::cursorBlinkSpeed(uint8_t rate) {
    bool ok = _escCursorBlink(rate); _lastError = ok?VFDError::Ok:VFDError::TransportFail; return ok;
}

bool VFD20T202HAL::changeCharSet(uint8_t setId) {
    if (setId == 0) return writeChar(0x18); // CT0
    if (setId == 1) return writeChar(0x19); // CT1
    return false;
}

bool VFD20T202HAL::sendEscapeSequence(const uint8_t* data) {
    if (!_transport || !data) return false;
    uint8_t esc = ESC_CHAR;
    if (!_transport->write(&esc, 1)) return false;
    // send until zero or 8 bytes
    uint8_t n=0; while (n<8 && data[n]!=0){ if(!_transport->write(&data[n],1)) return false; n++; }
    return true;
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
    uint8_t cmd = 0x49; // tentative init
    return _transport->write(&cmd, 1);
}

bool VFD20T202HAL::_escReset() {
    const uint8_t data[] = { 0x49 }; // 'I'
    return sendEscapeSequence(data);
}

bool VFD20T202HAL::_cmdClear() {
    uint8_t cmd = 0x09;
    return _transport->write(&cmd, 1);
}

bool VFD20T202HAL::_cmdHome() {
    uint8_t cmd = 0x0C;
    return _transport->write(&cmd, 1);
}

bool VFD20T202HAL::_posLinear(uint8_t addr) {
    const uint8_t data[] = { 0x48, addr };
    return sendEscapeSequence(data);
}

bool VFD20T202HAL::_posRowCol(uint8_t row, uint8_t col) {
    const uint8_t cols = _capabilities ? _capabilities->getTextColumns() : 20;
    const uint8_t addr = (uint8_t)(row * cols + col);
    return _posLinear(addr);
}

bool VFD20T202HAL::_escMode(uint8_t mode) {
    const uint8_t data[] = { mode };
    return sendEscapeSequence(data);
}

bool VFD20T202HAL::_escDimming(uint8_t level) {
    const uint8_t data[] = { 0x4C, level };
    return sendEscapeSequence(data);
}

bool VFD20T202HAL::_escCursorBlink(uint8_t rate) {
    const uint8_t data[] = { 0x42, rate };
    return sendEscapeSequence(data);
}

