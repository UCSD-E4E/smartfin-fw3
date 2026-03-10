/**
 * @file ensemble_commit.cpp
 * @brief Implements the commitEnsemble helper that writes to both recorder and the live BLE
 * streamer.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */

#include "ensemble_commit.hpp"

#include "ble/ble_live_stream.hpp"
#include "cli/flog.hpp"
#include "system.hpp"



// Transition period policy switches.
// Later, when recorder is removed, set ENABLE_RECORD_SINK to 0.
#define ENABLE_RECORD_SINK 1
#define ENABLE_STREAM_SINK 1

int sf::deploy::commitEnsemble(const void *pData, std::size_t len)
{
    if (pData == nullptr || len == 0)
    {
        FLOG_AddError(FLOG_REC_COMMIT_FAIL, 0);
        return COMMIT_INVALID_ARG;
    }

    bool recordEnabled = (ENABLE_RECORD_SINK != 0);
    bool streamEnabled = (ENABLE_STREAM_SINK != 0);

    if (!recordEnabled && !streamEnabled)
    {
        FLOG_AddError(FLOG_REC_COMMIT_FAIL, 1);
        return COMMIT_NO_SINKS_ENABLED;
    }

    bool recordOk = false;
    bool streamOk = false;

#if ENABLE_RECORD_SINK
    if (recordEnabled)
    {
        if (pSystemDesc != nullptr && pSystemDesc->pRecorder != nullptr)
        {
            const int writeResult = pSystemDesc->pRecorder->putBytes(pData, len);
            if (writeResult == 0)
            {
                recordOk = true;
            }
            else
            {
                FLOG_AddError(FLOG_REC_COMMIT_FAIL, writeResult);
            }
        }
        else
        {
            FLOG_AddError(FLOG_REC_COMMIT_FAIL, 2);
        }
    }
#endif

#if ENABLE_STREAM_SINK
    if (streamEnabled)
    {
        BleLiveStream &stream = BleLiveStream::getInstance();
        if (stream.enqueueEnsemble(pData, len))
        {
            streamOk = true;
        }
        else
        {
            FLOG_AddError(FLOG_COMMIT_STREAM_FAIL, 0);
        }
    }
#endif

    if (recordEnabled && streamEnabled)
    {
        if (recordOk && streamOk)
        {
            return COMMIT_OK;
        }
        if (!recordOk && !streamOk)
        {
            return COMMIT_BOTH_FAIL;
        }
        if (!recordOk)
        {
            return COMMIT_RECORD_FAIL;
        }
        return COMMIT_STREAM_FAIL;
    }

    if (recordEnabled)
    {
        return recordOk ? COMMIT_OK : COMMIT_RECORD_FAIL;
    }

    return streamOk ? COMMIT_OK : COMMIT_STREAM_FAIL;
}