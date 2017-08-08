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


#ifndef __GUIDER_INTERNAL_H__
#define __GUIDER_INTERNAL_H__
#define _GNU_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "iot_import.h"

#include "lite-log.h"
#include "lite-utils.h"
#include "utils_md5.h"
#include "utils_base64.h"
#include "utils_hmac.h"
#include "utils_httpc.h"
#include "ca.h"
#include "tfs/tfs.h"
#include "guider.h"

#define GUIDER_IOT_ID_LEN           (256)
#define GUIDER_IOT_TOKEN_LEN        (512)
#define GUIDER_DEFAULT_TS_STR       "2524608000000"

#define CONN_SECMODE_LEN            (32)

#define GUIDER_SIGN_LEN             (256)
#define GUIDER_PID_LEN              (64)
#define GUIDER_TS_LEN               (16)
#define GUIDER_URL_LEN              (256)

#define GUIDER_DEVCODE_LEN          (256)
#define GUIDER_URLENCODE_LEN        (256)

#define GUIDER_DIRECT_DOMAIN        "iot-as-mqtt.cn-shanghai.aliyuncs.com"

#endif  /* __GUIDER_INTERNAL_H__ */
