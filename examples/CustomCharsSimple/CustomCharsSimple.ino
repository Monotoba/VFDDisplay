// Arduino IDE wrapper and implementation for CustomCharsSimple
// Demonstrates defining a few custom characters and displaying them.

#include <Arduino.h>
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

HardwareSerial& VFD_SERIAL = Serial1;

IVFDHAL* vfdHAL = nullptr;
ITransport* transport = nullptr;
VFDDisplay* vfd = nullptr;

static void defineSimpleGlyphs() {
    // Row-major patterns: one byte per row, bits 0..4 = columns left->right
    // Provide 8 rows; on 5x7 devices, last row is ignored.
    uint8_t heart[8] = {
        0b00000,
        0b01010,
        0b11111,
        0b11111,
        0b01110,
        0b00100,
        0b00000,
        0b00000
    };

    uint8_t arrow[8] = {
        0b00100,
        0b01110,
        0b11111,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00000
    };

    uint8_t bell[8] = {
        0b00100,
        0b01110,
        0b01110,
        0b01110,
        0b01110,
        0b00100,
        0b00000,
        0b00000
    };

    vfd->setCustomChar(0, heart);
    vfd->setCustomChar(1, arrow);
    vfd->setCustomChar(2, bell);
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

    defineSimpleGlyphs();

    vfd->centerText("Custom Chars (Simple)", 0);
    vfd->setCursorPos(1, 0);
    vfd->write("Heart "); vfd->writeCustomChar(0);
    vfd->write("  Arrow "); vfd->writeCustomChar(1);
    vfd->write("  Bell "); vfd->writeCustomChar(2);

    // Show using the raw mapped code too
    uint8_t code0;
    if (vfd->getCustomCharCode(0, code0)) {
        vfd->setCursorPos(2, 0);
        vfd->write("Code for idx0: 0x");
        char buf[3];
        const char hex[] = "0123456789ABCDEF";
        buf[0] = hex[(code0 >> 4) & 0xF];
        buf[1] = hex[code0 & 0xF];
        buf[2] = 0;
        vfd->write(buf);
        vfd->write("  -> ");
        vfd->writeChar((char)code0);
    }
}

void loop() {
    // No animation here
}

