/**
 * @file utils.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Uno Q (QRB2210 Linux) implementation of miscellaneous SF_HAL
 *        utility functions.
 * @date 2026-07-23
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

#include "platform/hal.hpp"

namespace SF_HAL
{

int random(int min, int max)
{
    // TODO(unoq): return a pseudo-random int in [min, max].
    return min;
}

std::vector<std::string> board_version()
{
    // TODO(unoq): return board/firmware version components; scheme TBD.
    return {};
}

} // namespace SF_HAL

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
