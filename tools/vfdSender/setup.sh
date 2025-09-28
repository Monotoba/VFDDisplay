#!/usr/bin/env bash
set -e

# Ensure we’re in the project root
cd "$(dirname "$0")"

# Check if Python 3.13 is installed
if ! command -v python3.13 &>/dev/null; then
    echo "Error: python3.13 not found. Please install Python 3.13 first."
    exit 1
fi

# Create virtual environment
if [ ! -d ".venv" ]; then
    echo "Creating virtual environment with Python 3.13..."
    python3.13 -m venv .venv
else
    echo ".venv already exists, skipping creation."
fi

# Activate and install dependencies
echo "Activating virtual environment..."
source .venv/bin/activate

echo "Upgrading pip..."
pip install --upgrade pip

echo "Installing pyserial..."
pip install -r requirements.txt

echo "Setup complete. To use the environment, run:"
echo "  source .venv/bin/activate"
echo "Then run your script like:"
echo "  python vfdSender.py /dev/ttyUSB0 9600 \"Hello VFD!\""
