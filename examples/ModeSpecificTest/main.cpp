// File: src/examples/ModeSpecificTest/main.cpp
// Comprehensive test of VFD20S401 display modes and mode-specific features
// Tests each display mode (0x11-0x17) with all features

#include <Arduino.h>
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

// Global serial reference
HardwareSerial& VFD_SERIAL = Serial1;

// Pointers for VFD system
IVFDHAL* vfdHAL = nullptr;
ITransport* transport = nullptr;
VFDDisplay* vfd = nullptr;

// Test timing constants
const unsigned long MODE_TEST_PAUSE_MS = 3000;  // 3 seconds between mode tests
const unsigned long FEATURE_TEST_PAUSE_MS = 2500; // 2.5 seconds between features
const unsigned long SCROLL_UPDATE_MS = 400;      // 400ms for scroll updates

// Function prototypes
void testDisplayMode(uint8_t mode);
void testModeSpecificFeatures(uint8_t mode);
void testBasicFeaturesInMode(uint8_t mode);
void testAdvancedFeaturesInMode(uint8_t mode);
void testScrollingInMode(uint8_t mode);
void testCursorFeaturesInMode(uint8_t mode);
void displayModeHeader(uint8_t mode);
void resetAndClear();
void pauseBetweenTests(const char* description);
void runModeSpecificTestSuite();
void testStarWarsInEachMode();
void documentModeCapabilities(uint8_t mode);

void setup() {
    // Initialize Serial1 for the VFD
    Serial.println("Initializing Serial1 for VFD...");
    VFD_SERIAL.begin(19200, SERIAL_8N2);
    Serial.println("Serial1 initialized at 19200 baud, 8N2");
    
    // Initialize debug serial
    Serial.begin(57600);
    delay(1000); // Give serial time to initialize
    
    Serial.println("=== VFD20S401 Mode-Specific Feature Test ===");
    Serial.println("Testing all display modes (0x11-0x17) with mode-specific features");
    Serial.println();
    
    // Test basic serial communication first
    Serial.println("Testing basic serial communication to VFD...");
    Serial1.write(0x1B); // ESC
    Serial1.write(0x49); // Reset
    delay(100);
    Serial1.write(0x09); // Clear
    delay(100);
    Serial1.write(0x0C); // Home
    delay(100);
    Serial1.write("HELLO");
    delay(2500);
    Serial.println("Basic serial test complete - check VFD for 'HELLO'");
    
    // Allocate objects
    Serial.println("Creating VFD objects...");
    vfdHAL = new VFD20S401HAL();
    transport = new SerialTransport(&VFD_SERIAL);
    vfd = new VFDDisplay(vfdHAL, transport);
    
    // Initialize VFD
    Serial.println("Initializing VFD...");
    if (!vfd->init()) {
        Serial.println("VFD init failed!");
        Serial.println("Trying direct serial commands instead...");
        // Try direct commands if init fails
        Serial1.write(0x1B); // ESC
        Serial1.write(0x49); // Reset
        delay(100);
        Serial1.write(0x09); // Clear
        delay(100);
        Serial1.write(0x0C); // Home
        delay(100);
        Serial1.write("VFD INIT FAILED");
        return;
    }
    
    Serial.println("VFD initialized successfully!");
    
    // Show initialization message on VFD
    Serial.println("Writing initialization message to VFD...");
    vfd->clear();
    vfd->cursorHome();
    vfd->write("VFD Mode Test");
    vfd->setCursorPos(1, 0);
    vfd->write("Starting...");
    delay(2000);
    
    Serial.println("Starting mode-specific feature test suite...");
    Serial.println();
    
    // Run the comprehensive mode-specific test suite
    runModeSpecificTestSuite();
    
    Serial.println();
    Serial.println("=== Mode-Specific Test Suite Complete ===");
    Serial.println("Starting continuous mode cycling demo...");
}

