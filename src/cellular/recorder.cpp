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
#include "util.hpp"

#include <dirent.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>


#define REC_DEBUG

#define DATA_ROOT "/data"
#define METADATA_FILE DATA_ROOT "/.metadata"
static int REC_create_data_root(void);
/**
 * @brief Builds the data filename
 * 
 * @param session_idx Session index
 * @param p_filename_buf Pointer to filename buffer
 * @param filename_buf_len Length of filename buffer
 */
static inline void REC_build_data_filename(uint32_t session_idx,
                                    char* p_filename_buf,
                                    size_t filename_buf_len)
{
    snprintf(p_filename_buf, filename_buf_len, DATA_ROOT "/%010" PRIu32 ".bin", session_idx);
}

int Recorder::create_metadata_file(void)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
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
            if (sizeof(metadata_header_t) != ::read(fp,
                                                    &this->metadata_header,
                                                    sizeof(metadata_header_t)))
            {
                FLOG_AddError(FLOG_FS_READ_FAIL, errno);
                ::close(fp);
                return 0;
            }
            if (0 != ::close(fp))
            {
                FLOG_AddError(FLOG_FS_CLOSE_FAIL, errno);
                return 0;
            }
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
    if (sizeof(metadata_header_t) != ::write(fp,
                                             &this->metadata_header,
                                             sizeof(metadata_header_t)))
    {
        FLOG_AddError(FLOG_FS_WRITE_FAIL, errno);
        ::close(fp);
        return 0;
    }
    if (0 != ::close(fp))
    {
        FLOG_AddError(FLOG_FS_CLOSE_FAIL, errno);
        return 0;
    }
    this->metadata_header_valid = true;
#endif
    return 1;
}

int REC_create_data_root(void)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
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
#endif
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

/**
 * @brief Opens the last Session with data and stores the name
 * 
 * @param session Session object to populate
 * @param p_name_buf Session Name buffer, must be at least NAME_MAX long
 * @return 0 on success, otherwise error code
 */
int Recorder::openLastSession(Deployment& session, char* p_name_buf)
{
    timestamp_entry_t entry;
    int entry_length;

    /* If we don't have valid metadata header structure, then we are not
     * initialized
     */
    if (!this->metadata_header_valid)
    {
        FLOG_AddError(FLOG_REC_OPEN_LAST_SESSION_FAIL, 1);
        return 1;
    }
    /*
    If we don't have data, no session to populate.  This check should be run
    by the calling code as well!
    */
    if (!hasData())
    {
        FLOG_AddError(FLOG_REC_OPEN_LAST_SESSION_FAIL, 2);
        return 2;
    }

    while (1)
    {
        /*
        Get the last table entry.
        */
        peek_last_metadata_entry(entry);

        /*
        Get the corresponding file path from the table entry
        */
        REC_build_data_filename(entry.session_idx,
                                this->filename_buffer,
                                REC_SESSION_NAME_MAX_LEN);

        /*
        Load into Deployment object
        */
        if (!session.open(this->filename_buffer, Deployment::RDWR))
        {
            SF_OSAL_printf("REC::OPEN Fail to open %s" __NL__, this->filename_buffer);
            FLOG_AddError(FLOG_REC_OPEN_LAST_SESSION_FAIL, 3);
            // Instead of returning an error code, skip this file
            // return 3;
            if (pop_metadata_entry())
            {
                return 3;
            }
            continue;
        }

        /*
        If the session is empty, delete it
        */
        entry_length = session.getLength();
        if (0 == entry_length)
        {
            if (1 != session.remove())
            {
                SF_OSAL_printf("REC::OPEN Failed to remove empty session!" __NL__);
                FLOG_AddError(FLOG_REC_OPEN_LAST_SESSION_FAIL, 4);
                return 4;
            }
            /*
            Removes the last entry from the metadata file and update the metadata header
            */

            continue;
        }

        /*
        We now have the last session, build name
        */
        if (0 != entry.timestamp)
        {
            String fmt_time = Time.format(entry.timestamp, "%y%m%d-%H%M%S");
            snprintf(p_name_buf, NAME_MAX, "%s", fmt_time.c_str());
        }
        else
        {
            snprintf(
                p_name_buf, NAME_MAX, "%08" PRIu32, entry.session_idx);
        }
        SF_OSAL_printf("Set name to %s" __NL__, p_name_buf);
        return 0;
    }
    FLOG_AddError(FLOG_REC_OPEN_LAST_SESSION_FAIL, 5);
    return 5;
}


