/**
 * @file deploy.cpp
 * @author Nathan Hui (nthui@ucsd.edu)
 * @brief Deployment Module
 * @version 0.1
 * @date 2023-09-26
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "deploy.hpp"

#include "consts.hpp"
#include "system.hpp"

#include <fcntl.h>
#include <string.h>

#include "Particle.h"

#define DEP_DEBUG

Deployment& Deployment::getInstance(void)
{
    static Deployment DP_instance;
    return DP_instance;
}


int Deployment::open(const char* const name, Deployment::State_e state)
{
    if (this->currentFile == 0)
    {
        ::close(this->currentFile);
    }
#ifdef DEP_DEBUG
    SF_OSAL_printf("opening!");
#endif
    switch (state)
    {
    case Deployment::READ:
        this->currentFile = ::open(name, O_RDONLY);
        break;
    case Deployment::WRITE:
        this->currentFile = ::open(name, O_WRONLY | O_CREAT);
        break;
    case Deployment::RDWR:
        this->currentFile = ::open(name, O_RDWR);
        break;
    default:
        return 0;
    }
    if (this->currentFile == -1)
    {
        return 0;
    }
    strncpy(this->filename, name, SF_NAME_MAX);
    this->currentState = state;
    return 1;
}

int Deployment::write(void* pData, size_t nBytes)
{
    size_t bytesWritten = 0;
    if (currentFile == 0 || currentState != Deployment::WRITE)
    {
        return 0;
    }
    bytesWritten = ::write(currentFile, (uint8_t*)pData, nBytes);
    return bytesWritten;
}

int Deployment::read(void* pData, size_t nBytes)
{
    size_t bytesRead = 0;

    if (currentFile == 0)
    {
        return 0;
    }
    if (this->currentState != Deployment::READ &&
        this->currentState != Deployment::RDWR)
    {
        return 0;
    }
    bytesRead = ::read(this->currentFile, (char*)pData, nBytes);
    return bytesRead;
}

int Deployment::close(void)
{
    if (currentFile == 0)
    {
        return 1;
    }
    if (::close(this->currentFile) != 0)
    {
        return 0;
    }
    this->currentFile = 0;
    memset(this->filename, 0, SF_NAME_MAX);
    return 1;
}

int Deployment::seek(size_t loc)
{
    if (this->currentFile == 0)
    {
        return 0;
    }
    lseek(this->currentFile, loc, SEEK_SET);
    return 1;
}

int Deployment::getLength(void)
{
    if (this->currentFile == 0)
    {
    #ifdef DEP_DEBUG
        SF_OSAL_printf("DEP::getLength: invalid file!" __NL__);
    #endif
        return 0;
    }
    struct stat sbuf = {};
    if (fstat(currentFile, &sbuf) != 0)
    {
    #ifdef DEP_DEBUG
        SF_OSAL_printf("DEP::getLength: fstat failed!" __NL__);
    #endif
    }

#ifdef DEP_DEBUG
    SF_OSAL_printf("File length: %ld" __NL__, sbuf.st_size);
#endif

    return (int)sbuf.st_size;
}

int Deployment::remove(void)
{
    if (this->currentFile == 0)
    {
    #ifdef DEP_DEBUG
        SF_OSAL_printf("DEP::remove: invalid file!" __NL__);
    #endif
        return 0;
    }
    if (::close(this->currentFile) != 0)
    {
    #ifdef DEP_DEBUG
        SF_OSAL_printf("DEP::remove: Close failed!" __NL__);
    #endif
        return 0;
    }
    if (unlink(this->filename) != 0)
    {
    #ifdef DEP_DEBUG
        SF_OSAL_printf("DEP::remove: Unlink failed!" __NL__);
    #endif
        return 0;
    }
    memset(this->filename, 0, SF_NAME_MAX);
    this->currentFile = 0;
    return 1;
}

int Deployment::truncate(size_t nBytes)
{
    if (this->currentFile == 0)
    {
    #ifdef DEP_DEBUG
        SF_OSAL_printf("DEP::truncate: invalid file!" __NL__);
    #endif
        return 0;
    }
    if (ftruncate(this->currentFile, nBytes) != 0)
    {
    #ifdef DEP_DEBUG
        SF_OSAL_printf("DEP::truncate: ftruncate failed!" __NL__);
    #endif
        return 0;
    }
    return 1;
}