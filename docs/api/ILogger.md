# ILogger Interface Documentation

## Overview

The `ILogger` interface provides a standardized way to log and debug transport operations in the VFD Display library. It allows developers to monitor data transmission, receive operations, and control line changes for troubleshooting and development purposes.

## Interface Definition

```cpp
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
```

## Method Documentation

### void onWrite(const uint8_t* data, size_t len)

Called by transport implementations when data is written to the interface.

**Parameters:**
- `data` - Pointer to the data buffer that was written
- `len` - Number of bytes that were written

**Implementation Notes:**
- This method is called after the write operation completes
- The data pointer is valid only during the method call
- Implementations should not modify the data
- Called for both single-byte and multi-byte write operations

**Example Implementation:**
```cpp
void onWrite(const uint8_t* data, size_t len) override {
    // Log the write operation with timestamp
    Serial.print("[WRITE ");
    Serial.print(millis());
    Serial.print("] ");
    
    for (size_t i = 0; i < len; i++) {
        if (data[i] < 16) Serial.print('0'); // Leading zero for hex
        Serial.print(data[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
}
```

### void onRead(const uint8_t* data, size_t len)

Called by transport implementations when data is read from the interface.

**Parameters:**
- `data` - Pointer to the buffer containing received data
- `len` - Number of bytes that were read

**Implementation Notes:**
- This method is called after the read operation completes
- The data pointer is valid only during the method call
- Implementations should not modify the data
- May be called with len=0 if no data was available

**Example Implementation:**
```cpp
void onRead(const uint8_t* data, size_t len) override {
    if (len == 0) return; // Don't log empty reads
    
    Serial.print("[READ ");
    Serial.print(millis());
    Serial.print("] ");
    
    for (size_t i = 0; i < len; i++) {
        if (data[i] < 16) Serial.print('0'); // Leading zero for hex
        Serial.print(data[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
}
```

### void onControlLineChange(const char* lineName, bool level)

Called by transport implementations when a control line changes state.

**Parameters:**
- `lineName` - Name of the control line that changed (e.g., "RS", "RW", "EN")
- `level` - New logic level (true = HIGH, false = LOW)

**Implementation Notes:**
- Only called for parallel transport implementations
- Line names are transport-specific
- Called whenever setControlLine() is called
- Useful for debugging parallel interface timing

**Example Implementation:**
```cpp
void onControlLineChange(const char* lineName, bool level) override {
    Serial.print("[LINE] ");
    Serial.print(lineName);
    Serial.print(" = ");
    Serial.println(level ? "HIGH" : "LOW");
}
```

## SerialLogger Implementation

### Overview
`SerialLogger` is the standard implementation of `ILogger` that outputs log messages to an Arduino Stream object (typically Serial).

### Class Definition
```cpp
class SerialLogger : public ILogger {
public:
    SerialLogger(Stream* serial);
    
    void onWrite(const uint8_t* data, size_t len) override;
    void onRead(const uint8_t* data, size_t len) override;
    void onControlLineChange(const char* lineName, bool level) override;

private:
    Stream* _serial;
};
```

### Implementation Details

#### Constructor
```cpp
SerialLogger::SerialLogger(Stream* serial) : _serial(serial) {}
```

#### onWrite() Method
```cpp
void SerialLogger::onWrite(const uint8_t* data, size_t len) {
    _serial->print("[WRITE] ");
    for (size_t i = 0; i < len; ++i) {
        if (data[i] < 16) _serial->print('0'); // Leading zero
        _serial->print(data[i], HEX);
        _serial->print(' ');
    }
    _serial->println();
}
```

#### onRead() Method
```cpp
void SerialLogger::onRead(const uint8_t* data, size_t len) {
    _serial->print("[READ] ");
    for (size_t i = 0; i < len; ++i) {
        if (data[i] < 16) _serial->print('0'); // Leading zero
        _serial->print(data[i], HEX);
        _serial->print(' ');
    }
    _serial->println();
}
```

#### onControlLineChange() Method
```cpp
void SerialLogger::onControlLineChange(const char* lineName, bool level) {
    _serial->print("[LINE] ");
    _serial->print(lineName);
    _serial->print(" = ");
    _serial->println(level ? "HIGH" : "LOW");
}
```

## Usage Examples

### Basic Serial Logging
```cpp
#include "Logger/SerialLogger.h"
#include "Transports/SerialTransport.h"

SerialLogger logger(&Serial);
SerialTransport transport(&Serial1);

void setup() {
    Serial.begin(57600);  // Debug serial
    Serial1.begin(19200, SERIAL_8N2);  // VFD serial
    
    // Attach logger to transport
    transport.attachLogger(&logger);
    
    // All transport operations will now be logged
}
```

### Conditional Logging
```cpp
class ConditionalLogger : public ILogger {
private:
    Stream* _serial;
    bool _enabled;
    
public:
    ConditionalLogger(Stream* serial) : _serial(serial), _enabled(true) {}
    
    void setEnabled(bool enabled) { _enabled = enabled; }
    
    void onWrite(const uint8_t* data, size_t len) override {
        if (!_enabled) return;
        
        _serial->print("[WRITE] ");
        for (size_t i = 0; i < len; ++i) {
            if (data[i] < 16) _serial->print('0');
            _serial->print(data[i], HEX);
            _serial->print(' ');
        }
        _serial->println();
    }
    
    void onRead(const uint8_t* data, size_t len) override {
        if (!_enabled) return;
        
        _serial->print("[READ] ");
        for (size_t i = 0; i < len; ++i) {
            if (data[i] < 16) _serial->print('0');
            _serial->print(data[i], HEX);
            _serial->print(' ');
        }
        _serial->println();
    }
    
    void onControlLineChange(const char* lineName, bool level) override {
        if (!_enabled) return;
        
        _serial->print("[LINE] ");
        _serial->print(lineName);
        _serial->print(" = ");
        _serial->println(level ? "HIGH" : "LOW");
    }
};
```

