/**
 * @file utils.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief
 * @date 2026-05-17
 *
 *
 */

#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_PARTICLE
#include "Particle.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace SF_HAL
{
    int random(int min, int max)
    {
        return ::random(min, max);
    }

std::vector<std::string> board_version()
{
    std::vector<std::string> prints;
    char buf[80];

    snprintf(buf, sizeof(buf), "Device OS: %s", System.version().c_str());
    prints.emplace_back(buf);

    snprintf(buf, sizeof(buf), "CRC32=%lx", module_info_crc.crc32);
    prints.emplace_back(buf);

    // "SHA256=" (7) + 32 bytes * 2 hex chars (64) + null = 72
    char sha_str[72];
    snprintf(sha_str, sizeof(sha_str), "SHA256=");
    const uint8_t* sha = module_info_suffix.sha;
    for (size_t i = 0; i < 32; i++)
    {
        snprintf(sha_str + 7 + i * 2, 3, "%02x", sha[i]);
    }
    prints.emplace_back(sha_str);

    return prints;
}
} // namespace SF_HAL

#endif
