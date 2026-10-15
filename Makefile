.DEFAULT_GOAL := build
.PHONY: help setup wifi flash deploy clean

## help message to show available commands
help:
	@echo "Available options: setup, build, wifi, flash, deploy, fdeploy, clean"

## one-time board configuration (passwords, Wi-Fi, dependencies)
setup:
	@./scripts/setup.sh

## build firmware code
build:
	@./scripts/build.sh

## configure/update Wi-Fi credentials
wifi:
	@./scripts/wifi.sh

## build and flash MCU firmware
flash:
	@./scripts/flash.sh

## deploy MPU daemon (without force flash)
deploy:
	@./scripts/deploy.sh

## deploy MPU daemon (force flash)
fdeploy:
	@./scripts/deploy.sh --flash
## cleanup local build artifacts
clean:
	@rm -rf unoq_mcu/build
	@echo "Cleaned up MCU build artifacts."
