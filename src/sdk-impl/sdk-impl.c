#include "sdk-impl_internal.h"

void IOT_OpenLog(const char *ident)
{
    const char         *mod = ident;

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
    LOGLEVEL            lvl = level;

    if (lvl < LOG_EMERG_LEVEL || lvl > LOG_DEBUG_LEVEL) {
        log_err("Invalid input level: %d out of [%d, %d]", level,
                LOG_EMERG_LEVEL,
                LOG_DEBUG_LEVEL);
        return;
    }

    return LITE_set_loglevel(lvl);
}

void IOT_DumpMemoryStats(IOT_LogLevel level)
{
    LOGLEVEL            lvl = level;

    if (lvl < LOG_EMERG_LEVEL || lvl > LOG_DEBUG_LEVEL) {
        lvl = LOG_DEBUG_LEVEL;
        log_warning("Invalid input level, using default: %d => %d", level, lvl);
    }

    return LITE_dump_malloc_free_stats(lvl);
}

int IOT_SetupConnInfo(void)
{
    return iotx_guider_authenticate();
}
