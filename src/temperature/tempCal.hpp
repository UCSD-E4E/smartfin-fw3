/**
 * @file tempCal.hpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief Temperature Calibration
 * @version 0.1
 * @date 2025-06-26
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef __TEMPCAL_HPP__
#define __TEMPCAL_HPP__
#include "Particle.h"
#include "task.hpp"

#include <cstdint>

/**
 * @brief Temperature Calibration Task
 *
 * The temperature calibration capture is characterized by 4 parameters: p0, p1,
 * n0, and n1.  p0 and p1 control the burst and measurement periods, 
 * respectively.  n0 and n1 control the burst and measurement counts, 
 * respectively.  The temperature calibration capture state is characterized by
 * 2 parameters: i0, i1.  i0 and i1 maintain the burst and measurement indices,
 * respectively.  If the time of entry to the start of this task is t0, then the
 * time of each measurement is given by i1 * p1 + i0 * p0, where i1 < n1 and 
 * i0 < n0.
 * 
 * At each measurement point, we record both temperature and battery (Ensembles
 * 7 and 8).
 * 
 * See https://github.com/UCSD-E4E/smartfin-docs/wiki/Temperature-Calibration 
 * for a discussion of how to select appropriate p0, p1, n0, and n1.
 */
class TempCalTask : public Task
{
public:
    /**
     * @brief Initialization method
     * 
     */
    void init(void);
    /**
     * @brief Task body
     * 
     * @return STATES_e 
     */
    STATES_e run(void);
    /**
     * @brief Exit method
     * 
     */
    void exit(void);

private:
    /**
     * @brief LED Status
     * 
     */
    LEDStatus ledStatus;
    /**
     * @brief number of bursts
     * 
     */
    std::uint32_t burst_limit;
    /**
     * @brief measurements per burst
     * 
     */
    std::uint32_t measurement_limit;
    /**
     * @brief Burst period in seconds
     * 
     */
    std::uint32_t burst_period_s;
    /**
     * @brief Measurement period in seconds
     * 
     */
    std::uint32_t measurement_period_s;
    /**
     * @brief Burst index
     * 
     */
    std::uint32_t burst_idx;
    /**
     * @brief Measurement index
     * 
     */
    std::uint32_t measurement_idx;

    /**
     * @brief Sleeps until the specified timestamp
     * 
     * If duration is less than 10 minutes, simply does a delay, otherwise,
     * drops into STOP mode.
     * 
     * @param timestamp Timestamp to wait until
     */
    void sleep_until(system_tick_t timestamp);
    /**
     * @brief Measurement body
     * 
     */
    void do_measure(void);
};
#endif // __TEMPCAL_HPP__
