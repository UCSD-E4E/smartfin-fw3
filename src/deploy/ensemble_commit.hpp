/**
 * @file ensemble_commit.hpp
 * @brief Commit layer that writes ensembles to both storage and BLE.
 * @author Charlie Kushelevsky (ckushelevsky@ucsd.edu)
 * @date 2026-03-10
 */
#ifndef __ENSEMBLE_COMMIT_HPP__
#define __ENSEMBLE_COMMIT_HPP__

#include <cstdint>
#include <cstddef>


namespace sf
{
namespace deploy
{
    /**
     * @brief Different return codes for commit ensemble.
     *
     */
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
     * @brief Enqueue an ensemble blob to enabled sinks (recorder/stream).
     * @param pData Pointer to ensemble bytes.
     * @param len Size of the ensemble in bytes.
     * @return COMMIT_OK on success, otherwise a CommitResult failure code
     */
    int commitEnsemble(const void *pData, std::size_t len);
}
}

#endif // __ENSEMBLE_COMMIT_HPP__
