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
    if (!_transport) return false;
    
    uint8_t cmd = 0x49;
    _transport->write(&cmd, 1);
    return true;
}

bool VFD20S401HAL::reset() {
    // Send escape sequence for reset: ESC (0x1B) followed by 'I' (0x49)
    const uint8_t resetData[] = {0x49};
    return sendEscSequence(resetData, sizeof(resetData));
}

// --- Screen control ---
bool VFD20S401HAL::clear() {
    if (!_transport) return false;
    
    // Futaba VFD20S401 clear command
    uint8_t cmd = 0x09;
    _transport->write(&cmd, 1);
    return true;
}

bool VFD20S401HAL::cursorHome() {
    if (!_transport) return false;
    
    uint8_t cmd = 0x0C;
    _transport->write(&cmd, 1);
    return true;
}

bool VFD20S401HAL::setCursorPos(uint8_t row, uint8_t col) {
    // Set cursor using ESC + DDRAM address (row base + column)
    // Row bases for 4x20: 0x00, 0x20, 0x40, 0x60
    if (row >= 4 || col >= 20) return false;

    uint8_t base;
    switch (row) {
        case 0: base = 0x00; break;
        case 1: base = 0x20; break;
        case 2: base = 0x40; break;
        case 3: base = 0x60; break;
        default: return false;
    }

    const uint8_t addr = (uint8_t)(base + col);
    const uint8_t escData[] = { addr };
    return sendEscSequence(escData, sizeof(escData));
}

bool VFD20S401HAL::setCursorBlinkRate(uint8_t rate_ms) {
    // TODO: implement blink-rate command
    (void)rate_ms;
    return false;
}

// Cursor movement convenience methods (wrapper around writeChar)
bool VFD20S401HAL::backSpace() {
    // Backspace: move cursor one position left (ASCII 0x08)
    return writeChar(0x08);
}

bool VFD20S401HAL::hTab() {
    // Horizontal tab: move cursor to next tab stop (ASCII 0x09)
    return writeChar(0x09);
}

bool VFD20S401HAL::lineFeed() {
    // Line feed: move cursor to next line (ASCII 0x0A)
    return writeChar(0x0A);
}

bool VFD20S401HAL::carriageReturn() {
    // Carriage return: move cursor to beginning of current line (ASCII 0x0D)
    return writeChar(0x0D);
}

// --- Writing ---
bool VFD20S401HAL::writeChar(char c) {
    if (!_transport) return false;
    return _transport->write(reinterpret_cast<const uint8_t*>(&c), 1);
}

bool VFD20S401HAL::write(const char* msg) {
    if (!_transport || !msg) return false;
    size_t len = strlen(msg);
    return _transport->write(reinterpret_cast<const uint8_t*>(msg), len);
}

bool VFD20S401HAL::centerText(const char* str, uint8_t row) {
    if (!_transport || !str || !_capabilities) return false;
    
    // Get display dimensions from capabilities
    uint8_t textColumns = _capabilities->getTextColumns();
    uint8_t textRows = _capabilities->getTextRows();
    
    // Validate row parameter
    if (row >= textRows) return false;
    
    // Calculate string length
    size_t strLen = strlen(str);
    
    // If string is longer than display width, just write it normally
    if (strLen >= textColumns) {
        return setCursorPos(row, 0) && write(str);
    }
    
    // Calculate padding for centering
    uint8_t totalPadding = textColumns - strLen;
    uint8_t leftPadding = totalPadding / 2;
    uint8_t rightPadding = totalPadding - leftPadding;
    
    // Set cursor to beginning of specified row
    if (!setCursorPos(row, 0)) return false;
    
    // Write left padding (spaces)
    for (uint8_t i = 0; i < leftPadding; i++) {
        if (!writeChar(' ')) return false;
    }
    
    // Write the centered text
    if (!write(str)) return false;
    
    // Write right padding (spaces) - optional, for completeness
    for (uint8_t i = 0; i < rightPadding; i++) {
        if (!writeChar(' ')) return false;
    }
    
    return true;
}

// --- Features ---
bool VFD20S401HAL::setBrightness(uint8_t lumens) {
    // TODO: map lumens -> brightness command
    (void)lumens;
    return false;
}

bool VFD20S401HAL::saveCustomChar(uint8_t index, const uint8_t* pattern) {
    // TODO: implement custom char save
    (void)index; (void)pattern;
    return false;
}

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
    
    // Create escape sequence: ESC followed by mode byte and terminator
    const uint8_t escData[] = { mode };
    return sendEscSequence(escData, sizeof(escData));
}

bool VFD20S401HAL::setDimming(uint8_t level) {
    // Send escape sequence for dimming: ESC (0x1B) followed by 0x4C and level
    // Based on VFD20S401 datasheet - ESC + 0x4C for dimming control
    const uint8_t dimmingData[] = { 0x4C, level }; // 0x4C is the dimming command
    return sendEscSequence(dimmingData, sizeof(dimmingData));
}

bool VFD20S401HAL::cursorBlinkSpeed(uint8_t rate) {
    // Send escape sequence for cursor blink speed: ESC (0x1B) followed by blink command and rate
    // Based on VFD20S401 datasheet - using ESC + cursor control command
    // Rate parameter controls blink speed (0 = no blink, 1-255 = blink rates)
    const uint8_t blinkData[] = { 0x42, rate }; // 0x42 is cursor blink control command
    return sendEscSequence(blinkData, sizeof(blinkData));
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

// --- Scrolling ---
bool VFD20S401HAL::hScroll(const char* str, int dir, uint8_t row) {
    // TODO: implement horizontal scroll
    (void)str; (void)dir; (void)row;
    return false;
}

bool VFD20S401HAL::vScroll(const char* str, int dir) {
    // TODO: implement vertical scroll
    (void)str; (void)dir;
    return false;
}

bool VFD20S401HAL::vScrollText(const char* text, uint8_t startRow, ScrollDirection direction) {
    if (!_transport || !text || !_capabilities) return false;
    
    // Get display dimensions from capabilities
    uint8_t textRows = _capabilities->getTextRows();
    uint8_t textColumns = _capabilities->getTextColumns();
    
    // Validate parameters
    if (startRow >= textRows) return false;
    
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
    for (uint8_t row = 0; row < visibleRows; row++) {
        // Calculate which line to display (with wrapping)
        uint8_t displayLine = (_vScrollOffset + row) % _vScrollTotalLines;
        
        // Find the start of the display line
        const char* displayLineStart = _vScrollText;
        uint8_t lineCount = 0;
        
        while (lineCount < displayLine && *displayLineStart) {
            if (*displayLineStart == '\n') lineCount++;
            displayLineStart++;
        }
        
        // Find the end of this line or max column width
        const char* lineEnd = displayLineStart;
        uint8_t col = 0;
        while (*lineEnd && *lineEnd != '\n' && col < textColumns) {
            lineEnd++;
            col++;
        }
        
        // Write this line to the display using writeAt()
        uint8_t writeRow = startRow + row;
        for (uint8_t col = 0; col < textColumns; col++) {
            char c = ' '; // Default to space
            if (col < (lineEnd - displayLineStart)) {
                c = displayLineStart[col];
            }
            char buf[2] = {' ', '\0'};
            if (!writeAt(writeRow, col, buf)) return false;
        }
    }
    
    return true;
}

// --- Flash text ---
bool VFD20S401HAL::flashText(const char* str, uint8_t row, uint8_t col,
                             uint8_t on_ms, uint8_t off_ms) {
    // TODO: implement flash text
    (void)str; (void)row; (void)col; (void)on_ms; (void)off_ms;
    return false;
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
