/**
 * @file dmp_firmware.hpp
 * @brief Declaration of ICM-20948 DMP 3.0a firmware binary image.
 */
#ifndef SF_MCU_DMP_FIRMWARE_HPP
#define SF_MCU_DMP_FIRMWARE_HPP

#include <cstdint>
#include <cstddef>

namespace sf_mcu
{

constexpr std::size_t kDmpFirmwareSize = 14301;
extern const uint8_t kDmpFirmwareImage[kDmpFirmwareSize];

} // namespace sf_mcu

#endif // SF_MCU_DMP_FIRMWARE_HPP
