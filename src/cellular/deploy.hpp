#ifndef __DEPLOY_HPP__
#define __DEPLOY_HPP__

#include "product.hpp"

#include <stddef.h>
#include <limits.h>
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
    /**
     * @brief Opens a new deployment with the given name in the specified state
     *
     * @param name Session Name
     * @param state Read/Write State
     * @return int  1 if successful, otherwise 0
     */
    int open(const char* const name, State_e state);
    /**
     * @brief Writes to current deployment file
     *
     * @param pData data to write
     * @param nBytes number of bytes to write
     *
     * @return number of bytes written
    */
    int write(void* pData, size_t nBytes);
    /**
     * @brief Reads from current deployment file
     *
     * @param pData data to read
     * @param nBytes number of bytes to write
     *
     * @return bytesRead
    */
    int read(void* pData, size_t nBytes);
    /**
     * @brief Seek to location in file
     *
     * @param loc Location to seek to
     *
     * @return int 1 if successful, otherwise 0
    */
    int seek(size_t loc);
    /**
     * @brief Gets length of the file
     *
     * @return length
    */
    int getLength(void);
    /**
     * @brief Closes the session
     *
     * @return int 1 if successful, otherwise 0
    */
    int close(void);
    /**
     * @brief Removes the current file.
     * 
     * Once this method returns, the Deployment object may immediately open a
     * new session
     *
     * @return int 1 if successful, otherwise 0
    */
    int remove(void);
    /**
     * @brief Truncates current file.
     *
     * @param size_t Number of bytes to truncate to
     *
     * @return int 1 if successful, otherwise 0
    */
    int truncate(size_t nBytes);

private:
    Deployment() {}
    Deployment(Deployment const&);
    int currentFile;
    State_e currentState;
    char filename[SF_NAME_MAX];
};

#endif