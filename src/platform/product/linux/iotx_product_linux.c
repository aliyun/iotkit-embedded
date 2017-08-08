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



#include <stdio.h>
#include <stdint.h>

#include "iot_export.h"

#define IOTX_PRODUCT_KEY         "trTceekBd1P"
#define IOTX_DEVICE_NAME         "KAW7ihRrroLevlHN1y21"
#define IOTX_DEVICE_SECRET       "0yscIv4r7cIc3aDu6kKGvyVVEWvobQF6"
#define IOTX_DEVICE_ID           "trTceekBd1P.KAW7ihRrroLevlHN1y21"


int HAL_GetDeviceInfo(iotx_deviceinfo_t *p_devinfo)
{
    if(NULL == p_devinfo){
        return IOTX_ERR_INVALID_PARAM;
    }

    memset(p_devinfo, 0x00, sizeof(iotx_deviceinfo_t));
    strncpy(p_devinfo->device_id,    IOTX_DEVICE_ID,   IOTX_DEVICE_ID_LEN);
    strncpy(p_devinfo->product_key,  IOTX_PRODUCT_KEY, IOTX_PRODUCT_KEY_LEN);
    strncpy(p_devinfo->device_secret,IOTX_DEVICE_SECRET, IOTX_DEVICE_SECRET_LEN);
    strncpy(p_devinfo->device_name,  IOTX_DEVICE_NAME, IOTX_DEVICE_NAME_LEN);

    return IOTX_SUCCESS;
}

