#pragma once
#include "ITransport.h"


// ParallelTransport: Implements ITransport for parallel buses.
// Modes like 6800, 8080, 8086 should be added in actual implementation.
class ParallelTransport : public ITransport {
public:
ParallelTransport() {}


bool write(const uint8_t* data, size_t len) override {
if (_logger) _logger->onWrite(data, len);
return true; // TODO: implement actual parallel write
}


bool read(uint8_t* buffer, size_t len, size_t& outRead) override {
outRead = 0; return true; // TODO: implement parallel read
}


bool flush() override { return true; }
void delayMicroseconds(unsigned int us) override { ::delayMicroseconds(us); }


bool setControlLine(const char* name, bool level) override {
if (_logger) _logger->onControlLineChange(name, level);
return true;
}
bool pulseControlLine(const char* name, unsigned int us) override { return true; }
bool supportsControlLines() const override { return true; }
const char* name() const override { return "ParallelTransport"; }
};
