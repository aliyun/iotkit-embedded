#define LITE_syslog(fmt, args...)       do { \
        LITE_printf("%s(%d): ", __func__, __LINE__); \
        LITE_printf(fmt "\r\n", ##args); \
    } while(0)
#define LITE_openlog(args...)
#define LITE_closelog(args...)
#define LITE_getname(args...)
#define LITE_get_loglevel(args...)
#define LITE_set_loglevel(args...)
#define LITE_hexdump(args...)

#define log_emerg(fmt, args...)         LITE_syslog(fmt, ##args)
#define log_crit(fmt, args...)          LITE_syslog(fmt, ##args)
#define log_err(fmt, args...)           LITE_syslog(fmt, ##args)
#define log_warning(fmt, args...)       LITE_syslog(fmt, ##args)
#define log_info(fmt, args...)          LITE_syslog(fmt, ##args)
#define log_debug(args...)
#define log_multi_line(level, title, fmt, payload, mark) \
    do { \
        LITE_printf("%s(%d):\r\n\r\n", __func__, __LINE__); \
        LITE_printf(fmt "\r\n", payload); \
    } while(0)
#define HEXDUMP_INFO(args...)
#define HEXDUMP_DEBUG(args...)
