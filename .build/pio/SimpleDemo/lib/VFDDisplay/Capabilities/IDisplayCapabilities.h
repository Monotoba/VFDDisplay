#pragma once
#include <Arduino.h>

// Forward declaration
class DisplayCapabilities;

// Capability flags for quick querying
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

// Display mode support
enum DisplayMode : uint8_t {
    MODE_NORMAL = 0,
    MODE_INVERSE,
    MODE_BLINK,
    MODE_DIMMED,
    MODE_BRIGHT,
    MODE_COUNT
};

// Interface for display capabilities
class IDisplayCapabilities {
public:
    virtual ~IDisplayCapabilities() = default;
    
    // Basic display dimensions
    virtual uint8_t getTextRows() const = 0;
    virtual uint8_t getTextColumns() const = 0;
    virtual uint8_t getCharacterPixelWidth() const = 0;
    virtual uint8_t getCharacterPixelHeight() const = 0;
    
    // Physical dimensions in millimeters (if known)
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
    
    // Power consumption (if known)
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