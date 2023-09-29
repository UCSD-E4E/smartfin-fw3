/**
 * @file recorder.cpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief Data Recorder
 * @version 0.1
 * @date 2023-09-26
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "recorder.hpp"

#include "cellular/deploy.hpp"
#include "cli/conio.hpp"
#include "cli/flog.hpp"
#include "consts.hpp"
#include "product.hpp"
#include "system.hpp"

#include <dirent.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>


#define REC_DEBUG

#define DATA_ROOT "/data"
#define METADATA_FILE DATA_ROOT "/.metadata"
static int REC_create_data_root(void);
static void REC_create_data_filename(int data_index,
                                    char* p_filename,
                                    size_t filename_len);


int Recorder::create_metadata_file(void)
{
    struct stat stat_result;
    int result = stat(METADATA_FILE, &stat_result);
    int fp;

    memset(&this->metadata_header, 0, sizeof(metadata_header_t));

    if (0 == result)
    {
        // stat was successful
        if ((stat_result.st_mode & S_IFREG) != 0)
        {
            fp = ::open(METADATA_FILE, O_RDONLY);
            if (-1 == fp)
            {
                FLOG_AddError(FLOG_FS_OPEN_FAIL, errno);
                return 0;
            }
            ::read(fp, &this->metadata_header, sizeof(metadata_header_t));
            ::close(fp);
            this->metadata_header_valid = true;
            return 1;
        }
    #ifdef REC_DEBUG
        SF_OSAL_printf("REC::create_metadata_file: dir in the way!" __NL__);
    #endif
        unlink(METADATA_FILE);
    }
    else
    {
        if (ENOENT != errno)
        {
        #ifdef REC_DEBUG
            SF_OSAL_printf("REC::create_metadata_file: Failed to stat: %d" __NL__,
                           errno);
        #endif
        FLOG_AddError(FLOG_FS_STAT_FAIL, errno);
        return 0;
        }
    }

    fp = ::open(METADATA_FILE, O_WRONLY | O_CREAT);
    if (-1 == fp)
    {
        FLOG_AddError(FLOG_FS_CREAT_FAIL, errno);
        return 0;
    }
    ::write(fp, &this->metadata_header, sizeof(metadata_header_t));
    ::close(fp);
    this->metadata_header_valid = true;
    return 1;
}

void REC_create_data_filename(uint32_t data_index,
                             char* p_filename,
                             size_t filename_len)
{
    snprintf(p_filename, filename_len, DATA_ROOT "/%010u.bin", data_index);
}


int REC_create_data_root(void)
{
    struct stat stat_result;

    int result = stat(DATA_ROOT, &stat_result);
    if (0 == result)
    {
        if ((stat_result.st_mode & S_IFDIR) != 0)
        {
            return 1;
        }
    #ifdef REC_DEBUG
        SF_OSAL_printf("REC::create_data_root: file in the way!" __NL__);
    #endif
        unlink(DATA_ROOT);
    }
    else
    {
        if (ENOENT != errno)
        {
        #ifdef REC_DEBUG
            SF_OSAL_printf("REC::create_data_root: Failed to stat: %d" __NL__,
                           errno);
        #endif
        FLOG_AddError(FLOG_FS_STAT_FAIL, errno);
        return 0;
        }
    }

    result = mkdir(DATA_ROOT, 0777);
    if (0 == result)
    {
        return 1;
    }
#ifdef REC_DEBUG
    SF_OSAL_printf("REC::create_data_root: Failed to create: %d" __NL__, errno);
#endif
    FLOG_AddError(FLOG_FS_MKDIR_FAIL, errno);
    return 0;
}

int Recorder::init(void)
{
    // Create DATA_ROOT if necessary
    if (0 == REC_create_data_root())
    {
        FLOG_AddError(FLOG_REC_SETUP_FAIL, 0);
        return 0;
    }
    memset(this->lastSessionName, 0, REC_SESSION_NAME_MAX_LEN + 1);
    if (0 == create_metadata_file())
    {
        FLOG_AddError(FLOG_REC_SETUP_FAIL, 1);
        return 0;
    }
    return 1;
}


int Recorder::hasData(void)
{
    return this->metadata_header.n_entries != 0;
}

int Recorder::getNumFiles(void)
{
    return this->metadata_header.n_entries;
}

int Recorder::openLastSession(Deployment& session, char* pName)
{
    int fileIdx;
    int length;

    // if (REC_initializeTree())
    // {
    //     SF_OSAL_printf("Failed to initialize tree\n");
    //     return 1;
    // }
    // for (int i = 0; i < REC_DIR_TREE_SIZE; i++)
    // {
    //     SF_OSAL_printf("%d: %32s %d\n", i, REC_dirTree[i].filename,
    //         REC_dirTree[i].initialized);
    // }

    // fileIdx = REC_DIR_TREE_SIZE - 1;
    // // fileIdx = 0;
    // do
    // {
    //     if (REC_dirTree[fileIdx].initialized &&
    //         REC_dirTree[fileIdx].filename[0] != '.')
    //     {
    //         SF_OSAL_printf("Trying to open %d %32s %d\n",
    //                        fileIdx,
    //                        REC_dirTree[fileIdx].filename,
    //                        REC_dirTree[fileIdx].initialized);
    //         if (!session.open(REC_dirTree[fileIdx].filename, Deployment::RDWR))
    //         {
    //         #ifdef REC_DEBUG
    //             SF_OSAL_printf("REC::GLP open %s fail\n",
    //                            REC_dirTree[fileIdx].filename);
    //         #endif
    //             return 1;
    //         }
    //         else
    //         {
    //         #ifdef REC_DEBUG
    //             SF_OSAL_printf("REC::GLP open %s success\n",
    //                            REC_dirTree[fileIdx].filename);
    //             int file = open(REC_dirTree[fileIdx].filename, O_RDWR);

    //             length = session.getLength();
    //             SF_OSAL_printf("Length: %d\n", length);
    //             close(file);

    //             SF_OSAL_printf("Filename %s\n", REC_dirTree[fileIdx].filename);

    //         #endif
    //         }


    //         if (length == 0 || strcmp(REC_dirTree[fileIdx].filename, "") == 0)
    //         {
    //             SF_OSAL_printf("No bytes, removing\n");
    //             session.remove();
    //             REC_dirTree[fileIdx].initialized = 0;
    //             session.close();
    //         }
    //         else
    //         {
    //             strcpy(pName, REC_dirTree[fileIdx].filename);
    //             return 0;
    //         }
    //     }
    // } while (fileIdx-- > 0);

    SF_OSAL_printf("Failed to find session\n");
    return 1;
}


int Recorder::getLastPacket(void* pBuffer,
                            size_t bufferLen,
                            char* pName,
                            size_t nameLen)
{
    Deployment& session = Deployment::getInstance();
    int newLength;
    int bytesRead;
    char name[LITTLEFS_OBJ_NAME_LEN];

    if (this->openLastSession(session, name))
    {
        // memset(this->currentSessionName, 0, REC_SESSION_NAME_MAX_LEN + 1);
        return -1;
    }

    newLength = session.getLength() - bufferLen;
    session.seek(newLength);
    bytesRead = session.read(pBuffer, bufferLen);
    snprintf((char*)pName, nameLen, "Sfin-%s-%s-%d", pSystemDesc->deviceID,
             name, newLength / REC_MAX_PACKET_SIZE);
    session.close();
    strcpy(this->lastSessionName, name);
    return bytesRead;
}

/**
 * @brief Trims the last block with specified length from the recorder
 *
 * @param len Length of block to trim
 * @return int 1 if successful, otherwise 0
 */
