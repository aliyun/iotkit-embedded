#ifndef __LITE_LOG_H__
#define __LITE_LOG_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef LITE_LOG_CONFIG_H
#include LITE_LOG_CONFIG_H
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#define LOG_MOD_NAME_LEN                (7)
#define LOG_MSG_MAXLEN                  (255)
#define LOG_FMT_TAG                     "[%s] %s(%d): "
#define HEXDUMP_SEP_LINE                "+" \
                                        "-----------------------" \
                                        "-----------------------" \
                                        "-----------------------"

#define LITE_printf                     printf
#define LITE_sprintf                    sprintf
#define LITE_snprintf                   snprintf

#if defined(_PLATFORM_IS_LINUX_)
#undef  LOG_MSG_MAXLEN
#define LOG_MSG_MAXLEN                  (1023)
#endif

typedef enum _IOT_LOGLEVEL {
    LOG_EMERG_LEVEL = 0,    // OS system is unavailable
    LOG_CRIT_LEVEL,         // current application aborting
    LOG_ERR_LEVEL,          // current app-module error
    LOG_WARNING_LEVEL,      // using default parameters
    LOG_INFO_LEVEL,         // running messages
    LOG_DEBUG_LEVEL,        // debugging messages
} IOT_LOGLEVEL;

#if !defined(LITE_LOG_ENABLED)

#include "lite-log_disable.h"

#else

typedef struct {
    char            name[LOG_MOD_NAME_LEN+1];
    int             priority;
    char            text_buf[LOG_MSG_MAXLEN+1];
} log_client;

void     LITE_openlog(const char *ident);
void     LITE_closelog(void);
char *   LITE_getname(void);
int      LITE_get_loglevel(void);
void     LITE_set_loglevel(int level);
int      LITE_hexdump (const char *title, void * buf, int len);

extern log_client logcb;
extern char *pri_names[];

#define LITE_syslog(f, l, pri, fmt, args...)                do { \
    char *  tmpbuf = logcb.text_buf; \
    char *  o = tmpbuf; \
    int     truncated = 0; \
\
    if (!strlen(LITE_getname()) || LITE_get_loglevel() < pri || pri < LOG_EMERG_LEVEL) \
        break; \
\
    LITE_printf(LOG_FMT_TAG, pri_names[pri], f, l); \
\
    memset(tmpbuf, 0, sizeof(logcb.text_buf)); \
    o += LITE_snprintf(o, LOG_MSG_MAXLEN+1, fmt, ##args); \
    if (o - tmpbuf > LOG_MSG_MAXLEN) { \
        o = tmpbuf + strlen(tmpbuf); \
        truncated = 1; \
    } \
    if (strlen(tmpbuf) == LOG_MSG_MAXLEN)  truncated = 1; \
\
    LITE_printf("%s", tmpbuf); \
    if (truncated) \
        LITE_printf(" ..."); \
\
    LITE_printf("\r\n"); \
} while(0)

#define log_emerg(args...)      LITE_syslog(__FUNCTION__, __LINE__, LOG_EMERG_LEVEL, args)
#define log_crit(args...)       LITE_syslog(__FUNCTION__, __LINE__, LOG_CRIT_LEVEL, args)
#define log_err(args...)        LITE_syslog(__FUNCTION__, __LINE__, LOG_ERR_LEVEL, args)
#define log_warning(args...)    LITE_syslog(__FUNCTION__, __LINE__, LOG_WARNING_LEVEL, args)
#define log_info(args...)       LITE_syslog(__FUNCTION__, __LINE__, LOG_INFO_LEVEL, args)
#define log_debug(args...)      LITE_syslog(__FUNCTION__, __LINE__, LOG_DEBUG_LEVEL, args)

#define log_multi_line(level, title, fmt, payload, mark) \
        log_multi_line_internal(__func__, __LINE__, title, level, payload, mark)

int log_multi_line_internal(const char * f, const int l,
    const char * title, int level, char * payload, const char *mark);

#define HEXDUMP_DEBUG(buf, len)      do { \
    if(LITE_get_loglevel() >= LOG_DEBUG_LEVEL) { \
        log_debug("HEXDUMP %s @ %p[%d]", #buf, buf, (int)len); \
        LITE_hexdump(#buf, (void *)buf, (int)len); \
    } \
} while(0)

#define HEXDUMP_INFO(buf, len)      do { \
    if(LITE_get_loglevel() >= LOG_INFO_LEVEL) { \
        log_info("HEXDUMP %s @ %p[%d]", #buf, buf, (int)len); \
        LITE_hexdump(#buf, (void *)buf, len); \
    } \
} while(0)

#if defined(__cplusplus)
}
#endif
#endif  /* !defined(LITE_LOG_ENABLED) */
#endif  /* __LITE_LOG_H__ */