int Recorder::getLastPacket(void* pBuffer,
                            size_t bufferLen,
                            char* pName,
                            size_t nameLen)
{
    Deployment& session = Deployment::getInstance();
    int newLength;
    int bytesRead;
    char name[NAME_MAX];
    int current_length;
    size_t bytes_to_read;

    if (nullptr != this->pSession)
    {
        // Already have something open for writing!
        #ifdef REC_DEBUG
        SF_OSAL_printf("Already open for writing!" __NL__);
        #endif
        return -1;
    }

    if (this->openLastSession(session, name))
    {
        // memset(this->currentSessionName, 0, REC_SESSION_NAME_MAX_LEN + 1);
        #ifdef REC_DEBUG
        SF_OSAL_printf("Failed to open last session for get" __NL__);
#endif
        return -2;
    }

    current_length = session.getLength();
    if (current_length % SF_PACKET_SIZE == 0)
    {
        // only full packets available
        bytes_to_read = SF_PACKET_SIZE;
    }
    else
    {
        // partial packet
        bytes_to_read = current_length % SF_PACKET_SIZE;
    }

    #ifdef REC_DEBUG
    SF_OSAL_printf("Reading %d bytes" __NL__, bytes_to_read);
    #endif

    if (bytes_to_read > bufferLen)
    {
        #ifdef REC_DEBUG
        SF_OSAL_printf("Buffer overflow!" __NL__);
        #endif
        return -3;
    }

    newLength = current_length - bytes_to_read;
    session.seek(newLength);
    bytesRead = session.read(pBuffer, bytes_to_read);
    snprintf((char*)pName, nameLen, "Sfin-%s-%s-%d", pSystemDesc->deviceID,
             name, newLength / SF_PACKET_SIZE);
    session.close();
    return bytesRead;
}

/**
 * @brief Trims the last block with specified length from the recorder
 *
 * @param len Length of block to trim
 * @return int 0 if successful, otherwise error code.  -1 if another session is
 * already open.  -2 if previous session unopenable.
 */
int Recorder::popLastPacket(size_t len)
{
    Deployment& session = Deployment::getInstance();
    int newLength;
    char name[NAME_MAX];
    int current_length;
    size_t bytes_to_pop;

    if (nullptr != this->pSession)
    {
        // Already have something open for writing!
        #ifdef REC_DEBUG
        SF_OSAL_printf("Already open for writing!" __NL__);
        #endif
        return -1;
    }

    if (this->openLastSession(session, name))
    {
        // memset(this->currentSessionName, 0, REC_SESSION_NAME_MAX_LEN + 1);
        #ifdef REC_DEBUG
        SF_OSAL_printf("Failed to open last session for pop" __NL__);
#endif
        return -2;
    }

    current_length = session.getLength();
    if (current_length % SF_PACKET_SIZE == 0)
    {
        // only full packets available
        bytes_to_pop = SF_PACKET_SIZE;
    }
    else
    {
        // partial packet
        bytes_to_pop = current_length % SF_PACKET_SIZE;
    }

    newLength = current_length - bytes_to_pop;
    if (0 == newLength)
    {
        session.remove();
        pop_metadata_entry();
    }
    else
    {
        session.truncate(newLength);
        session.close();
    }
    return 0;
}

int Recorder::setSessionTime(uint32_t session_time)
{
    if (nullptr == this->pSession)
    {
        return 1;
    }
    if (this->time_set)
    {
        return 0;
    }

    this->time_set = true;    
    if (0 != set_metadata_entry_time(this->current_session_index, session_time))
    {
        return 3;
    }
    return 0;
}

int Recorder::openSession()
{
    if (!this->metadata_header_valid)
    {
        FLOG_AddError(FLOG_REC_INVALID_METADATA, 0);
        return 0;
    }
    if (nullptr != this->pSession)
    {
        SF_OSAL_printf("Double open!" __NL__);
        FLOG_AddError(FLOG_REC_DOUBLE_OPEN, 0);
        return 0;
    }

    this->current_session_index = this->metadata_header.next_session_index;
    push_metadata_entry(this->current_session_index);

    REC_build_data_filename(this->current_session_index,
                             this->filename_buffer,
                             REC_SESSION_NAME_MAX_LEN);

    this->pSession = &Deployment::getInstance();
    if (!this->pSession->open(this->filename_buffer, Deployment::WRITE))
    {
        this->pSession = nullptr;
        SF_OSAL_printf("REC::OPEN Fail to open" __NL__);
        return 0;
    }
    memset(this->dataBuffer, 0, REC_MEMORY_BUFFER_SIZE);
    this->dataIdx = 0;
    this->time_set = false;
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

    // flush buffer
    int x = this->pSession->write(this->dataBuffer, this->dataIdx);
    SF_OSAL_printf("Bytes written %d", x);

    // Close session
    this->pSession->close();
    this->pSession = nullptr;
    this->time_set = false;

    memset(this->dataBuffer, 0, REC_MEMORY_BUFFER_SIZE);
    this->dataIdx = 0;

    return 1;
}

