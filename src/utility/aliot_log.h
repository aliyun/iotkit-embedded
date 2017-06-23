

#ifndef _ALIOT_COMMON_LOG_H_
#define _ALIOT_COMMON_LOG_H_

#include <stdio.h>
#include <stdlib.h>
#include "aliot_platform.h"


typedef enum IOT_LOG_LEVEL {
    ALIOT_LOG_LEVEL_DEBUG = 0,
    ALIOT_LOG_LEVEL_INFO,
    ALIOT_LOG_LEVEL_WARN,
    ALIOT_LOG_LEVEL_ERROR,
    ALIOT_LOG_LEVEL_FATAL,
    ALIOT_LOG_LEVEL_NONE,
} aliot_log_level_t;

extern aliot_log_level_t g_iotLogLevel;

void aliot_log_set_level(aliot_log_level_t iotLogLevel);


#define ALIOT_LOG_DEBUG(format, ...) \
    {\
        if(g_iotLogLevel <= ALIOT_LOG_LEVEL_DEBUG)\
        {\
            printf("[debug] %s:%d %s()| "format"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
            fflush(stdout);\
        }\
    }

#define ALIOT_LOG_INFO(format, ...) \
    {\
        if(g_iotLogLevel <= ALIOT_LOG_LEVEL_INFO)\
        {\
            printf("[info] %s:%u %s()| "format"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
            fflush(stdout);\
        }\
    }

#define ALIOT_LOG_WARN(format, ...) \
    {\
        if(g_iotLogLevel <= ALIOT_LOG_LEVEL_WARN)\
        {\
            printf("[warn] %s:%d %s()| "format"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
            fflush(stdout);\
        }\
    }

#define ALIOT_LOG_ERROR(format,...) \
    {\
        if(g_iotLogLevel <= ALIOT_LOG_LEVEL_ERROR)\
        {\
            printf("[error] %s:%d %s()| "format"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
            fflush(stdout);\
        }\
    }

#define ALIOT_LOG_FATAL(format, ...) \
    {\
        if(g_iotLogLevel <= ALIOT_LOG_LEVEL_FATAL)\
        {\
            printf("[notice] %s:%d %s()| "format"\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
            fflush(stdout);\
        }\
    }

#endif
