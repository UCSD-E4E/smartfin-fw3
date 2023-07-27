#ifndef __GPSCMD_HPP__
#define __GPSCMD_HPP__
#include "../lib/gps-tracker/location_service.h"


/**
 * @brief Display info from GPS
 * 
 */
void displayInfo(LocationPoint point);
/**
 * @brief GPS command, displays data to serial repeatibly
 * 
 */
void CLI_GPS();


#endif