int Recorder::putBytes(const void* pData, size_t nBytes)
{
    if (nullptr == this->pSession)
    {
        FLOG_AddError(FLOG_REC_SESSION_CLOSED, 0);
        return 1;
    }
    if (nBytes > (SF_PACKET_SIZE - this->dataIdx))
    {
        // data will not fit, flush and clear
        // pad 0
        for (; this->dataIdx < SF_PACKET_SIZE; this->dataIdx++)
        {
            this->dataBuffer[this->dataIdx] = 0;
        }
        // SF_OSAL_printf("Flushing\n");
        this->pSession->write(this->dataBuffer, SF_PACKET_SIZE);

        memset(this->dataBuffer, 0, REC_MEMORY_BUFFER_SIZE);
        this->dataIdx = 0;
    }

    // data guaranteed to fit
    // SF_OSAL_printf("Putting %u bytes\n", nBytes);
    memcpy(&this->dataBuffer[this->dataIdx], pData, nBytes);
    this->dataIdx += nBytes;
    return 0;
}

/**
 * @brief Pops a metadata entry
 *
 * @return int 1 on failure, otherwise 0
 */
int Recorder::pop_metadata_entry(void)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    int fp;
    off_t final_length;

    this->metadata_header.n_entries -= 1;
    fp = ::open(METADATA_FILE, O_RDWR);
    if (-1 == fp)
    {
        FLOG_AddError(FLOG_FS_OPEN_FAIL, errno);
        return 1;
    }
    if (sizeof(metadata_header_t) != ::write(fp,
                                             &this->metadata_header,
                                             sizeof(metadata_header_t)))
    {
        FLOG_AddError(FLOG_FS_WRITE_FAIL, errno);
        ::close(fp);
        return 1;
    }

    final_length = sizeof(metadata_header_t) + 
        this->metadata_header.n_entries * sizeof(timestamp_entry_t);
    if (0 != ::ftruncate(fp, final_length))
    {
        FLOG_AddError(FLOG_FS_FTRUNC_FAIL, errno);
        ::close(fp);
        return 1;
    }
    
    if (0 != ::close(fp))
    {
        FLOG_AddError(FLOG_FS_CLOSE_FAIL, errno);
        return 1;
    }
#endif
    return 0;
}

int Recorder::push_metadata_entry(uint32_t session_idx)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    int fp;
    timestamp_entry_t entry;

    this->metadata_header.next_session_index++;
    this->metadata_header.n_entries++;

    entry.session_idx = session_idx;
    entry.timestamp = 0;
    fp = ::open(METADATA_FILE, O_WRONLY);
    if (-1 == fp)
    {
        FLOG_AddError(FLOG_FS_OPEN_FAIL, errno);
        return 1;
    }
    if (sizeof(metadata_header_t) != ::write(fp,
                                             &this->metadata_header,
                                             sizeof(metadata_header_t)))
    {
        FLOG_AddError(FLOG_FS_WRITE_FAIL, errno);
        ::close(fp);
        return 1;
    }
    ::lseek(fp, 0, SEEK_END);
    if (sizeof(timestamp_entry_t) != ::write(fp,
                                             &entry,
                                             sizeof(timestamp_entry_t)))
    {
        FLOG_AddError(FLOG_FS_WRITE_FAIL, errno);
        ::close(fp);
        return 1;
    }
    if (0 != ::close(fp))
    {
        FLOG_AddError(FLOG_FS_CLOSE_FAIL, errno);
        return 1;
    }
#endif
    return 0;

}
int Recorder::set_metadata_entry_time(uint32_t session_idx, uint32_t timestamp)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    int fp;
    timestamp_entry_t entry;
    int bytes_read;

    fp = ::open(METADATA_FILE, O_RDWR);
    ::lseek(fp, sizeof(metadata_header_t), SEEK_SET);
    while (1)
    {
        bytes_read = ::read(fp, &entry, sizeof(timestamp_entry_t));
        if (0 == bytes_read)
        {
            ::close(fp);
            return 1;
        }
        if (-1 == bytes_read)
        {
            FLOG_AddError(FLOG_FS_READ_FAIL, errno);
            ::close(fp);
            return 2;
        }
        if (sizeof(timestamp_entry_t) != bytes_read)
        {
            FLOG_AddError(FLOG_REC_METADATA_BAD, 0);
            ::close(fp);
            return 3;
        }
        if (entry.session_idx != session_idx)
        {
            continue;
        }
        entry.timestamp = timestamp;
        ::lseek(fp, -sizeof(timestamp_entry_t), SEEK_CUR);
        if (sizeof(timestamp_entry_t) != ::write(fp,
                                                 &entry,
                                                 sizeof(timestamp_entry_t)))
        {
            FLOG_AddError(FLOG_FS_WRITE_FAIL, errno);
            ::close(fp);
            return 4;
        }
        break;
    }
    if (0 != ::close(fp))
    {
        FLOG_AddError(FLOG_FS_CLOSE_FAIL, errno);
        return 5;
    }
