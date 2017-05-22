
#ifndef _SHADOW_ALIOT_SHADOW_COMMON_H_
#define _SHADOW_ALIOT_SHADOW_COMMON_H_


#include "aliot_platform.h"
#include "aliot_error.h"
#include "aliot_timer.h"
#include "aliot_list.h"
#include "aliot_shadow.h"
#include "aliot_shadow_config.h"

typedef struct aliot_update_ack_wait_list_st {
    bool flag_busy; //0, free; 1, busy.
    char token[ADS_TOKEN_LEN];
    aliot_update_cb_fpt callback;
    void *pcontext;
    aliot_time_t timer;
} aliot_update_ack_wait_list_t, *aliot_update_ack_wait_list_pt;


typedef struct aliot_inner_data_st {
    uint32_t token_num;
    uint32_t version; 
    aliot_shadow_time_t time;
    aliot_update_ack_wait_list_t update_ack_wait_list[ADS_UPDATE_WAIT_ACK_LIST_NUM];
    list_t *attr_list;
    char *ptopic_update;
    char *ptopic_get;
    int32_t sync_status;
} aliot_inner_data_t, *aliot_inner_data_pt;;


typedef struct aliot_shadow_st {
    void *mqtt;
    void *mutex;
    aliot_inner_data_t inner_data;
} aliot_shadow_t, *aliot_shadow_pt;


aliot_err_t ads_common_format_init(aliot_shadow_pt pshadow,
                                   format_data_pt pformat,
                                   char *buf,
                                   uint16_t size,
                                   const char *method,
                                   const char *head_str);

aliot_err_t ads_common_format_add(aliot_shadow_pt pshadow,
                                  format_data_pt pformat,
                                  const char *name,
                                  const void *pvalue,
                                  aliot_shadow_attr_datatype_t datatype);

aliot_err_t ads_common_format_finalize(aliot_shadow_pt pshadow, format_data_pt pformat, const char *tail_str);

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

aliot_err_t ads_common_register_attr(
            aliot_shadow_pt pshadow,
            aliot_shadow_attr_pt pattr);

aliot_err_t ads_common_remove_attr(
            aliot_shadow_pt pshadow,
            aliot_shadow_attr_pt pattr);

char *ads_common_generate_topic_name(aliot_shadow_pt pshadow, const char *topic);

aliot_err_t ads_common_publish2update(aliot_shadow_pt pshadow, char *data, uint32_t data_len);

void ads_common_update_version(aliot_shadow_pt pshadow, uint32_t version);

uint32_t ads_common_get_version(aliot_shadow_pt pshadow);

uint32_t ads_common_get_tokennum(aliot_shadow_pt pshadow);

#endif /* _SHADOW_ALIOT_SHADOW_COMMON_H_ */
