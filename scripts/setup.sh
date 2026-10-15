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

# Setup Board User Password
echo "[1/3] Configuring MPU user password..."
echo "Set the password interactively on the board when prompted below:"

"$ADB" -s "$DEVICE_ID" shell -t "passwd"

echo ""

# Connect to Wi-Fi to begin setting up dependencies
echo "[2/3] Ensuring Wi-Fi connectivity..."
"${SCRIPT_DIR}/wifi.sh"
echo ""

# Install build dependencies
echo "[3/3] Installing build packages on the board..."
echo "Enter your newly created board password when prompted:"
"$ADB" -s "$DEVICE_ID" shell -t "sudo apt update && sudo DEBIAN_FRONTEND=noninteractive apt install -y build-essential cmake git libsystemd-dev libmsgpack-c-dev libcap-dev"
echo ""

echo "================================="
echo "  Uno Q Board Setup Successful!  "
echo "================================="
echo ""
echo "Build with 'scripts/build.sh', flash with 'scripts/flash.sh', and deploy with 'scripts/deploy.sh'"
echo "Use 'scripts/wifi.sh' if you ever need to connect the board to WiFi."
echo "Make commands are available at the project root (eg. make deploy, make flash, etc.) for convenience (reference ${SF_ROOT}/Makefile for more details)"
