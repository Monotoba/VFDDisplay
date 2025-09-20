#pragma once
#include <Arduino.h>


class ILogger; // forward declaration


// ITransport: Abstract interface for all transport types (Serial, Stream, Parallel).
// Provides methods for writing/reading bytes and manipulating control lines.
class ITransport {
public:

virtual ~ITransport() = default;


// Write data to transport
virtual bool write(const uint8_t* data, size_t len) = 0;


// Read data from transport
virtual bool read(uint8_t* buffer, size_t len, size_t& outRead) = 0;


// Convenience method to write one byte
virtual bool writeByte(uint8_t b) { return write(&b, 1); }


// Flush buffered data (if applicable)
virtual bool flush() = 0;


// Control line manipulation (parallel buses)
virtual bool setControlLine(const char* name, bool level) { return false; }
virtual bool pulseControlLine(const char* name, unsigned int microseconds) { return false; }


// Timing utility
virtual void delayMicroseconds(unsigned int us) = 0;


// Capability query
virtual bool supportsControlLines() const = 0;


// Return name of the transport implementation
virtual const char* name() const = 0;


// Logger integration
virtual void attachLogger(ILogger* logger) { _logger = logger; }
virtual void detachLogger() { _logger = nullptr; }


protected:
ILogger* _logger = nullptr; // optional debug logger
};




