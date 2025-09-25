// Simple VFD Test - Diagnostic program to verify basic display functionality
#include <Arduino.h>

// Simple test without complex library dependencies
void setup() {
    // Initialize both serial ports
    Serial.begin(57600);     // Debug serial
    Serial1.begin(19200, SERIAL_8N2); // VFD serial
    
    delay(1000);
    Serial.println("=== Simple VFD Diagnostic Test ===");
    
    // Test 1: Send basic commands directly to VFD
    Serial.println("Test 1: Sending basic commands to VFD...");
    
    // Send ESC + 0x49 (reset command)
    Serial1.write(0x1B); // ESC
    Serial1.write(0x49); // Reset command
    delay(100);
    
    // Send clear display command (0x09)
    Serial1.write(0x09); // Clear display
    delay(100);
    
    // Send cursor home command (0x0C)
    Serial1.write(0x0C); // Cursor home
    delay(100);
    
    // Test 2: Send simple text
    Serial.println("Test 2: Sending simple text...");
    const char* testText = "HELLO VFD!";
    for (int i = 0; testText[i] != '\0'; i++) {
        Serial1.write(testText[i]);
        delay(50);
    }
    
    // Test 3: Send text to specific positions
    delay(1000);
    Serial.println("Test 3: Positioning text...");
    
    // Clear and home cursor
    Serial1.write(0x09); // Clear
    delay(100);
    Serial1.write(0x0C); // Home
    delay(100);
    
    // Send text to row 0
    Serial1.write("Row 0 Test");
    delay(200);
    
    // Send text to row 1 (ESC 'H' + 0x14)
    Serial1.write(0x1B); // ESC
    Serial1.write(0x48); // 'H' position command
    Serial1.write(0x14); // Row 1, col 0
    delay(100);
    Serial1.write("Row 1 Test");
    delay(200);
    
    // Send text to row 2 (ESC 'H' + 0x28)
    Serial1.write(0x1B); // ESC
    Serial1.write(0x48); // 'H' position command
    Serial1.write(0x28); // Row 2, col 0
    delay(100);
    Serial1.write("Row 2 Test");
    delay(200);
    
    // Send text to row 3 (ESC 'H' + 0x3C)
    Serial1.write(0x1B); // ESC
    Serial1.write(0x48); // 'H' position command
    Serial1.write(0x3C); // Row 3, col 0
    delay(100);
    Serial1.write("Row 3 Test");
    
    Serial.println("=== Basic tests complete ===");
    Serial.println("Check VFD display for output");
}

void loop() {
    // Simple heartbeat to show program is running
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 1000) {
        Serial.print(".");
        lastBlink = millis();
    }
}
