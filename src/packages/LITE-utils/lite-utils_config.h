 /*
  * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
  * License-Identifier: Apache-2.0
  *
  * Licensed under the Apache License, Version 2.0 (the "License"); you may
  * not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

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
