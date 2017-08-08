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
 */




#ifndef _IOTX_COMMON_ERROR_H_
#define _IOTX_COMMON_ERROR_H_




// #define IOTX_TRACE
/**
 * @brief Debug level trace logging macro.
 *
 * Macro to print message function entry and exit
 */
#ifdef IOTX_TRACE
#define IOTX_FUNC_ENTRY    \
    {\
        printf("FUNC_ENTRY:   %s L#%d \n", __PRETTY_FUNCTION__, __LINE__);  \
    }
#define IOTX_FUNC_EXIT    \
    {\
        printf("FUNC_EXIT:   %s L#%d \n", __PRETTY_FUNCTION__, __LINE__);  \
    }
#define IOTX_FUNC_EXIT_RC(x)    \
    {\
        printf("FUNC_EXIT:   %s L#%d Return Code : %d \n", __PRETTY_FUNCTION__, __LINE__, x);  \
        return x; \
    }
#else

#define IOTX_FUNC_ENTRY
#define IOTX_FUNC_EXIT
#define IOTX_FUNC_EXIT_RC(x) { return x; }
#endif

#endif
