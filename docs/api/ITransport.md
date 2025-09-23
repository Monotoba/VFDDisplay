# ITransport Interface Documentation

## Overview

The `ITransport` interface provides an abstraction layer for different communication methods between the microcontroller and VFD display controllers. It supports both serial and parallel communication protocols, allowing the same HAL implementation to work with different hardware interfaces.

## Interface Definition

```cpp
class ITransport {
public:
    virtual ~ITransport() = default;
    
    // Data transfer methods
    virtual bool write(const uint8_t* data, size_t len) = 0;
    virtual bool read(uint8_t* buffer, size_t len, size_t& outRead) = 0;
    virtual bool writeByte(uint8_t b) { return write(&b, 1); }
    
    // Buffer management
    virtual bool flush() = 0;
    
    // Control line methods (for parallel interfaces)
    virtual bool setControlLine(const char* name, bool level) { return false; }
    virtual bool pulseControlLine(const char* name, unsigned int microseconds) { return false; }
    
    // Utility methods
    virtual void delayMicroseconds(unsigned int us) = 0;
    virtual bool supportsControlLines() const = 0;
    virtual const char* name() const = 0;
    
    // Logging integration
    virtual void attachLogger(ILogger* logger) { _logger = logger; }
    virtual void detachLogger() { _logger = nullptr; }

protected:
    ILogger* _logger = nullptr;
};
```

## Method Documentation

### Data Transfer Methods

#### bool write(const uint8_t* data, size_t len)

Writes a block of data to the transport interface.

**Parameters:**
- `data` - Pointer to array of bytes to write
- `len` - Number of bytes to write

**Returns:** `true` if write operation successful, `false` otherwise

**Implementation Notes:**
- Must handle the entire data block atomically if possible
- Should validate parameters (null pointers, zero length)
- Call logger's `onWrite()` method if logger is attached
- Implementation should handle transport-specific error conditions

**Example Implementation:**
```cpp
bool SerialTransport::write(const uint8_t* data, size_t len) {
    if (_logger) _logger->onWrite(data, len);
    
    if (!data || len == 0) return false;
    
    size_t written = _serial->write(data, len);
    return written == len;
}
```

#### bool read(uint8_t* buffer, size_t len, size_t& outRead)

Reads data from the transport interface into a buffer.

**Parameters:**
- `buffer` - Pointer to buffer to receive data
- `len` - Maximum number of bytes to read
- `outRead` - Reference to variable that receives actual number of bytes read

**Returns:** `true` if read operation successful, `false` otherwise

**Implementation Notes:**
- Should not block indefinitely - return available data
- `outRead` should be set to actual number of bytes read (may be 0)
- Call logger's `onRead()` method if logger is attached
- Should handle partial reads gracefully

**Example Implementation:**
```cpp
bool SerialTransport::read(uint8_t* buffer, size_t len, size_t& outRead) {
    outRead = _serial->available();
    if (outRead > len) outRead = len;
    
    for (size_t i = 0; i < outRead; ++i) {
        buffer[i] = _serial->read();
    }
    
    if (_logger) _logger->onRead(buffer, outRead);
    
    return true;
}
```

#### bool writeByte(uint8_t b)

Convenience method to write a single byte.

**Parameters:**
- `b` - Byte to write

**Returns:** `true` if write successful, `false` otherwise

**Implementation Notes:**
- Default implementation provided that calls `write(&b, 1)`
- Can be overridden for optimized single-byte operations

### Buffer Management

#### bool flush()

Flushes any buffered output data.

**Returns:** `true` if flush successful, `false` otherwise

**Implementation Notes:**
- For serial transports, typically calls the underlying stream's flush method
- For parallel transports, may be a no-op if no buffering
- Should ensure all pending data is transmitted

### Control Line Methods

#### bool setControlLine(const char* name, bool level)

Sets the state of a named control line (for parallel interfaces).

**Parameters:**
- `name` - Name of the control line (e.g., "RS", "RW", "EN")
- `level` - Desired logic level (true = HIGH, false = LOW)

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- Default implementation returns `false` (not supported)
- Only meaningful for parallel transport implementations
- Should call logger's `onControlLineChange()` if logger attached
- Line names are transport-specific

