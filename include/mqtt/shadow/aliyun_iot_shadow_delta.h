/*
 * aliyun_iot_shadow_delta.h
 *
 *  Created on: May 17, 2017
 *      Author: qibiao.wqb
 */

#ifndef _MQTT_SHADOW_ALIYUN_IOT_SHADOW_DELTA_H_
#define _MQTT_SHADOW_ALIYUN_IOT_SHADOW_DELTA_H_

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "shadow/aliyun_iot_shadow.h"
#include "shadow/aliyun_iot_shadow_config.h"
#include "shadow/aliyun_iot_shadow_common.h"
#include "shadow/aliyun_iot_shadow_update.h"


int aliyun_iot_shadow_delta_init(aliot_shadow_pt pshadow);

bool aliyun_iot_shadow_delta_check_existence(const char *attr_name);

void aliyun_iot_shadow_delta_entry(
        aliot_shadow_pt pshadow,
        char *json_doc,
        size_t json_doc_len);

#endif /* _MQTT_SHADOW_ALIYUN_IOT_SHADOW_DELTA_H_ */