int Recorder::popLastPacket(size_t len)
{
    Deployment& session = Deployment::getInstance();
    int newLength;

    // have last file in dirEntry
    if (!session.open(this->lastSessionName, Deployment::RDWR))
    {
    #ifdef REC_DEBUG
        SF_OSAL_printf("REC::TRIM - Fail to open\n");
    #endif
        return 0;
    }

    newLength = session.getLength() - len;
    if (newLength <= 0)
    {
        session.remove();
    }
    else
    {
        session.truncate(newLength);
    }
    session.close();

    return 1;
}

void Recorder::setSessionName(const char* const sessionName)
{
    // memset(this->currentSessionName, 0, REC_SESSION_NAME_MAX_LEN + 1);
    // strncpy(this->currentSessionName, sessionName, REC_SESSION_NAME_MAX_LEN);
    // SF_OSAL_printf("Setting session name to %s\n", this->currentSessionName);
}

int Recorder::openSession()
{
    uint32_t data_index = this->metadata_header.next_data_index;
    int fp;
    timestamp_entry_t entry = {data_index, 0};

    this->metadata_header.next_data_index++;
    this->metadata_header.n_entries++;
    fp = ::open(METADATA_FILE, O_WRONLY);
    if (-1 == fp)
    {
        FLOG_AddError(FLOG_FS_OPEN_FAIL, errno);
        return 0;
    }
    ::write(fp, &this->metadata_header, sizeof(metadata_header_t));
    ::lseek(fp, 0, SEEK_END);
    ::write(fp, &entry, sizeof(timestamp_entry_t));
    ::close(fp);

    REC_create_data_filename(data_index, this->filename_buffer, REC_SESSION_NAME_MAX_LEN);

    this->pSession = &Deployment::getInstance();
    if (!this->pSession->open(this->filename_buffer, Deployment::WRITE))
    {
        this->pSession = nullptr;
        SF_OSAL_printf("REC::OPEN Fail to open" __NL__);
        return 0;
    }
    memset(this->dataBuffer, 0, REC_MEMORY_BUFFER_SIZE);
    this->dataIdx = 0;
    SF_OSAL_printf("REC::OPEN opened %s" __NL__, this->filename_buffer);
    return 1;
}

