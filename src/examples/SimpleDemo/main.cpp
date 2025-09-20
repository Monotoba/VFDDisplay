// File: src/examples/SimpleDemo/src/main.cpp
#include <Arduino.h>
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

// Global serial reference
HardwareSerial& VFD_SERIAL = Serial1;

// Pointers instead of unique_ptr
IVFDHAL* vfdHAL = nullptr;
ITransport* transport = nullptr;
VFDDisplay* vfd = nullptr;

void setup() {
    // Initialize Serial1 for the VFD
    VFD_SERIAL.begin(19200, SERIAL_8N2);

    // Allocate objects
    vfdHAL = new VFD20S401HAL();
    transport = new SerialTransport(&VFD_SERIAL);
    vfd = new VFDDisplay(vfdHAL, transport);

    // Optionally start Serial for debug
    Serial.begin(57600);

    if (!vfd->init()) {
        Serial.println("VFD init failed!");
        return;
    }

    //vfd->init();
    //vfd->reset();
    vfd->cursorHome();
    
    // Test regular writing
    const char* str = "hello, VFD!";
    vfd->write(str);
    
    delay(2000); // Wait 2 seconds
    
    // Test centerText functionality
    vfd->clear();
    vfd->centerText("Centered", 0);  // Center on row 0
    vfd->centerText("Text Demo", 1); // Center on row 1
    
    delay(3000); // Wait 3 seconds
    
    // Test escape sequence functionality
    vfd->clear();
    
    // Example escape sequences (these would be device-specific)
    // Send ESC followed by some example command bytes
    uint8_t escData1[] = {0x40, 0x00}; // Example: ESC 0x40 (might be display on)
    uint8_t escData2[] = {0x45, 0x01, 0x00}; // Example: ESC 0x45 0x01
    uint8_t escData3[] = {0x48, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00}; // Max 8 bytes
    
    Serial.println("Testing escape sequences...");
    
    // Send some example escape sequences
    if (vfd->sendEscapeSequence(escData1)) {
        Serial.println("Escape sequence 1 sent successfully");
    }
    delay(500);
    
    if (vfd->sendEscapeSequence(escData2)) {
        Serial.println("Escape sequence 2 sent successfully");
    }
    delay(500);
    
    if (vfd->sendEscapeSequence(escData3)) {
        Serial.println("Escape sequence 3 sent successfully");
    }
    delay(500);
    
    // Write some text after escape sequences
    vfd->cursorHome();
    vfd->write("After ESC");
}

void loop() {
    // Demonstrate centerText functionality
    static int loopCount = 0;
    
    if (loopCount % 3 == 0) {
        // Centered text demo
        vfd->clear();
        vfd->centerText("Centered Text", 0);
        vfd->centerText("Row 1", 1);
    } else if (loopCount % 3 == 1) {
        // Regular text demo
        vfd->clear();
        vfd->cursorHome();
        vfd->write("Regular Text");
        vfd->setCursorPos(1, 0);
        vfd->write("Also Row 1");
    } else {
        // Escape sequence demo
        vfd->clear();
        vfd->cursorHome();
        vfd->write("ESC Demo");
        
        // Send a simple escape sequence (example)
        uint8_t escData[] = {0x40, 0x00}; // ESC 0x40
        vfd->sendEscapeSequence(escData);
        
        delay(500);
        vfd->setCursorPos(1, 0);
        vfd->write("After ESC");
    }
    
    loopCount++;
    delay(2000);
}

