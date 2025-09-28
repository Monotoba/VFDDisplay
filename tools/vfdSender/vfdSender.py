# filename: send_to_vfd.py
import re
import serial
import argparse

def parse_escaped_string(input_str: str) -> bytes:
    """
    Parse a string with embedded hex escape sequences (\xHH) into bytes.
    Text is converted to ASCII, escapes to binary.
    Example: "Hello\x1B\x40World" -> b'Hello\x1b@World'
    """
    # Regex to find \xHH hex escape sequences
    pattern = re.compile(r'\\x([0-9A-Fa-f]{2})')
    output = bytearray()
    pos = 0

    for match in pattern.finditer(input_str):
        # Add plain text before the escape sequence
        output.extend(input_str[pos:match.start()].encode('ascii'))
        # Add the hex byte
        output.append(int(match.group(1), 16))
        pos = match.end()

    # Add any trailing plain text
    output.extend(input_str[pos:].encode('ascii'))
    return bytes(output)

def main():
    parser = argparse.ArgumentParser(description="Send strings with escape sequences to a serial port.")
    parser.add_argument("port", help="Serial port (e.g. COM3 or /dev/ttyUSB0)")
    parser.add_argument("baudrate", type=int, help="Baud rate (e.g. 9600, 19200, 115200)")
    parser.add_argument("message", help="Message string with optional \\xHH escapes")
    args = parser.parse_args()

    data = parse_escaped_string(args.message)

    with serial.Serial(args.port, args.baudrate, timeout=1) as ser:
        ser.write(data)
        print(f"Sent {len(data)} bytes: {data}")

if __name__ == "__main__":
    main()
