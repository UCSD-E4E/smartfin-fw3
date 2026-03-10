/**
 * @file ensemble_commit.hpp
 * @brief Commit layer that writes ensembles to both storage and BLE.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */
#ifndef __ENSEMBLE_COMMIT_HPP__
#define __ENSEMBLE_COMMIT_HPP__


#include <cstdint>


namespace sf
{
namespace deploy
{

    enum CommitResult
    {
        COMMIT_OK = 0,
        COMMIT_INVALID_ARG,
        COMMIT_RECORD_FAIL,
        COMMIT_STREAM_FAIL,
        COMMIT_BOTH_FAIL,
        COMMIT_NO_SINKS_ENABLED,
    };
    /**
     * @brief Write and optionally stream an ensemble blob.
     * @param pData Pointer to ensemble bytes.
     * @param len Size of the ensemble in bytes.
     * @return 0 if succesful, flog error if not
     */
    int commitEnsemble(const void *pData, std::size_t len);
}
}

#endif // __ENSEMBLE_COMMIT_HPP__
