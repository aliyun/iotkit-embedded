#ifndef _LOG_POST_
#define _LOG_POST_

#define LOG_PUBLIC_MSG_MAXLEN   (255)

void iotx_log_post_init(void);
void LITE_syslog_post(const char *module, const int level, const char *fmt, ...);

#endif