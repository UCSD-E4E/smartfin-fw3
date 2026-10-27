# smartfin-fw3
Smartfin FW v3
## Prerequisites
### Hardware
 - [Arduino UNO Q](https://docs.arduino.cc/hardware/uno-q/)
    - MCU: STM32U585
    - Linux MPU: Qualcomm QRB2210
 - USB-C Data Cable
### Software
> **IMPORTANT**: Check [Software Setup Instructions](#installing-prerequisite-software) to make sure your dependencies are setup properly.
 - [Android Debug Bridge (ADB)](https://developer.android.com/tools/adb)
 - [CMake](https://cmake.org)
 - [OpenOCD](https://openocd.org/)
 - [GNU ARM Embedded Toolchain](https://developer.arm.com/tools-and-software/gnu-toolchain) for Cross-Compilation
 - (Optional) [Arduino Software (IDE, AppLab, etc.)](https://www.arduino.cc/en/software/)



## Setup & Configuration
### Installing Prerequisite Software
#### MacOS
[Homebrew](https://brew.sh/) is the recommended package manager for installing and managing dependencies, you can install everything you need with the commands below:
```shell
brew install cmake open-ocd
brew install --cask android-platform-tools gcc-arm-embedded
```
#### Windows
For now development on [WSL](https://learn.microsoft.com/en-us/windows/wsl/) is the primary recommended/supported way to develop and deploy this project. Natively on Windows, you only need the following:
 - **WSL**: follow installation and setup instructions [here](https://learn.microsoft.com/en-us/windows/wsl/install) if not already installed
 -**ADB**: you may install from their [website](https://developer.android.com/tools/adb) or if you use a package manager like [winget](https://learn.microsoft.com/en-us/windows/package-manager/winget/) you can install it from Powershell with `winget install Google.PlatformTools`

From WSL, make sure Windows `ADB` is executable on you PATH by running `adb.exe`. Unlike from MacOS and Linux, we will be invoking Windows' ADB executable directly from WSL to access port connections since they are not natively exposed to WSL. If you wish to use an alternative method you can look into bridging USB connections with [usbipd](https://github.com/dorssel/usbipd-win).

 For remaining development and build tools, follow [Linux Setup Instructions](#linux) from WSL.

#### Linux
Use your preferred supported package manager available on your distro. Recommended defaults are provided here:

> *WSL users can ignore installing android-sdk-platform-tools if it is installed locally on Windows*

**Debian/Ubuntu**:
```shell
sudo apt update
sudo apt upgrade
sudo apt install build-essential cmake openocd gcc-arm-none-eabi android-sdk-platform-tools
```
### Getting Setup on a Board
#### Ensuring ADB is Added to PATH
First, it is important to make sure `adb` is added to your PATH, otherwise the provided setup tools/scripts will not work. Run `adb` (or `adb.exe` on WSL) to check.

If it says command not found and you don't know how to add it to your path, then look up how to add it to your path for your respective system. It can be helpful to restart your system/session if it is not showing up when it should be.

On MacOS and Linux you can typically add something like this to the end of your `rc` (ie. `~/.zshrc` or `~/.bashrc` for most users) file:
```shell
export PATH=$PATH:/absolute/path/to/android/platform-tools/
```
Then start a new session or `source` the file and you should be able to run `adb` directly.

#### Check USB Connection
Run `adb devices` (`adb.exe devices` for WSL) to list connected devices.

You should see a list of connected devices and next to the device ID should be the word 'device'.

If it says offline or is empty, you may need to wait for the board to fully boot (may need to wait up to several minutes) or power cycle the board.

#### Board Setup
All the steps needed to setup, build, and flash to a board are availble in the `scripts/` directory and exposed at the project root through `make` commands.

When on a fresh board (a board that has not yet been setup), run `make setup`. You will be prompted to connect to WiFi and setup a password for the device (eg. `smartfin`).

You should only need to connect to WiFi once to install dependencies on the Linux MPU, but if you ever need to reconfigure your connection, just run `make wifi`.

Proceed to [Flashing and Deploying](#flashing--deploying-via-usb) via USB to check your setup.

## Flashing & Deploying via USB
Available `make` commands for building, flashing, and deploying:
 - `make`/`make build`: compile/build Smartfin firmware code locally
 - `make flash`: flash firmware to the board, if it has not been built then will automatically build firmware first.
 - `make deploy`: deploys the current firmware on the board (run firmware without flashing first)
 - `make fdeploy`: forces a fresh build/flash to the board before deploying
 - `make clean`: cleans up local build artificats

# Contributing

Please review the following pages:
- [CONTRIBUTING.md](CONTRIBUTING.md)
- [docs/index.md](docs/index.md)

# External Library Register
Library         | Version | License | Source                                           | Location
----------------|---------|---------|--------------------------------------------------|---------
LIS3DH          | 0.2.8   | MIT     | https://github.com/rickkas7/LIS3DH               | lib/LIS3DH
gps-ublox       | 1.2.0   | Apache 2.0 | https://github.com/particle-iot/gps-ublox/       | lib/gps-ublox
gps-quectel     | 1.0.0   | Apache 2.0 | https://github.com/particle-iot/gps-quectel      | lib/gps-quectel
gps-nmea-parser | 1.2.0   | Apache 2.0 | https://github.com/particle-iot/gps-nmea-parser/ | lib/gps-nmea-parser
tracker edge    | v18     | Apache 2.0 | https://github.com/particle-iot/tracker-edge     | lib/tracker-edge
SparkFun ICM    | 1.2.0   | MIT     | https://github.com/UCSD-E4E/SparkFun_ICM-20948_ParticleLibrary | src/imu/ICM-20948
Max 31725       | 2019    | Apache 2.0 | https://os.mbed.com/teams/MaximIntegrated/code/MAX31725_Accurate_Temperature_Sensor//file/b4fdbbe79036/max31725.h/ | temperature/
urlsafe-b64     | 022c4cc | MIT     | https://github.com/Akagi201/urlsafe-b64 | src/cellular/encoding
base85          | 9c52d27 | MIT     | https://github.com/rafagafe/base85 | src/cellular/encoding



# LED Behavior

## Status LED
| State              | Color  | Pattern  |
|--------------------|--------|----------|
| Charge             | Yellow | Solid    |
| Sleep              | Black  | Solid    |
| CLI                | Red    | Solid    |
| Deploying          | White  | Blink    |
| Deployed           | White  | Fade     |
| Connecting         | Blue   | Fade     |
| Uploading          | Blue   | Blink    |

## Battery LED
| Condition                                    | Battery LED State |
|----------------------------------------------|-------------------|
| No charger                                   | LED OFF           |
| Is charging and less than 60 ms have passed  | No Change         |
| Is charging and at least 60 ms have passed   | LED Blinking      |
| Not charging and less than 10 ms have passed | No Change         |
| Not charging and at least 10 ms have passed  | LED ON            |


## Wet Dry LED
TODO

# Installation for for GCC
Run in a Linux environment

1. Install build-essentials and cmake, gdb
2. Ensure submodules are initialized and updated

