/**
 * @file nvm.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL non-volatile memory
 *        functions.
 * @date 2026-07-23
 *
 * Backed by a file, not the STM32U585: NVM contents must survive power
 * loss, so the eventual implementation needs an fsync'd file (or a small
 * block device), not a tmpfs/RAM-backed path.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

namespace SF_HAL
{

void nvm_read(uint32_t addr, void* out, std::size_t len)
{
    // TODO(unoq): read len bytes at addr from the backing file into out.
}

void nvm_write(uint32_t addr, const void* in, std::size_t len)
{
    // TODO(unoq): write len bytes from in to addr in the backing file and
    // fsync() so the write survives power loss.
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
