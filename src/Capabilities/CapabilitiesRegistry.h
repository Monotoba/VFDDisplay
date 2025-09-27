#pragma once
#include "IDisplayCapabilities.h"
#include <Arduino.h>

// Forward declaration
class DisplayCapabilities;

// Registry entry for a capability provider
struct CapabilityRegistryEntry {
    const char* deviceName;
    const char* partNumber;
    IDisplayCapabilities* capabilities;
    uint8_t priority;  // Higher priority entries override lower ones
};

// Shared registry for display capabilities
class CapabilitiesRegistry {
public:
    // Singleton access
    static CapabilitiesRegistry& getInstance();
    
    // Registration methods
    bool registerCapabilities(const char* deviceName, const char* partNumber, IDisplayCapabilities* capabilities, uint8_t priority = 0);
    bool registerCapabilities(IDisplayCapabilities* capabilities, uint8_t priority = 0);
    
    // Query methods
    IDisplayCapabilities* findByDeviceName(const char* deviceName) const;
    IDisplayCapabilities* findByPartNumber(const char* partNumber) const;
    IDisplayCapabilities* findByCapabilities(const IDisplayCapabilities* capabilities) const;
    
    // Get all capabilities for iteration
    uint8_t getRegisteredCount() const { return _registeredCount; }
    const CapabilityRegistryEntry* getEntry(uint8_t index) const;
    
    // Priority management
    bool setPriority(const char* deviceName, uint8_t newPriority);
    void sortByPriority();  // Call after bulk registrations
    
    // Utility methods
    void clearAll();
    bool isRegistered(const IDisplayCapabilities* capabilities) const;
    void printRegistry() const;  // For debugging
    
    // Pre-defined capability sets for common displays
    static DisplayCapabilities* createVFD20S401Capabilities();
    static DisplayCapabilities* createVFD20T202Capabilities();
    static DisplayCapabilities* createVFDCU20025Capabilities();
    static DisplayCapabilities* createVFDCU40026Capabilities();
    static DisplayCapabilities* createVFDHT16514Capabilities();
    static DisplayCapabilities* createVFDM202MD15Capabilities();
    static DisplayCapabilities* createVFDM202SD01Capabilities();
    static DisplayCapabilities* createGeneric20x2Capabilities();
    static DisplayCapabilities* createGeneric16x2Capabilities();
    
private:
    // Private constructor for singleton
    CapabilitiesRegistry();
    CapabilitiesRegistry(const CapabilitiesRegistry&) = delete;
    CapabilitiesRegistry& operator=(const CapabilitiesRegistry&) = delete;
    
    // Registry storage
    static const uint8_t MAX_REGISTRY_ENTRIES = 16;
    CapabilityRegistryEntry _entries[MAX_REGISTRY_ENTRIES];
    uint8_t _registeredCount;
    
    // Helper methods
    int8_t findEntryIndex(const char* deviceName, const char* partNumber) const;
    int8_t findEntryIndex(const IDisplayCapabilities* capabilities) const;
    void insertEntry(const CapabilityRegistryEntry& entry, uint8_t index);
    bool compareEntries(const CapabilityRegistryEntry& a, const CapabilityRegistryEntry& b) const;
};

// Global convenience function
inline CapabilitiesRegistry& getCapabilitiesRegistry() {
    return CapabilitiesRegistry::getInstance();
}
