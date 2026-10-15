#!/usr/bin/env bash
set -e

# determine script file path
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

# Resolve Smartfin root
SF_ROOT="$(cd "$(dirname "$this_script")/.." && pwd)"
export SF_ROOT

# Resolve ADB path
if grep -qi "microsoft" /proc/version 2>/dev/null; then
	echo "Running in WSL..."
	if command -v adb.exe >/dev/null 2>&1; then
		ADB="adb.exe"
	else
		echo "Error: 'adb.exe' not found." >&2
		return 1
	fi
elif command -v adb >/dev/null 2>&1; then
	ADB="adb"
else
	echo "Error: 'adb' not found." >&2
	return 1
fi
export ADB

# Ensure and retrieve connected device
connected_devices=$("$ADB" devices | tr -d '\r' | awk '$2 == "device" {print $1}')

if [ -z "$connected_devices" ]; then
	echo "Error: No online device found." >&2
	echo "Current '$ADB devices' output:" >&2
	"$ADB" devices >&2
	echo "Troubleshooting:" >&2
	echo " - double check the connection to make sure the board is connected to this device."
	echo " - if 'offline' or empty, may need to wait for the board to boot or power cycle the board"
	return 1
fi

# Returns the first active device
DEVICE_ID=$(echo "$connected_devices" | head -n 1)
export DEVICE_ID
