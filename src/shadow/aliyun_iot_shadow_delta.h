
#ifndef _MQTT_SHADOW_ALIYUN_IOT_SHADOW_DELTA_H_
#define _MQTT_SHADOW_ALIYUN_IOT_SHADOW_DELTA_H_

#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_shadow.h"
#include "aliyun_iot_shadow_config.h"
#include "aliyun_iot_shadow_common.h"
#include "aliyun_iot_shadow_update.h"


aliot_err_t aliyun_iot_shadow_delta_init(aliot_shadow_pt pshadow);

bool aliyun_iot_shadow_delta_check_existence(aliot_shadow_pt pshadow, const char *attr_name);

void aliyun_iot_shadow_delta_entry(
        aliot_shadow_pt pshadow,
        char *json_doc,
        size_t json_doc_len);

aliot_err_t aliyun_iot_shadow_delta_register_attr(
            aliot_shadow_pt pshadow,
            aliot_shadow_attr_pt pattr);

#endif /* _MQTT_SHADOW_ALIYUN_IOT_SHADOW_DELTA_H_ */
