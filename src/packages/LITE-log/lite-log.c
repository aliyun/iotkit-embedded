#include "lite-log.h"

#if defined(LITE_LOG_ENABLED)
log_client logcb;

char *pri_names[] = {
    "emg", "crt", "err", "wrn", "inf", "dbg",
};

void LITE_openlog(const char *ident)
{
    strncpy(logcb.name, ident, LOG_MOD_NAME_LEN);
    logcb.name[LOG_MOD_NAME_LEN] = 0;
    logcb.priority = 0;
}

void LITE_closelog(void)
{
    strncpy(logcb.name, "", LOG_MOD_NAME_LEN);
    logcb.name[LOG_MOD_NAME_LEN] = 0;
    logcb.priority = 0;
}

char * LITE_getname(void)
{
    return logcb.name;
}

int LITE_get_loglevel(void)
{
    return logcb.priority;
}

void LITE_set_loglevel(int pri)
{
    logcb.priority = pri;
}

#define LITE_HEXDUMP_DRAWLINE(start_mark, len, end_mark)    do { \
    int                     i; \
\
    LITE_printf("%s", start_mark); \
    for(i = 0; i < len; ++i) { LITE_printf("-"); } \
    LITE_printf("%s", end_mark); \
    LITE_printf("\r\n"); \
\
} while(0)

int LITE_hexdump(const char *title, void * buff, int len)
{
    int                     i, j, written;
    unsigned char           ascii[16+1] = {0};
    char                    header[64] = {0};
    unsigned char *         buf = (unsigned char *)buff;

    LITE_snprintf(header, sizeof(header), "| %s: (len=%d) |\r\n", title, (int)len);

    LITE_HEXDUMP_DRAWLINE("+", strlen(header)-4, "+");
    LITE_printf("%s", header);
    LITE_printf("%s\r\n", HEXDUMP_SEP_LINE);

    written = 0;
    for (i = 0; i < len; ++ i) {
        if (i % 16 == 0) {
            LITE_printf("| %08X: ", (unsigned int)(i+(long)buff));
            written += 8;
        }

        LITE_printf("%02X", buf[i]);
        written += 2;

        if (i % 2 == 1) {
            LITE_printf(" ");
            written += 1;
        }
        LITE_sprintf((char * __restrict__)ascii+i%16, "%c", ((buf[i]>=' ' && buf[i]<='~') ?  buf[i] : '.'));

        if (((i+1) % 16 == 0) || (i == len - 1)) {
            for(j = 0; j < 48-written; ++j)
                LITE_printf(" ");

            LITE_printf(" %s", ascii);
            LITE_printf("\r\n");

            written = 0;
            memset(ascii, 0, sizeof(ascii));
        }
    }
    LITE_printf("%s\r\n", HEXDUMP_SEP_LINE);

    return 0;
}

int log_multi_line_internal(const char * f, const int l,
    const char * title, int level, char * payload, const char *mark)
{
    const char *    pos;
    const char *    endl;
    int             i;

    if(LITE_get_loglevel() < level)     return 1;

    LITE_printf("%s(%d): %s (Length: %d Bytes)\r\n", f, l, title, (int)strlen(payload));

    pos = payload;
    while(pos && *pos) {
        LITE_printf("%s ", mark);

        if (*pos == '\r') {
            LITE_printf("\r\n");
            pos += 2;
            continue;
        }

        endl = strchr(pos+1, '\r');
        if(endl == NULL) {
            endl = pos;
            do {
                ++endl;
            } while(*endl);
        }

        for(i = 0; i < endl-pos; ++i) {
            LITE_printf("%c", pos[i]);
        }
        LITE_printf("\r\n");

        pos = *endl ? endl + 2 : 0;
    }

    return 0;
}

#endif  /* defined(LITE_LOG_ENABLED) */

