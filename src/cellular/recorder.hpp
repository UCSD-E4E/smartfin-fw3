#ifndef __RECORDER_HPP__
#define __RECORDER_HPP__

#include "SpiffsParticleRK.h"
#include <stddef.h>
#include "deploy.hpp"
#include "cli/conio.hpp"
#include "product.hpp"

/**
 * @brief Maximum Session Name Length
 * 
 */
#define REC_SESSION_NAME_MAX_LEN 31

#define REC_MEMORY_BUFFER_SIZE  512
#if SF_UPLOAD_ENCODING == SF_UPLOAD_BASE85
#define REC_MAX_PACKET_SIZE  496
#elif SF_UPLOAD_ENCODING == SF_UPLOAD_BASE64 || SF_UPLOAD_ENCODING == SF_UPLOAD_BASE64URL
#define REC_MAX_PACKET_SIZE  466
#endif

class Recorder
{
    public:
    int init(void);
    int hasData(void);
    int getLastPacket(void* pBuffer, size_t bufferLen, char* pName, size_t nameLen);
    void resetPacketNumber(void);
    void incrementPacketNumber(void);
    int popLastPacket(size_t len);
    void setSessionName(const char* const);
    int getNumFiles(void);

    int openSession(const char* const depName);
    int closeSession(void);
    int putBytes(const void* pData, size_t nBytes);

    template <typename T> int putData(T& data)
    {
        return this->putBytes(&data, sizeof(T));
    };

    private:
    const char* uploadIgnorePatterns[2] = 
    {
        "TMP116",
        NULL
    };
    char currentSessionName[REC_SESSION_NAME_MAX_LEN + 1];
    char lastSessionName[REC_SESSION_NAME_MAX_LEN + 1];
    uint8_t dataBuffer[REC_MEMORY_BUFFER_SIZE];
    uint32_t dataIdx;
    Deployment* pSession;

    void getSessionName(char* fileName);

    int openLastSession(Deployment &session, char* pName);
};

#endif