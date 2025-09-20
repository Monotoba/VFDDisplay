#pragma once
#include "ILogger.h"
#include <Arduino.h>


// SerialLogger: ILogger implementation that logs events over a Serial/Stream object.
class SerialLogger : public ILogger {
public:
SerialLogger(Stream* serial) : _serial(serial) {}


void onWrite(const uint8_t* data, size_t len) override {
_serial->print("[WRITE] ");
for (size_t i=0; i<len; ++i) _serial->print(data[i], HEX);
_serial->println();
}


void onRead(const uint8_t* data, size_t len) override {
_serial->print("[READ] ");
for (size_t i=0; i<len; ++i) _serial->print(data[i], HEX);
_serial->println();
}


void onControlLineChange(const char* lineName, bool level) override {
_serial->print("[LINE] "); _serial->print(lineName); _serial->print("=");
_serial->println(level ? "HIGH" : "LOW");
}


private:
Stream* _serial;
};
