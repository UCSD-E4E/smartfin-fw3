#include "dataUpload.hpp"

#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "cellular/sf_cloud.hpp"
#include "encoding/base85.h"
#include "encoding/base64.h"

#include "consts.hpp"
#include "system.hpp"
#include "product.hpp"
#include "sleepTask.hpp"

#include "Particle.h"

void DataUpload::init(void)
{
    SF_OSAL_printf("Entering SYSTEM_STATE_DATA_UPLOAD" __NL__);

    this->initSuccess = 1;
    if (sf::cloud::wait_connect(SF_CELL_SIGNAL_TIMEOUT_MS))
    {
        this->initSuccess = 0;
    }
}

STATES_e DataUpload::can_upload(void)
{
    if (!pSystemDesc->pRecorder->hasData())
    {
        return STATE_DEEP_SLEEP;
    }

    if (!sf::cloud::is_connected())
    {
        if (sf::cloud::wait_connect(SF_CELL_SIGNAL_TIMEOUT_MS))
        {
            // Lost connection and failed to reconnect
            return STATE_DEEP_SLEEP;
        }
    }

    if (pSystemDesc->pWaterSensor->getCurrentStatus())
    {
        // In water
        return STATE_DEPLOYED;
    }

    if (pSystemDesc->pBattery->getVCell() < SF_BATTERY_UPLOAD_VOLTAGE)
    {
        return STATE_DEEP_SLEEP;
    }
    // Don't change current state, continue looping
    return STATE_UPLOAD;
}

STATES_e DataUpload::run(void)
{
    uint16_t reupload_delay;

    pSystemDesc->pNvram->get(NVRAM::REUPLOAD_WAIT_TIME, reupload_delay);
    if (reupload_delay == 0)
    {
        pSystemDesc->pNvram->put(NVRAM::REUPLOAD_WAIT_TIME, 10);
    }
    else if (reupload_delay <= 900)
    {
        reupload_delay *= 2;
    }
    else
    {
        reupload_delay = 1800;
    }
    pSystemDesc->pNvram->put(NVRAM::REUPLOAD_WAIT_TIME, reupload_delay);

    uint8_t binary_packet_buffer[SF_PACKET_SIZE];
    char ascii_record_buffer[SF_RECORD_SIZE + 1];
    char publishName[DU_PUBLISH_ID_NAME_LEN + 1];
    int nBytesToEncode;
    size_t nBytesToSend;
    STATES_e next_state;
    int retval;

    if (!this->initSuccess)
    {
        SF_OSAL_printf("Failed to init\n");
        return STATE_DEEP_SLEEP;
    }

    while ((next_state = can_upload()) == STATE_UPLOAD)
    {
        memset(binary_packet_buffer, 0, SF_PACKET_SIZE);
        nBytesToEncode = pSystemDesc->pRecorder->getLastPacket(binary_packet_buffer, SF_PACKET_SIZE, publishName, DU_PUBLISH_ID_NAME_LEN);
        switch (nBytesToEncode)
        {
        case -2:
            // We already know that there is data, but we aren't able to retrieve
            // it.  This can indicate recorder failure.
            FLOG_AddError(FLOG_UPL_OPEN_FAIL, 0);
            return STATE_DEEP_SLEEP;
        case -1:
        case -3:
            // Either active session (bug) or buffer overflow (bug)
            SF_OSAL_printf("Failed to retrieve data: %d" __NL__, nBytesToEncode);
            return STATE_CLI;
        default:
            break;
        }

        SF_OSAL_printf("Publish ID: %s" __NL__, publishName);

        // 32-bit alignment? 4 byte alignment?
        if (nBytesToEncode % 4 != 0)
        {
            nBytesToEncode += 4 - (nBytesToEncode % 4);
        }
        SF_OSAL_printf("Got %d bytes to encode" __NL__, nBytesToEncode);

        // Encode
        memset(ascii_record_buffer, 0, SF_RECORD_SIZE + 1);
        nBytesToSend = SF_RECORD_SIZE + 1;
        if (retval = urlsafe_b64_encode(binary_packet_buffer, nBytesToEncode, ascii_record_buffer, &nBytesToSend))
        {
            // size limit violation is bug
            SF_OSAL_printf("Failed to encode: %d" __NL__, retval);
            return STATE_CLI;
        }

        SF_OSAL_printf("Got %u bytes to upload" __NL__, nBytesToSend);

        SF_OSAL_printf("Data: %s" __NL__, ascii_record_buffer);

        switch ((retval = sf::cloud::publish_blob(publishName, ascii_record_buffer)))
        {
        default:
        case sf::cloud::OVERSIZE_DATA:
        case sf::cloud::OVERSIZE_NAME:
            SF_OSAL_printf("Failed to publish: %d" __NL__, retval);
            return STATE_CLI;
        case sf::cloud::NOT_CONNECTED:
            FLOG_AddError(FLOG_UPL_CONNECT_FAIL, 1);
            return STATE_DEEP_SLEEP;
        case sf::cloud::PUBLISH_FAIL:
            FLOG_AddError(FLOG_UPL_PUB_FAIL, 0);
            return STATE_DEEP_SLEEP;
        case sf::cloud::SUCCESS:
            pSystemDesc->pNvram->put(NVRAM::REUPLOAD_WAIT_TIME, 0);
            break;
        }

        SF_OSAL_printf("Uploaded record" __NL__);

        Particle.process();

        switch (pSystemDesc->pRecorder->popLastPacket(nBytesToEncode))
        {
        case -1:
        case -2:
            // This is a bug
            SF_OSAL_printf("Session failure" __NL__);
            return STATE_CLI;
        case 0:
            break;
        }
    }
    return next_state;
}

void DataUpload::exit(void)
{
    if (sf::cloud::wait_disconnect(5000))
    {
        FLOG_AddError(FLOG_CELL_DISCONN_FAIL, 0);
    }
}

STATES_e DataUpload::exitState(void)
{
    return STATE_DEEP_SLEEP;
}