// MatrixRainDemo: Buffered, no-input "digital rain" animation
// - Uses BufferedVFD for smooth, device-agnostic rendering
// - No user input; runs continuously

#include <Arduino.h>
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"
#include "Buffered/BufferedVFD.h"

HardwareSerial& VFD_SERIAL = Serial1;

IVFDHAL* hal = nullptr;
ITransport* transport = nullptr;
VFDDisplay* vfd = nullptr;
BufferedVFD* bf = nullptr;

uint8_t ROWS = 4;
uint8_t COLS = 20;

struct Drop {
  int8_t row = -1;           // current head row (can start above top)
  uint8_t tail = 2;          // tail length
  uint16_t speed = 100;      // ms per step
  uint32_t last = 0;         // last update time
  bool active = false;       // active status
};

// One drop per column
Drop drops[40];

// Character set for the rain
char randGlyph() {
  const char* s = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  size_t n = strlen(s);
  return s[millis() % n];
}

void spawnDrop(uint8_t col) {
  Drop &d = drops[col];
  d.row = -(int8_t)(1 + (millis() % ROWS)); // start above screen
  d.tail = 1 + (millis() % ROWS);          // 1..ROWS
  d.speed = 70 + (millis() % 120);         // 70..189 ms
  d.last = 0;
  d.active = true;
}

void maybeSpawn(uint8_t col) {
  if (drops[col].active) return;
  // Low probability spawn per frame
  if ((millis() + col * 37) % 17 == 0) spawnDrop(col);
}

void stepDrops() {
  uint32_t now = millis();
  for (uint8_t c = 0; c < COLS; ++c) {
    maybeSpawn(c);
    Drop &d = drops[c];
    if (!d.active) continue;
    if (d.last == 0 || (now - d.last) >= d.speed) {
      d.row++;
      d.last = now;
      // Deactivate when head + tail is off-screen
      if (d.row - (int8_t)d.tail >= (int8_t)ROWS) d.active = false;
    }
  }
}

void drawFrame() {
  bf->clearBuffer();
  // Render each column's drop with head and fading tail
  for (uint8_t c = 0; c < COLS; ++c) {
    Drop &d = drops[c];
    if (!d.active) continue;

    // Draw head and tail
    for (uint8_t k = 0; k <= d.tail; ++k) {
      int8_t r = d.row - (int8_t)k;
      if (r < 0 || r >= (int8_t)ROWS) continue;
      char g[2] = {' ', '\0'};
      if (k == 0) {
        // Head: brighter looking glyph
        g[0] = randGlyph();
      } else if (k == 1) {
        // Near tail
        g[0] = '1';
      } else {
        // Far tail
        g[0] = '.';
      }
      bf->writeAt((uint8_t)r, c, g);
    }
  }
  bf->flushDiff();
}

void setup() {
  Serial.begin(57600);
  VFD_SERIAL.begin(19200, SERIAL_8N2);

  hal = new VFD20S401HAL();
  transport = new SerialTransport(&VFD_SERIAL);
  vfd = new VFDDisplay(hal, transport);
  bf = new BufferedVFD(hal);

  if (!vfd->init()) {
    Serial.println("Init failed");
    while (1) delay(1000);
  }

  // Standardized init: reset, clear, home
  vfd->reset();
  vfd->clear();
  vfd->cursorHome();
  vfd->changeCharSet(1); // CT1 (Extended)

  if (!bf->init()) {
    Serial.println("Buffered init failed");
    while (1) delay(1000);
  }

  const IDisplayCapabilities* caps = hal->getDisplayCapabilities();
  if (caps) { ROWS = caps->getTextRows(); COLS = caps->getTextColumns(); }
  if (COLS > 40) COLS = 40;

  // Title
  bf->clearBuffer();
  bf->centerText(0, "Matrix Rain");
  bf->flush();
  delay(1000);
}

void loop() {
  stepDrops();
  drawFrame();
  delay(20);
}
