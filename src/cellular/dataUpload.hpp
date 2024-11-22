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


class DataUpload : public Task{
    public:
    void init(void);
    STATES_e run(void);
    void exit(void);

    private:
    /**
     * @brief Stores if data upload state successfully initializes.
     * Variable storing 1 if system successfully enters data upload state, or 0 if system times out
     * before entering data upload state.
     */
    int initSuccess;
    system_tick_t lastConnectTime;
    //In smartfin-fw2/src/dataUpload::DataUpload::exitState(void), we return based on the water sensor state.  If the system is in the water, we redeploy, otherwise we go to sleep.
    //STATES_e exitState(void);
    /**
     * @brief Identifies if data upload is possible.
     * Returns that the data upload is possible if: the recorder has data, is connected to a cloud
     * service, is not in water, and the battery has enough voltage for an upload. If the system is currently in water, we redeploy, otherwise we go to sleep.
     * @return Returns state enumeration of STATE_UPLOAD, STATE_DEEP_SLEEP, or STATE_DEPLOYED upon execution.
     */
    STATES_e can_upload(void);
};
#endif