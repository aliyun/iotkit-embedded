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



#ifndef _IOTX_COMMON_DEBUG_H_
#define _IOTX_COMMON_DEBUG_H_

#include "lite-log.h"

#ifdef IOTX_DEBUG

#define DEBUG_PUTS(fmt, args ...) \
    do{ \
        HAL_Printf(fmt, ## args); \
    }while(0)

#define ASSERT_FAILED_DO() \
    do{ \
        while(1){\
            HAL_SleepMs(1000); \
            HAL_Printf("assert failed\r\n"); \
        }\
    }while(0)

#define IOTX_ASSERT(expr, fmt, args...) \
    do{ \
        if (!(expr)) { \
            DEBUG_PUTS("###ASSERT FAILED###, file=%s, line=%d\r\n", __FILE__, __LINE__); \
            if (NULL != fmt) { \
                DEBUG_PUTS(fmt, ## args); \
                HAL_Printf("\r\n"); \
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
