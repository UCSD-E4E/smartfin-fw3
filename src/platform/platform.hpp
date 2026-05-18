/**
 * @file platform.hpp
 * @author Charlie Kushelevsky (charliekushelevsky@gmail.com)
 * @brief Platform identifier constants and SF_PLATFORM selector.
 * @date 2026-05-17
 *
 * Include this header (not product.hpp) in platform-layer files that need
 * to guard code with #if SF_PLATFORM == SF_PLATFORM_*.
 */
#ifndef __SF_PLATFORM_IDS_HPP__
#define __SF_PLATFORM_IDS_HPP__

/** @brief Particle Device OS target (nRF52840 / Boron / Tracker). */
#define SF_PLATFORM_PARTICLE    1

/** @brief Desktop GCC / glibc target (PC simulation). */
#define SF_PLATFORM_GLIBC       2

/** @brief GoogleTest host target. */
#define SF_PLATFORM_GOOGLETEST  3

/**
 * @brief Active platform selector.
 *
 * Defined by the build system via -DSF_PLATFORM=... where possible.
 * Falls back to detection via the PARTICLE macro (always defined by the
 * Particle toolchain) so that files including only this header still get
 * the correct value.
 */
#ifndef SF_PLATFORM
#ifdef PARTICLE
#define SF_PLATFORM SF_PLATFORM_PARTICLE
#else
#define SF_PLATFORM SF_PLATFORM_GLIBC
#endif
#endif

namespace SF_HAL
{

/**
 * @brief Human-readable name of the active platform.
 *
 * Use in log/debug output instead of hardcoding platform names.
 */
#if SF_PLATFORM == SF_PLATFORM_PARTICLE
constexpr const char* PLATFORM = "Particle";
#elif SF_PLATFORM == SF_PLATFORM_GLIBC
constexpr const char* PLATFORM = "GLibC";
#elif SF_PLATFORM == SF_PLATFORM_GOOGLETEST
constexpr const char* PLATFORM = "GoogleTest";
#else
constexpr const char* PLATFORM = "Unknown";
#endif

} // namespace SF_HAL

#endif // __SF_PLATFORM_IDS_HPP__
