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
    status.setColor(SF_DUP_RGB_LED_COLOR);
    status.setPattern(SF_DUP_CONNECT_RGB_LED_PATTERN);
    status.setPeriod(SF_DUP_RGB_LED_PERIOD);
    status.setPriority(SF_DUP_RGB_LED_PRIORITY);
    status.setActive();
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
        if (pSystemDesc->flags->hasCharger)
        {
            return STATE_CHARGE;
        }
        else
        {
            return STATE_DEEP_SLEEP;
        }
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
#if SF_CAN_UPLOAD
    return STATE_UPLOAD;
#else
    return STATE_DEEP_SLEEP;
#endif
}

STATES_e DataUpload::run(void)
{
#if SF_CAN_UPLOAD
    uint8_t binary_packet_buffer[SF_PACKET_SIZE];
    char ascii_record_buffer[SF_RECORD_SIZE + 1];
    char publishName[DU_PUBLISH_ID_NAME_LEN + 1];
    int nBytesToEncode;
    size_t nBytesToSend;
    STATES_e next_state;
    int retval;
    size_t recordsUploaded = 0;

    if (!this->initSuccess)
    {
        SF_OSAL_printf("Failed to init\n");
        FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0401);
        return STATE_DEEP_SLEEP;
    }

    status.setPattern(SF_DUP_RGB_LED_PATTERN);
    status.setPeriod(SF_DUP_RGB_LED_PERIOD / 2);
    status.setActive();

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
            FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0402);
            return STATE_DEEP_SLEEP;
        case -1:
        case -3:
            // Either active session (bug) or buffer overflow (bug)
            SF_OSAL_printf("Failed to retrieve data: %d" __NL__, nBytesToEncode);
            FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0403);
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
        if ((retval = urlsafe_b64_encode(
                 binary_packet_buffer, nBytesToEncode, ascii_record_buffer, &nBytesToSend)))
        {
            // size limit violation is bug
            SF_OSAL_printf("Failed to encode: %d" __NL__, retval);
            FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0404);
            return STATE_CLI;
        }

        SF_OSAL_printf("Got %u bytes to upload" __NL__, nBytesToSend);

        SF_OSAL_printf("Data: %s" __NL__, ascii_record_buffer);
        retval = sf::cloud::publish_blob(publishName, ascii_record_buffer);
        recordsUploaded++;
        switch (retval)
        {
        default:
        case sf::cloud::OVERSIZE_DATA:
        case sf::cloud::OVERSIZE_NAME:
            SF_OSAL_printf("Failed to publish: %d" __NL__, retval);
            FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0405);
            FLOG_AddError(FLOG_UPL_COUNT, recordsUploaded);
            return STATE_CLI;
        case sf::cloud::NOT_CONNECTED:
            FLOG_AddError(FLOG_UPL_CONNECT_FAIL, 1);
            FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0406);
            FLOG_AddError(FLOG_UPL_COUNT, recordsUploaded);
            return STATE_DEEP_SLEEP;
        case sf::cloud::PUBLISH_FAIL:
            FLOG_AddError(FLOG_UPL_PUB_FAIL, 0);
            FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0407);
            FLOG_AddError(FLOG_UPL_COUNT, recordsUploaded);
            return STATE_DEEP_SLEEP;
        case sf::cloud::SUCCESS:
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
            FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0408);
            FLOG_AddError(FLOG_UPL_COUNT, recordsUploaded);
            return STATE_CLI;
        case 0:
            break;
        }
    }
    FLOG_AddError(FLOG_SYS_STARTSTATE_JUSTIFICATION, 0x0409);
    FLOG_AddError(FLOG_UPL_COUNT, recordsUploaded);
    return next_state;
#else
    return STATE_DEEP_SLEEP;
#endif
}

void DataUpload::exit(void)
{
    if (sf::cloud::wait_disconnect(5000))
    {
        FLOG_AddError(FLOG_CELL_DISCONN_FAIL, 0);
    }
    status.setActive(false);
}

// In smartfin-fw2/src/dataUpload::DataUpload::exitState(void), we return based on the water sensor state.  If the system is in the water, we redeploy, otherwise we go to sleep.
/*
STATES_e DataUpload::exitState(void)
{
    return STATE_DEEP_SLEEP;
}
*/