/**
 * @file main.cpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Entry point for the native (non-cross-compiled) Uno Q Linux build.
 * @date 2026-07-23
 *
 * Builds the same application entry point (smartfin-fw3.ino) used by the
 * Particle and pc_hal targets, linked against the src/platform/unoq/
 * backend instead of Particle's or pc_hal's. Compiles with the host's
 * native compiler for IDE tooling and compile-sanity checking; this is
 * not yet the real cross-compiled QRB2210 artifact (toolchain/sysroot
 * still TBD).
 */
#include "platform/platform.hpp"

#if SF_PLATFORM == SF_PLATFORM_UNOQ

/**
 * @brief No-op stand-in for Particle's SYSTEM_MODE() macro.
 *
 * smartfin-fw3.ino calls this unconditionally; cloud connection mode is
 * managed by SF_HAL::cloud_* on this platform instead.
 */
#define SYSTEM_MODE(a) ;

/**
 * @brief No-op stand-in for Particle's SYSTEM_THREAD() macro.
 */
#define SYSTEM_THREAD(a) ;

void setup();
void loop();

#include "smartfin-fw3.ino"

/**
 * @brief Native entry point.
 *
 * @return Never returns.
 */
int main()
{
    setup();
    while (true)
    {
        loop();
    }
}

#endif // SF_PLATFORM == SF_PLATFORM_UNOQ
