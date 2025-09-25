// Arduino IDE wrapper and implementation for CustomCharsAdvanced
// Demonstrates defining up to 16 custom characters and using writeCustomChar()

#include <Arduino.h>
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

HardwareSerial& VFD_SERIAL = Serial1;

IVFDHAL* vfdHAL = nullptr;
ITransport* transport = nullptr;
VFDDisplay* vfd = nullptr;

static void makeBarGlyph(uint8_t height, uint8_t totalRows, uint8_t patternOut[8]) {
    // Clear
    for (uint8_t r = 0; r < 8; ++r) patternOut[r] = 0;
    if (totalRows == 0) return;
    // Fill bottom-up bar across all 5 columns for 'height' rows
    for (int r = totalRows - 1; r >= 0 && height > 0; --r) {
        patternOut[r] = 0b11111;
        height--;
    }
}

void setup() {
    Serial.begin(57600);
    VFD_SERIAL.begin(19200, SERIAL_8N2);

    vfdHAL = new VFD20S401HAL();
    transport = new SerialTransport(&VFD_SERIAL);
    vfd = new VFDDisplay(vfdHAL, transport);

    if (!vfd->init()) {
        Serial.println("VFD init failed");
        return;
    }

    vfd->reset();
    vfd->clear();
    vfd->cursorHome();

    const IDisplayCapabilities* caps = vfd->getDisplayCapabilities();
    uint8_t H = caps ? caps->getCharacterPixelHeight() : 8; // typical 8
    uint8_t maxUDF = caps ? caps->getMaxUserDefinedCharacters() : 0;

    if (!caps || maxUDF == 0) {
        vfd->write("No UDF support");
        return;
    }

    // Define up to 16 glyphs as bars of varying heights
    uint8_t total = maxUDF;
    if (total > 16) total = 16;
    for (uint8_t i = 0; i < total; ++i) {
        uint8_t patt[8];
        uint8_t barH = (i % H) + 1; // vary 1..H
        makeBarGlyph(barH, H > 8 ? 8 : H, patt); // we provide up to 8 rows; HAL ignores extra rows on 5x7
        vfd->setCustomChar(i, patt);
    }

    vfd->centerText("Custom Chars (16)", 0);

    // Show indices 0..7 on row 1
    vfd->setCursorPos(1, 0);
    for (uint8_t i = 0; i < total && i < 8 && i < 20; ++i) {
        vfd->writeCustomChar(i);
        vfd->write(" ");
    }

    // Show indices 8..15 on row 2 (mapped to safe codes)
    if (total > 8) {
        vfd->setCursorPos(2, 0);
        for (uint8_t i = 8; i < total && i < 16 && (i - 8) * 2 < 20; ++i) {
            vfd->writeCustomChar(i);
            vfd->write(" ");
        }
    }

    // Print mapping to Serial for verification
    Serial.println("Logical index -> device code mapping:");
    for (uint8_t i = 0; i < total; ++i) {
        uint8_t code;
        if (vfd->getCustomCharCode(i, code)) {
            Serial.print("Index "); Serial.print(i); Serial.print(" => 0x");
            if (code < 16) Serial.print('0');
            Serial.print(code, HEX);
            Serial.println();
        }
    }
}

void loop() {
    // No animation here
}

