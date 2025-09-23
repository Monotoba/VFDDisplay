#pragma once
#include <Arduino.h>
#include <stdint.h>


// ILogger: Interface for debugging/logging transport events.
// Implementations may log to Serial, SD card, or disk.
class ILogger {
public:
virtual ~ILogger() = default;


// Called when data is written to transport
virtual void onWrite(const uint8_t* data, size_t len) = 0;


// Called when data is read from transport
virtual void onRead(const uint8_t* data, size_t len) = 0;


// Called when a control line changes state
virtual void onControlLineChange(const char* lineName, bool level) = 0;
};
