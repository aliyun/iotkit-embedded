#include "lite-utils_internal.h"
#include "mem_stats.h"

LIST_HEAD(mem_recs);

static int bytes_total_allocated;
static int bytes_total_freed;
static int bytes_total_in_use;
static int bytes_max_allocated;
static int bytes_max_in_use;
static int iterations_allocated;
static int iterations_freed;
static int iterations_in_use;
static int iterations_max_in_use;

#if defined(_PLATFORM_IS_LINUX_)
static int tracking_malloc_callstack = 1;

static int record_backtrace(int *level, char *** trace)
{
#define MAX_BT_LEVEL    20

    void       *buffer[MAX_BT_LEVEL];

    *level = backtrace(buffer, MAX_BT_LEVEL);
    *trace = backtrace_symbols(buffer, *level);
    if (*trace == NULL) {
        log_err("backtrace_symbols returns NULL!");
        return -1;
    }

    return 0;
}

void LITE_track_malloc_callstack(int state)
{
    tracking_malloc_callstack = state;
}
#endif  /* defined(_PLATFORM_IS_LINUX_) */

void *LITE_realloc_internal(const char *f, const int l, void *ptr, int size)
{
    void               *temp = NULL;

    if (size <= 0) {
        return NULL;
    }
    if (NULL == (temp = LITE_malloc_internal(f, l, size))) {
        log_err("allocate %d bytes from %s(%d) failed", size, f, l);
        return NULL;
    }

    if (ptr) {
        memcpy(temp, ptr, size);
        LITE_free(ptr);
    }

    return temp;
}

void *LITE_malloc_internal(const char *f, const int l, int size)
{
    void                   *ptr = NULL;
    OS_malloc_record       *pos;

    if (size <= 0) {
        return NULL;
    }

    ptr = malloc(size);
    if (!ptr) {
        return NULL;
    }

    iterations_allocated += 1;
    bytes_total_allocated += size;
    bytes_total_in_use += size;
    bytes_max_in_use = (bytes_max_in_use > bytes_total_in_use) ? bytes_max_in_use : bytes_total_in_use;
    bytes_max_allocated = (bytes_max_allocated >= size) ? bytes_max_allocated : size;

#if defined(WITH_TOTAL_COST_WARNING)
    if (bytes_total_in_use > WITH_TOTAL_COST_WARNING) {
        log_debug(" ");
        log_debug("==== PRETTY HIGH TOTAL IN USE: %d BYTES ====", bytes_total_in_use);
        LITE_dump_malloc_free_stats(LOG_DEBUG_LEVEL);
    }
#endif

    iterations_in_use += 1;
    iterations_max_in_use = (iterations_in_use > iterations_max_in_use) ? iterations_in_use : iterations_max_in_use;

    pos = malloc(sizeof(OS_malloc_record));
    memset(pos, 0, sizeof(OS_malloc_record));

    pos->buf = ptr;
    pos->buflen = size;
    pos->func = (char *)f;
    pos->line = (int)l;
#if defined(_PLATFORM_IS_LINUX_)
    if (tracking_malloc_callstack) {
        record_backtrace(&pos->bt_level, &pos->bt_symbols);
    }
#endif

    list_add_tail(&pos->list, &mem_recs);

#if defined(WITH_ALLOC_WARNING_THRESHOLD)
    if (size > WITH_ALLOC_WARNING_THRESHOLD) {
        int             k;

        log_warning("large allocating @ %s(%d) for %04d bytes!", f, l, size);
        LITE_printf("\r\n");
#if defined(_PLATFORM_IS_LINUX_)
        for (k = 0; k < pos->bt_level; ++k) {
            int             m;
            const char     *p = LITE_strchr(pos->bt_symbols[k], '(');

            if (p[1] == ')') {
                continue;
            }
            for (m = 0; m < k; ++m) {
                LITE_printf("  ");
            }

            LITE_printf("%s\r\n", p);
        }
#endif
        LITE_printf("\r\n");
    }
#endif

    memset(ptr, 0, size);
    return ptr;
}

void LITE_free_internal(void *ptr)
{
    OS_malloc_record       *pos;

    if (!ptr) {
        return;
    }

    pos = NULL;
    list_for_each_entry(pos, &mem_recs, list) {
        if (pos->buf == ptr) {
            break;
        }
    }

    if (NULL == pos) {
        log_warning("Cannot find %p allocated! Skip stat ...", ptr);
    } else {
        iterations_freed += 1;
        iterations_in_use -= 1;

        bytes_total_freed += pos->buflen;
        bytes_total_in_use -= pos->buflen;

        memset(pos->buf, 0xEE, pos->buflen);
        pos->buf = 0;
        pos->buflen = 0;
        pos->func = "";
        pos->line = 0;
#if defined(_PLATFORM_IS_LINUX_)
        pos->bt_level = 0;
        free(pos->bt_symbols);
        pos->bt_symbols = 0;
#endif

        list_del(&pos->list);
        free(pos);
    }

    free(ptr);
}

#if WITH_MEM_STATS
void LITE_dump_malloc_free_stats(int level)
{
    OS_malloc_record       *pos;

#if defined(LITE_LOG_ENABLED)
    if (level > LITE_get_loglevel()) {
        return;
    }
#endif

    LITE_printf("\r\n");
    LITE_printf("---------------------------------------------------\r\n");
    LITE_printf(". bytes_total_allocated:    %d\r\n", bytes_total_allocated);
    LITE_printf(". bytes_total_freed:        %d\r\n", bytes_total_freed);
    LITE_printf(". bytes_total_in_use:       %d\r\n", bytes_total_in_use);
    LITE_printf(". bytes_max_allocated:      %d\r\n", bytes_max_allocated);
    LITE_printf(". bytes_max_in_use:         %d\r\n", bytes_max_in_use);
    LITE_printf(". iterations_allocated:     %d\r\n", iterations_allocated);
    LITE_printf(". iterations_freed:         %d\r\n", iterations_freed);
    LITE_printf(". iterations_in_use:        %d\r\n", iterations_in_use);
    LITE_printf(". iterations_max_in_use:    %d\r\n", iterations_max_in_use);
    LITE_printf("---------------------------------------------------\r\n");

#if defined(LITE_LOG_ENABLED)
    if (LITE_get_loglevel() == level) {
#else
    {
#endif
        int         j;
        int         cnt = 0;

        list_for_each_entry(pos, &mem_recs, list)
        {
            if (pos->buf) {
                LITE_printf("%4d. %-24s Ln:%-5d @ %p: %4d bytes [",
                ++cnt,
                pos->func,
                pos->line,
                pos->buf,
                pos->buflen);
                for (j = 0; j < 32 && j < pos->buflen; ++j) {
                    char        c;

                    c = *(char *)(pos->buf + j);
                    if (c < ' ' || c > '~') {
                        c = '.';
                    }
                    LITE_printf("%c", c);
                }
                LITE_printf("]\r\n");

#if defined(_PLATFORM_IS_LINUX_)
                int             k;

                LITE_printf("\r\n");
                for (k = 0; k < pos->bt_level; ++k) {
                    int             m;
                    const char     *p = strchr(pos->bt_symbols[k], '(');

                    if (p[1] == ')') {
                        continue;
                    }
                    LITE_printf("    ");
                    for (m = 0; m < k; ++m) {
                        LITE_printf("  ");
                    }

                    LITE_printf("%s\r\n", p);
                }
#endif
                LITE_printf("\r\n");
            }
        }
    }

    return;
}
#endif  /* #if WITH_MEM_STATS */