#include "DisplayCapabilities.h"
#include <string.h>

DisplayCapabilities::DisplayCapabilities() 
    : _textRows(0)
    , _textColumns(0)
    , _charPixelWidth(0)
    , _charPixelHeight(0)
    , _displayWidthMM(0)
    , _displayHeightMM(0)
    , _capabilitiesFlags(0)
    , _maxCursorBlinkSpeeds(0)
    , _maxUserDefinedCharacters(0)
    , _dimmingLevels(0)
    , _maxBrightnessLevels(0)
    , _supportedInterfacesCount(0)
    , _supportedModesCount(0)
    , _minCommandDelayMicros(0)
    , _maxCommandDelayMicros(0)
    , _resetDelayMillis(0)
    , _typicalPowerMW(0)
    , _maxPowerMW(0)
    , _capabilityVersion(1) {
    
    _deviceName[0] = '\0';
    _deviceDescription[0] = '\0';
    _manufacturer[0] = '\0';
    _partNumber[0] = '\0';
}

DisplayCapabilities::DisplayCapabilities(
    const char* deviceName,
    const char* deviceDescription,
    const char* manufacturer,
    const char* partNumber,
    uint8_t textRows,
    uint8_t textColumns,
    uint8_t charPixelWidth,
    uint8_t charPixelHeight,
    uint16_t displayWidthMM,
    uint16_t displayHeightMM,
    uint16_t capabilitiesFlags,
    uint8_t maxCursorBlinkSpeeds,
    uint8_t maxUserDefinedCharacters,
    uint8_t dimmingLevels,
    uint8_t maxBrightnessLevels,
    uint16_t minCommandDelayMicros,
    uint16_t maxCommandDelayMicros,
    uint16_t resetDelayMillis,
    uint16_t typicalPowerMW,
    uint16_t maxPowerMW,
    uint8_t capabilityVersion)
    : _textRows(textRows)
    , _textColumns(textColumns)
    , _charPixelWidth(charPixelWidth)
    , _charPixelHeight(charPixelHeight)
    , _displayWidthMM(displayWidthMM)
    , _displayHeightMM(displayHeightMM)
    , _capabilitiesFlags(capabilitiesFlags)
    , _maxCursorBlinkSpeeds(maxCursorBlinkSpeeds)
    , _maxUserDefinedCharacters(maxUserDefinedCharacters)
    , _dimmingLevels(dimmingLevels)
    , _maxBrightnessLevels(maxBrightnessLevels)
    , _supportedInterfacesCount(0)
    , _supportedModesCount(0)
    , _minCommandDelayMicros(minCommandDelayMicros)
    , _maxCommandDelayMicros(maxCommandDelayMicros)
    , _resetDelayMillis(resetDelayMillis)
    , _typicalPowerMW(typicalPowerMW)
    , _maxPowerMW(maxPowerMW)
    , _capabilityVersion(capabilityVersion) {
    
    copyString(_deviceName, deviceName, sizeof(_deviceName));
    copyString(_deviceDescription, deviceDescription, sizeof(_deviceDescription));
    copyString(_manufacturer, manufacturer, sizeof(_manufacturer));
    copyString(_partNumber, partNumber, sizeof(_partNumber));
}

void DisplayCapabilities::copyString(char* dest, const char* src, size_t maxLen) {
    if (!dest || !src || maxLen == 0) return;
    
    size_t len = strlen(src);
    if (len >= maxLen) len = maxLen - 1;
    
    memcpy(dest, src, len);
    dest[len] = '\0';
}

void DisplayCapabilities::setDeviceInfo(const char* name, const char* description, const char* manufacturer, const char* partNumber) {
    copyString(_deviceName, name, sizeof(_deviceName));
    copyString(_deviceDescription, description, sizeof(_deviceDescription));
    copyString(_manufacturer, manufacturer, sizeof(_manufacturer));
    copyString(_partNumber, partNumber, sizeof(_partNumber));
}

void DisplayCapabilities::setTextDimensions(uint8_t rows, uint8_t columns) {
    _textRows = rows;
    _textColumns = columns;
}

void DisplayCapabilities::setCharacterPixels(uint8_t width, uint8_t height) {
    _charPixelWidth = width;
    _charPixelHeight = height;
}

void DisplayCapabilities::setPhysicalDimensions(uint16_t widthMM, uint16_t heightMM) {
    _displayWidthMM = widthMM;
    _displayHeightMM = heightMM;
}

void DisplayCapabilities::setCapabilityFlag(DisplayCapabilityFlag flag, bool enabled) {
    if (enabled) {
        _capabilitiesFlags |= flag;
    } else {
        _capabilitiesFlags &= ~flag;
    }
}

void DisplayCapabilities::setCapabilityFlags(uint16_t flags) {
    _capabilitiesFlags = flags;
}

void DisplayCapabilities::setTiming(uint16_t minDelayMicros, uint16_t maxDelayMicros, uint16_t resetDelayMillis) {
    _minCommandDelayMicros = minDelayMicros;
    _maxCommandDelayMicros = maxDelayMicros;
    _resetDelayMillis = resetDelayMillis;
}

void DisplayCapabilities::setPowerConsumption(uint16_t typicalMW, uint16_t maxMW) {
    _typicalPowerMW = typicalMW;
    _maxPowerMW = maxMW;
}

void DisplayCapabilities::setAdvancedFeatures(uint8_t maxBlinkSpeeds, uint8_t maxUserChars, uint8_t dimmingLevels, uint8_t brightnessLevels) {
    _maxCursorBlinkSpeeds = maxBlinkSpeeds;
    _maxUserDefinedCharacters = maxUserChars;
    _dimmingLevels = dimmingLevels;
    _maxBrightnessLevels = brightnessLevels;
}

void DisplayCapabilities::setCapabilityVersion(uint8_t version) {
    _capabilityVersion = version;
}

void DisplayCapabilities::addSupportedInterface(const char* interfaceName) {
    if (_supportedInterfacesCount >= 8 || !interfaceName) return;
    
    copyString(_supportedInterfaces[_supportedInterfacesCount], interfaceName, sizeof(_supportedInterfaces[0]));
    _supportedInterfacesCount++;
}

void DisplayCapabilities::addSupportedDisplayMode(DisplayMode mode) {
    if (_supportedModesCount >= 8) return;
    
    _supportedModes[_supportedModesCount] = mode;
    _supportedModesCount++;
}

bool DisplayCapabilities::hasCapability(DisplayCapabilityFlag flag) const {
    return (_capabilitiesFlags & flag) != 0;
}

bool DisplayCapabilities::supportsDisplayMode(DisplayMode mode) const {
    for (uint8_t i = 0; i < _supportedModesCount; i++) {
        if (_supportedModes[i] == mode) return true;
    }
    return false;
}

bool DisplayCapabilities::supportsInterface(const char* interfaceName) const {
    return findInterface(interfaceName);
}

const char* DisplayCapabilities::getSupportedInterface(uint8_t index) const {
    if (index >= _supportedInterfacesCount) return nullptr;
    return _supportedInterfaces[index];
}

bool DisplayCapabilities::findInterface(const char* interfaceName) const {
    if (!interfaceName) return false;
    
    for (uint8_t i = 0; i < _supportedInterfacesCount; i++) {
        if (strcmp(_supportedInterfaces[i], interfaceName) == 0) return true;
    }
    return false;
}