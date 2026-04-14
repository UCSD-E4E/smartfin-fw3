/**
 * @file sf_ble_defs.hpp
 * @brief Smartfin BLE protocol identifiers and sizing constants.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 3-9-2026
 */
#ifndef __SF_BLEDEFS_HPP__
#define __SF_BLEDEFS_HPP__


#include "ble_config.hpp"
#include "product.hpp"
#include <cstddef>

namespace sf
{
namespace bledefs
{

/**
 * @brief Protocol/interface identifier block for Smartfin BLE.
 *
 * These values are compile-time constants (not NVRAM settings). Replace the
 * example UUID strings with your own generated UUIDs when finalizing the BLE
 * profile.
 */

/** @brief Custom Smartfin live telemetry service UUID. */
inline constexpr const char *SERVICE_UUID = "a86d7b16-dd6c-434b-a7ee-f0ca33ac614c";

/** @brief Characteristic used for fin -> watch live telemetry. */
inline constexpr const char *TELEMETRY_CHAR_UUID = "deeddb00-166e-407c-8158-7b9693ad2685";

/** @brief Optional characteristic used for watch -> fin commands/config. */
inline constexpr const char *CONTROL_CHAR_UUID = "c39513e6-631e-439a-9b3b-affa0635b3d1";

/** @brief Short local BLE device name shown during advertising. */
inline constexpr const char* DEVICE_NAME = "Smartfin";

/** @brief Max notification payload bytes. */
inline constexpr std::size_t MAX_NOTIFY_LEN = SF_BLE_MAX_PACKET_SIZE;

/** @brief Max command/control payload bytes accepted from the peer. */
inline constexpr std::size_t MAX_CONTROL_LEN = SF_BLE_MAX_CONTROL_LEN;

}} // namespace sf::bledefs

#endif // __SF_BLEDEFS_HPP__
