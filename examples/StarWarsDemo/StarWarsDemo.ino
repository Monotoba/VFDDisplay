// StarWarsDemo: Buffered Star Wars intro crawl with starfield background.
// - Uses BufferedVFD to render a twinkling starfield and scrolling story text
// - Perspective effect: as lines move upward, they lose characters on both ends
//   to simulate text shrinking into the distance.

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

// 4x20 assumed; we query from capabilities at init
uint8_t ROWS = 4;
uint8_t COLS = 20;

// Simple PRNG for twinkling stars (LCG)
static uint32_t rngState = 0xC0FFEEU;
static inline uint32_t rng() { rngState = 1664525U * rngState + 1013904223U; return rngState; }

// Starfield glyphs: mix of space and star-like chars
static const char STAR_CHARS[] = " . .+*oO"; // bias toward more spaces

// Story text lines (keep it short to fit AVR RAM)
static const char* STORY[] = {
  "A long time ago",
  "in a galaxy far,",
  "far away...",
  "",
  "It is a period",
  "of civil war.",
  "Rebel spaceships,",
  "striking from a",
  "hidden base, have",
  "won their first",
  "victory against",
  "the evil Galactic",
  "Empire.",
};
static const uint8_t STORY_LINES = sizeof(STORY)/sizeof(STORY[0]);

// Scroll state
static int32_t baseLine = -4;   // line index shown at bottom row; start below to fade-in
static uint32_t lastStep = 0;
static const uint16_t STEP_MS = 900; // time between row shifts

// Draw a twinkling starfield into the buffer
void drawStarfield() {
  // Fill entire buffer with stars; keep more spaces than stars for sparse field
  for (uint8_t r = 0; r < ROWS; ++r) {
    for (uint8_t c = 0; c < COLS; ++c) {
      // Weighted choice: ~70% space, 20% '.', 5% '+', 5% '*'/o/O
      uint32_t v = rng() & 0xFFu;
      char ch = ' ';
      if (v < 26) ch = '.';         // 10%
      else if (v < 38) ch = '+';    // 5%
      else if (v < 51) ch = '*';    // 5%
      else if (v < 58) ch = 'o';    // ~3%
      else if (v < 64) ch = 'O';    // ~2%
      else ch = ' ';
      bf->writeAt(r, c, (const char[]){ch, '\0'});
    }
  }
}

// Render the current frame: starfield + perspective text overlay
void drawFrame() {
  bf->clearBuffer();
  drawStarfield();

  // Perspective trimming per row (bottom..top); symmetric trim at both ends
  // Increase aggressively toward top to simulate distance
  uint8_t trimPerRow[8];
  for (uint8_t r = 0; r < ROWS; ++r) {
    // Map bottom row -> 0, top row -> up to COLS/3
    uint8_t factor = (uint8_t)((uint16_t)(r) * (COLS / 3) / (ROWS > 1 ? (ROWS - 1) : 1));
    trimPerRow[r] = factor; // characters to trim from each side
  }

  // Overlay text lines; bottom row shows baseLine, row above shows baseLine-1, etc.
  for (int8_t vr = 0; vr < (int8_t)ROWS; ++vr) {
    int32_t li = baseLine - vr;
    if (li < 0 || li >= STORY_LINES) continue; // outside story => leave starfield

    const char* src = STORY[li];
    size_t len = strlen(src);
    uint8_t trim = trimPerRow[vr];
    // compute start and end indices after trimming both sides
    size_t start = (len > trim) ? trim : len;
    size_t end = (len > trim) ? len - trim : len;
    if (end <= start) continue; // nothing left to draw at this perspective
    size_t avail = end - start;
    if (avail > COLS) avail = COLS;

    // Center the trimmed slice
    uint8_t pad = (COLS > avail) ? (uint8_t)((COLS - avail) / 2) : 0;
    char line[41];
    if (avail > sizeof(line) - 1) avail = sizeof(line) - 1;
    for (size_t i = 0; i < avail; ++i) line[i] = src[start + i];
    line[avail] = '\0';
    bf->writeAt(ROWS - 1 - vr, pad, line);
  }

  bf->flushDiff();
}

void stepScroll(uint32_t now) {
  if (lastStep == 0) { lastStep = now; return; }
  if ((now - lastStep) >= STEP_MS) {
    lastStep = now;
    baseLine++;
    // Loop: reset when story scrolled off
    if (baseLine - (ROWS - 1) >= (int32_t)STORY_LINES + 3) {
      baseLine = -4; // restart beneath the view for a soft fade-in
    }
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

  // Standardized init
  vfd->reset();
  vfd->clear();
  vfd->cursorHome();
  vfd->setCursorMode(0);       // hide cursor for the crawl

  if (!bf->init()) {
    Serial.println("Buffered init failed");
    while (1) delay(1000);
  }

  const IDisplayCapabilities* caps = hal->getDisplayCapabilities();
  if (caps) { ROWS = caps->getTextRows(); COLS = caps->getTextColumns(); }

  // Seed RNG from millis()
  rngState ^= (uint32_t)millis();

  drawFrame();
}

void loop() {
  uint32_t now = millis();
  stepScroll(now);
  drawFrame();
  delay(40); // small delay for twinkle effect smoothness
}

