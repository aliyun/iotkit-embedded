
#ifndef _MQTT_SHADOW_ALIYUN_IOT_SHADOW_UPDATE_H_
#define _MQTT_SHADOW_ALIYUN_IOT_SHADOW_UPDATE_H_

#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_shadow.h"
#include "aliyun_iot_shadow_config.h"
#include "aliyun_iot_shadow_common.h"


aliot_update_ack_wait_list_pt aliyun_iot_shadow_update_wait_ack_list_add(
            aliot_shadow_pt pshadow,
            const char *token,
            size_t token_len,
            aliot_update_cb_fpt cb,
            uint32_t timeout);

void aliyun_iot_shadow_update_wait_ack_list_remove(aliot_shadow_pt pshadow, aliot_update_ack_wait_list_pt element);

void aliyun_iot_shadow_update_wait_ack_list_handle_expire(aliot_shadow_pt pshadow);

void aliyun_iot_shadow_update_wait_ack_list_handle_response(
            aliot_shadow_pt pshadow,
            char *json_doc,
            size_t json_doc_len);


#endif /* _MQTT_SHADOW_ALIYUN_IOT_SHADOW_UPDATE_H_ */
