// Arduino IDE wrapper and implementation for CustomCharsTetris
// Mini Tetris-style auto-drop demo using custom characters for blocks.

#include <Arduino.h>
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

HardwareSerial& VFD_SERIAL = Serial1;

IVFDHAL* vfdHAL = nullptr;
ITransport* transport = nullptr;
VFDDisplay* vfd = nullptr;

// Display/grid config
static const uint8_t SCREEN_ROWS = 4;  // VFD rows
static const uint8_t SCREEN_COLS = 20; // VFD cols
static const uint8_t FIELD_W = 10;
static const uint8_t FIELD_H = 4;
static const uint8_t FIELD_LEFT = 5; // centered: (20-10)/2
static const uint8_t FIELD_TOP = 0;   // start at row 0

// Custom glyph indices
static const uint8_t GLYPH_BLOCK = 0; // filled block

// Playfield: 0 = empty, 1 = filled
static uint8_t field[FIELD_H][FIELD_W];

// Tetrimino definitions: 7 shapes, each up to 4 rotations, each rotation has 4 (x,y) cells
struct Cell { int8_t x, y; };
struct PieceRot { Cell c[4]; };
struct Piece { uint8_t rots; PieceRot r[4]; };

static const Piece PIECES[] = {
  // I
  {2, { {{0,1},{1,1},{2,1},{3,1}}, {{2,0},{2,1},{2,2},{2,3}} }},
  // O
  {1, { {{1,0},{2,0},{1,1},{2,1}} }},
  // T
  {4, { {{1,0},{0,1},{1,1},{2,1}}, {{1,0},{1,1},{2,1},{1,2}}, {{0,1},{1,1},{2,1},{1,2}}, {{1,0},{0,1},{1,1},{1,2}} }},
  // S
  {2, { {{1,0},{2,0},{0,1},{1,1}}, {{1,0},{1,1},{2,1},{2,2}} }},
  // Z
  {2, { {{0,0},{1,0},{1,1},{2,1}}, {{2,0},{1,1},{2,1},{1,2}} }},
  // J
  {4, { {{0,0},{0,1},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{1,2}}, {{0,1},{1,1},{2,1},{2,2}}, {{1,0},{1,1},{0,2},{1,2}} }},
  // L
  {4, { {{2,0},{0,1},{1,1},{2,1}}, {{1,0},{1,1},{1,2},{2,2}}, {{0,1},{1,1},{2,1},{0,2}}, {{0,0},{1,0},{1,1},{1,2}} }}
};

struct Active { uint8_t id; uint8_t rot; int8_t x; int8_t y; };
static Active current;

static void defineGlyphs() {
  // Full 5x7 block
  uint8_t block[8];
  for (uint8_t r = 0; r < 7; ++r) block[r] = 0b11111; block[7] = 0;
  vfd->setCustomChar(GLYPH_BLOCK, block);
}

static void clearField() {
  for (uint8_t r = 0; r < FIELD_H; ++r) for (uint8_t c = 0; c < FIELD_W; ++c) field[r][c] = 0;
}

static bool collides(const Active& a, int dx, int dy, uint8_t nextRot) {
  const Piece& p = PIECES[a.id];
  const PieceRot& pr = p.r[nextRot];
  for (uint8_t i = 0; i < 4; ++i) {
    int nx = a.x + pr.c[i].x + dx;
    int ny = a.y + pr.c[i].y + dy;
    if (nx < 0 || nx >= FIELD_W || ny < 0 || ny >= FIELD_H) return true;
    if (field[ny][nx]) return true;
  }
  return false;
}

static void placePiece(const Active& a) {
  const Piece& p = PIECES[a.id];
  const PieceRot& pr = p.r[a.rot];
  for (uint8_t i = 0; i < 4; ++i) {
    int x = a.x + pr.c[i].x;
    int y = a.y + pr.c[i].y;
    if (x >= 0 && x < FIELD_W && y >= 0 && y < FIELD_H) field[y][x] = 1;
  }
}

static void clearLines() {
  for (int y = FIELD_H - 1; y >= 0; --y) {
    bool full = true;
    for (uint8_t x = 0; x < FIELD_W; ++x) if (!field[y][x]) { full = false; break; }
    if (full) {
      for (int yy = y; yy > 0; --yy) for (uint8_t x = 0; x < FIELD_W; ++x) field[yy][x] = field[yy - 1][x];
      for (uint8_t x = 0; x < FIELD_W; ++x) field[0][x] = 0;
      y++; // re-check this row after collapse
    }
  }
}

static void spawnPiece() {
  current.id = (uint8_t)(millis() % (sizeof(PIECES)/sizeof(PIECES[0])));
  current.rot = 0;
  current.x = FIELD_W/2 - 2; // spawn near center
  current.y = 0;
  // If spawn collides, reset field for endless demo
  if (collides(current, 0, 0, current.rot)) {
    clearField();
  }
}

static void rotatePiece() {
  const Piece& p = PIECES[current.id];
  uint8_t nextRot = (uint8_t)((current.rot + 1) % p.rots);
  if (!collides(current, 0, 0, nextRot)) current.rot = nextRot;
}

static void tick() {
  // Occasionally rotate for variety
  if ((millis() / 1000) % 3 == 0) rotatePiece();

  if (!collides(current, 0, 1, current.rot)) {
    current.y++;
  } else {
    placePiece(current);
    clearLines();
    spawnPiece();
  }
}

static void render() {
  // Draw border top/bottom labels
  vfd->centerText("Mini Tetris (auto)", 0);
  // Render field rows 0..3 below label? Here we draw field directly 4 rows.
  for (uint8_t r = 0; r < FIELD_H; ++r) {
    // Build row string of width FIELD_W plus 2 borders, padded to 20
    char line[21];
    for (uint8_t i = 0; i < 20; ++i) line[i] = ' ';
    line[20] = '\0';
    uint8_t col = FIELD_LEFT;
    line[col-1] = '|';
    for (uint8_t c = 0; c < FIELD_W; ++c) {
      // Check active piece occupancy at (c,r)
      bool on = field[r][c];
      if (!on) {
        const Piece& p = PIECES[current.id];
        const PieceRot& pr = p.r[current.rot];
        for (uint8_t i = 0; i < 4; ++i) {
          int px = current.x + pr.c[i].x;
          int py = current.y + pr.c[i].y;
          if (px == (int)c && py == (int)r) { on = true; break; }
        }
      }
      line[col + c] = on ? (char)GLYPH_BLOCK : ' ';
    }
    line[col + FIELD_W] = '|';
    vfd->writeAt(FIELD_TOP + r, 0, line);
  }
}

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

  defineGlyphs();
  clearField();
  spawnPiece();
}

void loop() {
  static unsigned long last = 0;
  const unsigned long now = millis();
  if (now - last >= 350) { // drop speed
    last = now;
    tick();
    render();
  }
}

