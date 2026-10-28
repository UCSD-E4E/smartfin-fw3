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

BUILD_DIR="${SF_ROOT}/unoq_mcu/build"
ELF_FILE="${BUILD_DIR}/unoq_mcu.elf"
REMOTE_TMP="/tmp/unoq_mcu.elf"

echo "[1/3] Compiling Smartfin firmware..."
"${SCRIPT_DIR}/build.sh"

echo "[2/3] Pushing ELF binary to board..."
LOCAL_ELF="$ELF_FILE"
[ "$ADB" = "adb.exe" ] && LOCAL_ELF="$(wslpath -w "$ELF_FILE")"
"$ADB" -s "$DEVICE_ID" push "$LOCAL_ELF" "$REMOTE_TMP"

echo "[3/3] Flashing STM32 via OpenOCD..."
"$ADB" -s "$DEVICE_ID" shell "/opt/openocd/bin/openocd -d2 -s /opt/openocd -f openocd_gpiod.cfg -c 'reset_config srst_only srst_push_pull; init; reset; halt; flash write_image erase ${REMOTE_TMP}; reset; shutdown'"

echo "MCU Flashed Successfully!"
