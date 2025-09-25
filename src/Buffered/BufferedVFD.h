#pragma once
#include <Arduino.h>
#include "HAL/IVFDHAL.h"

// BufferedVFD: device-agnostic buffered renderer + simple animations.
class BufferedVFD {
public:
  explicit BufferedVFD(IVFDHAL* hal) : _hal(hal) {}

  bool init() {
    if (!_hal) return false;
    const IDisplayCapabilities* caps = _hal->getDisplayCapabilities();
    if (!caps) return false;
    _rows = caps->getTextRows();
    _cols = caps->getTextColumns();
    if (_rows == 0 || _cols == 0 || _rows > MAX_ROWS || _cols > MAX_COLS) return false;
    // set buffers to spaces
    clearBuffer();
    memcpy(_back, _front, sizeof(_front));
    return true;
  }

  // Buffer API
  void clearBuffer() {
    for (uint8_t r=0; r<MAX_ROWS; ++r)
      for (uint8_t c=0; c<MAX_COLS; ++c)
        _front[r][c] = ' ';
  }

  bool writeAt(uint8_t row, uint8_t col, const char* text) {
    if (!text || row >= _rows || col >= _cols) return false;
    uint8_t i=0; while (text[i] && (col+i) < _cols) { _front[row][col+i] = text[i]; ++i; }
    return true;
  }

  bool centerText(uint8_t row, const char* text) {
    if (!text || row >= _rows) return false;
    size_t len = strlen(text); if (len > _cols) len = _cols;
    uint8_t pad = (_cols - len)/2;
    // clear row
    for (uint8_t c=0; c<_cols; ++c) _front[row][c] = ' ';
    for (uint8_t i=0; i<len; ++i) _front[row][pad+i] = text[i];
    return true;
  }

  // Flush full buffer to device
  bool flush() {
    if (!_hal) return false;
    bool ok=true;
    for (uint8_t r=0; r<_rows; ++r) {
      ok &= _hal->writeAt(r, 0, rowPtr(_front, r));
    }
    return ok;
  }

  // Flush only changed runs per row
  bool flushDiff() {
    if (!_hal) return false;
    bool ok=true;
    for (uint8_t r=0; r<_rows; ++r) {
      uint8_t c=0;
      while (c < _cols) {
        // find diff start
        while (c < _cols && _front[r][c] == _back[r][c]) c++;
        if (c >= _cols) break;
        // find run end
        uint8_t start = c;
        while (c < _cols && _front[r][c] != _back[r][c]) c++;
        uint8_t end = c; // [start,end)
        char tmp[MAX_COLS+1];
        uint8_t n=0; for (uint8_t i=start; i<end; ++i) tmp[n++]=_front[r][i];
        tmp[n]='\0';
        ok &= _hal->writeAt(r, start, tmp);
      }
    }
    // sync back buffer
    memcpy(_back, _front, sizeof(_front));
    return ok;
  }

  // Animations (non-blocking): call steps from loop with millis()
  bool hScrollBegin(uint8_t row, const char* text, uint16_t speedMs) {
    if (!text || row >= _rows) return false;
    strncpy(_h.text, text, sizeof(_h.text)-1); _h.text[sizeof(_h.text)-1]='\0';
    _h.row=row; _h.speed=speedMs; _h.offset=0; _h.active=true; _h.last=0;
    return true;
  }
  void hScrollStop() { _h.active=false; }
  void hScrollStep(uint32_t nowMs) {
    if (!_h.active) return;
    if (_h.last != 0 && (nowMs - _h.last) < _h.speed) return;
    _h.last = nowMs;
    // render window into buffer
    // shift left by one
    _h.offset = (_h.offset + 1) % (strlen(_h.text) + _cols);
    for (uint8_t i=0;i<_cols;++i) {
      int idx = _h.offset + i;
      char ch=' ';
      int tlen = (int)strlen(_h.text);
      if (idx < tlen) ch = _h.text[idx];
      else if (idx < tlen + _cols) ch = ' ';
      else { int wrap = idx - (tlen + _cols); if (wrap>=0 && wrap<tlen) ch=_h.text[wrap]; }
      _front[_h.row][i]=ch;
    }
  }

