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


#ifndef __HTTP_DEBUG_H__
#define __HTTP_DEBUG_H__

#include "lite-log.h"

#ifdef SDK_DEBUG_LEVEL_NONE
#define http_emerg
#define http_crit
#define http_err
#define http_warning
#define http_info
#define http_debug
#else
#define http_emerg(...)    log_emerg("http", __VA_ARGS__)
#define http_crit(...)     log_crit("http", __VA_ARGS__)
#define http_err(...)      log_err("http", __VA_ARGS__)
#define http_warning(...)  log_warning("http", __VA_ARGS__)
#define http_info(...)     log_info("http", __VA_ARGS__)
#define http_debug(...)    log_debug("http", __VA_ARGS__)
#endif

#endif  /* __HTTP_DEBUG_H__ */
