# smartfin-fw3
Smartfin FW v3


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