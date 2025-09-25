#!/usr/bin/env bash
# Run-all-demos: builds and uploads each example, waits, then advances
#
# Usage:
#   ./run_all_demos.sh [--backend pio|make] [--port /dev/ttyACM0] [--wait 60] [--protocol stk500|wiring] [--baud 57600]
#
# Env var overrides (preferred in headless CI):
#   BACKEND=pio|make PORT=/dev/ttyACM0 WAIT_SECS=60 PROTOCOL=stk500 BAUD=57600 ./run_all_demos.sh
#
# Notes:
#  - Defaults target Mega2560 (env: megaatmega2560)
#  - Examples are discovered dynamically by presence of examples/*/platformio.ini
#  - Script continues on errors, printing a summary at the end

set -euo pipefail

BACKEND="${BACKEND:-pio}"
PORT="${PORT:-/dev/ttyACM0}"
WAIT_SECS="${WAIT_SECS:-60}"
PROTOCOL="${PROTOCOL:-stk500}"
BAUD="${BAUD:-57600}"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --backend) BACKEND="$2"; shift 2;;
    --port) PORT="$2"; shift 2;;
    --wait) WAIT_SECS="$2"; shift 2;;
    --protocol) PROTOCOL="$2"; shift 2;;
    --baud) BAUD="$2"; shift 2;;
    -h|--help)
      sed -n '1,60p' "$0"; exit 0;;
    *) echo "Unknown arg: $1"; exit 1;;
  esac
done

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT_DIR"

# Discover examples that have a platformio.ini
mapfile -t DISCOVERED < <(find examples -mindepth 1 -maxdepth 1 -type d \
  -exec test -f '{}/platformio.ini' \; -print | sort)

# Curated execution order (only include if present)
PREFERRED=(
  "examples/SimpleDemo"
  "examples/ClockDemo"
  "examples/BargraphDemo"
  "examples/AnimationsDemo"
  "examples/MatrixRainDemo"
  "examples/FlappyBirdDemo"
  "examples/MinimalVFDDemo"
  "examples/SimpleVFDTest"
  "examples/BasicTest"
  "examples/CorrectCodesDemo"
  "examples/ModeSpecificTest"
)

# Build final ordered list: preferred (if discovered) + remaining discovered
declare -A seen=()
DEMOS=()
for d in "${PREFERRED[@]}"; do
  for x in "${DISCOVERED[@]}"; do
    if [[ "$x" == "$d" ]]; then DEMOS+=("$x"); seen["$x"]=1; fi
  done
done
for x in "${DISCOVERED[@]}"; do
  if [[ -z "${seen[$x]:-}" ]]; then DEMOS+=("$x"); fi
done

if [[ ${#DEMOS[@]} -eq 0 ]]; then
  echo "No examples with platformio.ini found under examples/." >&2
  exit 1
fi

echo "Execution order:" >&2
for d in "${DEMOS[@]}"; do echo "  - $d" >&2; done
echo >&2

declare -i ok_count=0
declare -i fail_count=0
FAILED_LIST=()

upload_with_pio() {
  local dir="$1" name work
  name="$(basename "$dir")"
  work="$ROOT_DIR/.build/pio-run/$name"
  mkdir -p "$work"
  # Create ephemeral project that points src_dir at the example and overrides upload settings
  cat > "$work/platformio.ini" <<EOF
[platformio]
src_dir = $(cd "$dir" && pwd)

[env:megaatmega2560]
platform = atmelavr
board = megaatmega2560
framework = arduino
lib_ldf_mode = deep+
build_type = release
build_flags = -std=gnu++11
monitor_speed = $BAUD
upload_protocol = $PROTOCOL
upload_speed = $BAUD
lib_extra_dirs = $ROOT_DIR
lib_deps = VFDDisplay
EOF
  echo "[PIO] Building + uploading: $name (port=$PORT proto=$PROTOCOL baud=$BAUD)" >&2
  pio run -d "$work" -e megaatmega2560 -t upload --upload-port "$PORT"
}

upload_with_make() {
  local dir="$1" name
  name="$(basename "$dir")"
  echo "[MAKE] $name.upload (PORT=$PORT PROTOCOL=$PROTOCOL BAUD=$BAUD)" >&2
  make "$name".upload PORT="$PORT" PROTOCOL="$PROTOCOL" BAUD="$BAUD"
}

for demo in "${DEMOS[@]}"; do
  echo "=================================================="
  echo "== Demo: $demo"
  echo "=================================================="
  if [[ "$BACKEND" == "pio" ]]; then
    if upload_with_pio "$demo"; then
      ((ok_count++))
    else
      echo "[ERROR] Upload failed for $demo" >&2
      FAILED_LIST+=("$demo")
      ((fail_count++))
    fi
  else
    if upload_with_make "$demo"; then
      ((ok_count++))
    else
      echo "[ERROR] Upload failed for $demo" >&2
      FAILED_LIST+=("$demo")
      ((fail_count++))
    fi
  fi

  echo "[WAIT] Sleeping ${WAIT_SECS}s before next demo..." >&2
  sleep "$WAIT_SECS"
done

echo
echo "Summary: ${ok_count} succeeded, ${fail_count} failed"
if [[ ${#FAILED_LIST[@]} -gt 0 ]]; then
  echo "Failed demos:"; for f in "${FAILED_LIST[@]}"; do echo "  - $f"; done
fi

exit 0
