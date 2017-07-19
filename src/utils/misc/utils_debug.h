
#ifndef _ALIOT_COMMON_DEBUG_H_
#define _ALIOT_COMMON_DEBUG_H_

#include "lite/lite-log.h"

#ifdef ALIOT_DEBUG

#define DEBUG_PUTS(fmt, args ...) \
    do{ \
        aliot_platform_printf(fmt, ## args); \
    }while(0)

#define ASSERT_FAILED_DO() \
    do{ \
        while(1){\
            aliot_platform_msleep(1000); \
            aliot_platform_printf("assert failed\r\n"); \
        }\
    }while(0)

#define ALIOT_ASSERT(expr, fmt, args...) \
    do{ \
        if (!(expr)) { \
            DEBUG_PUTS("###ASSERT FAILED###, file=%s, line=%d\r\n", __FILE__, __LINE__); \
            if (NULL != fmt) { \
                DEBUG_PUTS(fmt, ## args); \
                aliot_platform_printf("\r\n"); \
            } \
            ASSERT_FAILED_DO(); \
        } \
    }while(0)



#define ALIOT_TRACE(fmt, args...) \
    DEBUG_PUTS(fmt, ## args)
#else

#define TRACE(fmt, args...) do{ }while(0)

#endif


#endif /* _ALIOT_COMMON_DEBUG_H_ */
