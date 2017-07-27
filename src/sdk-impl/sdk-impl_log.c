#include "sdk-impl_internal.h"

void IOT_OpenLog(const char *ident)
{
    const char *        mod = ident;

    if (NULL == mod) {
        mod = "---";
    }
    return LITE_openlog(mod);
}

void IOT_CloseLog(void)
{
    return LITE_closelog();
}

void IOT_SetLogLevel(IOT_LogLevel level)
{
    return LITE_set_loglevel(level);
}