#include "CapabilitiesRegistry.h"
#include "DisplayCapabilities.h"
#include <string.h>

// Singleton instance
CapabilitiesRegistry& CapabilitiesRegistry::getInstance() {
    static CapabilitiesRegistry instance;
    return instance;
}

CapabilitiesRegistry::CapabilitiesRegistry() : _registeredCount(0) {
    // Initialize entries
    for (uint8_t i = 0; i < MAX_REGISTRY_ENTRIES; i++) {
        _entries[i].deviceName = nullptr;
        _entries[i].partNumber = nullptr;
        _entries[i].capabilities = nullptr;
        _entries[i].priority = 0;
    }
}

bool CapabilitiesRegistry::registerCapabilities(const char* deviceName, const char* partNumber, IDisplayCapabilities* capabilities, uint8_t priority) {
    if (!capabilities || _registeredCount >= MAX_REGISTRY_ENTRIES) return false;
    
    // Check if already registered
    int8_t existingIndex = findEntryIndex(capabilities);
    if (existingIndex >= 0) {
        // Update existing entry
        _entries[existingIndex].priority = priority;
        sortByPriority();
        return true;
    }
    
    // Create new entry
    CapabilityRegistryEntry newEntry;
    newEntry.deviceName = deviceName;
    newEntry.partNumber = partNumber;
    newEntry.capabilities = capabilities;
    newEntry.priority = priority;
    
    // Find insertion point (keep sorted by priority)
    uint8_t insertIndex = 0;
    while (insertIndex < _registeredCount && _entries[insertIndex].priority >= priority) {
        insertIndex++;
    }
    
    insertEntry(newEntry, insertIndex);
    _registeredCount++;
    return true;
}

bool CapabilitiesRegistry::registerCapabilities(IDisplayCapabilities* capabilities, uint8_t priority) {
    if (!capabilities) return false;
    return registerCapabilities(capabilities->getDeviceName(), capabilities->getPartNumber(), capabilities, priority);
}

IDisplayCapabilities* CapabilitiesRegistry::findByDeviceName(const char* deviceName) const {
    if (!deviceName) return nullptr;
    
    for (uint8_t i = 0; i < _registeredCount; i++) {
        if (_entries[i].deviceName && strcmp(_entries[i].deviceName, deviceName) == 0) {
            return _entries[i].capabilities;
        }
    }
    return nullptr;
}

IDisplayCapabilities* CapabilitiesRegistry::findByPartNumber(const char* partNumber) const {
    if (!partNumber) return nullptr;
    
    for (uint8_t i = 0; i < _registeredCount; i++) {
        if (_entries[i].partNumber && strcmp(_entries[i].partNumber, partNumber) == 0) {
            return _entries[i].capabilities;
        }
    }
    return nullptr;
}

IDisplayCapabilities* CapabilitiesRegistry::findByCapabilities(const IDisplayCapabilities* capabilities) const {
    if (!capabilities) return nullptr;
    
    int8_t index = findEntryIndex(capabilities);
    return (index >= 0) ? _entries[index].capabilities : nullptr;
}

const CapabilityRegistryEntry* CapabilitiesRegistry::getEntry(uint8_t index) const {
    if (index >= _registeredCount) return nullptr;
    return &_entries[index];
}

bool CapabilitiesRegistry::setPriority(const char* deviceName, uint8_t newPriority) {
    if (!deviceName) return false;
    
    for (uint8_t i = 0; i < _registeredCount; i++) {
        if (_entries[i].deviceName && strcmp(_entries[i].deviceName, deviceName) == 0) {
            _entries[i].priority = newPriority;
            sortByPriority();
            return true;
        }
    }
    return false;
}

void CapabilitiesRegistry::sortByPriority() {
    // Simple bubble sort for small number of entries
    for (uint8_t i = 0; i < _registeredCount - 1; i++) {
        for (uint8_t j = 0; j < _registeredCount - i - 1; j++) {
            if (_entries[j].priority < _entries[j + 1].priority) {
                // Swap entries
                CapabilityRegistryEntry temp = _entries[j];
                _entries[j] = _entries[j + 1];
                _entries[j + 1] = temp;
            }
        }
    }
}

