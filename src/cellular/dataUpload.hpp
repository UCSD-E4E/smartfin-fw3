#ifndef __DATAUPLOAD_HPP__
#define __DATAUPLOAD_HPP__

#include "task.hpp"
#include "Particle.h"
#include "product.hpp"


/**
 * RGB LED state is handled by the system theme.
 */



/**
 * @brief Number of bytes to buffer for upload
 * 
 */
#define DATA_UPLOAD_MAX_UPLOAD_LEN  (particle::protocol::MAX_EVENT_DATA_LENGTH)

/**
 * @brief Minimum time between publish
 * 
 */
#define DATA_UPLOAD_MIN_PUBLISH_TIME_MS SF_UPLOAD_MS_PER_TRANSMIT

/**
 * @brief Publish Event Name Length (not including NULL terminator)
 * 
 */
#define DU_PUBLISH_ID_NAME_LEN  (particle::protocol::MAX_EVENT_NAME_LENGTH)

/**
 * @brief Number of times to reattempt uploads
 * 
 */
#define DU_UPLOAD_MAX_REATTEMPTS    5

/**
 * @class DataUpload
 * @brief Handles the data upload task.
 *
 * This class implements the task methods and internal states
 * required for data uploading.
 */
class DataUpload : public Task
{
public:
    /**
     * @brief Initializes the data upload task.
     */
    void init(void);

    /**
     * @brief Executes the data upload task logic.
     *
     * @return The next state after executing the task.
     */
    STATES_e run(void);

    /**
     * @brief Cleans up resources and exits the task.
     */
    void exit(void);

    private:
        /**
         * @brief Flag storing if data upload state successfully initializes.
         * Variable storing 1 if system successfully enters data upload state, or 0
         * if system times out before entering data upload state.
         */
        int initSuccess;
        //! Tracks the last connection attempt time.
        system_tick_t lastConnectTime;
        /**
         * @brief Identifies if data upload is possible.
         *
         * The following conditions must all be true for data upload:
         * - Recorder has data
         * - System is connected to cloud (this function will attempt to connect
         * and will block up to SF_CELL_SIGNAL_TIMEOUT_MS to complete the attempt)
         * - System is not in water
         * - Battery voltage is greater than SF_BATTERY_UPLOAD_VOLTAGE
         *
         * If any of the above conditions do not hold, then this function will
         * return STATE_DEEP_SLEEP or STATE_DEPLOYED.
         *
         * Note that if we are in the water, the system is intended to immediately
         * wake out of sleep and self deploy if this runction returns
         * STATE_DEEP_SLEEP.
         * @return Indicates whether the system can upload (STATE_UPLOAD), or should
         * enter sleep (STATE_DEEP_SLEEP) or deploy (STATE_DEPLOYED).
         */
        STATES_e can_upload(void);
};
#endif