/**
 * @file high_rate_record.hpp
 * @brief Fixed-size IMU record passed from acquisition to transport.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#ifndef __HIGH_RATE_RECORD_HPP__
#define __HIGH_RATE_RECORD_HPP__

#include "deploy/ensembleTypes.hpp"

#include <cstdint>

#pragma pack(push, 1)
/**
 * @brief Fixed-size IMU sample captured for high-rate streaming.
 *
 * Packed so multiple records can be batched into a single BLE packet.
 */
struct HighRateImuRecord
{
    /** @brief Ensemble header carrying timestamp/type info. */
    EnsembleHeader_t header;
    /** @brief Quantized IMU/magnetometer values. */
    Ensemble12_data_t data;
};
#pragma pack(pop)

#endif // __HIGH_RATE_RECORD_HPP__
