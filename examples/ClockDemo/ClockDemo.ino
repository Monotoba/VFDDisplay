// ClockDemo: Buffered, no-input clock (HH:MM:SS) using BufferedVFD
// - No RTC; time since power-up via millis()
// - Uses BufferedVFD to center and update smoothly

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

// Clock state (24h)
uint8_t hh = 0, mm = 0, ss = 0;
uint32_t lastTick = 0; // last whole-second update
bool colonOn = true;   // blink colon every second

void formatTime(char* out, size_t n, bool showColon) {
  // Format HH:MM:SS with leading zeros; blink colon as desired
  char col = showColon ? ':' : ' ';
  snprintf(out, n, "%02u%c%02u%c%02u", (unsigned)hh, col, (unsigned)mm, col, (unsigned)ss);
}

void tickClock(uint32_t now) {
  if (lastTick == 0) { lastTick = now; return; }
  uint32_t delta = now - lastTick;
  if (delta >= 1000) {
    uint32_t steps = delta / 1000U;
    lastTick += steps * 1000U;
    // advance time by steps seconds
    uint32_t total = (uint32_t)hh*3600U + (uint32_t)mm*60U + (uint32_t)ss + steps;
    total %= 24U*3600U;
    hh = (uint8_t)(total / 3600U);
    total %= 3600U;
    mm = (uint8_t)(total / 60U);
    ss = (uint8_t)(total % 60U);
    colonOn = !colonOn;
  }
}

void drawClock() {
  bf->clearBuffer();
  // Optional title on first row
  if (ROWS >= 3) bf->centerText(0, "Clock Demo");
  // Time centered on middle row
  char line[16];
  formatTime(line, sizeof(line), colonOn);
  uint8_t mid = (ROWS >= 2) ? (ROWS/2) : 0;
  bf->centerText(mid, line);
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
  vfd->changeCharSet(1); // CT1 (optional)

  if (!bf->init()) {
    Serial.println("Buffered init failed");
    while (1) delay(1000);
  }

  const IDisplayCapabilities* caps = hal->getDisplayCapabilities();
  if (caps) { ROWS = caps->getTextRows(); COLS = caps->getTextColumns(); }

  // Show initial frame
  lastTick = millis();
  drawClock();
}

void loop() {
  uint32_t now = millis();
  tickClock(now);
  drawClock();
  delay(30);
}
