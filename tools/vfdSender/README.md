# vfdSender

---

A small Python 3.13 utility to send text and control sequences to VFD (Vacuum Fluorescent Display) modules via a serial port.  
Text is sent as ASCII, while embedded escape sequences (`\xHH`) are expanded into raw bytes.

---

## Features
- Send ASCII text directly to a serial port.
- Embed **hex escape sequences** (`\xHH`) to send control codes.
- Cross-platform support (Linux/macOS/Windows).
- Simple setup with virtual environment.

---

## Requirements
- Python **3.13**
- [pySerial](https://pypi.org/project/pyserial/)

---

## Setup

### Linux / macOS
```bash
./setup.sh
source .venv/bin/activate
````

### Windows (PowerShell)

```powershell
.\setup.ps1
.\.venv\Scripts\Activate.ps1
```

This creates a `.venv` virtual environment with Python 3.13 and all dependencies from `requirements.txt`.

---

## Usage

Run the script with:

```bash
python vfdSender.py <port> <baudrate> "<message>"
```

### Examples

```bash
# Send simple text
python vfdSender.py /dev/ttyUSB0 9600 "Hello VFD!"

# Send ESC (0x1B), '@', then "Clear"
python vfdSender.py /dev/ttyUSB0 9600 "Clear\x1B@"

# Mix text with hex codes
python vfdSender.py COM3 19200 "Hello\x1B\x40World"
```

---

## Development

### Install extra tools (optional)

```bash
pip install black flake8
```

### Run code formatting

```bash
black vfdSender.py
```

---

## Notes

* Hex escapes must be written as `\xHH` (two hex digits).
* Plain text is encoded in ASCII.
* Always activate the `.venv` before running the script.
* Example ports:

  * Linux: `/dev/ttyUSB0`, `/dev/ttyS0`
  * macOS: `/dev/tty.usbserial-XXXX`
  * Windows: `COM3`, `COM4`

---

## License

MIT License. See [LICENSE](LICENSE) for details.

```

---
