#include "vers.hpp"
#include "product.hpp"
#include "cli/conio.hpp"
#include "consts.hpp"
#include "util.hpp"

#include "Particle.h"

const char* BUILD_DATE = __DATE__;
const char* BUILD_TIME = __TIME__;

#if PRODUCT_VERSION_USE_HEX == 1
#if FW_MAJOR_VERSION > 7
#error Major Version exceeds field width!
#endif

#if FW_MINOR_VERSION > 63
#error Minor Version exceeds field width!
#endif

#if FW_BUILD_NUM > 127
#error Build Number exceeds field width!
#endif
#else
#if FW_MAJOR_VERSION > 6
#error Major Version exceeds field width!
#endif

#if FW_MINOR_VERSION > 99
#error Minor Version exceeds field width!
#endif

#if FW_BUILD_NUM > 99
#error Build Number exceeds field width!
#endif
#endif

#if PRODUCT_VERSION_VALUE > UINT16_MAX
#error Build Value exceeds field width!
#endif

#if SF_CHARGE_ALLOW_DEPLOY == 1
#warning Deploy is currently allowed during charge mode!
#endif

void VERS_printBanner(void)
{
    SF_OSAL_printf(__NL__ "Smartfin FW v%d.%d.%d%s" __NL__,
                   FW_MAJOR_VERSION,
                   FW_MINOR_VERSION,
                   FW_BUILD_NUM,
                   FW_BRANCH);
    SF_OSAL_printf("FW Build: %s %s" __NL__, BUILD_DATE, BUILD_TIME);
    SF_OSAL_printf("Device OS: %s" __NL__, System.version().c_str());
#if SF_PLATFORM == SF_PLATFORM_PARTICLE

    SF_OSAL_printf("CRC32=%lx" __NL__, module_info_crc.crc32);

    const uint8_t *sha = module_info_suffix.sha;
    SF_OSAL_printf("SHA256=");
    for (size_t byte_idx = 0; byte_idx < 32; byte_idx++)
    {
        SF_OSAL_printf("%02x", sha[byte_idx]);
    }
    SF_OSAL_printf(__NL__);
#endif
}

const char* VERS_getBuildDate(void)
{
    return BUILD_DATE;
}
const char* VERS_getBuildTime(void)
{
    return BUILD_TIME;
}
