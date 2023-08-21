#include "vers.hpp"
#include "product.hpp"
#include "cli/conio.hpp"

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

void VERS_printBanner(void)
{
    SF_OSAL_printf("Smartfin FW v%d.%d.%d%s\r\n", FW_MAJOR_VERSION, FW_MINOR_VERSION, FW_BUILD_NUM, FW_BRANCH);
    SF_OSAL_printf("FW Build: %s %s\n", BUILD_DATE, BUILD_TIME);
}

const char* VERS_getBuildDate(void)
{
    return BUILD_DATE;
}
const char* VERS_getBuildTime(void)
{
    return BUILD_TIME;
}
