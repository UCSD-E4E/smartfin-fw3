#ifndef __VERS_H__
#define __VERS_H__
/**
 * @brief This is the Smartfin FW codebase
 * 
 * DATE     WHO DESCRIPTION
 * ----------------------------------------------------------------------------
 * 
 */

#include "product.hpp"

#include <cstdint>
#define FW_MAJOR_VERSION    3
#define FW_MINOR_VERSION    20
#define FW_BUILD_NUM        3
#define FW_BRANCH           ""

#if PRODUCT_VERSION_USE_HEX == 1
#define PRODUCT_VERSION_VALUE (FW_MAJOR_VERSION << 13 | FW_MINOR_VERSION << 6 | FW_BUILD_NUM)
#else
#define PRODUCT_VERSION_VALUE (FW_MAJOR_VERSION * 10000 + FW_MINOR_VERSION * 100 + FW_BUILD_NUM)
#endif

void VERS_printBanner(void);
inline const char *VERS_getBuildDate(void);
inline const char *VERS_getBuildTime(void);
#endif