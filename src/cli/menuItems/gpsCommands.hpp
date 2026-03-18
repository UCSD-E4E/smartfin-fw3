#ifndef __GPSCMD_HPP__
#define __GPSCMD_HPP__

#include "product.hpp"

#if SF_ENABLE_GPS

#include "location_service.h"

void displayInfo(LocationPoint point);
void CLI_GPS();

#endif // SF_ENABLE_GPS
#endif // __GPSCMD_HPP__
