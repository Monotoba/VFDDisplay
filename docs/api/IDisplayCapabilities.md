# IDisplayCapabilities Interface Documentation

## Overview

The `IDisplayCapabilities` interface provides a standardized way to query display characteristics and supported features. It allows applications to adapt their behavior based on the specific capabilities of the connected VFD display, enabling portable code that works across different display models.

## Interface Definition

```cpp
class IDisplayCapabilities {
public:
    virtual ~IDisplayCapabilities() = default;
    
    // Basic display dimensions
    virtual uint8_t getTextRows() const = 0;
    virtual uint8_t getTextColumns() const = 0;
    virtual uint8_t getCharacterPixelWidth() const = 0;
    virtual uint8_t getCharacterPixelHeight() const = 0;
    
    // Physical dimensions in millimeters
    virtual uint16_t getDisplayWidthMM() const = 0;
    virtual uint16_t getDisplayHeightMM() const = 0;
    
    // Feature support queries
    virtual bool hasCapability(DisplayCapabilityFlag flag) const = 0;
    virtual uint16_t getAllCapabilities() const = 0;
    
    // Detailed capability information
    virtual uint8_t getMaxCursorBlinkSpeeds() const = 0;
    virtual uint8_t getMaxUserDefinedCharacters() const = 0;
    virtual uint8_t getDimmingLevels() const = 0;
    virtual uint8_t getMaxBrightnessLevels() const = 0;
    
    // Supported display modes
    virtual bool supportsDisplayMode(DisplayMode mode) const = 0;
    virtual uint8_t getSupportedDisplayModesCount() const = 0;
    
    // Interface support
    virtual bool supportsInterface(const char* interfaceName) const = 0;
    virtual uint8_t getSupportedInterfacesCount() const = 0;
    virtual const char* getSupportedInterface(uint8_t index) const = 0;
    
    // Timing information
    virtual uint16_t getMinCommandDelayMicros() const = 0;
    virtual uint16_t getMaxCommandDelayMicros() const = 0;
    virtual uint16_t getResetDelayMillis() const = 0;
    
    // Power consumption
    virtual uint16_t getTypicalPowerConsumptionMW() const = 0;
    virtual uint16_t getMaxPowerConsumptionMW() const = 0;
    
    // Device identification
    virtual const char* getDeviceName() const = 0;
    virtual const char* getDeviceDescription() const = 0;
    virtual const char* getManufacturer() const = 0;
    virtual const char* getPartNumber() const = 0;
    
    // Version and compatibility
    virtual uint8_t getCapabilityVersion() const = 0;
    virtual bool isCompatibleWith(uint8_t requiredVersion) const = 0;
};
```

## Capability Flags

```cpp
enum DisplayCapabilityFlag : uint16_t {
    CAP_NONE                    = 0,
    CAP_CURSOR                  = 1 << 0,
    CAP_CURSOR_BLINK            = 1 << 1,
    CAP_DIMMING                 = 1 << 2,
    CAP_SELF_TEST               = 1 << 3,
    CAP_USER_DEFINED_CHARS      = 1 << 4,
    CAP_DISPLAY_MODES           = 1 << 5,
    CAP_HORIZONTAL_SCROLL       = 1 << 6,
    CAP_VERTICAL_SCROLL         = 1 << 7,
    CAP_FLASH_TEXT              = 1 << 8,
    CAP_BRIGHTNESS_CONTROL      = 1 << 9,
    CAP_CUSTOM_COMMANDS         = 1 << 10,
    CAP_PARALLEL_INTERFACE      = 1 << 11,
    CAP_SERIAL_INTERFACE        = 1 << 12,
    CAP_SPI_INTERFACE           = 1 << 13,
    CAP_I2C_INTERFACE           = 1 << 14,
    CAP_ALL                     = 0xFFFF
};
```

## Display Modes

```cpp
enum DisplayMode : uint8_t {
    MODE_NORMAL = 0,
    MODE_INVERSE,
    MODE_BLINK,
    MODE_DIMMED,
    MODE_BRIGHT,
    MODE_COUNT
};
```

## Method Documentation

### Basic Display Dimensions

#### uint8_t getTextRows() const