void CapabilitiesRegistry::clearAll() {
    _registeredCount = 0;
}

bool CapabilitiesRegistry::isRegistered(const IDisplayCapabilities* capabilities) const {
    return findEntryIndex(capabilities) >= 0;
}

void CapabilitiesRegistry::printRegistry() const {
    // This would typically use a logger, but for now we'll use Serial if available
    #ifdef ARDUINO
    Serial.println("=== Capabilities Registry ===");
    Serial.print("Registered entries: ");
    Serial.println(_registeredCount);
    
    for (uint8_t i = 0; i < _registeredCount; i++) {
        Serial.print("Entry ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(_entries[i].deviceName ? _entries[i].deviceName : "Unknown");
        Serial.print(" (");
        Serial.print(_entries[i].partNumber ? _entries[i].partNumber : "Unknown");
        Serial.print(") Priority: ");
        Serial.println(_entries[i].priority);
    }
    Serial.println("=============================");
    #endif
}

int8_t CapabilitiesRegistry::findEntryIndex(const char* deviceName, const char* partNumber) const {
    for (uint8_t i = 0; i < _registeredCount; i++) {
        bool nameMatch = (!deviceName && !_entries[i].deviceName) || 
                        (deviceName && _entries[i].deviceName && strcmp(deviceName, _entries[i].deviceName) == 0);
        bool partMatch = (!partNumber && !_entries[i].partNumber) || 
                        (partNumber && _entries[i].partNumber && strcmp(partNumber, _entries[i].partNumber) == 0);
        
        if (nameMatch && partMatch) return i;
    }
    return -1;
}

int8_t CapabilitiesRegistry::findEntryIndex(const IDisplayCapabilities* capabilities) const {
    if (!capabilities) return -1;
    
    for (uint8_t i = 0; i < _registeredCount; i++) {
        if (_entries[i].capabilities == capabilities) return i;
    }
    return -1;
}

void CapabilitiesRegistry::insertEntry(const CapabilityRegistryEntry& entry, uint8_t index) {
    if (index >= MAX_REGISTRY_ENTRIES) return;
    
    // Shift entries to make room
    for (uint8_t i = _registeredCount; i > index; i--) {
        _entries[i] = _entries[i - 1];
    }
    
    _entries[index] = entry;
}

// Pre-defined capability creators
DisplayCapabilities* CapabilitiesRegistry::createVFD20S401Capabilities() {
    DisplayCapabilities* caps = new DisplayCapabilities(
        "VFD20S401",
        "20x4 Vacuum Fluorescent Display with 5x8 dot matrix characters",
        "Futaba",
        "VFD20S401DA1",
        4,  // 4 rows (corrected from 2 to 4)
        20, // 20 columns
        5,  // 5 pixels wide per character
        8,  // 8 pixels high per character
        116, // 116mm width
        32,  // 32mm height (corrected for 4-row display)
        CAP_CURSOR | CAP_CURSOR_BLINK | CAP_DIMMING | CAP_USER_DEFINED_CHARS | CAP_HORIZONTAL_SCROLL | CAP_VERTICAL_SCROLL | CAP_BRIGHTNESS_CONTROL | CAP_SERIAL_INTERFACE,
        4,   // 4 cursor blink speeds
        16,  // 16 user-defined characters (datasheet allows up to 16 UDFs)
        8,   // 8 dimming levels
        16,  // 16 brightness levels
        10,  // 10us min command delay
        100, // 100us max command delay
        100, // 100ms reset delay
        800, // 800mW typical power (corrected for 4-row display)
        1500, // 1500mW max power (corrected for 4-row display)
        1    // capability version 1
    );
    
    caps->addSupportedInterface("Serial");
    caps->addSupportedDisplayMode(MODE_NORMAL);
    caps->addSupportedDisplayMode(MODE_DIMMED);
    caps->addSupportedDisplayMode(MODE_BRIGHT);
    
    return caps;
}

DisplayCapabilities* CapabilitiesRegistry::createVFD20T202Capabilities() {
    DisplayCapabilities* caps = new DisplayCapabilities(
        "VFD20T202",
        "20x2 Vacuum Fluorescent Display module",
        "Futaba",
        "20T202",
        2,   // rows
        20,  // columns
        5,   // char pixel width (typical)
        8,   // char pixel height (typical)
        116, // width mm (typical 20x2)
        16,  // height mm (typical 20x2)
        CAP_CURSOR | CAP_CURSOR_BLINK | CAP_HORIZONTAL_SCROLL | CAP_SERIAL_INTERFACE | CAP_PARALLEL_INTERFACE | CAP_USER_DEFINED_CHARS | CAP_DIMMING,
        1,   // blink speeds (on/off)
        8,   // user-defined chars (HD44780-style CGRAM)
        0,   // dimming levels (unknown)
        0,   // brightness levels (unknown)
        10,  // min cmd delay us
        100, // max cmd delay us
        100, // reset delay ms
        400, // typical power mW
        800, // max power mW
        1
    );

    caps->addSupportedInterface("Serial");
    caps->addSupportedInterface("Parallel");
    caps->addSupportedDisplayMode(MODE_NORMAL);
    return caps;
}

DisplayCapabilities* CapabilitiesRegistry::createVFDCU20025Capabilities() {
    DisplayCapabilities* caps = new DisplayCapabilities(
        "CU20025ECPB-W1J",
        "Noritake 20x2 VFD module (5x7 dots)",
        "Noritake Itron",
        "CU20025ECPB-W1J",
        2, 20,
        5, 7,
        116, 16,
        CAP_CURSOR | CAP_CURSOR_BLINK | CAP_PARALLEL_INTERFACE | CAP_USER_DEFINED_CHARS | CAP_DIMMING,
        1,   // blink on/off
        8,   // CGRAM 8 glyphs
        4,   // 4 brightness levels
        0,
        10, 100, 100,
        400, 800,
        1
    );
    caps->addSupportedInterface("Parallel");
    caps->addSupportedDisplayMode(MODE_NORMAL);
    return caps;
}

DisplayCapabilities* CapabilitiesRegistry::createVFDCU40026Capabilities() {
    DisplayCapabilities* caps = new DisplayCapabilities(
        "CU40026",
        "Noritake 40x2 VFD module (5x7 dots)",
        "Noritake Itron",
        "CU40026-TW200A",
        2, 40,
        5, 7,
        188, 16,
        CAP_CURSOR | CAP_CURSOR_BLINK | CAP_SERIAL_INTERFACE | CAP_PARALLEL_INTERFACE | CAP_USER_DEFINED_CHARS | CAP_DIMMING | CAP_HORIZONTAL_SCROLL | CAP_VERTICAL_SCROLL,
        255, // blink period programmable
        16,  // 16 UDFs supported
        4,   // 4 luminance bands
        0,
        10, 100, 100,
        700, 1200,
        1
    );
    caps->addSupportedInterface("Serial");
    caps->addSupportedInterface("Parallel");
    caps->addSupportedDisplayMode(MODE_NORMAL);
    return caps;
}

DisplayCapabilities* CapabilitiesRegistry::createVFDHT16514Capabilities() {
    DisplayCapabilities* caps = new DisplayCapabilities(
        "HT16514",
        "Holtek HT16514 VFD Controller/Driver (supports 16/20/24 x 2)",
        "Holtek",
        "HT16514",
        2, 20,     // default to 20x2; memory supports 40x2 addressing
        5, 8,
        116, 16,
        CAP_CURSOR | CAP_CURSOR_BLINK | CAP_SERIAL_INTERFACE | CAP_PARALLEL_INTERFACE | CAP_USER_DEFINED_CHARS | CAP_DIMMING,
        1,   // blink on/off
        8,   // CGRAM 8 glyphs
        4,   // 4 brightness levels via Function Set
        0,
        10, 100, 100,
        500, 800,
        1
    );
    caps->addSupportedInterface("Serial");
    caps->addSupportedInterface("Parallel");
    caps->addSupportedDisplayMode(MODE_NORMAL);
    return caps;
}

DisplayCapabilities* CapabilitiesRegistry::createVFDM202MD15Capabilities() {
    DisplayCapabilities* caps = new DisplayCapabilities(
        "M202MD15",
        "Futaba M202MD15 20x2 VFD module",
        "Futaba",
        "M202MD15AJ",
        2, 20,
        5, 8,
        116, 16,
        CAP_CURSOR | CAP_CURSOR_BLINK | CAP_SERIAL_INTERFACE | CAP_PARALLEL_INTERFACE | CAP_USER_DEFINED_CHARS | CAP_DIMMING,
        1,   // blink on/off
        8,   // user-defined chars
        4,   // brightness via Function Set
        0,
        10, 100, 100,
        500, 800,
        1
    );
    caps->addSupportedInterface("Serial");
    caps->addSupportedInterface("Parallel");
    caps->addSupportedDisplayMode(MODE_NORMAL);
    return caps;
}

DisplayCapabilities* CapabilitiesRegistry::createVFDM202SD01Capabilities() {
    DisplayCapabilities* caps = new DisplayCapabilities(
        "M202SD01",
        "Futaba M202SD01HA 20x2 VFD module",
        "Futaba",
        "M202SD01HA",
        2, 20,
        5, 7,
        100, 13,
        CAP_CURSOR | CAP_CURSOR_BLINK | CAP_SERIAL_INTERFACE | CAP_PARALLEL_INTERFACE | CAP_DIMMING,
        1,   // blink on/off
        0,   // UDF not specified
        6,   // 6 dimming levels (00,20,40,60,80,FF)
        0,
        10, 100, 100,
        350, 700,
        1
    );
    caps->addSupportedInterface("Serial");
    caps->addSupportedInterface("Parallel");
    caps->addSupportedDisplayMode(MODE_NORMAL);
    return caps;
}

DisplayCapabilities* CapabilitiesRegistry::createGeneric20x2Capabilities() {
    DisplayCapabilities* caps = new DisplayCapabilities(
        "Generic 20x2 VFD",
        "Generic 20x2 Vacuum Fluorescent Display",
        "Generic",
        "VFD-20x2-GENERIC",
        2,  // 2 rows
        20, // 20 columns
        5,  // 5 pixels wide per character
        8,  // 8 pixels high per character
        116, // 116mm width (typical)
        16,  // 16mm height (typical)
        CAP_CURSOR | CAP_CURSOR_BLINK | CAP_DIMMING | CAP_USER_DEFINED_CHARS | CAP_HORIZONTAL_SCROLL | CAP_VERTICAL_SCROLL | CAP_BRIGHTNESS_CONTROL,
        3,   // 3 cursor blink speeds
        8,   // 8 user-defined characters
        8,   // 8 dimming levels
        16,  // 16 brightness levels
        10,  // 10us min command delay
        100, // 100us max command delay
        100, // 100ms reset delay
        400, // 400mW typical power
        800, // 800mW max power
        1    // capability version 1
    );
    
    return caps;
}

DisplayCapabilities* CapabilitiesRegistry::createGeneric16x2Capabilities() {
    DisplayCapabilities* caps = new DisplayCapabilities(
        "Generic 16x2 VFD",
        "Generic 16x2 Vacuum Fluorescent Display",
        "Generic",
        "VFD-16x2-GENERIC",
        2,  // 2 rows
        16, // 16 columns
        5,  // 5 pixels wide per character
        8,  // 8 pixels high per character
        80,  // 80mm width (typical)
        16,  // 16mm height (typical)
        CAP_CURSOR | CAP_CURSOR_BLINK | CAP_DIMMING | CAP_USER_DEFINED_CHARS | CAP_HORIZONTAL_SCROLL | CAP_VERTICAL_SCROLL | CAP_BRIGHTNESS_CONTROL,
        3,   // 3 cursor blink speeds
        8,   // 8 user-defined characters
        8,   // 8 dimming levels
        16,  // 16 brightness levels
        10,  // 10us min command delay
        100, // 100us max command delay
        100, // 100ms reset delay
        350, // 350mW typical power
        700, // 700mW max power
        1    // capability version 1
    );
    
    return caps;
}
