/*
 * aliyun_iot_shadow_inner.h
 *
 *  Created on: May 17, 2017
 *      Author: qibiao.wqb
 */

#ifndef _MQTT_SHADOW_ALIYUN_IOT_SHADOW_COMMON_H_
#define _MQTT_SHADOW_ALIYUN_IOT_SHADOW_COMMON_H_


#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "shadow/aliyun_iot_shadow.h"
#include "shadow/aliyun_iot_shadow_config.h"

typedef enum {
    ALIOT_SHADOW_NULL,
    ALIOT_SHADOW_INT32,
    ALIOT_SHADOW_STRING,
} aliot_shadow_attr_datatype_t;

typedef struct {
    bool flag_new;
    uint32_t buf_size;
    uint32_t offset;
    char *buf;
}format_data_t, *format_data_pt;


typedef struct {
    uint32_t base_system_time; //in second
    uint32_t epoch_time;
} aliot_shadow_time_t, *aliot_shadow_time_pt;


typedef struct aliot_shadow_st aliot_shadow_t, *aliot_shadow_pt;
typedef struct aliot_shadow_attr_st aliot_shadow_attr_t, *aliot_shadow_attr_pt;


aliot_shadow_pt ads_common_get_ads(void);

void ads_common_set_ads(aliot_shadow_pt pshadow);

aliot_err_t ads_common_format_init(format_data_pt pformat,
                char *buf,
                uint16_t size,
                const char *method,
                const char *head_str);

aliot_err_t ads_common_format_add(format_data_pt pformat,
                const char *name,
                const void *pvalue,
                aliot_shadow_attr_datatype_t datatype);

aliot_err_t ads_common_format_finalize(format_data_pt pformat, const char *tail_str);

void ads_common_update_time(aliot_shadow_pt pshadow, uint32_t new_timestamp);

int ads_common_convert_data2string(
                char *buf,
                size_t buf_len,
                aliot_shadow_attr_datatype_t type,
                const void *pData);

aliot_err_t ads_common_convert_string2data(
                const char *buf,
                size_t buf_len,
                aliot_shadow_attr_datatype_t type,
                void *pData);

bool ads_common_check_attr_existence(aliot_shadow_pt pshadow, const aliot_shadow_attr_pt pattr);

aliot_err_t ads_common_register_attr (
                aliot_shadow_pt pshadow,
                aliot_shadow_attr_pt pattr);

aliot_err_t ads_common_remove_attr (
                aliot_shadow_pt pshadow,
                aliot_shadow_attr_pt pattr);

char *ads_common_generate_topic_name(aliot_shadow_pt pshadow, const char *topic);

aliot_err_t ads_common_publish2update(aliot_shadow_pt pshadow, char *data, uint32_t data_len);

void ads_common_update_version(aliot_shadow_pt pshadow, uint32_t version);

void ads_common_increase_version(aliot_shadow_pt pshadow);

uint32_t ads_common_get_version(aliot_shadow_pt pshadow);

uint32_t ads_common_get_version(aliot_shadow_pt pshadow);

void ads_common_increase_tokennum(aliot_shadow_pt pshadow);

uint32_t ads_common_get_tokennum(aliot_shadow_pt pshadow);

#endif /* _MQTT_SHADOW_ALIYUN_IOT_SHADOW_COMMON_H_ */