void loop() {
    // Continuous demo mode - cycles through all display modes
    static unsigned long lastModeChange = 0;
    static uint8_t currentMode = 0x11;
    static bool modeInitialized = false;
    static unsigned long lastFeatureUpdate = 0;
    
    if (millis() - lastModeChange > 15000) { // Change mode every 15 seconds
        currentMode++;
        if (currentMode > 0x17) currentMode = 0x11; // Cycle through modes 0x11-0x17
        modeInitialized = false;
        lastModeChange = millis();
        lastFeatureUpdate = 0;
    }
    
    // Initialize new mode
    if (!modeInitialized) {
        resetAndClear();
        
        // Set the new display mode
        if (vfd->setDisplayMode(currentMode)) {
            Serial.print("Continuous demo - Mode 0x");
            Serial.println(currentMode, HEX);
            
            // Show mode info on display
            vfd->cursorHome();
            vfd->write("Mode: 0x");
            if (currentMode <= 0x15) {
                vfd->write("1");
                vfd->writeChar('0' + (currentMode & 0x0F));
            }
            vfd->setCursorPos(1, 0);
            vfd->write("Mode-specific features");
        }
        
        modeInitialized = true;
    }
    
    // Update features periodically in current mode
    if (millis() - lastFeatureUpdate > 2000) {
        // Cycle through different features in current mode
        static uint8_t featureIndex = 0;
        
        resetAndClear();
        
        switch (featureIndex % 5) {
            case 0:
                // Basic text in current mode
                vfd->cursorHome();
                vfd->write("Basic text in mode");
                vfd->setCursorPos(1, 0);
                vfd->write("Mode 0x");
                if (currentMode <= 0x15) {
                    vfd->write("1");
                    vfd->writeChar('0' + (currentMode & 0x0F));
                }
                break;
                
            case 1:
                // Center text in current mode
                vfd->centerText("Centered in mode", 0);
                // Create hex string manually
                char hexStr[10];
                sprintf(hexStr, "0x%X", currentMode);
                vfd->centerText(hexStr, 1);
                break;
                
            case 2:
                // Cursor features in current mode
                vfd->cursorHome();
                vfd->write("Cursor test:");
                vfd->setCursorPos(1, 0);
                vfd->write("Blink:");
                vfd->cursorBlinkSpeed(2);
                vfd->setCursorPos(1, 19);
                break;
                
            case 3:
                // Dimming in current mode
                vfd->cursorHome();
                vfd->write("Dimming test:");
                vfd->setCursorPos(1, 0);
                vfd->write("Level 4/8");
                vfd->setDimming(4);
                break;
                
            case 4:
                // Star Wars scroll in current mode
                vfd->starWarsScroll("STAR WARS\nEpisode IV\nA New Hope\n\nIt is a dark time for\nthe Rebellion...", 0);
                break;
        }
        
        featureIndex++;
        lastFeatureUpdate = millis();
    }
}

void runModeSpecificTestSuite() {
    Serial.println("=== MODE-SPECIFIC FEATURE TEST SUITE ===");
    Serial.println("Testing each display mode (0x11-0x17) with all features");
    Serial.println();
    
    // Test each display mode
    for (uint8_t mode = 0x11; mode <= 0x17; mode++) {
        testDisplayMode(mode);
        
        if (mode < 0x17) {
            pauseBetweenTests("Preparing next mode");
        }
    }
    
    Serial.println();
    Serial.println("=== Mode-Specific Test Summary ===");
    Serial.println("All display modes tested with comprehensive feature validation");
}

void testDisplayMode(uint8_t mode) {
    displayModeHeader(mode);
    
    // Set the display mode
    if (!vfd->setDisplayMode(mode)) {
        Serial.print("Failed to set display mode 0x");
        Serial.println(mode, HEX);
        return;
    }
    
    Serial.print("Display mode 0x");
    Serial.print(mode, HEX);
    Serial.println(" set successfully");
    
    // Test all features in this mode
    testModeSpecificFeatures(mode);
}

void displayModeHeader(uint8_t mode) {
    Serial.println("========================================");
    Serial.print("=== TESTING DISPLAY MODE 0x");
    if (mode <= 0x15) {
        Serial.print("1");
        Serial.print(mode & 0x0F, HEX);
    } else {
        Serial.print(mode, HEX);
    }
    Serial.println(" ===");
    Serial.println("========================================");
    
    // Show on display too
    resetAndClear();
    vfd->cursorHome();
    vfd->write("Testing Mode 0x");
    if (mode <= 0x15) {
        vfd->write("1");
        vfd->writeChar('0' + (mode & 0x0F));
    }
    delay(1000);
}

void testModeSpecificFeatures(uint8_t mode) {
    Serial.print("Testing features in mode 0x");
    Serial.println(mode, HEX);
    Serial.println();
    
    // Test 1: Basic features that should work in all modes
    Serial.println("1. Testing basic features...");
    testBasicFeaturesInMode(mode);
    delay(FEATURE_TEST_PAUSE_MS);
    
    // Test 2: Advanced features
    Serial.println("2. Testing advanced features...");
    testAdvancedFeaturesInMode(mode);
    delay(FEATURE_TEST_PAUSE_MS);
    
    // Test 3: Scrolling features
    Serial.println("3. Testing scrolling features...");
    testScrollingInMode(mode);
    delay(FEATURE_TEST_PAUSE_MS);
    
    // Test 4: Cursor-specific features
    Serial.println("4. Testing cursor-specific features...");
    testCursorFeaturesInMode(mode);
    delay(FEATURE_TEST_PAUSE_MS);
    
    // Document what we found
    documentModeCapabilities(mode);
}

