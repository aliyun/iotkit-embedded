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

#include <stdio.h>
#include <string.h>
#include "lite-system.h"
#include "iot_import.h"
#include "iot_export.h"

#include "iotx_utils.h"

int iotx_midreport_reqid(char *requestId, char *product_key, char *device_name)
{
    int ret;
    /* requestId = pk+devicename+mid */
    ret = HAL_Snprintf(requestId,
                       MIDREPORT_REQID_LEN,
                       "%s_%s_mid",
                       product_key,
                       device_name);
    return ret;
}

int iotx_midreport_payload(char *msg, char *requestId, char *mid, char *pid)
{
    int ret;
    /*topic's json data: {"id":"requestId" ,"params":{"_sys_device_mid":mid,"_sys_device_pid":pid }}*/
    ret = HAL_Snprintf(msg,
                       MIDREPORT_PAYLOAD_LEN,
                       "{\"id\":\"%s\",\"params\":{\"_sys_device_mid\":\"%s\",\"_sys_device_pid\":\"%s\"}}",
                       requestId,
                       mid,
                       pid);
    return ret;
}

int iotx_midreport_topic(char *topic_name, char *topic_head, char *product_key, char *device_name)
{
    int ret;
    /* reported topic name: "/sys/${productKey}/${deviceName}/thing/status/update" */
    ret = HAL_Snprintf(topic_name,
                       IOTX_URI_MAX_LEN,
                       "%s/sys/%s/%s/thing/status/update",
                       topic_head,
                       product_key,
                       device_name);
    return ret;
}

/* generate AOS versoin report topic */
int iotx_gen_aos_report_topic(char *topic_name, char *product_key, char *device_name)
{
    int ret;
    /* linkkit version topic name: "/sys/${productKey}/${deviceName}/thing/deviceinfo/update" */
    ret = HAL_Snprintf(topic_name, 
                        IOTX_URI_MAX_LEN, 
                        "/sys/%s/%s/thing/deviceinfo/update",
                        product_key,
                        device_name);
    return ret;
}

/* generate AOS version report message json data */
int iotx_gen_aos_report_payload(char *msg, int requestId, char *versionData)
{
    /* Topic's Json data
    {
        "id" : 123,                     
        "version":"1.0",                
        "params" : [{
            "attrKey" : "SYS_ALIOS_ACTIVATION",
            "attrValue" : "some data",
            "domain" : "SYSTEM"
        }],
        "method" : "thing.deviceinfo.update"
    }
    */
    int ret;
    ret = HAL_Snprintf(msg,
                       AOS_VERSON_MSG_LEN,
                       "{\"id\":\"%d\",\"versoin\":\"1.0\",\"params\":[{\"attrKey\":\"SYS_ALIOS_ACTIVATION\",\"attrValue\":\"%s\",\"domain\":\"SYSTEM\"}],\"method\":\"thing.deviceinfo.update\"}",
                       requestId,
                       versionData
                       );
    return ret;
}

int iotx_get_aos_hex_version(char *str, char hex[4])
{
    // AOS-R-1.3.0.0
    char *p = NULL;
    char *q = NULL;
    int i = 0;
    char str_ver[32] = {0};
    if (str == NULL) {
        return -1;
    }
    if (hex == NULL) {
        return -1;
    }
    strncpy(str_ver, str, sizeof(str_ver) - 1);
    p = strtok(str_ver, "-");
    for (i = 0; i < 2; i++) {
        if ( p == NULL) {
            return -1;
        }
        p = strtok(NULL, "-");
    }

    q = strtok(p, ".");
    for (i = 0; i < 4; i++) {
        if ( q == NULL) {
            break;
        } else {
            hex[i] = atoi(q);
        }
        q = strtok(NULL, ".");

    }
    return 0;
}

#undef AOS_IMPLEMENT_DEMO
#ifdef AOS_IMPLEMENT_DEMO
unsigned int aos_get_version_info(unsigned char version_num[4], unsigned char random_num[4], unsigned char mac_address[4], unsigned char chip_code[4], unsigned char *output_buffer, unsigned int output_buffer_size)
{
    strncpy((char *)output_buffer, "01234567890123456789", 21);
    return 0;
}

const char *aos_version_get(void)
{
    return "AOS-R-1.3.0";
}

char* aos_mac_get(char* mac_str)
{
    strncpy(mac_str, "\x08\x08\x08\x08\x08\x08\x08\x08", 8);
    return mac_str;
}

char* aos_chipCode_get(char* cid_str)
{
    strncpy(cid_str, "\x08\x08\x08\x08", 4);
    return cid_str;
}

#endif