Returns the number of text rows supported by the display.

**Returns:** Number of rows (e.g., 2 for 16x2, 4 for 20x4)

**Example:**
```cpp
const IDisplayCapabilities* caps = vfd->getDisplayCapabilities();
uint8_t rows = caps->getTextRows(); // Returns 4 for VFD20S401
```

#### uint8_t getTextColumns() const

Returns the number of text columns supported by the display.

**Returns:** Number of columns (e.g., 16 for 16x2, 20 for 20x4)

**Example:**
```cpp
uint8_t cols = caps->getTextColumns(); // Returns 20 for VFD20S401
```

#### uint8_t getCharacterPixelWidth() const

Returns the pixel width of individual characters.

**Returns:** Character width in pixels (typically 5 for dot matrix displays)

**Example:**
```cpp
uint8_t charWidth = caps->getCharacterPixelWidth(); // Returns 5 for VFD20S401
```

#### uint8_t getCharacterPixelHeight() const

Returns the pixel height of individual characters.

**Returns:** Character height in pixels (typically 8 for dot matrix displays)

**Example:**
```cpp
uint8_t charHeight = caps->getCharacterPixelHeight(); // Returns 8 for VFD20S401
```

### Physical Dimensions

#### uint16_t getDisplayWidthMM() const

Returns the physical width of the display in millimeters.

**Returns:** Width in millimeters, or 0 if unknown

**Example:**
```cpp
uint16_t width = caps->getDisplayWidthMM(); // Returns 116 for VFD20S401
```

#### uint16_t getDisplayHeightMM() const

Returns the physical height of the display in millimeters.

**Returns:** Height in millimeters, or 0 if unknown

**Example:**
```cpp
uint16_t height = caps->getDisplayHeightMM(); // Returns 32 for VFD20S401
```

### Feature Support Queries

#### bool hasCapability(DisplayCapabilityFlag flag) const

Checks if the display supports a specific capability.

**Parameters:**
- `flag` - Capability flag to check

**Returns:** `true` if capability is supported, `false` otherwise

**Example:**
```cpp
if (caps->hasCapability(CAP_VERTICAL_SCROLL)) {
    vfd->vScrollText("Scrolling text", 0, SCROLL_UP);
}

if (caps->hasCapability(CAP_BRIGHTNESS_CONTROL)) {
    vfd->setBrightness(128);
}
```

#### uint16_t getAllCapabilities() const

Returns a bitmask of all supported capabilities.

**Returns:** Bitmask of capability flags

**Example:**
```cpp
uint16_t allCaps = caps->getAllCapabilities();
if (allCaps & CAP_CURSOR_BLINK) {
    // Cursor blinking is supported
}
```

### Detailed Capability Information

#### uint8_t getMaxCursorBlinkSpeeds() const

Returns the number of different cursor blink speeds supported.

**Returns:** Number of blink speed settings (0 if not supported)

**Example:**
```cpp
uint8_t blinkSpeeds = caps->getMaxCursorBlinkSpeeds(); // Returns 4 for VFD20S401
```

#### uint8_t getMaxUserDefinedCharacters() const

Returns the maximum number of user-defined characters that can be stored.

**Returns:** Number of custom character slots (0 if not supported)

**Example:**
```cpp
uint8_t customChars = caps->getMaxUserDefinedCharacters(); // Returns 8 for VFD20S401
```

#### uint8_t getDimmingLevels() const

Returns the number of dimming levels supported.

**Returns:** Number of dimming settings (0 if not supported)

**Example:**
```cpp
uint8_t dimLevels = caps->getDimmingLevels(); // Returns 8 for VFD20S401
```

#### uint8_t getMaxBrightnessLevels() const

Returns the number of brightness levels supported.

**Returns:** Number of brightness settings (0 if not supported)

**Example:**
```cpp
uint8_t brightLevels = caps->getMaxBrightnessLevels(); // Returns 16 for VFD20S401
```

### Display Mode Support

#### bool supportsDisplayMode(DisplayMode mode) const

Checks if a specific display mode is supported.

**Parameters:**
- `mode` - Display mode to check

**Returns:** `true` if mode is supported, `false` otherwise

