# PlatformIO Guide

## Overview

This project ships PlatformIO configuration for tests at the root (`platformio.ini`) and per‑example `platformio.ini` files under `examples/*`. You can build examples directly with PlatformIO, or use the unified Makefile which creates ephemeral PlatformIO projects per example with upload settings synchronized to your flags.

## Build and Upload (Per‑Example)

Each example folder contains a `platformio.ini` targeting Mega 2560.

- Build: `pio run -d examples/ClockDemo -e megaatmega2560`
- Upload: `pio run -d examples/ClockDemo -e megaatmega2560 -t upload --upload-port /dev/ttyACM0`
- Monitor: `pio device monitor -b 57600 -p /dev/ttyACM0`

Defaults in examples
- `upload_protocol = stk500` (override with `--upload-port` and Makefile if you prefer `wiring`)
- `upload_speed = 57600`
- `monitor_speed = 57600`

Tip: If you encounter protocol/baud issues, try `upload_protocol = wiring` and a different `upload_speed` depending on your bootloader.

## Build and Upload (Unified Makefile)

The root `Makefile` can drive PlatformIO builds via ephemeral, per‑example projects. This preserves your existing example layout without moving files.

- Build (default backend): `make ClockDemo`
- Upload: `make ClockDemo.upload PORT=/dev/ttyACM0`
- Force PlatformIO backend: `make -- --pio ClockDemo`
- Override protocol/baud: `make -- --pio --protocol=wiring ClockDemo.upload PORT=/dev/ttyUSB0 BAUD=115200`

Under the hood, the Makefile writes `.build/pio/<Example>/platformio.ini` pointing `src_dir` at `examples/<Example>`, vendors `src/` as a library, sets `upload_protocol` and `upload_speed` from your flags, and runs `pio run` in that directory.

## Test Environments

The root `platformio.ini` defines test environments that build the embedded test runner:

- `env:megaatmega2560-tests` — default AVR Mega 2560 test environment
- `env:leonardo-tests` — constrained profile with `VFD_TEST_PROFILE_LEONARDO`
- `env:uno-tests` — AVR Uno

Build tests
- `pio run -e megaatmega2560-tests`

Notes
- These envs set `-I.` and `-Isrc`, define `-DVFD_EMBEDDED_TEST_RUNNER`, and rely on `src/EmbeddedTestsMain.cpp` to include `tests/embedded_runner/main.cpp`.

## Caches and Cleaning

- Project caches live under `.pio/` and `.pioenvs/` in each PlatformIO project folder.
- With the unified Makefile: `make clean` removes `.build/pio/` (ephemeral projects). `make deepclean` also removes `.pio`, `.pioenvs`, and `.piolibdeps` in the repo.
- Manual clean for a per‑example project: `pio run -d .build/pio/<Example> -t clean`.

## CI Notes

CI builds use PlatformIO to build tests and examples with `-e megaatmega2560` and the test envs from `platformio.ini`. Keep example `platformio.ini` minimal and consistent (board, protocol, upload speed, monitor speed).

