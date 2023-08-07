#ifndef __DEPLOY_HPP__
#define __DEPLOY_HPP__

#include <cstddef>
#include "SpiffsParticleRK.h"

class Deployment
{
    public:
    typedef enum State_
    {
        READ,
        WRITE,
        RDWR,
    }State_e;

    static Deployment& getInstance(void);

    int open(const char* const name, State_e state);
    int write(void* pData, size_t nBytes);
    int read(void* pData, size_t nBytes);
    int seek(size_t loc);
    size_t getLength(void);
    int close(void);
    int remove(void);
    int truncate(size_t nBytes);

    private:
    Deployment(){}
    Deployment(Deployment const&);
    void operator=(Deployment const&);
    SpiffsParticleFile currentFile;
    State_e currentState;
};

#endif