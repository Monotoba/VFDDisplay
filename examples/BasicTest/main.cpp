// Basic test example for VFD Display using VFDDisplay library
#include <Arduino.h>
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

// Global serial reference for VFD
HardwareSerial& VFD_SERIAL = Serial1;

// Pointers for VFD system
IVFDHAL* vfdHAL = nullptr;
ITransport* transport = nullptr;
VFDDisplay* vfd = nullptr;
uint8_t displayMode = 0;

void setup() {
    // Initialize debug serial
    Serial.begin(57600);
    delay(1000);
    Serial.println("=== VFD Display Library Test ===");
    Serial.println("Testing VFDDisplay library with writeAt() method");
    
    // Initialize VFD system using our library
    Serial.println("Creating VFD display objects...");
    vfdHAL = new VFD20S401HAL();
    transport = new SerialTransport(&VFD_SERIAL);
    vfd = new VFDDisplay(vfdHAL, transport);
    
    // Simple heartbeat to show program is running
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 2000) {
        Serial1.print(".");
        lastBlink = millis();
    }
    
    // Initialize VFD at 19200 baud, 8N2
    Serial.println("Initializing VFD at 19200 baud, 8N2...");
    VFD_SERIAL.begin(19200, SERIAL_8N2);
    delay(500);
    
    // Initialize the display
    Serial.println("Initializing VFD display...");
    if (!vfd->init()) {
        Serial.println("VFD init failed!");
        return;
    }
    
    Serial.println("VFD initialized successfully!");
    delay(1000);
    vfd->reset();
    vfd->clear();
}

void loop() {
    vfd->setDisplayMode(displayMode);
    
    // Test 1: Basic text writing using library methods
    Serial.println("Test 1: Basic text using library methods...");
    vfd->clear();
    vfd->cursorHome();
    vfd->write("HELLO VFD!");
    delay(1000);
    
    // Test 2: Using writeAt() for precise positioning
    vfd->reset();
    vfd->clear();
    Serial.println("Test 2: Using writeAt() for precise positioning...");
    delay(1000);
    vfd->clear();
    
    
    // Write to specific positions using writeAt() for strings
    vfd->writeAt(0, 0, "Row 0: TOP");        // Row 0, Column 0
    vfd->writeAt(1, 0, "Row 1: MIDDLE");     // Row 1, Column 0  
    vfd->writeAt(2, 0, "Row 2: MIDDLE");     // Row 2, Column 0
    vfd->writeAt(3, 0, "Row 3: BOTTOM");     // Row 3, Column 0
    
    delay(2000);
    
    // Test 3: Demonstrate library features
    Serial.println("Test 3: Demonstrating library features...");
    vfd->clear();
    
    // Center text using library
    vfd->centerText("Centered Text", 0);
    vfd->centerText("Using Library", 1);
    
    delay(1000);
    vfd->reset();
    vfd->clear();
    
    // Test cursor features
    vfd->setCursorPos(2, 5);
    vfd->write("Cursor at (2,5)");
    
    delay(1000);
    
    Serial.println("Library test complete!");
    Serial.println("The VFD should show text positioned using the VFDDisplay library.");
    Serial.println("This demonstrates proper library usage instead of direct serial commands.");
    
    Serial.print("Starting Ad");
    vfd->reset();
    vfd->clear();
    
    for (int i=0; i < 4; i++) {
      vfd->reset();
      vfd->clear();
      vfd->centerText("BIG SKY VCR", 0);
      vfd->centerText("406 256-2331", 1);
      vfd->centerText("www.bigskyvcr.com", 2);
      delay(750);
    }
    
    for (int i=0; i < 255; i+=50) {
      char buf[4];
      vfd->setBrightness(itoa(i, buf, 10));
      delay(500);
    }
    
    displayMode++;
}
