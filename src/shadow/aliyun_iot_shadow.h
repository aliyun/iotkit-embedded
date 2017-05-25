/*
 * aliyun_iot_shadow.h
 *
 *  Created on: May 15, 2017
 *      Author: qibiao.wqb
 */


#ifndef _MQTT_ALIYUN_IOT_SHADOW_H_
#define _MQTT_ALIYUN_IOT_SHADOW_H_

#include "aliyun_iot_mqtt_client.h"
#include "aliyun_iot_auth.h"

#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_common_error.h"

#include "aliyun_iot_shadow_config.h"


//ads, aliot device shadow

typedef enum {
    ALIOT_SHADOW_ACK_TIMEOUT = -1,
    ALIOT_SHADOW_ACK_NONE = 0,
    ALIOT_SHADOW_ACK_SUCCESS = 200,
    ALIOT_SHADOW_ACK_ERR_JSON_FMT_IS_INVALID = 400,
    ALIOT_SHADOW_ACK_ERR_METHOD_IS_NULL = 401,
    ALIOT_SHADOW_ACK_ERR_STATE_IS_NULL = 402,
    ALIOT_SHADOW_ACK_ERR_VERSION_IS_INVALID = 403,
    ALIOT_SHADOW_ACK_ERR_REPORTED_IS_NULL = 404,
    ALIOT_SHADOW_ACK_ERR_REPORTED_ATTRIBUTE_IS_NULL = 405,
    ALIOT_SHADOW_ACK_ERR_METHOD_IS_INVALID = 406,
    ALIOT_SHADOW_ACK_ERR_SHADOW_DOCUMENT_IS_NULL = 407,
    ALIOT_SHADOW_ACK_ERR_ATTRIBUTE_EXCEEDED = 408,
    ALIOT_SHADOW_ACK_ERR_SERVER_FAILED = 500,
} aliot_shadow_ack_code_t;


typedef enum {
    ALIOT_SHADOW_READONLY,
    ALIOT_SHADOW_WRITEONLY,
    ALIOT_SHADOW_RW
} aliot_shadow_datamode_t;


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


/**
 * @brief Function Pointer typedef used as the callback for every action
 *
 * This function will be called from the context of \c aws_iot_shadow_yield() context
 *
 * @param pThingName Thing Name of the response received
 * @param action The response of the action
 * @param status Informs if the action was Accepted/Rejected or Timed out
 * @param pReceivedJsonDocument Received JSON document
 * @param pContextData the void* data passed in during the action call(update, get or delete)
 *
 */
typedef void (*aliot_update_cb_fpt)(
        aliot_shadow_ack_code_t ack_code,
        const char *ack_msg, //NOTE: NOT a string.
        uint32_t ack_msg_len);


struct aliot_shadow_attr_st;

typedef void (*aliot_shadow_attr_cb_t)(struct aliot_shadow_attr_st *pattr);

typedef struct aliot_shadow_attr_st {
    aliot_shadow_datamode_t mode;       ///< data mode
    const char *pattr_name;             ///< attribute name
    void *pattr_data;                   ///< pointer to the attribute data
    aliot_shadow_attr_datatype_t attr_type;    ///< data type
    uint32_t timestamp;                 ///TODO < timestamp in Epoch & Unix Timestamp format.
    aliot_shadow_attr_cb_t callback;    ///< callback to be executed on receiving the Key value pair
}aliot_shadow_attr_t, *aliot_shadow_attr_pt;


typedef struct aliot_update_ack_wait_list_st {
    bool flag_busy; //0, free; 1, busy.
    char token[ALIOT_SHADOW_TOKEN_LEN]; //
    aliot_update_cb_fpt callback;
    aliot_time_t timer;
}aliot_update_ack_wait_list_t, *aliot_update_ack_wait_list_pt;


typedef struct aliot_inner_data_st {
    uint32_t token_num;
    uint32_t version; //todo what will happen if overflow.
    aliot_shadow_time_t time;

    aliot_update_ack_wait_list_t update_ack_wait_list[ALIOT_SHADOW_UPDATE_WAIT_ACK_LIST_NUM];
    list_t *attr_list;

    char *ptopic_update;
    char *ptopic_get;

}aliot_inner_data_t, *aliot_inner_data_pt;;


typedef struct {
    aliot_mqtt_param_t mqtt;
}aliot_shadow_para_t, *aliot_shadow_para_pt;


typedef struct aliot_shadow_st {
    MQTTClient_t mqtt;
    aliot_inner_data_t inner_data;
    ALIYUN_IOT_MUTEX_S mutex;
}aliot_shadow_t, *aliot_shadow_pt;


/**
 * @brief Construct the Device Shadow
 *
 * This function initialize the data structures, establish MQTT connection
 *   and subscribe the topic: "/shadow/get/${product_key}/${device_name}"
 *
 * @param pClient, A MQTT Client data structure.
 * @param pParams, The specific initial parameter.
 * @return An error code.
 */
aliot_err_t aliyun_iot_shadow_construct(aliot_shadow_pt pshadow, aliot_shadow_para_pt pParams);



void aliyun_iot_shadow_yield(aliot_shadow_pt pshadow, uint32_t timeout);

aliot_err_t aliyun_iot_shadow_deconstruct(aliot_shadow_pt pshadow);

aliot_err_t aliyun_iot_shadow_register_attribute(aliot_shadow_pt pshadow, aliot_shadow_attr_pt pattr);

aliot_err_t aliyun_iot_shadow_delete_attribute(aliot_shadow_pt pshadow, aliot_shadow_attr_pt pattr);

aliot_err_t aliyun_iot_shadow_update_format_init(format_data_pt pformat, char *buf, uint16_t size);

aliot_err_t aliyun_iot_shadow_update_format_add(format_data_pt pformat, aliot_shadow_attr_pt pattr);

aliot_err_t aliyun_iot_shadow_update_format_finalize(format_data_pt pformat);



aliot_err_t aliyun_iot_shadow_update(
                aliot_shadow_pt pshadow,
                char *data,
                uint32_t data_len,
                uint16_t timeout_s);


aliot_err_t aliyun_iot_shadow_update_asyn(
                aliot_shadow_pt pshadow,
                char *data,
                size_t data_len,
                uint16_t timeout_s,
                aliot_update_cb_fpt cb_fpt);


aliot_err_t aliyun_iot_shadow_sync( aliot_shadow_pt pshadow );


#endif /* INCLUDE_MQTT_ALIYUN_IOT_SHADOW_H_ */
