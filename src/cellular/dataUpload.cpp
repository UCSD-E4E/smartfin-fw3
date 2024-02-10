#include "dataUpload.hpp"

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "cellular/sf_cloud.hpp"
#include "encoding/base85.h"
#include "encoding/base64.h"

#include "system.hpp"
#include "product.hpp"
#include "sleepTask.hpp"

#include "Particle.h"

void DataUpload::init(void)
{
    SF_OSAL_printf("Entering SYSTEM_STATE_DATA_UPLOAD\n");

    this->initSuccess = 1;
    if(sf::cloud::wait_connect(30000))
    {
        this->initSuccess = 0;
    }
}

STATES_e DataUpload::run(void)
{


    uint8_t dataEncodeBuffer[DATA_UPLOAD_MAX_BLOCK_LEN];
    char dataPublishBuffer[DATA_UPLOAD_MAX_UPLOAD_LEN];
    char publishName[DU_PUBLISH_ID_NAME_LEN + 1];
    int nBytesToEncode;
    size_t nBytesToSend;

    if (!this->initSuccess)
    {
        SF_OSAL_printf("Failed to init\n");
        return STATE_DEEP_SLEEP;
    }

    memset(dataEncodeBuffer, 0, DATA_UPLOAD_MAX_BLOCK_LEN);
    nBytesToEncode = pSystemDesc->pRecorder->getLastPacket(dataEncodeBuffer, DATA_UPLOAD_MAX_BLOCK_LEN, publishName, DU_PUBLISH_ID_NAME_LEN);
    if (-1 == nBytesToEncode)
    {
        SF_OSAL_printf("Failed to retrive data\n");
        return STATE_CLI;
    }

    SF_OSAL_printf("Publish ID: %s\n", publishName);
    if (nBytesToEncode % 4 != 0)
    {
        nBytesToEncode += 4 - (nBytesToEncode % 4);
    }
    SF_OSAL_printf("Got %d bytes to encode\n", nBytesToEncode);

    memset(dataPublishBuffer, 0, DATA_UPLOAD_MAX_UPLOAD_LEN);
    nBytesToSend = DATA_UPLOAD_MAX_UPLOAD_LEN;
    urlsafe_b64_encode(dataEncodeBuffer, nBytesToEncode, dataPublishBuffer, &nBytesToSend);

    SF_OSAL_printf("Got %u bytes to upload\n", nBytesToSend);

    if (!sf::cloud::is_connected())
    {
        // we're not connected!  abort
        SF_OSAL_printf("not connected :(");
        return STATE_CLI;
    }

    SF_OSAL_printf("Data: %s", dataPublishBuffer);

    bool sucsess = Particle.publish(publishName, dataPublishBuffer);
    if (!sucsess)
    {
        SF_OSAL_printf("Failed to upload data!\n");
    }

    SF_OSAL_printf("Uploaded record %s\n", dataPublishBuffer);
    Particle.process();

    pSystemDesc->pRecorder->popLastPacket(nBytesToEncode);

    return STATE_CLI;
}

void DataUpload::exit(void)
{
    if(sf::cloud::wait_disconnect(5000))
    {
        FLOG_AddError(FLOG_CELL_DISCONN_FAIL, 0);
    }
}

STATES_e DataUpload::exitState(void)
{
    return STATE_DEEP_SLEEP;
}