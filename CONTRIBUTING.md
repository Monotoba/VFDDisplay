Contributing to VFDDisplay

Thanks for your interest in improving VFDDisplay! This guide explains how to build, test, and submit changes.

Getting Started
- Discuss significant changes in an issue first when in doubt.
- Please read AGENTS.md for project conventions, safety rules, and commit policy.

Build Requirements
- Arduino IDE or Arduino CLI
- PlatformIO (recommended for CI‑parity)

Build Examples
- PlatformIO: `pio run -d examples/ClockDemo -e megaatmega2560`
- Arduino CLI: `arduino-cli compile --fqbn arduino:avr:mega examples/ClockDemo`

Run Embedded Tests
- PlatformIO env: `pio run -e megaatmega2560-tests`
- Arduino IDE: open `tests/arduino/IVFDHAL_And_Device_Tests/IVFDHAL_And_Device_Tests.ino`

Coding Style
- Use 2‑space indentation and LF newlines (see .editorconfig).
- C/C++ formatting guidelines in `.clang-format`.
- Do not modify locked interfaces (IVFDHAL) or NO_TOUCH regions.

Commit Policy
- Small, focused commits with clear messages.
- Update `CHANGELOG.md` in the same commit describing what and why.

Pull Requests
- Include a summary, motivation, and testing notes.
- Link related issues and datasheets where relevant.
- New HALs must include docs (docs/api), capabilities registration, and tests.

Reporting Issues
- Provide board, wiring, PlatformIO/Arduino versions, and minimal repro.

