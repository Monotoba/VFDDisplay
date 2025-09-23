---

# **User Guide – vfdSender Utility**

## Overview

`vfdSender` is a command-line utility for sending **text and control sequences** to a Vacuum Fluorescent Display (VFD) over a serial connection.

* Regular text is sent as **ASCII**.
* Embedded `\xHH` escape sequences are sent as **raw bytes**.

This allows easy manual testing of VFDs without writing custom software.

---

## Prerequisites

* **Python 3.13** installed on your system.
* A working **serial connection** to your VFD module (USB-to-serial adapter or onboard UART).
* Your system’s serial port name (e.g. `/dev/ttyUSB0`, `COM3`, etc.).

---

## Installation

### Linux / macOS

1. Open a terminal.
2. Navigate to the project folder.
3. Run the setup script:

   ```bash
   ./setup.sh
   source .venv/bin/activate
   ```

### Windows

1. Open **PowerShell**.
2. Navigate to the project folder.
3. Run the setup script:

   ```powershell
   .\setup.ps1
   .\.venv\Scripts\Activate.ps1
   ```

This installs the required dependency: **pyserial**.

---

## Usage

### Basic Command

```bash
python vfdSender.py <port> <baudrate> "<message>"
```

* `<port>` → Serial port name

  * Linux: `/dev/ttyUSB0`, `/dev/ttyS0`
  * macOS: `/dev/tty.usbserial-XXXX`
  * Windows: `COM3`, `COM4`

* `<baudrate>` → Communication speed (e.g. `9600`, `19200`, `115200`)

* `<message>` → Text to send.

  * Plain text → sent as ASCII
  * `\xHH` sequences → sent as raw bytes

---

### Examples

#### Send text

```bash
python vfdSender.py /dev/ttyUSB0 9600 "Hello VFD!"
```

➡ Displays `Hello VFD!` on the screen.

#### Send an **ESC command (0x1B)** followed by `@`

```bash
python vfdSender.py /dev/ttyUSB0 9600 "Clear\x1B@"
```

➡ Sends ASCII `"Clear"`, then hex bytes `0x1B 0x40`.

#### Mix text with control sequences

```bash
python vfdSender.py COM3 19200 "Hello\x1B\x40World"
```

➡ Sends `"Hello"`, then `0x1B 0x40`, then `"World"`.

---

## Troubleshooting

* **“python3.13 not found”** → Install Python 3.13 from [python.org](https://www.python.org/downloads/).
* **Permission denied on Linux/macOS** → Add your user to the `dialout` group or run with `sudo`.

  ```bash
  sudo usermod -aG dialout $USER
  ```

  Then log out and back in.
* **No output on VFD**

  * Double-check baud rate and serial port.
  * Ensure wiring matches VFD requirements (TX → RX, GND shared).
  * Some VFDs require initialization bytes before accepting text.

---

## Best Practices

* Always quote messages to prevent shell from misinterpreting `\xHH`.
* Use small test strings before sending long sequences.
* Keep `.venv` activated while testing.
* If automating tests, wrap `vfdSender.py` inside shell scripts or batch files.

---

## License

MIT License. Free to use and modify.

---


