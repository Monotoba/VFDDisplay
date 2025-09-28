// PCStatusDisplay demo: show CPU/MEM/GPU/TEMP via Serial with animation
#include <Arduino.h>
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

// Use Serial (USB) for host data input, Serial1 for VFD transport
HardwareSerial& VFD_SERIAL = Serial1;

IVFDHAL* vfdHAL = nullptr;
ITransport* transport = nullptr;
VFDDisplay* vfd = nullptr;

// Parsed metrics
static uint8_t g_cpu = 0, g_mem = 0, g_gpu = 0, g_tmp = 0;
static unsigned long g_lastUpdate = 0;
static const unsigned long kStaleMs = 2000;

static const char kSpinner[] = "-\\|/"; // spinner frames

static void standardInit() {
  vfd->reset();
  vfd->clear();
  vfd->cursorHome();
}

static void drawBar(uint8_t row, uint8_t col, uint8_t pct, uint8_t width) {
  uint8_t filled = (uint8_t)((pct * width + 50) / 100); // round
  char buf[22];
  if (width > 18) width = 18;
  buf[0] = '[';
  for (uint8_t i = 0; i < width; ++i) buf[1 + i] = (i < filled) ? '#' : ' ';
  buf[1 + width] = ']';
  buf[2 + width] = '\0';
  vfd->writeAt(row, col, buf);
}

static void parseLine(char* line) {
  // Accept formats: "CPU=55 MEM=62 GPU=40 TEMP=52" (space separated)
  // or "CPU:55;MEM:62;GPU:40;TEMP:52"
  char* p = line;
  while (*p) {
    // Skip delimiters
    while (*p == ' ' || *p == ';' || *p == ',') ++p;
    if (!*p) break;
    char key[5] = {0};
    uint8_t ki = 0;
    while (*p && isalpha(*p) && ki < 4) key[ki++] = toupper(*p++);
    if (*p == '=' || *p == ':') ++p;
    int val = 0;
    while (*p && isdigit(*p)) { val = val * 10 + (*p - '0'); ++p; }
    if (strcmp(key, "CPU") == 0) g_cpu = (uint8_t)constrain(val, 0, 100);
    else if (strcmp(key, "MEM") == 0) g_mem = (uint8_t)constrain(val, 0, 100);
    else if (strcmp(key, "GPU") == 0) g_gpu = (uint8_t)constrain(val, 0, 100);
    else if (strcmp(key, "TEMP") == 0 || strcmp(key, "TMP") == 0) g_tmp = (uint8_t)constrain(val, 0, 125);
  }
  g_lastUpdate = millis();
}

static void maybeSimulate() {
  if (millis() - g_lastUpdate < kStaleMs) return;
  // Simple simulation: wander values slowly
  static uint8_t dir = 0;
  auto step = [](uint8_t v, uint8_t d) -> uint8_t {
    int dv = (d & 1) ? 1 : -1;
    int nv = (int)v + dv * (2 + (millis() >> 10) % 3);
    if (nv < 0) nv = 0; if (nv > 100) nv = 100;
    return (uint8_t)nv;
  };
  g_cpu = step(g_cpu, dir++);
  g_mem = step(g_mem, dir++);
  g_gpu = step(g_gpu, dir++);
  g_tmp = (uint8_t)(30 + (g_cpu + g_gpu) / 8); // rough temp from load
}

void setup() {
  Serial.begin(115200); // input from PC
  delay(400);
  Serial.println(F("PCStatusDisplay starting... Send lines like: CPU=55 MEM=62 GPU=40 TEMP=52"));

  vfdHAL = new VFD20S401HAL();
  transport = new SerialTransport(&VFD_SERIAL);
  vfd = new VFDDisplay(vfdHAL, transport);

  VFD_SERIAL.begin(19200, SERIAL_8N2);
  delay(300);
  if (!vfd->init()) {
    Serial.println(F("VFD init failed"));
    return;
  }
  standardInit();
}

void loop() {
  // Non-blocking read of a line from Serial
  static char line[128];
  static uint8_t li = 0;
  while (Serial.available()) {
    char ch = (char)Serial.read();
    if (ch == '\r') continue;
    if (ch == '\n') {
      line[li] = '\0';
      if (li > 0) parseLine(line);
      li = 0;
    } else if (li + 1 < sizeof(line)) {
      line[li++] = ch;
    }
  }

  maybeSimulate();

  // Draw UI
  static uint8_t spin = 0;
  char hdr[21]; snprintf(hdr, sizeof(hdr), "PC STATUS  [%c]", kSpinner[spin++ & 3]);
  vfd->writeAt(0, 0, "                    ");
  vfd->writeAt(0, 0, hdr);

  char row1[21]; snprintf(row1, sizeof(row1), "CPU %3u%%  MEM %3u%%", g_cpu, g_mem);
  vfd->writeAt(1, 0, "                    ");
  vfd->writeAt(1, 0, row1);

  char row2[21]; snprintf(row2, sizeof(row2), "GPU %3u%%  TMP %3uC", g_gpu, g_tmp);
  vfd->writeAt(2, 0, "                    ");
  vfd->writeAt(2, 0, row2);

  // Bars on bottom row: CPU and MEM split
  vfd->writeAt(3, 0, "C:");
  drawBar(3, 2, g_cpu, 8);
  vfd->writeAt(3, 12, "M:");
  drawBar(3, 14, g_mem, 6);

  delay(250);
}

