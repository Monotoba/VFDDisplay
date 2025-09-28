// MovieHouseAd demo: animated marquee of "Now Showing" movies
#include <Arduino.h>
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

// Use Serial1 for the display transport on Mega 2560
HardwareSerial& VFD_SERIAL = Serial1;

IVFDHAL* vfdHAL = nullptr;
ITransport* transport = nullptr;
VFDDisplay* vfd = nullptr;

struct Movie { const char* title; const char* time; };

static const Movie kMovies[] = {
  { "DUNE PART II",     "7:30P" },
  { "TOP GUN MAVERICK", "9:00P" },
  { "BARBIE",           "6:15P" },
  { "OPPENHEIMER",      "8:45P" },
  { "INSIDE OUT 2",     "5:00P" },
};

static const char* kMarquee = "Fresh Popcorn & Ice Cold Drinks at Concessions!";

static void standardInit() {
  vfd->reset();
  vfd->clear();
  vfd->cursorHome();
}

static void printCentered(const char* s, uint8_t row) {
  vfd->centerText(s, row);
}

static void showHeader() {
  printCentered("NOW SHOWING", 0);
}

static void showMovieLine(uint8_t row, const Movie& m) {
  // Format: "TITLE ........ 7:30P" within 20 cols
  char buf[21];
  memset(buf, ' ', sizeof(buf));
  buf[20] = '\0';
  // Title (trim/pad)
  uint8_t col = 0;
  for (; col < 14 && m.title[col]; ++col) buf[col] = m.title[col];
  // Spacer dots before time
  const char* tm = m.time;
  uint8_t timeLen = (uint8_t)strlen(tm);
  uint8_t timeCol = 19 - (timeLen - 1); // right-align ending at col 19
  for (uint8_t i = col; i < timeCol - 1 && i < 18; ++i) buf[i] = '.';
  // Time at end
  for (uint8_t i = 0; i < timeLen && (timeCol + i) < 20; ++i) buf[timeCol + i] = tm[i];
  vfd->writeAt(row, 0, buf);
}

static void marqueeRow(uint8_t row, const char* msg, uint16_t delayMs, uint8_t cycles) {
  // Scroll message across a 20-col window
  const uint8_t W = 20;
  const uint16_t pad = W; // leading/trailing spaces
  const size_t msgLen = strlen(msg);
  const size_t total = pad + msgLen + pad;
  // Draw cycles
  for (uint8_t c = 0; c < cycles; ++c) {
    for (size_t off = 0; off < total - W + 1; ++off) {
      char frame[W + 1]; frame[W] = '\0';
      for (uint8_t i = 0; i < W; ++i) {
        size_t src = off + i;
        char ch = ' ';
        if (src >= pad && src < pad + msgLen) ch = msg[src - pad];
        frame[i] = ch;
      }
      vfd->writeAt(row, 0, frame);
      delay(delayMs);
    }
  }
}

void setup() {
  Serial.begin(57600);
  delay(500);
  Serial.println("MovieHouseAd starting...");

  vfdHAL = new VFD20S401HAL();
  transport = new SerialTransport(&VFD_SERIAL);
  vfd = new VFDDisplay(vfdHAL, transport);

  VFD_SERIAL.begin(19200, SERIAL_8N2);
  delay(300);
  if (!vfd->init()) {
    Serial.println("VFD init failed");
    return;
  }
  standardInit();
}

void loop() {
  showHeader();

  // Show two movies per page on rows 2 and 3, animate row 1 as marquee
  const uint8_t rowsBase = 2;
  const uint8_t perPage = 2;
  const uint8_t total = (uint8_t)(sizeof(kMovies) / sizeof(kMovies[0]));
  for (uint8_t start = 0; start < total; start += perPage) {
    vfd->writeAt(1, 0, "                    "); // clear row 1
    vfd->writeAt(rowsBase + 0, 0, "                    ");
    vfd->writeAt(rowsBase + 1, 0, "                    ");

    if (start + 0 < total) showMovieLine(rowsBase + 0, kMovies[start + 0]);
    if (start + 1 < total) showMovieLine(rowsBase + 1, kMovies[start + 1]);

    // Marquee across row 1 while this page is visible
    marqueeRow(1, kMarquee, 120, 1);
    delay(600);
  }
}

