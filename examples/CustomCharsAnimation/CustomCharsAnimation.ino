// Arduino IDE wrapper and implementation for CustomCharsAnimation
// Demonstrates sprite-style animation using custom characters.

#include <Arduino.h>
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

HardwareSerial& VFD_SERIAL = Serial1;

IVFDHAL* vfdHAL = nullptr;
ITransport* transport = nullptr;
VFDDisplay* vfd = nullptr;

// 3-frame running person (row-major; bits 0..4 left->right; 8 rows provided, last row ignored on 5x7)
static const uint8_t RUNNER_FRAMES[3][8] = {
  {
    0b00100, // head
    0b00100,
    0b01110, // shoulders
    0b00100, // torso
    0b01010, // arms out
    0b10001, // legs apart
    0b00000,
    0b00000
  },
  {
    0b00100,
    0b00100,
    0b01110,
    0b00100,
    0b00100, // arms down
    0b01110, // stride mid
    0b00000,
    0b00000
  },
  {
    0b00100,
    0b00100,
    0b01110,
    0b00100,
    0b01010, // arms out
    0b00110, // legs together forward
    0b00000,
    0b00000
  }
};

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

  vfd->centerText("Custom Char Animation", 0);
  vfd->centerText("Runner sprite ->", 1);
}

void loop() {
  static uint8_t frame = 0;
  static uint8_t col = 0;
  const uint8_t row = 2; // animate on row 2

  // Define current frame in logical index 0
  vfd->setCustomChar(0, RUNNER_FRAMES[frame]);

  // Erase previous position (write space) unless at start
  if (col > 0) {
    vfd->writeAt(row, col - 1, " ");
  }

  // Draw sprite at current position
  vfd->writeCharAt(row, col, 0); // index 0 glyph (legacy-friendly for first 8 indices)

  // Advance frame and position
  frame = (frame + 1) % 3;
  col = (uint8_t)((col + 1) % 20);

  // Small pacing delay for visible animation
  delay(120);
}

