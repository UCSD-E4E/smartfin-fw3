/*
 * Project smartfin-fw3
 * Description:
 * Author:
 * Date:
 */
#include "Particle.h"

// setup() runs once, when the device is first turned on.
void setup() {
    // Put initialization like pinMode and begin functions here.
    Serial.begin(9500);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
    // The core of your code will likely live here.
    Serial.printf("Hello World!\n");
    delay(1000);
}