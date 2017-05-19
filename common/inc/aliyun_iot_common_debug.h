/*
 * aliyun_iot_common_debug.h
 *
 *  Created on: May 16, 2017
 *      Author: qibiao.wqb
 */

#ifndef COMMON_INC_ALIYUN_IOT_COMMON_DEBUG_H_
#define COMMON_INC_ALIYUN_IOT_COMMON_DEBUG_H_

#include "aliyun_iot_common_log.h"

#define DEBUG

#ifdef DEBUG

#define DEBUG_PUTS(fmt, args ...) \
    do{ \
            printf(fmt, ## args); \
    }while(0)

#define ASSERT_FAILED_DO() \
    do{ \
        while(1){\
            ; \
        }\
    }while(0)

#define ALIOT_ASSERT(expr, fmt, args...) \
    do{ \
        if (!(expr)) { \
            DEBUG_PUTS("###ASSERT FAILED###, file=%s, line=%d\r\n", __FILE__, __LINE__); \
            DEBUG_PUTS(fmt, ## args); \
            ASSERT_FAILED_DO(); \
        } \
    }while(0)



#define ALIOT_TRACE(fmt, args...) \
    DEBUG_PUTS(fmt, ## args)
#else

#define TRACE(fmt, args...) do{ }while(0)

#endif


#endif /* COMMON_INC_ALIYUN_IOT_COMMON_DEBUG_H_ */
