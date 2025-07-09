# smartfin-fw3
Smartfin FW v3

## Flashing via USB
### Required materials:
- firmware-tracker-5.3.0.bin from https://github.com/UCSD-E4E/smartfin-fw3/release/latest, stored at `${fw_path}`
- Particle Workbench from https://docs.particle.io/quickstart/workbench/
- Smartfin v3
- USB Cable

1. Connect the Smartfin v3 to the computer using the USB Cable.  The LED should become solid yellow.
2. Open Particle Workbench
3. Open the Command Palette
4. Execute `Particle: Launch CLI` in the Command Palette
5. Log in to Particle using `particle login`
6. Put the device into DFU mode using `particle usb dfu`.  The LED should be blinking yellow.
7. Flash the firmware using `particle flash --usb ${fw_path}`.

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

# Connecting via Serial
## Windows
1. We recommend using a serial terminal such as PuTTY.
2. Find the COM port for the Smartfin using Device Manager.
3. Open PuTTY.
4. Set the `Connection Type` to `Serial`.
5. Set `Serial Line` to the COM port discovered in Device Manager (e.g. COM3).

Note that there is no need to modify the `Speed`, as Smartfin uses Serial over
USB, which does not have a physical speed parameter.

6. Select the `Terminal` category.
7. Ensure the `Implicit CR in every LF` checkbox is checked.
8. Click `Open`.
9. Type in `#CLI` without hitting `ENTER` to access the CLI.

## MacOS and Linux
1. We recommend using a serial terminal such as `minicom` or `picocom`.
2. Find the tty port for the Smartfin by running `ls /dev/tty*` in the terminal.
3. Open the port using the following command:
```
picocom --omap lfcrlf /dev/ttyUSB0
```
where `/dev/ttyUSB0` is the correct TTY port for the Smartfin.

You may need to run this command as `sudo` if your user is not part of the
`dialout` group!
4. Type in `#CLI` without hitting `ENTER` to access the CLI.