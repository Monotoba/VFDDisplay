#include "VFDSTV7710HAL.h"
#include "../Capabilities/CapabilitiesRegistry.h"

VFDSTV7710HAL::VFDSTV7710HAL() {
    _capabilities = CapabilitiesRegistry::createVFDSTV7710Capabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
}

bool VFDSTV7710HAL::init() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    // No standard text init; treat as ready
    _lastError = VFDError::Ok; return true;
}

bool VFDSTV7710HAL::reset() { return init(); }

bool VFDSTV7710HAL::clear() { _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::cursorHome() { _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::setCursorPos(uint8_t row, uint8_t col) { (void)row;(void)col; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::setCursorBlinkRate(uint8_t rate_ms) { (void)rate_ms; _lastError=VFDError::NotSupported; return false; }

bool VFDSTV7710HAL::writeCharAt(uint8_t row, uint8_t column, char c) { (void)row;(void)column;(void)c; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::writeAt(uint8_t row, uint8_t column, const char* text) { (void)row;(void)column;(void)text; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::moveTo(uint8_t row, uint8_t column) { (void)row;(void)column; _lastError=VFDError::NotSupported; return false; }

bool VFDSTV7710HAL::backSpace() { _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::hTab() { _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::lineFeed() { _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::carriageReturn() { _lastError=VFDError::NotSupported; return false; }

bool VFDSTV7710HAL::writeChar(char c) { (void)c; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::write(const char* msg) { (void)msg; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::centerText(const char* str, uint8_t row) { (void)str;(void)row; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::writeCustomChar(uint8_t index) { (void)index; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::getCustomCharCode(uint8_t index, uint8_t& codeOut) const { (void)index;(void)codeOut; return false; }

bool VFDSTV7710HAL::setBrightness(uint8_t lumens) { (void)lumens; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::saveCustomChar(uint8_t index, const uint8_t* pattern) { (void)index;(void)pattern; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::setCustomChar(uint8_t index, const uint8_t* pattern) { (void)index;(void)pattern; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::setDisplayMode(uint8_t mode) { (void)mode; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::setDimming(uint8_t level) { (void)level; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::cursorBlinkSpeed(uint8_t rate) { (void)rate; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::changeCharSet(uint8_t setId) { (void)setId; _lastError=VFDError::NotSupported; return false; }

bool VFDSTV7710HAL::sendEscapeSequence(const uint8_t* data) { (void)data; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::hScroll(const char* str, int dir, uint8_t row){ (void)str;(void)dir;(void)row; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::vScroll(const char* str, int dir){ (void)str;(void)dir; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction){ (void)text;(void)startRow;(void)direction; _lastError=VFDError::NotSupported; return false; }
bool VFDSTV7710HAL::starWarsScroll(const char* text, uint8_t startRow){ (void)text;(void)startRow; _lastError=VFDError::NotSupported; return false; }

bool VFDSTV7710HAL::flashText(const char* str, uint8_t row, uint8_t col, uint8_t on_ms, uint8_t off_ms){ (void)str;(void)row;(void)col;(void)on_ms;(void)off_ms; _lastError=VFDError::NotSupported; return false; }

int VFDSTV7710HAL::getCapabilities() const { return _capabilities?_capabilities->getAllCapabilities():0; }
const char* VFDSTV7710HAL::getDeviceName() const { return _capabilities?_capabilities->getDeviceName():"STV7710"; }

