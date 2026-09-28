/**
 * @file dmp_firmware.cpp
 * @brief Definition of ICM-20948 DMP 3.0a firmware binary image.
 */
#include "dmp_firmware.hpp"

namespace sf_mcu
{

const uint8_t kDmpFirmwareImage[kDmpFirmwareSize] = {
#include "../src/imu/ICM-20948/util/icm20948_img.dmp3a.h"
};

} // namespace sf_mcu