int Recorder::closeSession(void)
{
    if (nullptr == this->pSession)
    {
        SF_OSAL_printf("REC::CLOSE Already closed\n");
        return 1;
    }

    // fill zeros at end
    for (; this->dataIdx < REC_MAX_PACKET_SIZE; this->dataIdx++)
    {
        this->dataBuffer[this->dataIdx] = 0;
    }
    // flush buffer
    int x = this->pSession->write(this->dataBuffer, REC_MAX_PACKET_SIZE);
    SF_OSAL_printf("Bytes written %d", x);

    // Close session
    this->pSession->close();
    this->pSession = nullptr;

    memset(this->dataBuffer, 0, REC_MEMORY_BUFFER_SIZE);
    this->dataIdx = 0;

    return 1;
}

int Recorder::putBytes(const void* pData, size_t nBytes)
{
    if (nBytes > (REC_MAX_PACKET_SIZE - this->dataIdx))
    {
        // data will not fit, flush and clear
        // pad 0
        for (; this->dataIdx < REC_MAX_PACKET_SIZE; this->dataIdx++)
        {
            this->dataBuffer[this->dataIdx] = 0;
        }
        SF_OSAL_printf("Flushing\n");
        this->pSession->write(this->dataBuffer, REC_MAX_PACKET_SIZE);

        memset(this->dataBuffer, 0, REC_MEMORY_BUFFER_SIZE);
        this->dataIdx = 0;
    }

    // data guaranteed to fit
    SF_OSAL_printf("Putting %u bytes\n", nBytes);
    memcpy(&this->dataBuffer[this->dataIdx], pData, nBytes);
    this->dataIdx += nBytes;
    return 1;
}