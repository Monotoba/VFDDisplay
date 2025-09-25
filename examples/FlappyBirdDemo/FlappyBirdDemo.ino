// FlappyBirdDemo: Buffered, no-input animation using CT1
// - Shows a bird navigating obstacles automatically
// - Uses BufferedVFD for smooth, device-agnostic animation

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

struct Obstacle {
  int8_t col;
  uint8_t gapRow; // row index [0..rows-1]
};

// Simple ring buffer for obstacles
static const uint8_t MAX_OBS = 8;
Obstacle obs[MAX_OBS];
uint8_t obsCount = 0;

// Display dims (filled after init)
uint8_t ROWS = 4;
uint8_t COLS = 20;

// Bird state
uint8_t birdRow = 1;
uint8_t birdCol = 3; // keep near left for better visibility
char birdChar = '>';

// Timing
uint32_t lastTick = 0;
const uint16_t TICK_MS = 120; // animation cadence
uint16_t spawnCounter = 0;
const uint16_t SPAWN_EVERY_TICKS = 10; // new obstacle every ~1.2s

// Helpers
void obstaclesInit() {
  obsCount = 0;
}

void obstacleSpawn() {
  if (obsCount >= MAX_OBS) return;
  // Choose a random gap row
  uint8_t gap = (uint8_t)(millis() % ROWS);
  obs[obsCount++] = Obstacle{ (int8_t)(COLS - 1), gap };
}

void obstaclesAdvance() {
  uint8_t w = 0;
  for (uint8_t i = 0; i < obsCount; ++i) {
    obs[i].col--;
    if (obs[i].col >= 0) {
      // keep
      if (i != w) obs[w] = obs[i];
      w++;
    }
  }
  obsCount = w;
}

// Find obstacle nearest to birdCol at or to the right of birdCol
bool nextObstacle(int8_t atOrRightCol, Obstacle& out) {
  bool found = false;
  int8_t bestCol = 127;
  for (uint8_t i = 0; i < obsCount; ++i) {
    if (obs[i].col >= atOrRightCol && obs[i].col < bestCol) {
      bestCol = obs[i].col;
      out = obs[i];
      found = true;
    }
  }
  return found;
}

void drawFrame() {
  // Clear buffered screen
  bf->clearBuffer();

  // Draw obstacles
  for (uint8_t i = 0; i < obsCount; ++i) {
    int8_t c = obs[i].col;
    if (c < 0 || c >= (int8_t)COLS) continue;
    for (uint8_t r = 0; r < ROWS; ++r) {
      if (r == obs[i].gapRow) continue; // gap
      bf->writeAt(r, (uint8_t)c, "#");
    }
  }

  // Draw bird
  bf->writeAt(birdRow, birdCol, ">" );

  // Flush diffs
  bf->flushDiff();
}

void autopilotMove() {
  // Look ahead to the next obstacle at/after birdCol
  Obstacle n;
  if (nextObstacle(birdCol, n)) {
    // If obstacle is close to bird column, steer toward gap row
    if (n.col <= (int8_t)(birdCol + 2)) {
      if (n.gapRow > birdRow) birdRow++;
      else if (n.gapRow < birdRow) birdRow--;
    }
  }
  // Constrain
  if ((int8_t)birdRow < 0) birdRow = 0;
  if (birdRow >= ROWS) birdRow = ROWS - 1;
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
    while (true) delay(1000);
  }

  vfd->clear();
  vfd->cursorHome();
  vfd->changeCharSet(1); // CT1 (Extended)

  if (!bf->init()) {
    Serial.println("Buffered init failed");
    while (true) delay(1000);
  }

  const IDisplayCapabilities* caps = hal->getDisplayCapabilities();
  if (caps) { ROWS = caps->getTextRows(); COLS = caps->getTextColumns(); }

  // Title
  bf->centerText(0, "Flappy Bird");
  bf->flush();
  delay(1200);

  obstaclesInit();
  lastTick = millis();
}

void loop() {
  uint32_t now = millis();
  if ((now - lastTick) >= TICK_MS) {
    lastTick = now;

    // Advance obstacles / spawn new
    obstaclesAdvance();
    if (++spawnCounter >= SPAWN_EVERY_TICKS) { spawnCounter = 0; obstacleSpawn(); }

    // Autopilot adjust
    autopilotMove();

    // Draw
    drawFrame();
  }

  // Small sleep to reduce busy loop
  delay(5);
}

