$ErrorActionPreference = "Stop"
cd $PSScriptRoot

if (-not (Get-Command python3.13 -ErrorAction SilentlyContinue)) {
    Write-Error "Python 3.13 not found. Please install Python 3.13 first."
    exit 1
}

if (-not (Test-Path ".venv")) {
    Write-Output "Creating virtual environment with Python 3.13..."
    python3.13 -m venv .venv
} else {
    Write-Output ".venv already exists, skipping creation."
}

Write-Output "Activating virtual environment..."
.venv\Scripts\Activate.ps1

Write-Output "Upgrading pip..."
pip install --upgrade pip

Write-Output "Installing dependencies from requirements.txt..."
pip install -r requirements.txt

Write-Output "Setup complete. To use the environment, run:"
Write-Output "  .\.venv\Scripts\Activate.ps1"
Write-Output "Then run:"
Write-Output "  python vfdSender.py COM3 9600 'Hello VFD!'"