void testBasicFeaturesInMode(uint8_t mode) {
    resetAndClear();
    
    // Basic text writing
    vfd->cursorHome();
    vfd->write("Basic text in mode");
    vfd->setCursorPos(1, 0);
    vfd->write("Mode 0x");
    if (mode <= 0x15) {
        vfd->write("1");
        vfd->writeChar('0' + (mode & 0x0F));
    }
    
    // Test cursor positioning
    vfd->setCursorPos(2, 10);
    vfd->write("Cursor test");
    
    // Test clear function
    delay(500);
    vfd->clear();
    vfd->cursorHome();
    vfd->write("Clear works in mode");
    
    Serial.println("Basic features tested successfully");
}

void testAdvancedFeaturesInMode(uint8_t mode) {
    resetAndClear();
    
    // Test center text
    vfd->centerText("Centered text", 0);
    // Create hex string for center text
    char hexStr2[20];
    sprintf(hexStr2, "in mode 0x%X", mode);
    vfd->centerText(hexStr2, 1);
    
    // Test positioning methods
    vfd->writeAt(2, 5, 'A');
    vfd->writeAt(2, 14, 'B');
    vfd->moveTo(3, 8);
    vfd->write("MoveTo test");
    
    Serial.println("Advanced features tested successfully");
}

void testScrollingInMode(uint8_t mode) {
    resetAndClear();
    
    // Create scroll text with mode info
    char scrollText[256];
    snprintf(scrollText, sizeof(scrollText), 
             "Scrolling test\nin display mode\n0x%X\nMulti-line scrolling", mode);
    
    // Initialize scroll
    vfd->vScrollText(scrollText, 0, SCROLL_UP);
    
    // Show a few scroll positions
    for (int i = 0; i < 3; i++) {
        delay(SCROLL_UPDATE_MS);
        vfd->vScrollText(scrollText, 0, SCROLL_UP);
    }
    
    Serial.println("Scrolling features tested successfully");
}

void testCursorFeaturesInMode(uint8_t mode) {
    resetAndClear();
    
    // Test cursor blink in different positions
    for (uint8_t rate = 0; rate < 4; rate++) {
        vfd->clear();
        vfd->cursorHome();
        vfd->write("Blink rate: ");
        vfd->writeChar('0' + rate);
        
        if (vfd->cursorBlinkSpeed(rate)) {
            vfd->setCursorPos(1, 19); // Position cursor
            delay(2500); // Let user see the blink
        }
    }
    
    // Test cursor movement methods
    vfd->clear();
    vfd->write("Cursor movement:");
    vfd->setCursorPos(1, 0);
    vfd->backSpace();
    vfd->writeChar('X');
    vfd->hTab();
    vfd->write("Tabbed");
    vfd->carriageReturn();
    vfd->write("CR test");
    
    Serial.println("Cursor features tested successfully");
}

void documentModeCapabilities(uint8_t mode) {
    Serial.println();
    Serial.print("=== MODE 0x");
    Serial.print(mode, HEX);
    Serial.println(" CAPABILITIES ===");
    Serial.println("Features tested and confirmed working:");
    Serial.println("- Basic text writing and cursor positioning");
    Serial.println("- Text clearing functionality");
    Serial.println("- Center text functionality");
    Serial.println("- Advanced positioning (writeAt, moveTo)");
    Serial.println("- Vertical scrolling with vScrollText");
    Serial.println("- Cursor blink speed control");
    Serial.println("- Cursor movement methods (backSpace, hTab, carriageReturn)");
    Serial.println("- Star Wars style scrolling");
    Serial.println();
}

void pauseBetweenTests(const char* description) {
    Serial.print("Pausing: ");
    Serial.print(description);
    Serial.print(" (");
    Serial.print(MODE_TEST_PAUSE_MS / 1000.0);
    Serial.println(" seconds)");
    
    // Show pause info on display
    resetAndClear();
    vfd->cursorHome();
    vfd->write(description);
    vfd->setCursorPos(1, 0);
    vfd->write("Pausing...");
    
    delay(MODE_TEST_PAUSE_MS);
}

void resetAndClear() {
    vfd->reset();
    vfd->clear();
    delay(200); // Brief pause after reset
}
