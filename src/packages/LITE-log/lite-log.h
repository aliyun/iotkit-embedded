/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef __LITE_LOG_H__
#define __LITE_LOG_H__
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define LITE_printf                     printf
#define LITE_sprintf                    sprintf
#define LITE_snprintf                   snprintf

typedef enum _LOGLEVEL {
    LOG_EMERG_LEVEL = 0,    // OS system is unavailable
    LOG_CRIT_LEVEL,         // current application aborting
    LOG_ERR_LEVEL,          // current app-module error
    LOG_WARNING_LEVEL,      // using default parameters
    LOG_INFO_LEVEL,         // running messages
    LOG_DEBUG_LEVEL,        // debugging messages
} LOGLEVEL;

void    LITE_openlog(const char *ident);
void    LITE_closelog(void);
int     LITE_log_enabled(void);
char   *LITE_get_logname(void);
int     LITE_get_loglevel(void);
void    LITE_set_loglevel(int level);
int     LITE_hexdump(const char *title, const void *buf, const int len);

void    LITE_syslog(const char *f, const int l, const int level, const char *fmt, ...);
#define log_emerg(args...)      LITE_syslog(__FUNCTION__, __LINE__, LOG_EMERG_LEVEL, args)
#define log_crit(args...)       LITE_syslog(__FUNCTION__, __LINE__, LOG_CRIT_LEVEL, args)
#define log_err(args...)        LITE_syslog(__FUNCTION__, __LINE__, LOG_ERR_LEVEL, args)
#define log_warning(args...)    LITE_syslog(__FUNCTION__, __LINE__, LOG_WARNING_LEVEL, args)
#define log_info(args...)       LITE_syslog(__FUNCTION__, __LINE__, LOG_INFO_LEVEL, args)
#define log_debug(args...)      LITE_syslog(__FUNCTION__, __LINE__, LOG_DEBUG_LEVEL, args)

int     log_multi_line_internal(const char *f, const int l,
                                const char *title, int level, char *payload, const char *mark);
#define log_multi_line(level, title, fmt, payload, mark) \
    log_multi_line_internal(__func__, __LINE__, title, level, payload, mark)

void    LITE_rich_hexdump(const char *f, const int l,
            const int level,
            const char *buf_str,
            const void *buf_ptr,
            const int buf_len
);
#define HEXDUMP_DEBUG(buf, len) \
    LITE_rich_hexdump(__func__, __LINE__, LOG_DEBUG_LEVEL, #buf, (const void *)buf, (const int)len)

#define HEXDUMP_INFO(buf, len)      \
    LITE_rich_hexdump(__func__, __LINE__, LOG_INFO_LEVEL, #buf, (const void *)buf, (const int)len)


#if defined(__cplusplus)
}
#endif
#endif  /* __LITE_LOG_H__ */
