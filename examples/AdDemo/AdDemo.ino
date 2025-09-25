// AdDemo: Fancy animated ad using BufferedVFD + simple effects (no input)
// Content:
//   BIG SKY VCR REPAIR - BILLINGS MT
//   ELECTRONICS REPAIR FOR ALL YOUR NEEDS
//   CUSTOM DISPLAYS * AUDIO * AUTOMOTIVE
//   3D PRINTING SERVICES AVAILABLE
//   CALL (406) 256-2578

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

// Timing helpers
uint32_t nowMs() { return millis(); }

// Scenes
enum Scene : uint8_t {
  INTRO_FADEIN = 0,
  TITLE_HOLD,
  TITLE_FADEOUT,
  ELEC_SCROLL,
  CUSTOM_SCROLL,
  PRINT_SCROLL,
  CALL_FLASH,
};

Scene scene = INTRO_FADEIN;
uint32_t sceneStart = 0;
uint8_t fadeIdx = 0;
const uint8_t DIM[] = { 0x00, 0x40, 0x80, 0xC0 }; // device-specific levels

// Copy-safe center helper into buffer
void centerRow(uint8_t row, const char* text) {
  bf->centerText(row, text);
}

void drawTitle() {
  bf->clearBuffer();
  centerRow(0, "BIG SKY VCR REPAIR");
  centerRow(1, "BILLINGS MT");
  bf->flushDiff();
}

void startIntroFadeIn() {
  drawTitle();
  // Fade from dim to bright
  fadeIdx = 0; // start at 0x00, then 0x40, ... 0xC0 (brightest)
  vfd->setDimming(DIM[fadeIdx]);
  sceneStart = nowMs();
}

void stepIntroFadeIn() {
  if (nowMs() - sceneStart >= 350) {
    sceneStart = nowMs();
    if (fadeIdx + 1 < sizeof(DIM)) {
      fadeIdx++;
      vfd->setDimming(DIM[fadeIdx]);
    } else {
      scene = TITLE_HOLD;
      sceneStart = nowMs();
    }
  }
}

void stepTitleHold() {
  if (nowMs() - sceneStart >= 2500) { // hold 2.5s
    scene = TITLE_FADEOUT;
    sceneStart = nowMs();
    fadeIdx = sizeof(DIM) - 1; // start bright
  }
}

void stepTitleFadeOut() {
  if (nowMs() - sceneStart >= 250) {
    sceneStart = nowMs();
    if (fadeIdx > 0) {
      fadeIdx--;
      vfd->setDimming(DIM[fadeIdx]);
    } else {
      // proceed to next scene
      scene = ELEC_SCROLL;
      // Reset brightness to bright
      vfd->setDimming(DIM[sizeof(DIM)-1]);
      // Prepare scroll line
      bf->clearBuffer();
      centerRow(0, "ELECTRONICS REPAIR");
      bf->flush();
      bf->hScrollBegin(2, "ELECTRONICS REPAIR FOR ALL YOUR NEEDS   ", 140);
      sceneStart = nowMs();
    }
  }
}

void stepElecScroll() {
  bf->hScrollStep(nowMs());
  bf->flushDiff();
  if (nowMs() - sceneStart >= 6000) {
    scene = CUSTOM_SCROLL;
    bf->hScrollStop();
    bf->clearBuffer();
    centerRow(0, "CUSTOM DISPLAYS");
    bf->flush();
    bf->hScrollBegin(2, "CUSTOM DISPLAYS * AUDIO * AUTOMOTIVE   ", 140);
    sceneStart = nowMs();
  }
}

void stepCustomScroll() {
  bf->hScrollStep(nowMs());
  bf->flushDiff();
  if (nowMs() - sceneStart >= 6000) {
    scene = PRINT_SCROLL;
    bf->hScrollStop();
    bf->clearBuffer();
    centerRow(0, "3D PRINTING");
    bf->flush();
    bf->hScrollBegin(2, "3D PRINTING SERVICES AVAILABLE   ", 140);
    sceneStart = nowMs();
  }
}

void stepPrintScroll() {
  bf->hScrollStep(nowMs());
  bf->flushDiff();
  if (nowMs() - sceneStart >= 6000) {
    scene = CALL_FLASH;
    bf->hScrollStop();
    bf->clearBuffer();
    centerRow(1, "CALL (406) 256-2578");
    bf->flush();
    // Flash "CALL" on and off at left to draw attention
    bf->flashBegin(1, 0, "CALL", 300, 300, 10);
    sceneStart = nowMs();
  }
}

void stepCallFlash() {
  bf->flashStep(nowMs());
  bf->flushDiff();
  if (nowMs() - sceneStart >= 5000) {
    // Loop back to intro
    bf->flashStop();
    scene = INTRO_FADEIN;
    startIntroFadeIn();
  }
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

  // Start scene
  startIntroFadeIn();
}

void loop() {
  switch (scene) {
    case INTRO_FADEIN:   stepIntroFadeIn();   break;
    case TITLE_HOLD:     stepTitleHold();     break;
    case TITLE_FADEOUT:  stepTitleFadeOut();  break;
    case ELEC_SCROLL:    stepElecScroll();    break;
    case CUSTOM_SCROLL:  stepCustomScroll();  break;
    case PRINT_SCROLL:   stepPrintScroll();   break;
    case CALL_FLASH:     stepCallFlash();     break;
  }
  delay(10);
}