#### bool pulseControlLine(const char* name, unsigned int microseconds)

Pulses a control line high then low with specified duration.

**Parameters:**
- `name` - Name of the control line to pulse
- `microseconds` - Duration of pulse in microseconds

**Returns:** `true` if operation successful, `false` otherwise

**Implementation Notes:**
- Default implementation returns `false` (not supported)
- Typically implemented as: set HIGH → delay → set LOW
- Duration should be reasonably accurate
- Used for timing-critical parallel interface operations

### Utility Methods

#### void delayMicroseconds(unsigned int us)

Provides microsecond-precision delay.

**Parameters:**
- `us` - Microseconds to delay

**Implementation Notes:**
- Typically delegates to Arduino's `delayMicroseconds()` function
- Can be overridden for custom timing requirements
- Should be accurate for short delays (< 1ms)

#### bool supportsControlLines() const

Reports whether this transport supports control line operations.

**Returns:** `true` if control lines are supported, `false` otherwise

**Implementation Notes:**
- Serial transports typically return `false`
- Parallel transports should return `true`
- Used by HAL to determine available functionality

#### const char* name() const

Returns a descriptive name for the transport implementation.

**Returns:** Static string identifying the transport type

**Implementation Notes:**
- Should return a unique, descriptive name
- Examples: "SerialTransport", "ParallelTransport", "I2CTransport"
- Used for debugging and logging purposes

### Logging Integration

#### void attachLogger(ILogger* logger)

Attaches a logger for debugging transport operations.

**Parameters:**
- `logger` - Pointer to logger implementation, or nullptr to detach

**Implementation Notes:**
- Stores logger pointer in protected `_logger` member
- Logger methods are called for significant operations
- Should handle nullptr gracefully (detach logging)

#### void detachLogger()

Detaches the current logger.

**Implementation Notes:**
- Sets `_logger` to nullptr
- Equivalent to calling `attachLogger(nullptr)`

## SerialTransport Implementation

### Overview
`SerialTransport` implements the `ITransport` interface for Arduino Stream objects (HardwareSerial, SoftwareSerial, etc.).

### Class Definition
```cpp
class SerialTransport : public ITransport {
public:
    SerialTransport(Stream* serial);
    
    bool write(const uint8_t* data, size_t len) override;
    bool read(uint8_t* buffer, size_t len, size_t& outRead) override;
    bool flush() override;
    void delayMicroseconds(unsigned int us) override;
    bool supportsControlLines() const override;
    const char* name() const override;

private:
    Stream* _serial;
};
```

### Implementation Details

#### Constructor
```cpp
SerialTransport::SerialTransport(Stream* serial) : _serial(serial) {}
```

#### write() Method
```cpp
bool SerialTransport::write(const uint8_t* data, size_t len) {
    if (_logger) _logger->onWrite(data, len);
    
    _serial->write(data, len);
    return true;
}
```

#### read() Method
```cpp
bool SerialTransport::read(uint8_t* buffer, size_t len, size_t& outRead) {
    outRead = _serial->available();
    if (outRead > len) outRead = len;
    
    for (size_t i = 0; i < outRead; ++i) {
        buffer[i] = _serial->read();
    }
    
    if (_logger) _logger->onRead(buffer, outRead);
    
    return true;
}
```

#### flush() Method
```cpp
bool SerialTransport::flush() { 
    _serial->flush(); 
    return true; 
}
```

#### Utility Methods
```cpp
void delayMicroseconds(unsigned int us) override { 
    ::delayMicroseconds(us); 
}

bool supportsControlLines() const override { 
    return false; 
}

const char* name() const override { 
    return "SerialTransport"; 
}
```

## ParallelTransport Implementation

### Overview
`ParallelTransport` provides a stub implementation for parallel bus communication. This is a template that needs to be completed with actual parallel interface code.

