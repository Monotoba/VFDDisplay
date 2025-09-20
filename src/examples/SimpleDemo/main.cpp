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
    
    // Test centerText functionality with 4x20 display
    vfd->clear();
    
    // Test different text lengths for centering on 4x20 display
    vfd->centerText("Short", 0);           // 5 chars - should have 7 spaces each side
    vfd->centerText("Medium Length", 1);   // 15 chars - should have 2-3 spaces each side  
    vfd->centerText("Very Long Text Example", 2); // 22 chars - longer than display width
    vfd->centerText("Center", 3);          // 6 chars - should have 7 spaces each side
    
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
    delay(3000);
    vfd->reset();
}

void loop() {
    // Demonstrate display features
    static int loopCount = 0;
    
    if (loopCount % 6 == 0) {
        // Centered text demo - test all 4 rows of 4x20 display
        vfd->clear();
        
        // Test centering on all 4 rows with different text lengths
        const char* testTexts[4] = {
            "Row 0 Centered",      // 16 chars
            "Center Row 1",        // 14 chars  
            "This is Row 2 Text",  // 18 chars
            "Row 3 Center"         // 12 chars
        };
        
        for (uint8_t row = 0; row < 4; row++) {
            if (vfd->centerText(testTexts[row], row)) {
                Serial.print("Centered text on row ");
                Serial.print(row);
                Serial.println(" successfully");
            } else {
                Serial.print("Failed to center text on row ");
                Serial.println(row);
            }
        }
    } else if (loopCount % 6 == 1) {
        // Regular text demo
        vfd->clear();
        vfd->cursorHome();
        vfd->write("Regular Text");
        vfd->setCursorPos(1, 0);
        vfd->write("Also Row 1");
    } else if (loopCount % 6 == 2) {
        // Display mode demo - cycle through modes 0x11-0x17
        vfd->clear();
        vfd->cursorHome();
        
        uint8_t mode = 0x11 + (loopCount / 6) % 7; // Cycle through modes 0x11-0x17
        if (vfd->setDisplayMode(mode)) {
            vfd->write("Mode Set!");
            vfd->setCursorPos(1, 0);
            vfd->write("Mode: 0x");
            // Simple hex display (would need proper hex conversion in real code)
            if (mode <= 0x15) {
                vfd->write("1");
                vfd->writeChar('0' + (mode & 0x0F));
            }
        } else {
            vfd->write("Mode Failed");
        }
    } else if (loopCount % 6 == 3) {
        // Cursor blink speed demo - requires appropriate display mode
        vfd->clear();
        vfd->cursorHome();
        
        // Set display mode that supports cursor blink (if needed)
        // Some displays require specific modes for cursor functionality
        vfd->setDisplayMode(0x11); // Normal mode with cursor support
        
        uint8_t blinkRate = (loopCount / 6) % 4; // Cycle through 4 different blink rates
        if (vfd->cursorBlinkSpeed(blinkRate)) {
            vfd->write("Blink Set!");
            vfd->setCursorPos(1, 0);
            vfd->write("Rate: ");
            vfd->writeChar('0' + blinkRate);
            // Show cursor position to make blink visible
            vfd->setCursorPos(1, 19); // End of row to show cursor
        } else {
            vfd->write("Blink Fail");
        }
    } else if (loopCount % 6 == 4) {
        // Convenience methods demo - cursor movement controls
        vfd->clear();
        vfd->cursorHome();
        
        // Write some text then demonstrate cursor movement
        vfd->write("Controls:");
        
        // Move to second line
        vfd->setCursorPos(1, 0);
        vfd->write("Testing");
        
        // Demonstrate each convenience method
        delay(500);
        
        // Backspace - should move cursor left one position
        if (vfd->backSpace()) {
            vfd->writeChar('X'); // Should overwrite the 'g' in "Testing"
        }
        
        delay(500);
        
        // Horizontal tab - move to next tab stop
        if (vfd->hTab()) {
            vfd->writeChar('T'); // Write at tab position
        }
        
        delay(500);
        
        // Carriage return - move to beginning of current line
        if (vfd->carriageReturn()) {
            vfd->write("CR"); // Should write at beginning of line 1
        }
        
        delay(500);
        
        // Line feed - move to next line (if available)
        if (vfd->lineFeed()) {
            // This would move to next line, but we're on a 2-line display
            // So it might wrap or stay depending on display behavior
        }
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