  bool vScrollBegin(const char* text, uint8_t startRow, int8_t dir, uint16_t speedMs) {
    if (!text || startRow >= _rows) return false;
    strncpy(_v.text, text, sizeof(_v.text)-1); _v.text[sizeof(_v.text)-1]='\0';
    _v.start=startRow; _v.dir=dir; _v.speed=speedMs; _v.last=0; _v.active=true; _v.offset=0;
    // count lines
    _v.lines=1; for (const char* p=_v.text; *p; ++p) if (*p=='\n') _v.lines++;
    return true;
  }
  void vScrollStop() { _v.active=false; }
  void vScrollStep(uint32_t nowMs) {
    if (!_v.active) return;
    if (_v.last != 0 && (nowMs - _v.last) < _v.speed) return;
    _v.last = nowMs;
    // advance offset
    if (_v.dir>0) _v.offset = (_v.offset+1) % _v.lines; else _v.offset = (_v.offset+_v.lines-1)%_v.lines;
    // render visible rows
    for (uint8_t r=0; r<(_rows - _v.start); ++r) {
      uint8_t line = (_v.offset + r) % _v.lines;
      const char* s = nthLine(_v.text, line);
      // copy up to _cols
      for (uint8_t c=0; c<_cols; ++c) { char ch = s && s[c] && s[c] != '\n' ? s[c] : ' '; _front[_v.start+r][c]=ch; }
    }
  }

  bool flashBegin(uint8_t row,uint8_t col,const char* text,uint16_t onMs,uint16_t offMs,uint8_t repeat=2){
    if (!text || row>=_rows || col>=_cols) return false;
    strncpy(_f.text,text,sizeof(_f.text)-1); _f.text[sizeof(_f.text)-1]='\0';
    _f.row=row; _f.col=col; _f.on=onMs; _f.off=offMs; _f.repeat=repeat; _f.state=0; _f.last=0; _f.active=true;
    return true;
  }
  void flashStop(){ _f.active=false; }
  void flashStep(uint32_t nowMs){
    if (!_f.active) return;
    if (_f.last==0){ _f.last = nowMs; drawFlash(true); return; }
    uint16_t span = (_f.state%2==0)? _f.on : _f.off;
    if ((nowMs - _f.last) >= span){ _f.state++; _f.last = nowMs; bool on = (_f.state%2==0); drawFlash(on); if (_f.state/2 >= _f.repeat) _f.active=false; }
  }

private:
  static constexpr uint8_t MAX_ROWS = 8;
  static constexpr uint8_t MAX_COLS = 40;
  IVFDHAL* _hal = nullptr;
  uint8_t _rows=0, _cols=0;
  char _front[MAX_ROWS][MAX_COLS]{};
  char _back[MAX_ROWS][MAX_COLS]{};

  struct HState { uint8_t row=0; uint16_t speed=0; uint16_t offset=0; bool active=false; uint32_t last=0; char text[160]{}; } _h;
  struct VState { uint8_t start=0; int8_t dir=1; uint16_t speed=0; uint32_t last=0; bool active=false; uint8_t offset=0; uint8_t lines=0; char text[256]{}; } _v;
  struct FState { uint8_t row=0,col=0; uint16_t on=0,off=0; uint8_t repeat=0; bool active=false; uint32_t last=0; uint8_t state=0; char text[40]{}; } _f;

  static const char* nthLine(const char* s, uint8_t n){
    if (!s) return nullptr; uint8_t cur=0; const char* p=s; if (n==0) return s; while (*p){ if (*p=='\n'){ cur++; if (cur==n) return p+1; } p++; } return nullptr; }
  static char* rowPtr(char buf[][MAX_COLS], uint8_t r){ buf[r][MAX_COLS-0] = '\0'; return buf[r]; }
  void drawFlash(bool on){ if (on) writeAt(_f.row,_f.col,_f.text); else { char tmp[40]; uint8_t n=min<uint8_t>(strlen(_f.text), _cols - _f.col); for(uint8_t i=0;i<n;++i) tmp[i]=' '; tmp[n]='\0'; writeAt(_f.row,_f.col,tmp);} }
};

