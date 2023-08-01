#ifndef __DATAUPLOAD_HPP__
#define __DATAUPLOAD_HPP__

#include "task.hpp"
#include "Particle.h"
#include "product.hpp"
#include <SpiffsParticleRK.h>


/**
 * RGB LED state is handled by the system theme.
 */

#if SF_UPLOAD_ENCODING == SF_UPLOAD_BASE85
/**
 * How many bytes to store chunks of data in on the SPI flash.
 * 
 * 496 * 5/4 (base85 encoding compression rate) = 620 which is less than the 622
 * bytes which is the maximum size of publish events.
 */
#define DATA_UPLOAD_MAX_BLOCK_LEN   496
#elif SF_UPLOAD_ENCODING == SF_UPLOAD_BASE64 || SF_UPLOAD_ENCODING == SF_UPLOAD_BASE64URL
/**
 * How many bytes to store chunks of data in on the SPI flash.
 * 
 * 466 * 4/3 (base64 encoding compression rate) = 621 which is less than the 622
 * bytes which is the maximum size of publish events.
 */
#define DATA_UPLOAD_MAX_BLOCK_LEN   466
#endif

/**
 * @brief Number of bytes to buffer for upload
 * 
 */
#define DATA_UPLOAD_MAX_UPLOAD_LEN  (particle::protocol::MAX_EVENT_DATA_LENGTH)

/**
 * @brief Minimum time between publish
 * 
 */
#define DATA_UPLOAD_MIN_PUBLISH_TIME_MS 1000

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
    spiffs_DIR dir;
    int initSuccess;
    system_tick_t lastConnectTime;
    STATES_e exitState(void);
};
#endif