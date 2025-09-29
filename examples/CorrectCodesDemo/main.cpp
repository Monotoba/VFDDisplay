// Demonstrates corrected control and escape codes for Futaba VFD20S401
// - Init/Reset: ESC 'I' (handled by vfd->init()/reset())
// - Positioning: ESC 'H' + position (via vfd->setCursorPos / writeAt / moveTo)
// - Display mode: control codes 0x11–0x13 (vfd->setDisplayMode)
// - Blink speed: ESC 'T' + rate (vfd->setCursorBlinkRate)
// - Dimming: ESC 'L' + level (vfd->setDimming / setBrightness)

#include <Arduino.h>
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

// Use Serial1 for the VFD module (Mega2560 default secondary UART)
HardwareSerial& VFD_SERIAL = Serial1;

// Library objects
IVFDHAL* vfdHAL = nullptr;
ITransport* transport = nullptr;
VFDDisplay* vfd = nullptr;

static void wait_ms(unsigned long ms) { delay(ms); }

void setup() {
  // Debug serial
  Serial.begin(57600);
  wait_ms(500);
  Serial.println("=== CorrectCodesDemo for VFD20S401 ===");

  // Init VFD serial (Futaba VFD20S401: 19200, 8N2)
  VFD_SERIAL.begin(19200, SERIAL_8N2);
  wait_ms(200);

  // Create display stack
  vfdHAL = new VFD20S401HAL();
  transport = new SerialTransport(&VFD_SERIAL);
  vfd = new VFDDisplay(vfdHAL, transport);

  // Initialize using ESC 'I'
  Serial.println("Initializing VFD (ESC 'I')...");
  if (!vfd->init()) {
    Serial.println("Init failed; halting.");
    while (true) { wait_ms(1000); }
  }

  // Standardized init: reset, clear, home
  vfd->reset();
  vfd->clear();
  vfd->cursorHome();
  vfd->write("Init via ESC 'I'");
  wait_ms(2500);

  // Demonstrate reset
  Serial.println("Sending reset (ESC 'I')...");
  vfd->reset();
  vfd->clear();
  vfd->cursorHome();
  vfd->write("Reset OK");
  wait_ms(2500);

  // Demonstrate cursor positioning via ESC 'H'
  Serial.println("Positioning demo via ESC 'H':");
  vfd->clear();
  vfd->writeAt(0, 0, "Row0 Col0");
  vfd->writeAt(1, 0, "Row1 Col0");
  vfd->writeAt(2, 19 - 7, "Row2 End"); // keep within 20 cols
  vfd->writeAt(3, 10, "Row3 Col10");
  wait_ms(2500);

  // Display mode demo (control codes 0x11–0x13)
  Serial.println("Cycling display modes 0x11–0x13...");
  for (uint8_t mode = 0x11; mode <= 0x13; ++mode) {
    vfd->reset();
    vfd->clear();
    vfd->setDisplayMode(mode);
    vfd->centerText("Display Mode", 0);
    char buf[20];
    snprintf(buf, sizeof(buf), "0x%02X", mode);
    vfd->centerText(buf, 1);
    wait_ms(2500);
  }
  // Extra pause between test groups
  wait_ms(2500);

  // Dimming demo (ESC 'L' bands)
  Serial.println("Dimming demo via ESC 'L' (00/40/80/C0)...");
  const uint8_t dims[] = {0x00, 0x40, 0x80, 0xC0};
  for (uint8_t i = 0; i < 4; ++i) {
    vfd->reset();
    vfd->clear();
    vfd->centerText("Dimming Level", 0);
    char buf[20];
    snprintf(buf, sizeof(buf), "0x%02X", dims[i]);
    vfd->centerText(buf, 1);
    vfd->setDimming(dims[i]);
    wait_ms(2500);
  }
  // Extra pause between test groups
  wait_ms(2500);

  // Cursor blink speed demo (ESC 'T' + rate)
  Serial.println("Cursor blink speed demo via ESC 'T'...");
  vfd->reset();
  vfd->clear();
  vfd->cursorHome();
  vfd->write("Blink rates:");
  // Ensure cursor is visible: switch to DC5 (cursor on) before changing blink speed
  vfd->setCursorMode(1);
  const uint8_t rates[] = {0x10, 0x20, 0x40};
  for (uint8_t i = 0; i < 3; ++i) {
    vfd->setCursorPos(1 + i, 0);
    char line[20];
    snprintf(line, sizeof(line), "Rate 0x%02X", rates[i]);
    vfd->write(line);
  }
  // Position visible cursor and apply each rate
  for (uint8_t i = 0; i < 3; ++i) {
    vfd->setCursorPos(1 + i, 19); // right edge
    vfd->setCursorBlinkRate(rates[i]);
    wait_ms(2500);
  }

  // Character set toggle (0x18 / 0x19)
  Serial.println("Character set toggle (CT0/CT1)...");
  vfd->reset();
  vfd->clear();
  vfd->centerText("CT0 (Standard)", 0);
  vfd->changeCharSet(0);
  wait_ms(2500);
  vfd->reset();
  vfd->clear();
  vfd->centerText("CT1 (Extended)", 0);
  vfd->changeCharSet(1);
  wait_ms(2500);

  Serial.println("CorrectCodesDemo complete. Loop will show a status heartbeat.");
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last > 2000) {
    Serial.println("Heartbeat: demo complete");
    last = millis();
  }
}
