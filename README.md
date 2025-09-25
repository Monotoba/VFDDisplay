# VFDDisplay

High-level Arduino library for character Vacuum Fluorescent Displays (VFDs) with a clean HAL (device layer) and a buffered top layer for smooth animations. Includes many ready-to-run demos for Arduino Mega 2560.

## Run All Demos

Use the helper script to build and upload every example sequentially with a countdown between demos.

- Script: `./run_all_demos.sh`
- Defaults: PlatformIO backend, port `/dev/ttyACM0`, wait 60s, protocol `stk500`, baud `57600`.

Examples:

- PlatformIO backend (override protocol/baud/port/wait):
  - `./run_all_demos.sh --backend pio --port /dev/ttyACM0 --wait 30 --protocol wiring --baud 115200`
- Make backend (mirrors your working manual command):
  - `./run_all_demos.sh --backend make --port /dev/ttyACM0 --wait 30 --protocol wiring --baud 115200`

Notes:
- Curated order: SimpleDemo → AdDemo → ClockDemo → BargraphDemo → AnimationsDemo → MatrixRainDemo → FlappyBirdDemo → MinimalVFDDemo → SimpleVFDTest → BasicTest → CorrectCodesDemo → ModeSpecificTest, then any remaining.
- PlatformIO backend uses a temporary per‑demo project that overrides upload protocol/baud, so flags are honored across PlatformIO versions.
- The script continues on errors, prints a visible countdown between demos, and summarizes results at the end.

## Build Single Demo

- PlatformIO: `pio run -d examples/ClockDemo -e megaatmega2560 -t upload --upload-port /dev/ttyACM0`
- Make: `make ClockDemo.upload PORT=/dev/ttyACM0 PROTOCOL=stk500 BAUD=57600`

## Standardized Demo Init

All demos call the same initialization sequence after `vfd->init()` to ensure a clean start:

```
vfd->reset();
vfd->clear();
vfd->cursorHome();
```

## Documentation

See `docs/README.md` for full library documentation, command reference, gotchas, and guidance for adding new device HALs.

