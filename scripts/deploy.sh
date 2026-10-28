#!/usr/bin/env bash
set -e

if [ -n "$BASH_SOURCE" ]; then
	this_script=$BASH_SOURCE
elif [ -n "$ZSH_VERSION" ]; then
	setopt function_argzero
	this_script=$0
elif eval '[[ -n ${.sh.file} ]]' 2>/dev/null; then
	eval 'this_script=${.sh.file}'
else
	echo 1>&2 "Unsupported shell. Please use bash, ksh93 or zsh."
	exit 2
fi

SCRIPT_DIR="$(cd "$(dirname "$this_script")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

FORCE_FLASH=false

for arg in "$@"; do
	case "$arg" in
		--flash|-f)
			FORCE_FLASH=true
			;;
		*)
			echo "Unknown option: $arg"
			echo "Usage: ./scripts/deploy.sh [--flash|-f]"
			exit 1
			;;
	esac
done

REMOTE_MCU_BIN="/tmp/unoq_mcu.elf"
LOCAL_MCU_BIN="${SF_ROOT}/unoq_mcu/build/unoq_mcu.elf"

REMOTE_FLASHED=$("$ADB" -s "$DEVICE_ID" shell "[ -f $REMOTE_MCU_BIN ] && echo 'yes' || echo 'no'" | tr -d '\r')

if [ "$FORCE_FLASH" = true ] || [ "$REMOTE_FLASHED" != "yes" ]; then
    echo "Requesting to flash to the board..."
    "${SCRIPT_DIR}/flash.sh"
else
    echo "MCU firmware is already flashed on the board. Skipping MCU flash step."
fi


# Push source files to board
REMOTE_DIR="/home/arduino/smartfin"

echo "[1/2] Syncing source files..."
"$ADB" -s "$DEVICE_ID" shell "mkdir -p ${REMOTE_DIR}"

cd "${SF_ROOT}"
"$ADB" -s "$DEVICE_ID" push src ipc unoq_hal lib pc_hal "${REMOTE_DIR}/"

# Build & Run smartfin daemon remotely
echo "[2/2] Building and launching unoq_hal on board..."

# temporarily disable 'exit on error' so SIGINT does not abort the script
set +e

"$ADB" -s "$DEVICE_ID" shell -t "bash -c '
set -e

BUILD_DIR=\"${REMOTE_DIR}/unoq_hal/build\"

cmake -S ${REMOTE_DIR}/unoq_hal -B \"\$BUILD_DIR\" -DCMAKE_BUILD_TYPE=Release

echo \"==> Compiling unoq_hal...\"
make -C \"\$BUILD_DIR\"

echo \"==> Restarting router and freeing SPI bus...\"
sudo killall unoq_hal 2>/dev/null || true
sudo systemctl restart arduino-router 2>/dev/null || true

echo \"==> Starting daemon (Press Ctrl+C to stop)...\"
sudo \"\$BUILD_DIR/unoq_hal\"
'"

EXIT_CODE=$?

# reenable standard error handling
set -e

# check if exiting normally
if [ "$EXIT_CODE" -eq 130 ] || [ "$EXIT_CODE" -eq 0 ]; then
    echo -e "\nStopped unoq_hal daemon."
    exit 0
else
    echo -e "\nunoq_hal exited with error code: $EXIT_CODE"
    exit "$EXIT_CODE"
fi
