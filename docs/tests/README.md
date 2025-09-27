# Tests Overview

This repository includes an embedded-friendly unit test setup focused on validating HALs (device implementations) against the `IVFDHAL` interface contract and verifying device-specific behavior.

What’s included:
- Minimal header-only test framework: `tests/framework/EmbeddedTest.h`
- Mock transport for byte capture: `tests/mocks/MockTransport.h`
- Reusable `IVFDHAL` contract tests: `tests/common/IVFDHALContractTests.hpp`
- Device-specific tests for `VFD20S401HAL`: `tests/device/VFD20S401HALTests.hpp`
- Arduino test sketch: `tests/arduino/IVFDHAL_And_Device_Tests/IVFDHAL_And_Device_Tests.ino`
- PlatformIO runner: `tests/embedded_runner/main.cpp`

The framework avoids external dependencies so it runs on Arduino IDE, PlatformIO, and any AVR-compatible toolchain that provides `Arduino.h`.

## Running Tests (Arduino IDE)

1. Open `tests/arduino/IVFDHAL_And_Device_Tests/IVFDHAL_And_Device_Tests.ino` in the Arduino IDE.
2. Select your AVR board (e.g., Arduino Mega 2560) and the correct serial port.
3. Upload the sketch.
4. Open the Serial Monitor at `115200` baud to view results.

You should see output like:

```
[TEST] Starting tests
[RUN ] IVFDHAL.init_without_transport_fails
[ OK ] IVFDHAL.init_without_transport_fails
...
[DONE] tests=N failed=0 asserts=M assert_fails=0
```

## Running Tests (PlatformIO)

An environment is provided in the root `platformio.ini`:

- `env:megaatmega2560-tests` builds `tests/embedded_runner/main.cpp` and links this repo as a library via `library.json`.

Commands:
- Build: `pio run -e megaatmega2560-tests`
- Upload: `pio run -e megaatmega2560-tests -t upload`
- Monitor: `pio device monitor -b 115200`

## Running Tests (Makefile)

Use the new test targets without changing your existing example flow:

- List tests: `make tests`
- Build a specific test: `make tests/embedded_runner/main.cpp`
- Build a specific Arduino test: `make tests/arduino/IVFDHAL_And_Device_Tests/IVFDHAL_And_Device_Tests.ino BACKEND=arduino`
  - If Arduino CLI doesn’t resolve the library when building from the repo, point to your sketchbook libraries path:
    - `make tests/arduino/IVFDHAL_And_Device_Tests/IVFDHAL_And_Device_Tests.ino BACKEND=arduino ARDUINO_LIB_DIRS="~/Arduino/libraries"`
- Build all tests: `make tests/all`
- Upload after build: append `UPLOAD=1 PORT=/dev/ttyACM0` to the commands above.

By default BACKEND=pio; you can override with `BACKEND=arduino` (for `.ino`) or `BACKEND=avr` (for `.cpp`).

## Adding Tests for New HALs

When adding a new device HAL that implements `IVFDHAL`:

1. Create device tests under `tests/device/<YourHALName>Tests.hpp`.
   - Use `MockTransport` to assert on exact bytes for your device’s commands.
   - Example patterns: init, clear, home, cursor positioning, custom char, escape sequences.
2. Reuse the generic interface tests by registering them for your HAL type:
   - In the runner (Arduino or PIO), include `tests/common/IVFDHALContractTests.hpp`.
   - Call `register_IVFDHAL_contract_tests<YourHALType>("YourHALName");` before running.
3. Include your device test registration function (e.g., `register_YourHAL_device_tests();`).

Tip: keep device tests focused on observable transport writes and public API behavior. Do not modify NO_TOUCH regions in the HAL.

## Framework API (EmbeddedTest)

- Register: `ET_ADD_TEST("name", test_function);`
- Assertions: `ET_ASSERT_TRUE(expr)`, `ET_ASSERT_EQ(a,b)`
- Runner: set output with `EmbeddedTest::setOutput(&Serial);`, then `EmbeddedTest::begin();` and `EmbeddedTest::runAll();`

## Notes

- The mock transport stores up to 1024 bytes (`data()`, `size()`, `at(i)` helpers) and should be reset between operations using `clear()`.
- The contract tests compute VFD20S401 linear addresses as `row*20 + col`; adjust or extend for different geometries if needed.
- For CI or host-based runs, you can adapt the runner to a native environment if an Arduino shim is available, but the current setup targets real AVR boards.
