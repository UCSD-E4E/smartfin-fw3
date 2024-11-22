#ifndef __DEPLOY_HPP__
#define __DEPLOY_HPP__

#include "product.hpp"

#include <limits.h>
#include <stddef.h>

/**
 * @brief Handles file-based deployment operations such as read,
 * write, and management.
 */
class Deployment
{
public:
    /**
     * @brief Enum representing file states for deployment operations.
     */
    typedef enum State_
    {
        READ,  /**< Read-only mode. */
        WRITE, /**< Write-only mode. */
        RDWR   /**< Read-write mode. */
    } State_e;

    /**
     * @brief Provides a singleton instance of the Deployment class.
     *
     * @return A reference to the Deployment instance.
     */
    static Deployment& getInstance(void);

    /**
     * @brief Opens a new deployment with the given name in the specified state.
     *
     * @param name Name of the session to open.
     * @param state File access state (READ, WRITE, RDWR).
     * @return int 1 if successful, otherwise 0.
     */
    int open(const char* const name, State_e state);

    /**
     * @brief Writes to the current deployment file.
     *
     * @param pData Data to write.
     * @param nBytes Number of bytes to write.
     * @return int Number of bytes written.
     */
    int write(void* pData, size_t nBytes);

    /**
     * @brief Reads from the current deployment file.
     *
     * @param pData Buffer to store the read data.
     * @param nBytes Number of bytes to read.
     * @return int Number of bytes read.
     */
    int read(void* pData, size_t nBytes);

    /**
     * @brief Seek to a specific location in the file.
     *
     * @param loc Location to seek to.
     * @return int 1 if successful, otherwise 0.
     */
    int seek(size_t loc);

    /**
     * @brief Gets the length of the file.
     *
     * @return int Length of the file in bytes.
     */
    int getLength(void);

    /**
     * @brief Closes the current session.
     *
     * @return int 1 if successful, otherwise 0.
     */
    int close(void);

    /**
     * @brief Removes the current file.
     *
     * @return int 1 if successful, otherwise 0.
     */
    int remove(void);

    /**
     * @brief Truncates the current file.
     *
     * @param nBytes Number of bytes to truncate the file to.
     * @return int 1 if successful, otherwise 0.
     */
    int truncate(size_t nBytes);

private:
    /**
     * @brief Constructs a Deployment object.
     *
     * Private to ensure singleton behavior.
     */
    Deployment() {}

    /**
     * @brief Copy constructor
     */
    Deployment(Deployment const&);

    /**
     * @brief File descriptor for the currently opened file.
     */
    int currentFile;

    /**
     * @brief State of the current file session.
     */
    State_e currentState;

    /**
     * @brief Name of the file currently being handled.
     */
    char filename[SF_NAME_MAX];
};

#endif
