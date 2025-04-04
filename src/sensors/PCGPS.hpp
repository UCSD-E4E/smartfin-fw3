/**
 * @file PCGPS.hpp
 * @author Charlie Kushelevsky  (charliekushelevsky@gmail.com)
 * @brief PC GPS definition
 * @version 0.1
 * @date 2025-04-04
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef __PCGPS_HPP__
#define __PCGPS_HPP__

#include "abstractGPS.hpp"

#include <cstdint>

/**
 * @brief PC GPS Module
 *
 * GPS modules should provide a nominal 1 Hz refresh rate.  Polls files in
 * gps_data
 *
 */
class PCGPSModule : public AbstractGPSModule
{
public:
    /**
     * @brief Initializes the PCGPS Module
     *
     * This function should take in the underlying hardware handles and
     * set up internal structures.  This function must only be called once
     * during firmware startup.  Subsequent calls to this function may
     * result in undefined behavior.  This function must not start any
     * threads or enable any processing.  This function must not take longer
     * than 100 ms to execute on target.  The GPS module should be in a low
     * power mode after this function completes.
     *
     * @return Error flag.  True if the GPS module has failed to initialize.
     * If false, then the following conditions must hold:
     * - The GPS module is accessible on its bus
     * - The GPS module is appropriately configured
     * - The GPS module is in a low power mode
     */
    bool init(void) override;

    /**
     * @brief Starts the PCGPS Module
     *
     * This function should enable data acquisition.  This function may be
     * called more than once during firmware runtime.  Sequential calls to
     * this function must be idempotent.  This function may start additional
     * threads.  This function must not take longer than 10 ms to execute
     * on target.  The module state after calling this function must be the
     * same whether or not stop was called previously.
     *
     * @return Error flag.  True if the GPS module has failed to start.  If
     * false, then the following conditions must hold:
     * - The GPS module is accessible on its bus
     * - The GPS module is appropriately configured
     * - The GPS module is ready to emit location data
     */
    bool start(void) override;

    /**
     * @brief Stops the PCGPS Module
     *
     * This function must disable data acquisition.  This function may be
     * called more than once during firmware runtime.  Sequential calls to
     * this function must be idempotent.  This function must stop any
     * threads started by start.  The module state after this call completes
     * must be functionally identical to the module after init is called but
     * before start is called.  This function must not take longer than 10
     * ms to execute on target.
     *
     * @return Error flag.  True if the GPS module has failed to stop.  If
     * false, then the following conditions must hold:
     * - The GPS module is accessible on its bus
     * - The GPS module is appropriately configured
     * - The GPS module is in a low power mode
     */
    bool stop(void);

    /**
     * @brief Deinitializes the PCGPS Module
     *
     * This function should release any resources acquired during init. This
     * function may not be called more than once during firmware runtime.
     * Subsequent calls to deinit may result in undefined behavior. This
     * function may not take more than 100 ms to run on target.
     *
     *
     * @return Error flag.  True if the GPS module has failed to
     * deinitialize.
     */
    bool deinit(void) override;

    /**
     * @brief Retrieves a location estimate
     *
     * This function should retrieve the most up-to-date location estimate.
     * Sequential calls may return the same data until the next location
     * estimate. Implementations must guarantee that data are consistent -
     * that is, a new estimate coming in at the same time this function is
     * called must not yield some values from the old estimate and some
     * values from the new estimate.
     *
     * @param lat Reference to variable to store latitude in decimal degrees
     * WGS84.
     * @param lon Reference to variable to store longitude in decimal
     * degrees WGS84
     * @param alt Reference to variable to store altitude in meters above
     * mean sea level (WGS84)
     * @param timestamp Pointer to variable to store timestamp in seconds
     * since UTC epoch.  If null, do not store data.
     * @param nSats Pointer to variable to store number of satellites in
     * fix.  If null do not store data.
     * @return Error flag.  True if no GPS data or invalid GPS data
     * returned.  If false, then the following conditions hold:
     * - lat, lon, and alt were modified to reflect the most up-to-date
     * location estimate
     * - If not null, *timestamp was modified to reflect the timestamp of
     * the most up-to-date location estiamte
     * - If not null, *nSats was modified to reflect the number of
     * satellites used in the most up-to-date location estimate
     * - All modified values are consistent (from the same estimate)
     */
    bool getLocation(float &lat, float &lon, float &alt, uint64_t *timestamp, 
                                                        int *nSats) override;
};

#endif // __PCGPS_HPP__
