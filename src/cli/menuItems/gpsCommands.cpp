#include "Particle.h"

#include "gpsCommands.hpp"
#include "../lib/AssetTrackerRK/src/AssetTrackerRK.h"
// #include "../../lib/AssetTrackerRK/src/TinyGPS++.h"
#include "../lib/AssetTrackerRK/src/TinyGPS++.h"
#include "../conio.hpp"



SYSTEM_THREAD(ENABLED);

 // forward declaration


const unsigned long PUBLISH_PERIOD = 120000;
const unsigned long SERIAL_PERIOD = 5000;
const unsigned long MAX_GPS_AGE_MS = 10000; // GPS location must be newer than this to be considered valid

// The TinyGPS++ object
TinyGPSPlus gps;
unsigned long lastSerial = 0;
unsigned long lastPublish = 0;
unsigned long startFix = 0;
bool gettingFix = false;


void CLI_GPS() 
{
    setupGPS();
    bool run = true;
    while (Serial1.available() > 0 && run) 
    {
        if (gps.encode(Serial1.read())) 
        {
			displayInfo();
		}
    }
}

void setupGPS() 
{
    Serial1.begin(9600);
    pinMode(D6, OUTPUT);
    digitalWrite(D6, LOW);
    startFix = millis();
    gettingFix = true;
}

void displayInfo()
{
	if (millis() - lastSerial >= SERIAL_PERIOD) {
		lastSerial = millis();

		char buf[128];
		if (gps.location.isValid() && gps.location.age() < MAX_GPS_AGE_MS) {
			snprintf(buf, sizeof(buf), "%f,%f,%f", gps.location.lat(), gps.location.lng(), gps.altitude.meters());
			if (gettingFix) {
				gettingFix = false;
				unsigned long elapsed = millis() - startFix;
				Serial.printlnf("%lu milliseconds to get GPS fix", elapsed);
			}
		}
		else {
			strcpy(buf, "no location");
			if (!gettingFix) {
				gettingFix = true;
				startFix = millis();
			}
		}
		Serial.println(buf);

		if (Particle.connected()) {
			if (millis() - lastPublish >= PUBLISH_PERIOD) {
				lastPublish = millis();
				Particle.publish("gps", buf, PRIVATE);
			}
		}
	}

}


