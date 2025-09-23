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
    const char* str = "hello, VFD!";
    vfd->write(str);
}

void loop() {
    // Nothing yet
    Serial.print("Writing message...");
    Serial1.print("Test message");
    delay(1000);
}

