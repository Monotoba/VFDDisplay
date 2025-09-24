# VFDDisplay Library Documentation

## Overview

The VFDDisplay library provides a comprehensive, object-oriented interface for controlling Vacuum Fluorescent Displays (VFDs) in Arduino projects. It features a layered architecture with hardware abstraction, transport abstraction, and extensive capabilities management.

## Architecture

The library is built on a clean, layered architecture:

```
┌─────────────────────────────────────┐
│         Application Layer           │
│         (Your Arduino Code)         │
├─────────────────────────────────────┤
│         VFDDisplay Class            │
│      (High-level Interface)         │
├─────────────────────────────────────┤
│         IVFDHAL Interface           │
│     (Hardware Abstraction)          │
├─────────────────────────────────────┤
│      VFD20S401HAL (Example)         │
│    (Controller Implementation)      │
├─────────────────────────────────────┤
│        ITransport Interface         │
│     (Transport Abstraction)         │
├─────────────────────────────────────┤
│   SerialTransport / ParallelTransport│
│    (Communication Implementation)   │
└─────────────────────────────────────┘
```

## Key Features

### Core Functionality
- **Text Display**: Write strings and characters at any position
- **Cursor Control**: Position, blink, and movement commands
- **Display Management**: Clear, reset, and home operations
- **Text Formatting**: Centering, alignment, and positioning

### Advanced Features
- **Custom Characters**: Define and display custom 5x8 character patterns
- **Scrolling Effects**: Horizontal, vertical, and Star Wars-style scrolling
- **Display Modes**: Multiple display modes (normal, inverse, blink, dimmed)
- **Brightness Control**: Adjustable display brightness levels
- **Escape Sequences**: Direct controller command access

### Capabilities System
- **Feature Detection**: Runtime capability queries
- **Display Information**: Dimensions, characteristics, and specifications
- **Interface Support**: Multiple communication protocols
- **Power Management**: Consumption and timing information

### Development Support
- **Logging System**: Debug and monitor transport operations
- **Multiple Build Systems**: Arduino IDE, PlatformIO, and Make
- **Comprehensive Examples**: Various usage patterns and demonstrations
- **Extensive Documentation**: Detailed API reference and guides

## Command Reference (VFD20S401)

Quick reference for the bytes used by the VFD20S401 HAL. Values are hexadecimal. ESC sequences are prefixed with `0x1B` and followed by a fixed number of data bytes (no 0-termination).

- Initialize: `0x49`
- Reset: `ESC 0x49`
- Clear Display: `0x09`
- Cursor Home: `0x0C`
- Set Cursor Position: `ESC 'H' <addr>` where `<addr> = row*20 + col` (0-based, 0x00–0x4F)
- Display Mode: `ESC 0x11` .. `ESC 0x17`
- Dimming: `ESC 0x4C <level>` (e.g., 0x00, 0x40, 0x80, 0xC0)
- Cursor Blink Speed: `ESC 0x42 <rate>`
- Character Set: `0x18` (CT0 Standard), `0x19` (CT1 Extended)

Notes:
- After `clear`, call `cursorHome` before writing to ensure column 0.
- Library API uses 0-based row/column.

## Gotchas (VFD20S401)

Practical tips and pitfalls we encountered while validating against real hardware:

- Clear does not home: `clear (0x09)` does not move the cursor. Call `cursorHome (0x0C)` before writing headings to avoid a leading space.
- Positioning is linear, not row/col pair: The controller expects `ESC 'H'` followed by a single 0x00–0x4F linear address (computed as `row*20 + col`). Sending separate row/col bytes (or `0x80|addr` directly) can print stray glyphs and misplace text.
- ESC sequences are fixed-length: Do not rely on 0-termination when sending ESC sequences. Always send the exact number of bytes; parameters can legitimately be `0x00` (e.g., dimming level).
- Avoid raw DDRAM command bytes: Writing high-bit command bytes (like `0x80|addr`) into the data stream shows as visible characters on some units (e.g., `0xCB` rendered as `Ë`). Use the documented ESC methods instead.
- Rows/columns are 0-based in API: The library uses 0-based indexing; internally it converts to the controller’s linear address.

## Quick Start

### Basic Usage
```cpp
#include "VFDDisplay.h"
#include "HAL/VFD20S401HAL.h"
#include "Transports/SerialTransport.h"

// Create VFD system
VFD20S401HAL* hal = new VFD20S401HAL();
SerialTransport* transport = new SerialTransport(&Serial1);
VFDDisplay* vfd = new VFDDisplay(hal, transport);

void setup() {
    // Initialize serial for VFD
    Serial1.begin(19200, SERIAL_8N2);
    
    // Initialize VFD
    if (!vfd->init()) {
        Serial.println("VFD init failed!");
        return;
    }
    
    // Display message
    vfd->clear();
    vfd->write("Hello, VFD World!");
}

void loop() {
    // Your application code
}
```

### Advanced Features
```cpp
// Center text
vfd->centerText("Centered Title", 0);

// Position text
vfd->writeAt(1, 5, "Positioned Text");

// Star Wars scroll effect
const char* story = "A long time ago\nin a galaxy far,\nfar away...";
vfd->starWarsScroll(story, 2);

// Custom characters
uint8_t smiley[8] = {0x00, 0x0A, 0x00, 0x11, 0x0E, 0x00, 0x0A, 0x00};
vfd->saveCustomChar(0, smiley);
vfd->writeChar(0); // Display custom character
```

