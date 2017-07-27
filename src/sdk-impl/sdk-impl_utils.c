#include "sdk-impl_internal.h"

void IOT_DumpMemoryStats(IOT_LogLevel level)
{
    LOGLEVEL            lvl = level;

    if (lvl < LOG_EMERG_LEVEL || lvl > LOG_DEBUG_LEVEL) {
        lvl = LOG_DEBUG_LEVEL;
        log_warning("Invalid input level, using default: %d => %d", level, lvl);
    }

    return LITE_dump_malloc_free_stats(lvl);
}