**Example:**
```cpp
if (caps->supportsDisplayMode(MODE_INVERSE)) {
    vfd->setDisplayMode(inverseMode);
}
```

#### uint8_t getSupportedDisplayModesCount() const

Returns the number of display modes supported.

**Returns:** Number of supported modes

**Example:**
```cpp
uint8_t modeCount = caps->getSupportedDisplayModesCount();
```

### Interface Support

#### bool supportsInterface(const char* interfaceName) const

Checks if the display supports a specific communication interface.

**Parameters:**
- `interfaceName` - Name of the interface (e.g., "Serial", "Parallel", "I2C")

**Returns:** `true` if interface is supported, `false` otherwise

**Example:**
```cpp
if (caps->supportsInterface("Serial")) {
    // Use serial communication
}
```

#### uint8_t getSupportedInterfacesCount() const

Returns the number of communication interfaces supported.

**Returns:** Number of supported interfaces

**Example:**
```cpp
uint8_t interfaceCount = caps->getSupportedInterfacesCount();
```

#### const char* getSupportedInterface(uint8_t index) const

Returns the name of a supported interface by index.

**Parameters:**
- `index` - Zero-based index of the interface

**Returns:** Interface name string, or nullptr if index is invalid

**Example:**
```cpp
for (uint8_t i = 0; i < caps->getSupportedInterfacesCount(); i++) {
    const char* interface = caps->getSupportedInterface(i);
    Serial.print("Supports: ");
    Serial.println(interface);
}
```

### Timing Information

#### uint16_t getMinCommandDelayMicros() const

Returns the minimum delay required between commands in microseconds.

**Returns:** Minimum delay in microseconds

**Example:**
```cpp
uint16_t minDelay = caps->getMinCommandDelayMicros(); // Returns 10 for VFD20S401
```

#### uint16_t getMaxCommandDelayMicros() const

Returns the maximum delay that might be needed between commands in microseconds.

**Returns:** Maximum delay in microseconds

**Example:**
```cpp
uint16_t maxDelay = caps->getMaxCommandDelayMicros(); // Returns 100 for VFD20S401
```

#### uint16_t getResetDelayMillis() const

Returns the delay required after a reset operation in milliseconds.

**Returns:** Reset delay in milliseconds

**Example:**
```cpp
uint16_t resetDelay = caps->getResetDelayMillis(); // Returns 100 for VFD20S401
```

### Power Consumption

#### uint16_t getTypicalPowerConsumptionMW() const

Returns the typical power consumption in milliwatts.

**Returns:** Typical power consumption, or 0 if unknown

**Example:**
```cpp
uint16_t typicalPower = caps->getTypicalPowerConsumptionMW(); // Returns 800 for VFD20S401
```

#### uint16_t getMaxPowerConsumptionMW() const

Returns the maximum power consumption in milliwatts.

**Returns:** Maximum power consumption, or 0 if unknown

**Example:**
```cpp
uint16_t maxPower = caps->getMaxPowerConsumptionMW(); // Returns 1500 for VFD20S401
```

### Device Identification

#### const char* getDeviceName() const

Returns the device name.

**Returns:** Static string with device name

**Example:**
```cpp
const char* name = caps->getDeviceName(); // Returns "VFD20S401"
```

#### const char* getDeviceDescription() const

Returns a detailed description of the device.

**Returns:** Static string with device description

**Example:**
```cpp
const char* desc = caps->getDeviceDescription(); 
// Returns "20x4 Vacuum Fluorescent Display with 5x8 dot matrix characters"
```

#### const char* getManufacturer() const

Returns the device manufacturer.

**Returns:** Static string with manufacturer name

**Example:**
```cpp
const char* manufacturer = caps->getManufacturer(); // Returns "Futaba"
```

#### const char* getPartNumber() const

Returns the device part number.

**Returns:** Static string with part number

**Example:**
```cpp
const char* partNumber = caps->getPartNumber(); // Returns "VFD20S401DA1"
```

### Version and Compatibility

#### uint8_t getCapabilityVersion() const

Returns the capability interface version.

**Returns:** Version number (currently 1)

