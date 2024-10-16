# smartfin-fw3
Smartfin FW v3

# Developer Getting Started
1. Clone this repository
2. Open in Visual Studio Code
3. Install all recommended extensions
4. Initialize submodules
```
git submodule init
git submodule update --recursive
```
5. Ensure tooling is installed
    - `cmake`
    - `g++` (usually from `build-essential`)
    - `gdb`

## x86 Mode Debugging
Please use the CMake Tools debugger

# Contributing

Please see [docs/index.md](docs/index.md) for an overview of the codebase and how to get started contributing.

# External Library Register
Library         | Version | License | Source                                           | Location
----------------|---------|---------|--------------------------------------------------|---------
LIS3DH          | 0.2.8   | MIT     | https://github.com/rickkas7/LIS3DH               | lib/LIS3DH
gps-ublox       | 1.2.0   | Apache 2.0 | https://github.com/particle-iot/gps-ublox/       | lib/gps-ublox
gps-quectel     | 1.0.0   | Apache 2.0 | https://github.com/particle-iot/gps-quectel      | lib/gps-quectel
gps-nmea-parser | 1.2.0   | Apache 2.0 | https://github.com/particle-iot/gps-nmea-parser/ | lib/gps-nmea-parser
tracker edge    | v18     | Apache 2.0 | https://github.com/particle-iot/tracker-edge     | lib/tracker-edge
SparkFun ICM    | 1.2.0   | MIT     | https://github.com/UCSD-E4E/SparkFun_ICM-20948_ParticleLibrary | lib/SparkFun_ICM-20948_ParticleLibrary
Max 31725       | 2019    | Apache 2.0 | https://os.mbed.com/teams/MaximIntegrated/code/MAX31725_Accurate_Temperature_Sensor//file/b4fdbbe79036/max31725.h/ | temperature/
urlsafe-b64     | 022c4cc | MIT     | https://github.com/Akagi201/urlsafe-b64 | src/cellular/encoding
base85          | 9c52d27 | MIT     | https://github.com/rafagafe/base85 | src/cellular/encoding



# LED Behavior

## Status LED
State              | Color    | Pattern
---------------------------------------
Charge             | Yellow   | Solid
Sleep              | Black    | Solid
CLI                | Red      | Solid
Network Off        | Black    | Solid
Network On         | Blue     | Solid
Network Connecting | Blue     | Solid
Network DHCP       | Blue     | Solid
Cloud Connecting   | Blue     | Solid
Cloud Connected    | Blue     | Blink
Cloud Handshake    | Blue     | Blink


## Battery LED
Condition                                    | Battery LED State   |
--------------------------------------------------------------------|
No charger                                    | LED OFF             |
Is charging and less than 60 ms have passed   | No Change           |
Is charging and at least 60 ms have passed    | LED Blinking        |
Not charging and less than 10 ms have passed  | No Change           |
Not charging and at least 10 ms have passed   | LED ON              |

## Wet Dry LED
TODO