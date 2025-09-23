#pragma once
#include "Transports/ITransport.h"
#include <Arduino.h>


// SerialTransport: Implements ITransport for Serial/Stream objects.
class SerialTransport : public ITransport {
public:
    SerialTransport(Stream* serial) : _serial(serial) {}


bool write(const uint8_t* data, size_t len) override {
    if (_logger) _logger->onWrite(data, len);
    
    _serial->write(data, len);
    
    return true;
}


bool read(uint8_t* buffer, size_t len, size_t& outRead) override {
    outRead = _serial->available();

    if (outRead > len) outRead = len;

    for (size_t i=0; i<outRead; ++i) buffer[i] = _serial->read();

    if (_logger) _logger->onRead(buffer, outRead);

    return true;
}


bool flush() override { _serial->flush(); return true; }

void delayMicroseconds(unsigned int us) override { ::delayMicroseconds(us); }

bool supportsControlLines() const override { return false; }

const char* name() const override { return "SerialTransport"; }


private:
    Stream* _serial;
};