**Example:**
```cpp
uint8_t version = caps->getCapabilityVersion(); // Returns 1
```

#### bool isCompatibleWith(uint8_t requiredVersion) const

Checks if the capabilities are compatible with a required version.

**Parameters:**
- `requiredVersion` - Minimum required version

**Returns:** `true` if compatible, `false` otherwise

**Example:**
```cpp
if (caps->isCompatibleWith(1)) {
    // Capabilities are compatible with version 1
}
```

## Usage Examples

### Basic Capability Query
```cpp
const IDisplayCapabilities* caps = vfd->getDisplayCapabilities();

Serial.print("Display: ");
Serial.println(caps->getDeviceName());

Serial.print("Size: ");
Serial.print(caps->getTextColumns());
Serial.print("x");
Serial.println(caps->getTextRows());
```

### Feature Detection
```cpp
// Check available features
if (caps->hasCapability(CAP_VERTICAL_SCROLL)) {
    Serial.println("Vertical scrolling supported");
}

if (caps->hasCapability(CAP_USER_DEFINED_CHARS)) {
    Serial.print("Custom characters: ");
    Serial.print(caps->getMaxUserDefinedCharacters());
    Serial.println(" slots available");
}

if (caps->hasCapability(CAP_BRIGHTNESS_CONTROL)) {
    Serial.print("Brightness levels: ");
    Serial.println(caps->getMaxBrightnessLevels());
}
```

### Adaptive Interface Selection
```cpp
// Choose communication interface
if (caps->supportsInterface("I2C")) {
    // Use I2C transport
} else if (caps->supportsInterface("Serial")) {
    // Use serial transport
} else if (caps->supportsInterface("Parallel")) {
    // Use parallel transport
}
```

### Power Management
```cpp
// Display power consumption information
uint16_t typical = caps->getTypicalPowerConsumptionMW();
uint16_t maximum = caps->getMaxPowerConsumptionMW();

if (typical > 0) {
    Serial.print("Typical power: ");
    Serial.print(typical);
    Serial.println(" mW");
}

if (maximum > 0) {
    Serial.print("Maximum power: ");
    Serial.print(maximum);
    Serial.println(" mW");
}
```

### Timing Configuration
```cpp
// Get timing requirements
uint16_t minDelay = caps->getMinCommandDelayMicros();
uint16_t resetDelay = caps->getResetDelayMillis();

Serial.print("Min command delay: ");
Serial.print(minDelay);
Serial.println(" μs");

Serial.print("Reset delay: ");
Serial.print(resetDelay);
Serial.println(" ms");
```

## Implementation Notes

### Creating Custom Capabilities

To implement custom display capabilities:

1. Inherit from `IDisplayCapabilities`
2. Implement all pure virtual methods
3. Provide accurate information for your specific display
4. Consider using `DisplayCapabilities` as a base class for convenience

### Example Custom Implementation
```cpp
class MyCustomCapabilities : public IDisplayCapabilities {
public:
    uint8_t getTextRows() const override { return 2; }
    uint8_t getTextColumns() const override { return 16; }
    uint8_t getCharacterPixelWidth() const override { return 5; }
    uint8_t getCharacterPixelHeight() const override { return 8; }
    
    bool hasCapability(DisplayCapabilityFlag flag) const override {
        return flag & (CAP_CURSOR | CAP_DIMMING | CAP_SERIAL_INTERFACE);
    }
    
    // Implement remaining methods...
};
```

## Best Practices

### Capability Checking
- Always check capabilities before using advanced features
- Provide fallback behavior for unsupported features
- Use capability queries to optimize user interfaces

### Version Compatibility
- Check version compatibility when working with multiple display types
- Handle capability evolution gracefully
- Document minimum required capability versions

### Performance Considerations
- Cache capability values that don't change
- Avoid repeated capability queries in tight loops
- Use `getAllCapabilities()` for bulk capability checking

## See Also

- [DisplayCapabilities Class](DisplayCapabilities.md)
- [CapabilitiesRegistry Class](CapabilitiesRegistry.md)
- [VFDDisplay Class](VFDDisplay.md)
- [VFD20S401HAL Implementation](VFD20S401HAL.md)