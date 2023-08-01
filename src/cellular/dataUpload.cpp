#include "dataUpload.hpp"

#include "Particle.h"
#include "system.hpp"
#include "conio.hpp"
#include "product.hpp"
#include "base85.h"
#include "base64.h"
#include "sleepTask.hpp"
#include "flog.hpp"

void DataUpload::init(void)
{
    SF_OSAL_printf("Entering SYSTEM_STATE_DATA_UPLOAD\n");

    this->initSuccess = 0;
    Particle.connect();
    os_thread_yield();
    this->initSuccess = 1;
}

STATES_e DataUpload::run(void)
{
    if(!this->initSuccess)
    {
        SF_OSAL_printf("Failed to init\n");
        return STATE_DEEP_SLEEP;
    }

    if(!Particle.connected())
    {
        // we're not connected!  abort
        return STATE_CLI;
    }

    if(!Particle.publish("Test Publish", "Hi!!!", PRIVATE | WITH_ACK))
    {
        SF_OSAL_printf("Failed to upload data!\n");
        continue;
    }

    SF_OSAL_printf("Uploaded record %s\n", dataPublishBuffer);
    Particle.process();
    lastSendTime = millis();


    return STATE_CLI;
}

void DataUpload::exit(void)
{
    Cellular.off();
}

STATES_e DataUpload::exitState(void)
{
    return STATE_DEEP_SLEEP;
}