### Class Definition
```cpp
class ParallelTransport : public ITransport {
public:
    ParallelTransport() {}
    
    bool write(const uint8_t* data, size_t len) override;
    bool read(uint8_t* buffer, size_t len, size_t& outRead) override;
    bool flush() override;
    bool setControlLine(const char* name, bool level) override;
    bool pulseControlLine(const char* name, unsigned int us) override;
    void delayMicroseconds(unsigned int us) override;
    bool supportsControlLines() const override;
    const char* name() const override;
};
```

### Implementation Details

#### write() Method
```cpp
bool ParallelTransport::write(const uint8_t* data, size_t len) {
    if (_logger) _logger->onWrite(data, len);
    return true; // TODO: implement actual parallel write
}
```

#### Control Line Methods
```cpp
bool setControlLine(const char* name, bool level) override {
    if (_logger) _logger->onControlLineChange(name, level);
    return true; // TODO: implement actual control line setting
}

bool pulseControlLine(const char* name, unsigned int us) override {
    // TODO: implement actual control line pulsing
    return true;
}
```

#### Utility Methods
```cpp
bool supportsControlLines() const override { 
    return true; 
}

const char* name() const override { 
    return "ParallelTransport"; 
}
```

## Usage Examples

### Basic Serial Communication
```cpp
#include "Transports/SerialTransport.h"

// Using HardwareSerial
HardwareSerial& vfdSerial = Serial1;
SerialTransport transport(&vfdSerial);

void setup() {
    vfdSerial.begin(19200, SERIAL_8N2);
    
    // Send data
    uint8_t data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
    transport.write(data, sizeof(data));
    
    // Flush to ensure transmission
    transport.flush();
}
```

### With Logging
```cpp
#include "Transports/SerialTransport.h"
#include "Logger/SerialLogger.h"

SerialTransport transport(&Serial1);
SerialLogger logger(&Serial);

void setup() {
    Serial.begin(57600);  // Debug serial
    Serial1.begin(19200, SERIAL_8N2);  // VFD serial
    
    // Attach logger
    transport.attachLogger(&logger);
    
    // All transport operations will now be logged
    transport.writeByte(0x48); // Will log: [WRITE] 48
}
```

### Reading Data
```cpp
uint8_t buffer[32];
size_t bytesRead;

if (transport.read(buffer, sizeof(buffer), bytesRead)) {
    Serial.print("Received ");
    Serial.print(bytesRead);
    Serial.println(" bytes");
    
    for (size_t i = 0; i < bytesRead; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(' ');
    }
}
```

## Error Handling

Transport implementations should handle common error conditions:

- **Null Pointers**: Check for null data pointers before operations
- **Zero Length**: Handle zero-length operations gracefully
- **Transport Errors**: Detect and report hardware communication errors
- **Buffer Overflows**: Prevent buffer overflows in read operations

## Performance Considerations

- **Serial Transports**: Performance limited by baud rate and serial buffer size
- **Parallel Transports**: Generally faster but require more pins
- **Blocking vs Non-blocking**: Read operations should be non-blocking
- **Buffer Management**: Consider buffer sizes for your application requirements

## Thread Safety

Transport implementations are not thread-safe. All operations should be called from the same thread/context, typically the main Arduino loop.

## Extending the Transport Layer

To create custom transport implementations:

1. Inherit from `ITransport`
2. Implement all pure virtual methods
3. Override default methods as needed
4. Handle logging integration properly
5. Provide descriptive name() implementation

### Example Custom Transport
```cpp
class CustomTransport : public ITransport {
private:
    CustomHardware* _hardware;
    
public:
    CustomTransport(CustomHardware* hw) : _hardware(hw) {}
    
    bool write(const uint8_t* data, size_t len) override {
        if (_logger) _logger->onWrite(data, len);
        return _hardware->send(data, len);
    }
    
    bool read(uint8_t* buffer, size_t len, size_t& outRead) override {
        outRead = _hardware->receive(buffer, len);
        if (_logger) _logger->onRead(buffer, outRead);
        return true;
    }
    
    // Implement other methods...
};
```

## See Also

- [VFDDisplay Class](VFDDisplay.md)
- [IVFDHAL Interface](IVFDHAL.md)
- [SerialLogger Class](SerialLogger.md)
- [VFD20S401HAL Implementation](VFD20S401HAL.md)