## Documentation Structure

### API Documentation
- **[VFDDisplay Class](api/VFDDisplay.md)**: Main library interface
- **[IVFDHAL Interface](api/IVFDHAL.md)**: Hardware abstraction layer
- **[VFD20S401HAL Implementation](api/VFD20S401HAL.md)**: VFD20S401 controller implementation
- **[ITransport Interface](api/ITransport.md)**: Transport abstraction layer
- **[IDisplayCapabilities Interface](api/IDisplayCapabilities.md)**: Display capabilities system
- **[ILogger Interface](api/ILogger.md)**: Logging and debugging system

### Build System Documentation
- **[Makefiles](build/Makefiles.md)**: Comprehensive Makefile documentation
- **[PlatformIO Configuration](build/PlatformIO.md)**: PlatformIO build system
- **[Build Commands](build/build_cmd.txt)**: Quick reference build commands

### Examples and Usage
- **[Usage Examples](examples/README.md)**: Various usage patterns and examples
- **[Flappy Bird Demo](examples/flappyBirdDemo.md)**: Advanced game demonstration

## Supported Hardware

### Primary Support
- **VFD20S401**: 4x20 character VFD display with serial interface
- **Arduino Mega2560**: Primary development platform
- **Serial Communication**: 19200 baud, 8N2 configuration

### Extensible Architecture
The library's layered design allows easy extension to:
- Different VFD controllers (implement `IVFDHAL`)
- Different communication methods (implement `ITransport`)
- Different display sizes and capabilities

## Build Systems

### Arduino IDE
1. Install library in Arduino/libraries folder
2. Include headers in your sketch
3. Use provided examples as starting points

### PlatformIO
1. Add library to `platformio.ini`
2. Configure build flags as needed
3. Build and upload through PlatformIO

### Make (avr-gcc)
1. Ensure avr-gcc toolchain is installed
2. Configure Arduino paths in Makefile
3. Use provided make targets

## Examples Included

### Basic Examples
- **SimpleDemo**: Basic library usage demonstration
- **BasicTest**: Comprehensive feature testing
- **MinimalVFDDemo**: Direct serial commands (no library)
- **SimpleVFDTest**: Simple functionality test
- **ModeSpecificTest**: Display mode testing

### Advanced Examples
- **FlappyBird**: Complete game implementation
- **StarWarsScroll**: Movie-style text effects
- **CustomCharacters**: User-defined character patterns

## Development

### Extending the Library

#### Adding New Controllers
1. Implement `IVFDHAL` interface
2. Create controller-specific command sequences
3. Register capabilities with `CapabilitiesRegistry`
4. Test with provided examples

#### Adding New Transports
1. Implement `ITransport` interface
2. Handle communication protocol specifics
3. Integrate with logging system
4. Test with existing HAL implementations

#### Adding New Capabilities
1. Extend `IDisplayCapabilities` interface
2. Update capability flags and enums
3. Implement in `DisplayCapabilities` class
4. Update registry with new capability sets

### Contributing
- Follow existing code style and patterns
- Add comprehensive documentation
- Include usage examples
- Test with multiple build systems
- Maintain backward compatibility

## Performance Characteristics

### Memory Usage
- **Flash**: ~8-12KB for basic functionality
- **RAM**: ~200-500 bytes depending on features used
- **Stack**: Minimal stack usage, suitable for Arduino

### Timing
- **Command Processing**: 10-100μs per command
- **Text Display**: Limited by serial baud rate
- **Scrolling**: Software-based, CPU dependent

### Optimization
- Use release builds for production
- Minimize capability queries in tight loops
- Consider direct HAL access for performance-critical code

## Troubleshooting

### Common Issues
- **Initialization Failure**: Check serial connection and baud rate
- **Display Not Responding**: Verify wiring and power supply
- **Garbled Text**: Ensure correct character set and encoding
- **Scrolling Issues**: Check text buffer sizes and memory constraints

### Debug Techniques
- Enable logging with `SerialLogger`
- Use `getCapabilities()` to verify feature support
- Check return values from all operations
- Monitor serial communication with logic analyzer

## Resources

### Datasheets
- [VFD20S401 Datasheet](datasheets/20s401da1.pdf)
- [M202MD15AJ Datasheet](datasheets/M202MD15AJ.pdf)
- [M202MD15FA Futaba Datasheet](datasheets/M202MD15FA-Futaba.pdf)
- [M202SD01HA Datasheet](datasheets/M202SD01HA.pdf)

### External Resources
- [Arduino Reference](https://arduino.cc/reference)
- [AVR Libc Documentation](https://www.nongnu.org/avr-libc/)
- [PlatformIO Documentation](https://docs.platformio.org/)

## License

This library is released under the MIT License. See the library.json file for details.

## Support

For issues, questions, or contributions:
- Report issues on the GitHub repository
- Check existing documentation and examples
- Review troubleshooting section
- Test with provided examples first

## Version History

- **1.0.0**: Initial release with VFD20S401 support
- Complete API documentation
- Multiple build system support
- Comprehensive examples
- Full capabilities system

---

*This documentation is part of the VFDDisplay library. For the most up-to-date information, please refer to the source code and examples.*
