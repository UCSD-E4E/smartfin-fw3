/**
 * @file nvm.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of SF_HAL non-volatile memory
 *        functions.
 * @date 2026-07-23
 *
 * Backed by a plain file on the Linux side, not the STM32U585: the MCU is a
 * dumb GPIO/I2C peripheral server with no NVM RPC verb, and NVM contents
 * must survive power loss, so reads/writes go through a byte-addressable
 * file that is fsync'd after every write.
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

#include "cli/flog.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace SF_HAL
{

#define NVM_FILE_PATH "/data/.nvm"

/**
 * @brief Open (creating if necessary) the NVM backing file.
 *
 * @param flags POSIX open() flags; O_CREAT is always added.
 * @return Open file descriptor, or -1 on failure (an FLOG error is logged).
 */
static int nvm_open(int flags)
{
    int fd = ::open(NVM_FILE_PATH, flags | O_CREAT, 0644);
    if (-1 == fd)
    {
        FLOG_AddError(FLOG_FS_OPEN_FAIL, errno);
    }
    return fd;
}

void nvm_read(uint32_t addr, void* out, std::size_t len)
{
    std::memset(out, 0, len);

    int fd = nvm_open(O_RDONLY);
    if (-1 == fd)
    {
        return;
    }

    ssize_t n = ::pread(fd, out, len, static_cast<off_t>(addr));
    if (-1 == n)
    {
        FLOG_AddError(FLOG_FS_READ_FAIL, errno);
    }

    if (0 != ::close(fd))
    {
        FLOG_AddError(FLOG_FS_CLOSE_FAIL, errno);
    }
}

void nvm_write(uint32_t addr, const void* in, std::size_t len)
{
    int fd = nvm_open(O_WRONLY);
    if (-1 == fd)
    {
        return;
    }

    ssize_t n = ::pwrite(fd, in, len, static_cast<off_t>(addr));
    if (n != static_cast<ssize_t>(len))
    {
        FLOG_AddError(FLOG_FS_WRITE_FAIL, errno);
    }
    else if (0 != ::fsync(fd))
    {
        FLOG_AddError(FLOG_FS_WRITE_FAIL, errno);
    }

    if (0 != ::close(fd))
    {
        FLOG_AddError(FLOG_FS_CLOSE_FAIL, errno);
    }
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
