// AnimationsDemo: Demonstrates BufferedVFD non-blocking hScroll, vScroll, and flash
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

  if (!bf->init()) {
    Serial.println("Buffered init failed");
    while (1) delay(1000);
  }

  // Center title in buffer and flush
  bf->centerText(0, "Animations Demo");
  bf->flush();
  // Show title for ~2 seconds, then clear before starting animations
  delay(2000);
  vfd->clear();
  vfd->cursorHome();
  bf->clearBuffer();
  bf->flush();

  // Start animations
  bf->hScrollBegin(1, "Hello from BufferedVFD ", 150);
  bf->vScrollBegin("Line A\nLine B\nLine C\nLine D", 2, -1, 600);
  bf->flashBegin(0, 0, "FLASH", 300, 300, 4);
}

void loop() {
  uint32_t now = millis();
  bf->hScrollStep(now);
  bf->vScrollStep(now);
  bf->flashStep(now);
  bf->flushDiff();
  delay(20);
}
