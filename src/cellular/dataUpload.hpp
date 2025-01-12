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
     * Variable storing 1 if system successfully enters data upload state, or 0 if system times out
     * before entering data upload state.
     */
    int initSuccess;
    //! Tracks the last connection attempt time.
    system_tick_t lastConnectTime;
    /**
     * @brief Identifies if data upload is possible.
     * 
     * |                      | Return State                                                                                                                                                           |                                                                                                                                                                              |                                                                                                                                                                             |
     * |----------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
     * |                      | data upload is possible                                                                                                                                                | smartfin redeploys                                                                                                                                                           | smartfin goes to sleep                                                                                                                                                      |
     * | required conditions: | All of the following conditions must be met:<br><br>- Recorder has data is TRUE<br>- Connected to cloud is TRUE<br>- In water is FALSE<br>- Sufficient voltage is TRUE | All of the following conditions must be met:<br><br>- Recorder has data is TRUE<br>- Connected to cloud is TRUE<br>- In water is TRUE<br>- Sufficient voltage: TRUE or FALSE | Any one of the conditions are met:<br><br>- Recorder has data is FALSE<br>- Connected to cloud is FALSE<br>- In water is FALSE at the same time sufficient voltage is FALSE |
     * @return Returns current state indicationg upload readiness in form of state enumeration of STATE_UPLOAD, STATE_DEEP_SLEEP, or STATE_DEPLOYED upon execution.
     */
    STATES_e can_upload(void);
};
#endif