### Formatted Logging with ASCII
```cpp
class FormattedLogger : public ILogger {
private:
    Stream* _serial;
    
public:
    FormattedLogger(Stream* serial) : _serial(serial) {}
    
    void onWrite(const uint8_t* data, size_t len) override {
        _serial->print("[WRITE] ");
        
        // Hex output
        for (size_t i = 0; i < len; ++i) {
            if (data[i] < 16) _serial->print('0');
            _serial->print(data[i], HEX);
            _serial->print(' ');
        }
        
        // ASCII output (for printable characters)
        _serial->print(" (");
        for (size_t i = 0; i < len; ++i) {
            if (data[i] >= 32 && data[i] <= 126) {
                _serial->print((char)data[i]);
            } else {
                _serial->print('.'); // Non-printable
            }
        }
        _serial->println(")");
    }
    
    void onRead(const uint8_t* data, size_t len) override {
        _serial->print("[READ] ");
        
        // Similar formatting as onWrite...
    }
    
    void onControlLineChange(const char* lineName, bool level) override {
        _serial->print("[LINE] ");
        _serial->print(lineName);
        _serial->print(" = ");
        _serial->println(level ? "HIGH" : "LOW");
    }
};
```

### SD Card Logging
```cpp
#include <SD.h>

class SDLogger : public ILogger {
private:
    File _logFile;
    
public:
    SDLogger(const char* filename) {
        _logFile = SD.open(filename, FILE_WRITE);
    }
    
    ~SDLogger() {
        if (_logFile) _logFile.close();
    }
    
    void onWrite(const uint8_t* data, size_t len) override {
        if (!_logFile) return;
        
        _logFile.print("[WRITE ");
        _logFile.print(millis());
        _logFile.print("] ");
        
        for (size_t i = 0; i < len; ++i) {
            if (data[i] < 16) _logFile.print('0');
            _logFile.print(data[i], HEX);
            _logFile.print(' ');
        }
        _logFile.println();
        _logFile.flush(); // Ensure data is written
    }
    
    void onRead(const uint8_t* data, size_t len) override {
        if (!_logFile) return;
        
        // Similar implementation as onWrite...
    }
    
    void onControlLineChange(const char* lineName, bool level) override {
        if (!_logFile) return;
        
        _logFile.print("[LINE ");
        _logFile.print(millis());
        _logFile.print("] ");
        _logFile.print(lineName);
        _logFile.print(" = ");
        _logFile.println(level ? "HIGH" : "LOW");
        _logFile.flush();
    }
};
```

## Performance Considerations

### Serial Output Impact
- Logging to Serial can significantly slow down communication
- Consider using higher baud rates for debug serial (e.g., 115200)
- Disable logging in production code

### Memory Usage
- Loggers typically use minimal memory (just a pointer to the stream)
- SD card loggers may need additional buffer space
- Consider the impact of frequent flush() operations

### Timing Impact
- Logging adds delay to transport operations
- May affect timing-critical applications
- Consider conditional logging or buffering

## Best Practices

### Development vs Production
```cpp
#ifdef DEBUG
    SerialLogger logger(&Serial);
    transport.attachLogger(&logger);
#else
    // No logging in production
#endif
```

### Selective Logging
```cpp
class SelectiveLogger : public ILogger {
private:
    Stream* _serial;
    bool _logWrites;
    bool _logReads;
    bool _logControlLines;
    
public:
    SelectiveLogger(Stream* serial) : _serial(serial) {
        _logWrites = true;
        _logReads = false;  // Don't log reads by default
        _logControlLines = true;
    }
    
    void setLogWrites(bool enable) { _logWrites = enable; }
    void setLogReads(bool enable) { _logReads = enable; }
    void setLogControlLines(bool enable) { _logControlLines = enable; }
    
    void onWrite(const uint8_t* data, size_t len) override {
        if (!_logWrites) return;
        // Implementation...
    }
    
    void onRead(const uint8_t* data, size_t len) override {
        if (!_logReads) return;
        // Implementation...
    }
    
    void onControlLineChange(const char* lineName, bool level) override {
        if (!_logControlLines) return;
        // Implementation...
    }
};
```

### Error Handling
- Always validate parameters in logger implementations
- Handle null pointers gracefully
- Consider what happens if the output stream becomes unavailable

## Integration with Transport Layer

The logging system is integrated into the transport layer through the `attachLogger()` and `detachLogger()` methods:

```cpp
class ITransport {
public:
    virtual void attachLogger(ILogger* logger) { _logger = logger; }
    virtual void detachLogger() { _logger = nullptr; }
    
protected:
    ILogger* _logger = nullptr;
};
```

Transport implementations should call the appropriate logger methods:

```cpp
bool MyTransport::write(const uint8_t* data, size_t len) {
    if (_logger) _logger->onWrite(data, len);
    
    // Actual write implementation...
    return result;
}
```

## See Also

- [ITransport Interface](ITransport.md)
- [SerialTransport Class](ITransport.md#serialtransport-implementation)
- [VFDDisplay Class](VFDDisplay.md)
- [VFD20S401HAL Implementation](VFD20S401HAL.md)