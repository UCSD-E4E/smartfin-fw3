#ifndef __GPSCMD_HPP__
#define __GPSCMD_HPP__


#include "ubloxGPS.h"
#include "quecGNSS.h"

/**
 * @brief Display info from GPS
 * 
 */
void displayInfo();
/**
 * @brief Setup GPS serial
 * 
 */
void setupGPS();
/**
 * @brief GPS command, displays data to serial repeatibly
 * 
 */
void CLI_GPS();


#endif