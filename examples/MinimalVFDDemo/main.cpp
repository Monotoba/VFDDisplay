// Minimal VFD Demo - Direct serial commands to test basic functionality
#include <Arduino.h>

void setup() {
    // Initialize both serial ports
    Serial.begin(57600);     // Debug serial
    Serial1.begin(19200, SERIAL_8N2); // VFD serial
    
    delay(1000);
    Serial.println("=== Minimal VFD Test ===");
    
    // Send basic commands directly to VFD
    Serial.println("Sending commands to VFD...");
    
    // Reset sequence: ESC + 0x49
    Serial1.write(0x1B); // ESC
    Serial1.write(0x49); // Reset command
    delay(200);
    
    // Clear display: 0x09
    Serial1.write(0x09); // Clear display
    delay(200);
    
    // Home cursor: 0x0C
    Serial1.write(0x0C); // Cursor home
    delay(200);
    
    // Write simple text
    const char* text1 = "HELLO VFD!";
    for (int i = 0; text1[i] != '\0'; i++) {
        Serial1.write(text1[i]);
        delay(50);
    }
    delay(1000);
    
    // Test cursor positioning (ESC 'H' + linear address)
    // Move to row 1, column 0: address 0x14 for 20x4
    Serial1.write(0x1B); // ESC
    Serial1.write(0x48); // 'H' = position command
    Serial1.write(0x14); // Row 1, Col 0
    delay(100);
    
    const char* text2 = "Row 1 Test";
    for (int i = 0; text2[i] != '\0'; i++) {
        Serial1.write(text2[i]);
        delay(50);
    }
    delay(1000);
    
    // Test different display modes
    for (uint8_t mode = 0x11; mode <= 0x13; mode++) {
        // Set display mode
        Serial1.write(0x1B); // ESC
        Serial1.write(mode); // Mode command
        delay(200);
        
        // Show mode on display
        Serial1.write(0x09); // Clear
        delay(100);
        Serial1.write(0x0C); // Home
        delay(100);
        
        char modeText[20];
        sprintf(modeText, "Mode 0x%X", mode);
        for (int i = 0; modeText[i] != '\0'; i++) {
            Serial1.write(modeText[i]);
            delay(50);
        }
        
        delay(2000); // Show for 2 seconds
    }
    
    Serial.println("Basic test complete - check VFD display!");
}

void loop() {
    // Simple heartbeat
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 2000) {
        Serial.print(".");
        lastUpdate = millis();
    }
}
