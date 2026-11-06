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

SF_ROOT="$(cd "$(dirname "$this_script")/.." && pwd)"
BUILD_DIR="${SF_ROOT}/unoq_mcu/build"

echo "Building STM32 MCU firmware..."

if [ ! -d "$BUILD_DIR" ]; then
	cmake -S "${SF_ROOT}/unoq_mcu" -B "$BUILD_DIR" -DCMAKE_TOOLCHAIN_FILE="${SF_ROOT}/unoq_mcu/cmake/gcc-arm-none-eabi.cmake"
fi
make -C "$BUILD_DIR"

echo "Build complete!"
