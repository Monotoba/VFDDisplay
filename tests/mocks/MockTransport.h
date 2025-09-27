// Simple mock transport to capture bytes written by HALs
#pragma once

#include <Arduino.h>
#include "Transports/ITransport.h"

class MockTransport : public ITransport {
public:
  MockTransport() { clear(); }

  bool write(const uint8_t* data, size_t len) override {
    if (!data && len > 0) return false;
    for (size_t i = 0; i < len && _wpos < sizeof(_buf); ++i) {
      _buf[_wpos++] = data[i];
    }
    return true;
  }

  bool read(uint8_t* buffer, size_t len, size_t& outRead) override {
    outRead = 0; (void)buffer; (void)len; return true;
  }

  bool flush() override { return true; }

  void delayMicroseconds(unsigned int us) override { ::delayMicroseconds(us); }

  bool supportsControlLines() const override { return false; }

  const char* name() const override { return "MockTransport"; }

  void clear() { _wpos = 0; }

  size_t size() const { return _wpos; }

  const uint8_t* data() const { return _buf; }

  uint8_t at(size_t i) const { return (i < _wpos) ? _buf[i] : 0; }

  bool equals(const uint8_t* expected, size_t len) const {
    if (len != _wpos) return false;
    for (size_t i=0; i<len; ++i) if (_buf[i] != expected[i]) return false;
    return true;
  }

private:
  uint8_t _buf[1024];
  size_t _wpos = 0;
};
