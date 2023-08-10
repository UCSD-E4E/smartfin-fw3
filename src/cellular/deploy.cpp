#include "deploy.hpp"

#include "Particle.h"
#include <fcntl.h>
#include "system.hpp"

Deployment &Deployment::getInstance(void)
{
    static Deployment DP_instance;
    return DP_instance;
}

const char *filename;

int Deployment::open(const char *const name, Deployment::State_e state)
{
    if (this->currentFile == 0)
    {
        ::close(this->currentFile);
    }
    SF_OSAL_printf("opening!");

    filename = name;
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
    if (currentFile == 0)
    {
        return 0;
    }
    else
    {
        this->currentState = state;
        return 1;
    }
}

int Deployment::write(void *pData, size_t nBytes)
{
    size_t bytesWritten = 0;
    if (currentFile == 0 || currentState != Deployment::WRITE)
    {
        return 0;
    }
    bytesWritten = ::write(currentFile, (uint8_t *)pData, nBytes);
    return bytesWritten;
}

int Deployment::read(void *pData, size_t nBytes)
{
    size_t bytesRead = 0;

    if (currentFile == 0)
    {
        return 0;
    }
    if (this->currentState != Deployment::READ && this->currentState != Deployment::RDWR)
    {
        return 0;
    }
    bytesRead = ::read(this->currentFile, (char *)pData, nBytes);
    return bytesRead;
}

int Deployment::close(void)
{
    if (currentFile == 0)
    {
        return 1;
    }
    ::close(this->currentFile);
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
        SF_OSAL_printf("DEP::getLength: invalid file!\n");
        return 0;
    }

    struct stat *sbuf = {};
    fstat(currentFile, sbuf);

    SF_OSAL_printf("File length: %ld \n", sbuf->st_size);

    return (int)sbuf->st_size;
}

int Deployment::remove(void)
{
    if (this->currentFile == 0)
    {
        return 0;
    }
    if (::ftruncate(this->currentFile, 0) == 0)
    {
        return 0;
    }
    ::close(this->currentFile);
    return 1;
}

int Deployment::truncate(size_t nBytes)
{
    if (this->currentFile == 0)
    {
        return 0;
    }
    // Don't need to check truncation length because SpiFFS handles that for us
    if (ftruncate(this->currentFile, 0) == 0)
    {
        return 0;
    }
    return 1;
}