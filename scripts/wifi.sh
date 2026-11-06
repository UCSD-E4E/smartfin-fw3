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

echo "Checking Wi-Fi connectivity..."

if "$ADB" -s "$DEVICE_ID" shell "ping -c 1 -W 2 8.8.8.8 >/dev/null 2>&1"; then
    CURRENT_SSID=$("$ADB" -s "$DEVICE_ID" shell "nmcli -t -f active,ssid dev wifi | grep '^yes:' | cut -d: -f2" | tr -d '\r')
    echo "Board is already connected to Wi-Fi (SSID: ${CURRENT_SSID:-Unknown})."
    read -rp "Do you want to reconnect to a different network? (y/N): " RECONNECT
    if [[ "$RECONNECT" =~ ^[Yy]$ ]]; then
        WIFI_NEEDED=true
    else
        WIFI_NEEDED=false
    fi
else
    WIFI_NEEDED=true
fi

if [ "$WIFI_NEEDED" = true ]; then
    read -rp "Enter Wi-Fi Network Name (SSID): " WIFI_SSID
    read -s -rp "Enter Wi-Fi Password (leave blank for open networks): " WIFI_PASS
    echo ""

    echo "Connecting Uno Q to '${WIFI_SSID}'..."
    if [ -z "$WIFI_PASS" ]; then
        "$ADB" -s "$DEVICE_ID" shell "nmcli dev wifi connect '${WIFI_SSID}'"
    else
        "$ADB" -s "$DEVICE_ID" shell "nmcli dev wifi connect '${WIFI_SSID}' password '${WIFI_PASS}'"
    fi

    echo "Verifying network connectivity..."
    if ! "$ADB" -s "$DEVICE_ID" shell "ping -c 2 -W 3 8.8.8.8 >/dev/null 2>&1"; then
        echo "Error: Failed to connect to the internet. Please verify your SSID and password."
        exit 1
    fi
    echo "Connected to Wi-Fi successfully!"
fi
