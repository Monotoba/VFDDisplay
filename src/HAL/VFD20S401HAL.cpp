#include "VFD20S401HAL.h"
#include "../Transports/ITransport.h"
#include "../Capabilities/CapabilitiesRegistry.h"
#include <Arduino.h>
#include <string.h>

static constexpr uint8_t ESC_CHAR = 0x1B;

// Constructor
VFD20S401HAL::VFD20S401HAL() : _transport(nullptr) {
    // Create and register capabilities
    _capabilities = CapabilitiesRegistry::createVFD20S401Capabilities();
    CapabilitiesRegistry::getInstance().registerCapabilities(_capabilities);
    // Initialize scroll buffers
    _vScrollOffset = 0;
    _vScrollText[0] = '\0';
    _vScrollTotalLines = 0;
    _vScrollStartRow = 0;
    _hScrollOffset = 0;
    _hScrollRow = 0;
    _hScrollText[0] = '\0';
}


// ==========================================================
// ==================== Required Methods ====================
// ==========================================================

// --- Transport injection ---
void VFD20S401HAL::setTransport(ITransport* transport) {
    _transport = transport;
}

// --- Lifecycle ---
bool VFD20S401HAL::init() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    bool ok = _cmdInit();
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::reset() {
    bool ok = _escReset();
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

// --- Screen control ---
bool VFD20S401HAL::clear() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    bool ok = _cmdClear();
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::cursorHome() {
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    bool ok = _cmdHome();
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::setCursorPos(uint8_t row, uint8_t col) {
    // Datasheet: ESC 'H' followed by linear address 0x00..0x4F (row-major for 4x20)
    if (!_transport) { _lastError = VFDError::TransportFail; return false; }
    if (row >= 4 || col >= 20) { _lastError = VFDError::InvalidArgs; return false; }
    bool ok = _posRowCol(row, col);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::setCursorBlinkRate(uint8_t rate_ms) {
    // TODO: implement blink-rate command (not supported in this HAL)
    (void)rate_ms;
    _lastError = VFDError::NotSupported;
    return false;
}

// Cursor movement convenience methods (wrapper around writeChar)
bool VFD20S401HAL::backSpace() {
    // Backspace: move cursor one position left (ASCII 0x08)
    bool ok = writeChar(0x08);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::hTab() {
    // Horizontal tab: move cursor to next tab stop (ASCII 0x09)
    bool ok = writeChar(0x09);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::lineFeed() {
    // Line feed: move cursor to next line (ASCII 0x0A)
    bool ok = writeChar(0x0A);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::carriageReturn() {
    // Carriage return: move cursor to beginning of current line (ASCII 0x0D)
    bool ok = writeChar(0x0D);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

// --- Writing ---
bool VFD20S401HAL::writeChar(char c) {
    if (!_transport) return false;
    bool ok = _transport->write(reinterpret_cast<const uint8_t*>(&c), 1);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::write(const char* msg) {
    if (!_transport || !msg) { _lastError = VFDError::InvalidArgs; return false; }
    size_t len = strlen(msg);
    bool ok = _transport->write(reinterpret_cast<const uint8_t*>(msg), len);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::centerText(const char* str, uint8_t row) {
    if (!_transport || !str || !_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    
    // Get display dimensions from capabilities
    uint8_t textColumns = _capabilities->getTextColumns();
    uint8_t textRows = _capabilities->getTextRows();
    
    // Validate row parameter
    if (row >= textRows) { _lastError = VFDError::InvalidArgs; return false; }
    
    // Calculate string length
    size_t strLen = strlen(str);
    
    // If string is longer than display width, just write it normally
    if (strLen >= textColumns) {
        bool ok = setCursorPos(row, 0) && write(str);
        _lastError = ok ? VFDError::Ok : lastError();
        return ok;
    }
    
    // Calculate padding for centering
    uint8_t totalPadding = textColumns - strLen;
    uint8_t leftPadding = totalPadding / 2;
    uint8_t rightPadding = totalPadding - leftPadding;
    
    // Set cursor to beginning of specified row
    if (!setCursorPos(row, 0)) { return false; }
    
    // Write left padding (spaces)
    for (uint8_t i = 0; i < leftPadding; i++) {
        if (!writeChar(' ')) { return false; }
    }
    
    // Write the centered text
    if (!write(str)) { return false; }
    
    // Write right padding (spaces) - optional, for completeness
    for (uint8_t i = 0; i < rightPadding; i++) {
        if (!writeChar(' ')) { return false; }
    }
    
    _lastError = VFDError::Ok;
    return true;
}

bool VFD20S401HAL::writeCustomChar(uint8_t index) {
    if (!_transport || !_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    uint8_t maxUdf = _capabilities->getMaxUserDefinedCharacters();
    if (maxUdf == 0 || index >= maxUdf) { _lastError = VFDError::InvalidArgs; return false; }
    uint8_t chrCode = 0;
    if (!_mapIndexToCHR(index, chrCode)) { _lastError = VFDError::InvalidArgs; return false; }
    bool ok = writeChar((char)chrCode);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::getCustomCharCode(uint8_t index, uint8_t& codeOut) const {
    if (!_capabilities) return false;
    uint8_t maxUdf = _capabilities->getMaxUserDefinedCharacters();
    if (maxUdf == 0 || index >= maxUdf) return false;
    // Use the same mapping used by set/write to ensure consistency
    return _mapIndexToCHR(index, codeOut);
}

// --- Features ---
bool VFD20S401HAL::setBrightness(uint8_t lumens) {
    // TODO: map lumens -> brightness command
    (void)lumens;
    _lastError = VFDError::NotSupported;
    return false;
}

bool VFD20S401HAL::saveCustomChar(uint8_t index, const uint8_t* pattern) {
    // Backward-compatible alias for setCustomChar
    return setCustomChar(index, pattern);
}

bool VFD20S401HAL::setCustomChar(uint8_t index, const uint8_t* pattern) {
    // Datasheet 20S401DA1 5.2.16 [1] UDF: ESC 'C' + CHR + PT1..PT5 (5 bytes bit-packed)
    if (!_transport || !_capabilities || !pattern) { _lastError = VFDError::InvalidArgs; return false; }
    if (!_capabilities->hasCapability(CAP_USER_DEFINED_CHARS)) { _lastError = VFDError::NotSupported; return false; }
    uint8_t maxUdf = _capabilities->getMaxUserDefinedCharacters();
    if (maxUdf == 0 || index >= maxUdf) { _lastError = VFDError::InvalidArgs; return false; }

    // Map logical index to CHR code that is safe to use for this controller
    uint8_t chrCode = 0;
    if (!_mapIndexToCHR(index, chrCode)) { _lastError = VFDError::InvalidArgs; return false; }

    // Pack 8x5 row pattern (rows 0..6 used) into 5 bytes per Table 12.1
    uint8_t packed[5] = {0,0,0,0,0};
    _pack5x7ToBytes(pattern, packed);

    const uint8_t data[] = { 0x43, chrCode, packed[0], packed[1], packed[2], packed[3], packed[4] };
    bool ok = sendEscSequence(data, sizeof(data));
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

// ===== NO_TOUCH: Low-level ESC sender (null-terminated variant) =====
bool VFD20S401HAL::sendEscapeSequence(const uint8_t* data) {
    if (!_transport || !data) return false;
    
    // Send ESC character (0x1B)
    uint8_t esc = 0x1B;
    if (!_transport->write(&esc, 1)) return false;
    
    // Send data bytes until we find a zero or reach 8 bytes
    uint8_t byteCount = 0;
    while (byteCount < 8 && data[byteCount] != 0) {
        if (!_transport->write(&data[byteCount], 1)) return false;
        byteCount++;
    }
    
    return true;
}


bool VFD20S401HAL::setDisplayMode(uint8_t mode) {
    // Validate mode range (0x11-0x17)
    if (mode < 0x11 || mode > 0x17) return false;
    bool ok = _escMode(mode);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::setDimming(uint8_t level) {
    // Send escape sequence for dimming: ESC (0x1B) followed by 0x4C and level
    // Based on VFD20S401 datasheet - ESC + 0x4C for dimming control
    bool ok = _escDimming(level);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::cursorBlinkSpeed(uint8_t rate) {
    // Send escape sequence for cursor blink speed: ESC (0x1B) followed by blink command and rate
    // Based on VFD20S401 datasheet - using ESC + cursor control command
    // Rate parameter controls blink speed (0 = no blink, 1-255 = blink rates)
    bool ok = _escCursorBlink(rate);
    _lastError = ok ? VFDError::Ok : VFDError::TransportFail;
    return ok;
}

bool VFD20S401HAL::changeCharSet(uint8_t setId) {
    // VFD20S401 character set selection using single byte commands
    // 0x18 = CT0 (Character Table 0) - Standard ASCII
    // 0x19 = CT1 (Character Table 1) - Extended/Custom characters
    
    if (setId == 0) {
        // Select CT0 - Standard character set
        return writeChar(0x18);
    } else if (setId == 1) {
        // Select CT1 - Extended character set
        return writeChar(0x19);
    } else {
        // Invalid character set ID
        return false;
    }
}

// Enhanced positioning methods for 4x20 display
bool VFD20S401HAL::writeCharAt(uint8_t row, uint8_t column, char c) {
    if (!moveTo(row, column)) return false;
    return writeChar(c);
}

bool VFD20S401HAL::moveTo(uint8_t row, uint8_t column) {
    return setCursorPos(row, column);
}

bool VFD20S401HAL::writeAt(uint8_t row, uint8_t column, const char* text) {
    if (!text) return false;
    if (!moveTo(row, column)) return false;
    return write(text);
}

// ===== Device-specific primitives =====
bool VFD20S401HAL::_cmdInit() {
    uint8_t cmd = 0x49;
    return _transport->write(&cmd, 1);
}

bool VFD20S401HAL::_escReset() {
    const uint8_t data[] = { 0x49 }; // 'I'
    return sendEscSequence(data, sizeof(data));
}

bool VFD20S401HAL::_cmdClear() {
    uint8_t cmd = 0x09;
    return _transport->write(&cmd, 1);
}

bool VFD20S401HAL::_cmdHome() {
    uint8_t cmd = 0x0C;
    return _transport->write(&cmd, 1);
}

bool VFD20S401HAL::_posLinear(uint8_t addr) {
    const uint8_t data[] = { 0x48, addr };
    return sendEscSequence(data, sizeof(data));
}

bool VFD20S401HAL::_posRowCol(uint8_t row, uint8_t col) {
    const uint8_t addr = (uint8_t)(row * 20 + col);
    return _posLinear(addr);
}

bool VFD20S401HAL::_escMode(uint8_t mode) {
    const uint8_t data[] = { mode };
    return sendEscSequence(data, sizeof(data));
}

bool VFD20S401HAL::_escDimming(uint8_t level) {
    const uint8_t data[] = { 0x4C, level };
    return sendEscSequence(data, sizeof(data));
}

bool VFD20S401HAL::_escCursorBlink(uint8_t rate) {
    const uint8_t data[] = { 0x42, rate };
    return sendEscSequence(data, sizeof(data));
}

// --- Scrolling ---
bool VFD20S401HAL::hScroll(const char* str, int dir, uint8_t row) {
    if (!_transport || !_capabilities || !str) { _lastError = VFDError::InvalidArgs; return false; }
    uint8_t cols = _capabilities->getTextColumns();
    if (row >= _capabilities->getTextRows()) { _lastError = VFDError::InvalidArgs; return false; }

    // If text changed or row changed, reset state
    if (strncmp(str, _hScrollText, sizeof(_hScrollText)) != 0 || _hScrollRow != row) {
        size_t len = strlen(str);
        if (len >= sizeof(_hScrollText)) len = sizeof(_hScrollText) - 1;
        memcpy(_hScrollText, str, len);
        _hScrollText[len] = '\0';
        _hScrollOffset = 0;
        _hScrollRow = row;
    }

    // Advance offset based on direction
    int textLen = (int)strlen(_hScrollText);
    if (textLen == 0) { bool ok = setCursorPos(row, 0) && write(""); _lastError = ok ? VFDError::Ok : lastError(); return ok; }
    if (dir > 0) {
        _hScrollOffset = (_hScrollOffset + 1) % (textLen + cols);
    } else if (dir < 0) {
        _hScrollOffset = (_hScrollOffset - 1);
        if (_hScrollOffset < 0) _hScrollOffset = textLen + cols - 1;
    }

    // Build visible window of width 'cols'
    char window[40]; // 20 columns max supported by capabilities
    if (cols > sizeof(window) - 1) cols = sizeof(window) - 1;
    for (uint8_t i = 0; i < cols; ++i) {
        int idx = _hScrollOffset + i;
        char c = ' ';
        if (idx < textLen) c = _hScrollText[idx];
        else if (idx < textLen + cols) c = ' ';
        else {
            int wrap = idx - (textLen + cols);
            if (wrap >= 0 && wrap < textLen) c = _hScrollText[wrap];
        }
        window[i] = c;
    }
    window[cols] = '\0';

    bool ok = writeAt(row, 0, window);
    _lastError = ok ? VFDError::Ok : lastError();
    return ok;
}

bool VFD20S401HAL::vScroll(const char* str, int dir) {
    // Convenience wrapper: scroll multi-line text starting at row 0
    ScrollDirection d = (dir > 0) ? SCROLL_DOWN : SCROLL_UP;
    bool ok = vScrollText(str, 0, d);
    _lastError = ok ? VFDError::Ok : lastError();
    return ok;
}

bool VFD20S401HAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) {
    if (!_transport || !text || !_capabilities) { _lastError = VFDError::InvalidArgs; return false; }
    
    // Get display dimensions from capabilities
    uint8_t textRows = _capabilities->getTextRows();
    uint8_t textColumns = _capabilities->getTextColumns();
    
    // Validate parameters
    if (startRow >= textRows) { _lastError = VFDError::InvalidArgs; return false; }
    
    // Store the new text if it differs from current scroll text
    if (strcmp(text, _vScrollText) != 0) {
        // Copy new text to internal buffer (truncate if necessary)
        size_t textLen = strlen(text);
        if (textLen >= sizeof(_vScrollText) - 1) textLen = sizeof(_vScrollText) - 1;
        memcpy(_vScrollText, text, textLen);
        _vScrollText[textLen] = '\0';
        _vScrollOffset = 0; // Reset scroll position for new text
        _vScrollStartRow = startRow;
        
        // Count total lines in the text
        _vScrollTotalLines = 1; // Start with 1 (at least one line)
        for (size_t i = 0; i < textLen; i++) {
            if (_vScrollText[i] == '\n') _vScrollTotalLines++;
        }
    }
    
    // Update scroll offset based on direction
    if (direction == SCROLL_DOWN) {
        _vScrollOffset++;
        if (_vScrollOffset >= _vScrollTotalLines) _vScrollOffset = 0; // Wrap around
    } else if (direction == SCROLL_UP) {
        _vScrollOffset--;
        if (_vScrollOffset < 0) _vScrollOffset = _vScrollTotalLines - 1; // Wrap around
    } else {
        return false; // Invalid direction
    }
    
    // Calculate visible window
    uint8_t visibleRows = textRows - startRow; // Number of rows available for scrolling
    if (visibleRows == 0) return false;
    
    // Parse text into lines and display visible portion
    for (uint8_t r = 0; r < visibleRows; r++) {
        // Calculate which line to display (with wrapping)
        uint8_t displayLine = (_vScrollOffset + r) % _vScrollTotalLines;

        // Find the start of the display line
        const char* displayLineStart = _vScrollText;
        uint8_t lineCount = 0;
        while (lineCount < displayLine && *displayLineStart) {
            if (*displayLineStart == '\n') lineCount++;
            displayLineStart++;
        }

        // Find the end of this line or max column width
        const char* p = displayLineStart;
        uint8_t count = 0;
        while (*p && *p != '\n' && count < textColumns) { p++; count++; }

        // Build a fixed-width line buffer
        char lineBuf[40];
        if (textColumns > sizeof(lineBuf) - 1) textColumns = sizeof(lineBuf) - 1;
        uint8_t i = 0;
        for (; i < count; ++i) lineBuf[i] = displayLineStart[i];
        for (; i < textColumns; ++i) lineBuf[i] = ' ';
        lineBuf[textColumns] = '\0';

        // Write the line at the appropriate row
        uint8_t writeRow = startRow + r;
        if (!writeAt(writeRow, 0, lineBuf)) { return false; }
    }
    _lastError = VFDError::Ok;
    return true;
}

// --- Flash text ---
bool VFD20S401HAL::flashText(const char* str, uint8_t row, uint8_t col,
                             uint8_t on_ms, uint8_t off_ms) {
    if (!_transport || !str) { _lastError = VFDError::InvalidArgs; return false; }
    if (!writeAt(row, col, str)) return false;
    delayMicroseconds((unsigned int)on_ms * 1000U);
    // overwrite with spaces
    uint8_t len = strlen(str);
    if (len > 20) len = 20; // clamp to row width
    char spaces[21];
    for (uint8_t i = 0; i < len; ++i) spaces[i] = ' ';
    spaces[len] = '\0';
    if (!writeAt(row, col, spaces)) return false;
    delayMicroseconds((unsigned int)off_ms * 1000U);
    _lastError = VFDError::Ok;
    return true;
}

// --- Capabilities and diagnostics ---
int VFD20S401HAL::getCapabilities() const {
    // TODO: return capability flags (bitmask)
    return 0;
}

const char* VFD20S401HAL::getDeviceName() const {
    return "VFD20S401";
}

const IDisplayCapabilities* VFD20S401HAL::getDisplayCapabilities() const {
    return _capabilities;
}

// --- Timing utility ---
void VFD20S401HAL::delayMicroseconds(unsigned int us) const {
    // Forward to global Arduino delay if available.
    ::delayMicroseconds(us);
}

// ==========================================================
// ===================== Utility Methods ====================
// ==========================================================

// Custom command: send ESC (0x1B) followed by up to 8 bytes
// ===== NO_TOUCH: Low-level ESC sender (length-aware) =====
bool VFD20S401HAL::sendEscSequence(const uint8_t* data, size_t len) {
    // check that the class transport is valid and input is sane
    if (!_transport || !data || len == 0 || len > 8) return false;

    uint8_t esc = 0x1B;

    // send the ESC byte
    if (!_transport->write(&esc, 1)) return false;

    // send the data bytes
    if (!_transport->write(data, len)) return false;

    return true;
}

// Star Wars style opening crawl - centered text scrolling from bottom to top
bool VFD20S401HAL::starWarsScroll(const char* text, uint8_t startRow) {
    if (!text || !_capabilities) return false;
    
    // Get display dimensions from capabilities
    uint8_t textRows = _capabilities->getTextRows();
    
    // Validate parameters
    if (startRow >= textRows) return false;
    
    // Use the existing vScrollText infrastructure but with centered formatting
    // The key difference is that we'll center each line before storing it
    
    // First, we need to format the text with centered lines
    char centeredText[256];
    if (!formatStarWarsText(text, centeredText, sizeof(centeredText))) {
        return false;
    }
    
    // Now use vScrollText with the formatted (centered) text
    // Use SCROLL_UP to create the bottom-to-top scrolling effect
    return vScrollText(centeredText, startRow, SCROLL_UP);
}

// Helper method to count lines in text
uint8_t VFD20S401HAL::countLines(const char* text) {
    if (!text) return 0;
    
    uint8_t count = 1; // Start with 1 (at least one line)
    for (size_t i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') count++;
    }
    return count;
}

// Helper method to center a single line of text
void VFD20S401HAL::centerTextLine(const char* line, char* output, uint8_t maxLen) {
    if (!line || !output || maxLen == 0) return;
    
    uint8_t lineLen = strlen(line);
    if (lineLen >= maxLen) {
        // Line too long, just copy what fits
        memcpy(output, line, maxLen - 1);
        output[maxLen - 1] = '\0';
        return;
    }
    
    uint8_t textColumns = _capabilities->getTextColumns();
    if (lineLen >= textColumns) {
        // Line longer than display width, just copy
        strcpy(output, line);
        return;
    }
    
    // Calculate padding for centering
    uint8_t totalPadding = textColumns - lineLen;
    uint8_t leftPadding = totalPadding / 2;
    uint8_t rightPadding = totalPadding - leftPadding;
    
    // Build centered line
    uint8_t pos = 0;
    
    // Left padding (spaces)
    for (uint8_t i = 0; i < leftPadding && pos < maxLen - 1; i++) {
        output[pos++] = ' ';
    }
    
    // Original text
    for (uint8_t i = 0; i < lineLen && pos < maxLen - 1; i++) {
        output[pos++] = line[i];
    }
    
    // Right padding (spaces)
    for (uint8_t i = 0; i < rightPadding && pos < maxLen - 1; i++) {
        output[pos++] = ' ';
    }
    
    output[pos] = '\0';
}

// Helper method to format text for Star Wars scroll (center each line)
bool VFD20S401HAL::formatStarWarsText(const char* input, char* output, size_t outputSize) {
    if (!input || !output || outputSize == 0) return false;
    
    output[0] = '\0'; // Start with empty string
    size_t outputPos = 0;
    
    const char* lineStart = input;
    const char* lineEnd = input;
    
    while (*lineStart != '\0' && outputPos < outputSize - 1) {
        // Find end of current line
        lineEnd = lineStart;
        while (*lineEnd != '\0' && *lineEnd != '\n') {
            lineEnd++;
        }
        
        // Extract this line
        uint8_t lineLen = lineEnd - lineStart;
        char line[64]; // Max line length
        if (lineLen >= sizeof(line)) lineLen = sizeof(line) - 1;
        memcpy(line, lineStart, lineLen);
        line[lineLen] = '\0';
        
        // Center this line
        char centeredLine[64];
        centerTextLine(line, centeredLine, sizeof(centeredLine));
        
        // Add to output with newline if not first line
        if (outputPos > 0) {
            if (outputPos < outputSize - 1) {
                output[outputPos++] = '\n';
            }
        }
        
        // Add centered line
        size_t centeredLen = strlen(centeredLine);
        if (outputPos + centeredLen < outputSize - 1) {
            strcpy(output + outputPos, centeredLine);
            outputPos += centeredLen;
        }
        
        // Move to next line
        if (*lineEnd == '\n') lineEnd++;
        lineStart = lineEnd;
    }
    
    output[outputPos] = '\0';
    return true;
}

// Pack 7x5 matrix from 8 rows x 5 bits (LSB=leftmost) into 5 bytes per datasheet Table 12.1
void VFD20S401HAL::_pack5x7ToBytes(const uint8_t* rowPattern8x5, uint8_t out5[5]) {
    if (!out5) return;
    out5[0] = out5[1] = out5[2] = out5[3] = out5[4] = 0;
    if (!rowPattern8x5) return;
    // Only top 7 rows used on 5x7; bits 0..4 per row left->right
    for (uint8_t r = 0; r < 7; ++r) {
        uint8_t row = rowPattern8x5[r] & 0x1F; // 5 columns
        for (uint8_t c = 0; c < 5; ++c) {
            if ((row >> c) & 0x01) {
                uint8_t p = (uint8_t)(r * 5 + c); // 0..34
                uint8_t b = (uint8_t)(p / 8);     // 0..4 selects PT1..PT5
                uint8_t bit = (uint8_t)(p % 8);   // bit within byte
                out5[b] |= (uint8_t)(1u << bit);
            }
        }
    }
}

bool VFD20S401HAL::_isUnsafeCHR(uint8_t chr) {
    // Avoid known single-byte commands and ESC for this controller.
    // Known commands used in this HAL/datasheet: 0x09 (Clear), 0x0C (Home),
    // 0x08 (BS), 0x09 (HT/Clear), 0x0A (LF), 0x0D (CR),
    // 0x11..0x17 (DC1..DC7 display/cursor modes), 0x18..0x19 (CT0/CT1),
    // 0x1B (ESC), 0x49 ('I' Initialize as single-byte for this module).
    if (chr == 0x08 || chr == 0x09 || chr == 0x0A || chr == 0x0C || chr == 0x0D || chr == 0x1B || chr == 0x49)
        return true;
    if (chr >= 0x11 && chr <= 0x19) return true;
    return false;
}

bool VFD20S401HAL::_mapIndexToCHR(uint8_t index, uint8_t& chrOut) {
    // Support up to 16 logical indices (0..15).
    if (index < 8) {
        chrOut = index; // preserve legacy mapping for first 8
    } else if (index < 16) {
        chrOut = (uint8_t)(0x80 + (index - 8));
    } else {
        return false;
    }
    if (_isUnsafeCHR(chrOut)) return false;
    return true;
}
