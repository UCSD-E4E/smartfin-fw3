#include "deploy.hpp"

#include "Particle.h"
#include "SpiffsParticleRK.h"
#include "system.hpp"

Deployment& Deployment::getInstance(void)
{
    static Deployment DP_instance;
    return DP_instance;
}

/**
 * @brief Opens a new deployment with the given name in the specified state
 * 
 * @param name Session Name
 * @param state Read/Write State
 * @return int  1 if successful, otherwise 0
 */
int Deployment::open(const char* const name, Deployment::State_e state)
{
    if(this->currentFile.isValid())
    {
        pSystemDesc->pFileSystem->close(this->currentFile);
    }
    switch(state)
    {
        case Deployment::READ:
            this->currentFile = pSystemDesc->pFileSystem->openFile(name, SPIFFS_O_RDONLY);
            break;
        case Deployment::WRITE:
            this->currentFile = pSystemDesc->pFileSystem->openFile(name, SPIFFS_O_WRONLY | SPIFFS_O_CREAT);
            break;
        case Deployment::RDWR:
            this->currentFile = pSystemDesc->pFileSystem->openFile(name, SPIFFS_O_RDWR);
            break;
        default:
            return 0;
    }
    if(!this->currentFile.isValid())
    {
        return 0;
    }
    else
    {
        this->currentState = state;
        return 1;
    }
}

int Deployment::write(void* pData, size_t nBytes)
{
    size_t bytesWritten = 0;
    if(!this->currentFile.isValid() || currentState != Deployment::WRITE)
    {
        return 0;
    }
    bytesWritten = this->currentFile.write((uint8_t*) pData, nBytes);
    this->currentFile.flush();
    return bytesWritten;
}

int Deployment::read(void* pData, size_t nBytes)
{
    size_t bytesRead = 0;
    
    if(!this->currentFile.isValid())
    {
        return 0;
    }
    if(this->currentState != Deployment::READ && this->currentState != Deployment::RDWR)
    {
        return 0;
    }
    bytesRead = this->currentFile.readBytes((char*) pData, nBytes);
    return bytesRead;
}

/**
 * @brief Closes the session.
 * 
 * @return int 1 if successful, otherwise 0
 */
int Deployment::close(void)
{
    if(!this->currentFile.isValid())
    {
        return 1;
    }
    this->currentFile.flush();
    this->currentFile.close();
    return 1;
}

int Deployment::seek(size_t loc)
{
    if(!this->currentFile.isValid())
    {
        return 0;
    }
    this->currentFile.lseek(loc, SPIFFS_SEEK_SET);
    return 1;
}
size_t Deployment::getLength(void)
{
    if(!this->currentFile.isValid())
    {
        SF_OSAL_printf("DEP::getLength: invalid file!\n");
        return 0;
    }
    return this->currentFile.length();
}

int Deployment::remove(void)
{
    if(!this->currentFile.isValid())
    {
        return 0;
    }
    if(!this->currentFile.remove())
    {
        return 0;
    }
    this->currentFile.flush();
    this->currentFile.close();
    return 1;
}

int Deployment::truncate(size_t nBytes)
{
    if(!this->currentFile.isValid())
    {
        return 0;
    }
    // Don't need to check truncation length because SpiFFS handles that for us
    if(!currentFile.truncate(nBytes))
    {
        return 0;
    }
    this->currentFile.flush();
    return 1;
}