#endif
    return 0;
}

int Recorder::peek_last_metadata_entry(timestamp_entry_t& entry)
{
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
    int fp;
    off_t entry_pos;
    entry_pos = sizeof(metadata_header_t) + 
                (this->metadata_header.n_entries - 1) * sizeof(timestamp_entry_t);
    fp = ::open(METADATA_FILE, O_RDONLY);
    if (-1 == fp)
    {
        FLOG_AddError(FLOG_FS_OPEN_FAIL, errno);
        return 1;
    }
    ::lseek(fp, entry_pos, SEEK_SET);
    if (sizeof(timestamp_entry_t) != ::read(fp,
                                            &entry,
                                            sizeof(timestamp_entry_t)))
    {
        FLOG_AddError(FLOG_FS_READ_FAIL, errno);
        ::close(fp);
        return 2;
    }
    if (0 != ::close(fp))
    {
        FLOG_AddError(FLOG_FS_CLOSE_FAIL, errno);
        return 3;
    }
#endif
    return 0;
}

char path_buf[PATH_MAX + 1];
/**
 * @brief Recursively removes the entries inside this directory
 *
 * @param dir_to_remove Directory to remove the contents of
 * @param prefix Unix path (including path separator) representing the directory.
 * This buffer must have enough space to store all possible path names.
 * @return int 0 on success, otherwise 1
 */
int rmtree(DIR *const dir_to_remove, char *const prefix)
{
    const struct dirent *dir_entry;
    int retval;
    char *const next_path_element = prefix + strlen(prefix);
    errno = 0;
    for (dir_entry = readdir(dir_to_remove); dir_entry; dir_entry = readdir(dir_to_remove))
    {
        if (dir_entry->d_type & DT_REG)
        {
            // This is a regular file.  Make full path, then unlink
            strncpy(next_path_element, dir_entry->d_name, NAME_MAX);
            errno = 0;
            retval = unlink(prefix);
            *next_path_element = 0;
            if (-1 == retval)
            {
                FLOG_AddError(FLOG_REC_RMTREE_RM_FILE, errno);
                return 1;
            }
        }
        else if (dir_entry->d_type & DT_DIR)
        {
            if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0)
            {
                // this is the current/upper directory, continue
                errno = 0;
                continue;
            }
            // This is a regular directory.  Make full path, then rmtree it
            strncpy(next_path_element, dir_entry->d_name, NAME_MAX);
            strcat(next_path_element, PATH_SEP);
            errno = 0;
            DIR *next_dir = opendir(prefix);
            if (!next_dir)
            {
                FLOG_AddError(FLOG_REC_RMTREE_OPENDIR, errno);
                return 1;
            }
            SF_OSAL_printf("rmtree %s", prefix);
            retval = rmtree(next_dir, prefix);
            closedir(next_dir);
            if (retval)
            {
                *next_path_element = 0;
                return 1;
            }
            errno = 0;
            retval = rmdir(prefix);
            *next_path_element = 0;
            if (retval)
            {
                FLOG_AddError(FLOG_REC_RMTREE_RMDIR, errno);
                return 1;
            }
        }
        errno = 0;
    }
    if (errno)
    {
        FLOG_AddError(FLOG_REC_RMTREE_READDIR, errno);
        return 1;
    }
    return 0;
}

int Recorder::reformat(void)
{
    int retval;
    // Check that nothing is open
    if (this->pSession)
    {
        return 1;
    }

    // stat DATA_ROOT
    struct stat data_root_stat;
    if (0 == stat(DATA_ROOT, &data_root_stat))
    {
        // This path exists!  is it a file or a dir?
        if (data_root_stat.st_mode & S_IFREG)
        {
            // This is a file!
            // Remove it
            if (-1 == unlink(DATA_ROOT))
            {
                // failed to remove
                FLOG_AddError(FLOG_REC_FORMAT_RM_FILE, errno);
                return 1;
            }
            // File doesn't exist anymore, we can move on
        }
        else if (data_root_stat.st_mode & S_IFDIR)
        {
            // This is a folder.  We need to recursively search and remove it.
            memset(path_buf, 0, NAME_MAX + 1);
            strncpy(path_buf, DATA_ROOT, PATH_MAX);
            strcat(path_buf, PATH_SEP);
            DIR *data_dir = opendir(DATA_ROOT);
            retval = rmtree(data_dir, path_buf);
            closedir(data_dir);
            if (retval)
            {
                FLOG_AddError(FLOG_REC_FORMAT_RMTREE, 0);
                return 1;
            }
        }
    }

    // At this point, `/data` should not exist.
    if (!this->init())
    {
        // this logs its own FLOG, no need to do so again
        return 1;
    }
    return 0;
}