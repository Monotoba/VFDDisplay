// CursorDemo: Demonstrates display DCs (0x11–0x13), cursor control (DC4–DC7),
// and text wrapping behavior on VFD20S401.

#include <Arduino.h>
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

HardwareSerial& VFD_SERIAL = Serial1;

IVFDHAL* hal = nullptr;
ITransport* transport = nullptr;
VFDDisplay* vfd = nullptr;

static void wait_ms(unsigned long ms) { delay(ms); }

void showDisplayModes() {
  // Cycle DC1..DC3 as single-byte display control codes
  const uint8_t modes[] = { 0x11, 0x12, 0x13 };
  const char* names[] = { "Normal", "Inverse", "Blink" };

  for (uint8_t i = 0; i < 3; ++i) {
    vfd->reset();
    vfd->clear();
    vfd->cursorHome();
    vfd->setDisplayMode(modes[i]);
    vfd->centerText("Display Mode", 0);
    vfd->centerText(names[i], 1);
    wait_ms(2000);
  }
}

void showCursorControl() {
  // DC5 (0x15) shows the cursor; DC4 (0x14) hides; adjust blink via ESC 'T'.
  vfd->reset();
  vfd->clear();
  vfd->cursorHome();
  vfd->centerText("Cursor Control", 0);
  vfd->writeAt(2, 0, "Cursor ON, blink...");

  // Turn cursor ON and set blink rate
  vfd->setCursorMode(1);          // DC5
  vfd->setCursorBlinkRate(0x20);  // non-zero enables blink
  vfd->setCursorPos(3, 19);
  wait_ms(2500);

  // Cursor steady ON (no blink)
  vfd->setCursorBlinkRate(0x00);
  vfd->writeAt(2, 0, "Cursor ON, steady   ");
  vfd->setCursorPos(3, 0);
  wait_ms(2500);

  // Cursor OFF
  vfd->setCursorMode(0);          // DC4
  vfd->writeAt(2, 0, "Cursor OFF         ");
  wait_ms(2500);
}

void showWrappingDemo() {
  // Write a long line at row 0 to observe wrapping/continuation to next DDRAM.
  vfd->reset();
  vfd->clear();
  vfd->cursorHome();
  vfd->centerText("Wrapping Demo", 0);

  const char* longLine = "This is a long line that should continue across columns and wrap to next row.";
  vfd->setCursorPos(1, 0);
  vfd->write(longLine);
  wait_ms(3500);

  // Manual wrapping using writeAt() on each row
  vfd->clear();
  vfd->cursorHome();
  vfd->centerText("Manual Wrap", 0);
  vfd->writeAt(1, 0, "Row1: 0123456789012345");
  vfd->writeAt(2, 0, "Row2: 0123456789012345");
  vfd->writeAt(3, 0, "Row3: 0123456789012345");
  wait_ms(3000);
}

void setup() {
  Serial.begin(57600);
  VFD_SERIAL.begin(19200, SERIAL_8N2);

  hal = new VFD20S401HAL();
  transport = new SerialTransport(&VFD_SERIAL);
  vfd = new VFDDisplay(hal, transport);

  if (!vfd->init()) {
    Serial.println("Init failed");
    while (1) wait_ms(1000);
  }

  // Standardized init
  vfd->reset();
  vfd->clear();
  vfd->cursorHome();
}

void loop() {
  showDisplayModes();
  showCursorControl();
  showWrappingDemo();
}

