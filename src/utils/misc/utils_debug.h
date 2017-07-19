
#ifndef _IOTX_COMMON_DEBUG_H_
#define _IOTX_COMMON_DEBUG_H_

#include "lite/lite-log.h"

#ifdef IOTX_DEBUG

#define DEBUG_PUTS(fmt, args ...) \
    do{ \
        iotx_platform_printf(fmt, ## args); \
    }while(0)

#define ASSERT_FAILED_DO() \
    do{ \
        while(1){\
            iotx_platform_msleep(1000); \
            iotx_platform_printf("assert failed\r\n"); \
        }\
    }while(0)

#define IOTX_ASSERT(expr, fmt, args...) \
    do{ \
        if (!(expr)) { \
            DEBUG_PUTS("###ASSERT FAILED###, file=%s, line=%d\r\n", __FILE__, __LINE__); \
            if (NULL != fmt) { \
                DEBUG_PUTS(fmt, ## args); \
                iotx_platform_printf("\r\n"); \
            } \
            ASSERT_FAILED_DO(); \
        } \
    }while(0)



#define IOTX_TRACE(fmt, args...) \
    DEBUG_PUTS(fmt, ## args)
#else

#define TRACE(fmt, args...) do{ }while(0)

#endif


#endif /* _IOTX_COMMON_DEBUG_H_ */
