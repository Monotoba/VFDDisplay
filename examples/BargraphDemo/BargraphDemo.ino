// BargraphDemo: Buffered, no-input bar graph for 4 channels (CH1-CH4)
// - Labels rows top to bottom: CH1, CH2, CH3, CH4
// - Simulates changing levels using simple waveforms
// - Uses BufferedVFD for flicker-free updates

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

// Bar area starts after label "CHx:" = 4 chars + space
static const uint8_t LABEL_WIDTH = 5; // e.g., "CH1: "

uint32_t lastTick = 0;
const uint16_t TICK_MS = 120;

// Simple pseudo-sine using millis for each channel
uint8_t waveLevel(uint8_t ch, uint8_t maxWidth) {
  // Phase shift per channel
  uint32_t t = millis() + (uint32_t)ch * 400;
  // Triangle wave from 0..maxWidth
  uint32_t period = 2400; // ms
  uint32_t x = t % period;
  uint32_t half = period / 2;
  uint32_t val = (x <= half) ? (x * maxWidth / half) : ((period - x) * maxWidth / half);
  return (uint8_t)val;
}

void drawBars() {
  bf->clearBuffer();
  const uint8_t barMax = (COLS > LABEL_WIDTH) ? (COLS - LABEL_WIDTH) : 0;
  // Draw each channel row
  for (uint8_t r = 0; r < ROWS && r < 4; ++r) {
    // Label: CH1:, CH2:, etc.
    char label[8];
    snprintf(label, sizeof(label), "CH%u:", (unsigned)(r+1));
    bf->writeAt(r, 0, label);
    bf->writeAt(r, 4, " ");

    // Level
    uint8_t level = waveLevel(r, barMax);
    // Fill bar with '#'
    char rowbuf[32];
    uint8_t i = 0;
    for (; i < level && i < sizeof(rowbuf)-1; ++i) rowbuf[i] = '#';
    // Remaining area as '.'
    for (; i < barMax && i < sizeof(rowbuf)-1; ++i) rowbuf[i] = '.';
    rowbuf[i] = '\0';
    if (barMax > 0) bf->writeAt(r, LABEL_WIDTH, rowbuf);
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
  vfd->clear();
  vfd->cursorHome();
  vfd->changeCharSet(1); // CT1 (Extended), though we use ASCII here

  if (!bf->init()) {
    Serial.println("Buffered init failed");
    while (1) delay(1000);
  }

  const IDisplayCapabilities* caps = hal->getDisplayCapabilities();
  if (caps) { ROWS = caps->getTextRows(); COLS = caps->getTextColumns(); }

  // Title
  bf->clearBuffer();
  bf->centerText(0, "Bargraph Demo");
  bf->flush();
  delay(1200);

  lastTick = millis();
}

void loop() {
  uint32_t now = millis();
  if ((now - lastTick) >= TICK_MS) {
    lastTick = now;
    drawBars();
  }
  delay(5);
}

