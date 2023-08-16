# smartfin-fw3
Smartfin FW v3

# External Library Register
Library         | Version | License | Source                                           | Location
----------------|---------|---------|--------------------------------------------------|---------
LIS3DH          | 0.2.8   | MIT     | https://github.com/rickkas7/LIS3DH               | lib/LIS3DH
gps-ublox       | 1.2.0   | Apache 2.0 | https://github.com/particle-iot/gps-ublox/       | lib/gps-ublox
gps-quectel     | 1.0.0   | Apache 2.0 | https://github.com/particle-iot/gps-quectel      | lib/gps-quectel
gps-nmea-parser | 1.2.0   | Apache 2.0 | https://github.com/particle-iot/gps-nmea-parser/ | lib/gps-nmea-parser
tracker edge    | v18     | Apache 2.0 | https://github.com/particle-iot/tracker-edge     | lib/tracker-edge


# LED Behavior

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