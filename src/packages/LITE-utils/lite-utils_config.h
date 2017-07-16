#ifndef __LITE_UTILS_CONFIG_H__
#define __LITE_UTILS_CONFIG_H__

#define LITE_snprintf   snprintf
#define LITE_printf     printf

#define LITE_log(fmt, args...) \
    do { \
        printf(fmt, ##args); \
        printf("\r\n"); \
        fflush(stdout); \
    } while (0)

#define log_crit        LITE_log
#define log_err         LITE_log
#define log_info        LITE_log
#define log_warning     LITE_log
#define log_debug       LITE_log

#define WITH_MEM_STATS  1

#endif  /* __LITE_UTILS_CONFIG_H__ */
