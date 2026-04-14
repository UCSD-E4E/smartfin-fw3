#ifndef __BLE_CONFIG_HPP__
#define __BLE_CONFIG_HPP__

/**
 * @file ble_config.hpp
 * @brief Local configuration knobs for BLE transport helpers.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

/**
 * @brief Depth of the BLE transmit queue (in packets).
 */
#define SF_BLE_QUEUE_CAPACITY 16

/**
 * @brief Maximum on-air packet size (bytes) aligned with BLE MTU planning.
 */
#define SF_BLE_MAX_PACKET_SIZE 236

/**
 * @brief BLE transport protocol version byte.
 */
#define SF_BLE_PROTOCOL_VERSION 1

/**
 * @brief Maximum control (write) payload length (bytes).
 */
#define SF_BLE_MAX_CONTROL_LEN 64

#endif // __BLE_CONFIG_HPP__
