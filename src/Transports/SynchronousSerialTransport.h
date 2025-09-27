#pragma once
#include "Transports/ITransport.h"
#include <Arduino.h>

// SynchronousSerialTransport (/STB, SCK, SIO) for 20T202-like modules.
// Implements the 3‑wire serial protocol with a Start byte containing RS (bit6)
// and R/W (bit5), followed by one instruction/data byte per transfer.
//
// Notes:
// - Bit order is MSB-first; data is sampled on the falling edge of SCK per datasheets.
// - /STB is driven low for the duration of a transfer, then returned high.
// - Only write path (R/W=0) is implemented; read() returns false.
// - HALs can set RS via setControlLine("RS", level). Other control lines are ignored.
class SynchronousSerialTransport : public ITransport {
public:
  SynchronousSerialTransport(uint8_t stbPin, uint8_t sckPin, uint8_t sioPin,
                             unsigned int sckDelayUs = 2)
    : _stb(stbPin), _sck(sckPin), _sio(sioPin), _sckDelay(sckDelayUs) {
    pinMode(_stb, OUTPUT);
    pinMode(_sck, OUTPUT);
    pinMode(_sio, OUTPUT);
    digitalWrite(_stb, HIGH); // idle
    digitalWrite(_sck, LOW);
    digitalWrite(_sio, LOW);
  }

  bool write(const uint8_t* data, size_t len) override {
    if (!data || len == 0) return false;
    for (size_t i = 0; i < len; ++i) {
      if (!beginTransfer()) return false;
      // Start byte: include RS (bit6) and R/W=0 (bit5=0)
      uint8_t start = (uint8_t)((_rs ? 0x40 : 0x00) | 0x00);
      if (!shiftOutByte(start)) { endTransfer(); return false; }
      if (!shiftOutByte(data[i])) { endTransfer(); return false; }
      if (!endTransfer()) return false;
      if (_logger) _logger->onWrite(&data[i], 1);
    }
    return true;
  }

  bool read(uint8_t* /*buffer*/, size_t /*len*/, size_t& outRead) override {
    outRead = 0; return false; // not implemented
  }

  bool flush() override { return true; }

  void delayMicroseconds(unsigned int us) override { ::delayMicroseconds(us); }

  bool supportsControlLines() const override { return true; }

  const char* name() const override { return "SynchronousSerialTransport"; }

  bool setControlLine(const char* name, bool level) override {
    if (!name) return false;
    if (strcmp(name, "RS") == 0) { _rs = level; return true; }
    return false;
  }

  bool pulseControlLine(const char* /*name*/, unsigned int /*microseconds*/) override {
    return true; // not used
  }

private:
  uint8_t _stb, _sck, _sio;
  bool _rs = false;
  unsigned int _sckDelay; // half-cycle delay in us

  bool beginTransfer() {
    digitalWrite(_stb, LOW);
    delayMicroseconds(_sckDelay);
    return true;
  }

  bool endTransfer() {
    delayMicroseconds(_sckDelay);
    digitalWrite(_stb, HIGH);
    delayMicroseconds(_sckDelay);
    return true;
  }

  bool shiftOutByte(uint8_t b) {
    for (int i = 7; i >= 0; --i) {
      // Present bit while SCK low
      digitalWrite(_sio, (b >> i) & 0x01);
      delayMicroseconds(_sckDelay);
      // Rising edge
      digitalWrite(_sck, HIGH);
      delayMicroseconds(_sckDelay);
      // Falling edge (sampling edge on receiver)
      digitalWrite(_sck, LOW);
    }
    return true;
  }
};

