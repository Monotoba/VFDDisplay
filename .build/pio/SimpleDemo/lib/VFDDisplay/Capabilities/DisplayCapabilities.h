#pragma once
#include "IDisplayCapabilities.h"
#include <string.h>

// Concrete implementation of display capabilities
class DisplayCapabilities : public IDisplayCapabilities {
public:
    // Constructor with all parameters
    DisplayCapabilities(
        const char* deviceName,
        const char* deviceDescription,
        const char* manufacturer,
        const char* partNumber,
        uint8_t textRows,
        uint8_t textColumns,
        uint8_t charPixelWidth,
        uint8_t charPixelHeight,
        uint16_t displayWidthMM = 0,
        uint16_t displayHeightMM = 0,
        uint16_t capabilitiesFlags = 0,
        uint8_t maxCursorBlinkSpeeds = 0,
        uint8_t maxUserDefinedCharacters = 0,
        uint8_t dimmingLevels = 0,
        uint8_t maxBrightnessLevels = 0,
        uint16_t minCommandDelayMicros = 0,
        uint16_t maxCommandDelayMicros = 0,
        uint16_t resetDelayMillis = 0,
        uint16_t typicalPowerMW = 0,
        uint16_t maxPowerMW = 0,
        uint8_t capabilityVersion = 1
    );
    
    // Default constructor for building capabilities incrementally
    DisplayCapabilities();
    
    // Setters for building capabilities
    void setDeviceInfo(const char* name, const char* description, const char* manufacturer, const char* partNumber);
    void setTextDimensions(uint8_t rows, uint8_t columns);
    void setCharacterPixels(uint8_t width, uint8_t height);
    void setPhysicalDimensions(uint16_t widthMM, uint16_t heightMM);
    void setCapabilityFlag(DisplayCapabilityFlag flag, bool enabled);
    void setCapabilityFlags(uint16_t flags);
    void setTiming(uint16_t minDelayMicros, uint16_t maxDelayMicros, uint16_t resetDelayMillis);
    void setPowerConsumption(uint16_t typicalMW, uint16_t maxMW);
    void setAdvancedFeatures(uint8_t maxBlinkSpeeds, uint8_t maxUserChars, uint8_t dimmingLevels, uint8_t brightnessLevels);
    void setCapabilityVersion(uint8_t version);
    void addSupportedInterface(const char* interfaceName);
    void addSupportedDisplayMode(DisplayMode mode);
    
    // IDisplayCapabilities implementation
    uint8_t getTextRows() const override { return _textRows; }
    uint8_t getTextColumns() const override { return _textColumns; }
    uint8_t getCharacterPixelWidth() const override { return _charPixelWidth; }
    uint8_t getCharacterPixelHeight() const override { return _charPixelHeight; }
    uint16_t getDisplayWidthMM() const override { return _displayWidthMM; }
    uint16_t getDisplayHeightMM() const override { return _displayHeightMM; }
    
    bool hasCapability(DisplayCapabilityFlag flag) const override;
    uint16_t getAllCapabilities() const override { return _capabilitiesFlags; }
    
    uint8_t getMaxCursorBlinkSpeeds() const override { return _maxCursorBlinkSpeeds; }
    uint8_t getMaxUserDefinedCharacters() const override { return _maxUserDefinedCharacters; }
    uint8_t getDimmingLevels() const override { return _dimmingLevels; }
    uint8_t getMaxBrightnessLevels() const override { return _maxBrightnessLevels; }
    
    bool supportsDisplayMode(DisplayMode mode) const override;
    uint8_t getSupportedDisplayModesCount() const override { return _supportedModesCount; }
    
    bool supportsInterface(const char* interfaceName) const override;
    uint8_t getSupportedInterfacesCount() const override { return _supportedInterfacesCount; }
    const char* getSupportedInterface(uint8_t index) const override;
    
    uint16_t getMinCommandDelayMicros() const override { return _minCommandDelayMicros; }
    uint16_t getMaxCommandDelayMicros() const override { return _maxCommandDelayMicros; }
    uint16_t getResetDelayMillis() const override { return _resetDelayMillis; }
    
    uint16_t getTypicalPowerConsumptionMW() const override { return _typicalPowerMW; }
    uint16_t getMaxPowerConsumptionMW() const override { return _maxPowerMW; }
    
    const char* getDeviceName() const override { return _deviceName; }
    const char* getDeviceDescription() const override { return _deviceDescription; }
    const char* getManufacturer() const override { return _manufacturer; }
    const char* getPartNumber() const override { return _partNumber; }
    
    uint8_t getCapabilityVersion() const override { return _capabilityVersion; }
    bool isCompatibleWith(uint8_t requiredVersion) const override { return _capabilityVersion >= requiredVersion; }

private:
    // Device identification
    char _deviceName[32];
    char _deviceDescription[64];
    char _manufacturer[32];
    char _partNumber[32];
    
    // Display dimensions
    uint8_t _textRows;
    uint8_t _textColumns;
    uint8_t _charPixelWidth;
    uint8_t _charPixelHeight;
    uint16_t _displayWidthMM;
    uint16_t _displayHeightMM;
    
    // Capability flags
    uint16_t _capabilitiesFlags;
    
    // Advanced features
    uint8_t _maxCursorBlinkSpeeds;
    uint8_t _maxUserDefinedCharacters;
    uint8_t _dimmingLevels;
    uint8_t _maxBrightnessLevels;
    
    // Supported interfaces (up to 8)
    char _supportedInterfaces[8][16];
    uint8_t _supportedInterfacesCount;
    
    // Supported display modes
    DisplayMode _supportedModes[8];
    uint8_t _supportedModesCount;
    
    // Timing
    uint16_t _minCommandDelayMicros;
    uint16_t _maxCommandDelayMicros;
    uint16_t _resetDelayMillis;
    
    // Power consumption
    uint16_t _typicalPowerMW;
    uint16_t _maxPowerMW;
    
    // Version
    uint8_t _capabilityVersion;
    
    // Helper methods
    void copyString(char* dest, const char* src, size_t maxLen);
    bool findInterface(const char* interfaceName) const;
};