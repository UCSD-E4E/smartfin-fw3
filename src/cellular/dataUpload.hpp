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
    //! Flag to indicate successful initialization.
    int initSuccess;
    //! Tracks the last connection attempt time.
    system_tick_t lastConnectTime;

    /**
     * @brief State to transition to upon task exit.
     *
     * @return The exit state.
     */
    STATES_e exitState(void);

    /**
     * @brief Checks if data can be uploaded.
     *
     * @return The current state indicating upload readiness.
     */
    STATES_e can_upload(void);
};
#endif