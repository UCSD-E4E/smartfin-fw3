#ifndef __RECORDER_HPP__
#define __RECORDER_HPP__

#include "cli/conio.hpp"
#include "deploy.hpp"
#include "product.hpp"

#include <stddef.h>
#include <limits.h>

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
    /**
     * @brief Initializes the Recorder to an idle state
     *
     * @return int 1 if successful, otherwise 0
     */
    int init(void);
    /**
     * @brief Checks if the Recorder has data to upload
     *
     * @return int  1 if data exists, otherwise 0
     */
    int hasData(void);
    /**
     * @brief Retrieves the last packet of data into pBuffer, and puts the 
     *  session name into pName.
     *
     * @param pBuffer Buffer to place last packet into
     * @param bufferLen Length of packet buffer
     * @param pName Buffer to place session name into
     * @param nameLen Length of name buffer
     * @return int -1 on failure, number of bytes placed into data buffer 
     *  otherwise
     */
    int getLastPacket(void* pBuffer,
                      size_t bufferLen,
                      char* pName,
                      size_t nameLen);
    /**
     * @brief Trims the last block with specified length from the recorder
     *
     * @param len Length of block to trim
     * @return int 1 if successful, otherwise 0
     */
    int popLastPacket(size_t len);
    /**
     * @brief Set the current session name
     *
     * @param sessionName Current name to set
     */
    void setSessionName(const char* const);
    /**
     * @brief Gets number of files in filesystem
     *
     * @return int number of files
     */
    int getNumFiles(void);
    /**
     * @brief Opens a session and configures the Recorder to record data
     *
     * @return int 1 if successful, otherwise 0
     */
    int openSession(void);
    /**
     * @brief Closes the current session
     *
     * If the session was already closed, treated as success.
     *
     * @return int  1 if successful, otherwise 0
     */
    int closeSession(void);
    /**
     * @brief Put bytes into current data buffer
     *
     * @param pData data to put
     * @param nBytes number of bytes to put
     *
     * @return sucsess
     */
    int putBytes(const void* pData, size_t nBytes);

    template <typename T> int putData(T& data)
    {
        return this->putBytes(&data, sizeof(T));
    };

private:
    typedef struct timestamp_entry_
    {
        uint32_t data_index;
        uint32_t timestamp;
    }timestamp_entry_t;

    typedef struct metadata_header_
    {
        uint32_t next_data_index;
        uint32_t n_entries;
    }metadata_header_t;

    char filename_buffer[REC_SESSION_NAME_MAX_LEN];
    char lastSessionName[REC_SESSION_NAME_MAX_LEN + 1];
    uint8_t dataBuffer[REC_MEMORY_BUFFER_SIZE];
    uint32_t dataIdx;
    Deployment* pSession;
    metadata_header_t metadata_header;
    bool metadata_header_valid;

    int create_metadata_file(void);

    int openLastSession(Deployment& session, char* pName);
};

#endif