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

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"

#include "shadow/aliyun_iot_shadow_common.h"
#include "shadow/aliyun_iot_shadow_config.h"

/**
 * @brief Thing Shadow Acknowledgment enum
 *
 * This enum type is use in the callback for the action response
 *
 */
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

typedef struct aliot_shadow_attr_st aliot_shadow_attr_t, *aliot_shadow_attr_pt;

typedef void (*aliot_shadow_attr_cb_t)(aliot_shadow_attr_pt pattr);

struct aliot_shadow_attr_st {
    aliot_shadow_datamode_t mode;       ///< data mode
    const char *pattr_name;             ///< attribute name
    void *pattr_data;                   ///< pointer to the attribute data
    aliot_shadow_attr_datatype_t attr_type;    ///< data type
    uint32_t timestamp;                 ///TODO < timestamp in Epoch & Unix Timestamp format.
    aliot_shadow_attr_cb_t callback;    ///< callback to be executed on receiving the Key value pair
};


//typedef struct {
//    bool flag_busy;
//    const char *pattr_name;
//    aliot_shadow_attr_pt pattr;
//    aliot_shadow_attr_cb_t callback;
//} aliot_shadow_attr_list_t, *aliot_shadow_attr_list_pt;


typedef struct aliot_update_ack_wait_list_st {
    bool flag_busy; //0, free; 1, busy.
    char token[ALIOT_SHADOW_TOKEN_LEN]; //
    aliot_update_cb_fpt callback;
    aliot_timer_t timer;
}aliot_update_ack_wait_list_t, *aliot_update_ack_wait_list_pt;






typedef struct aliot_inner_data_st {

    uint32_t token_num;
    uint32_t version; //todo what will happen if overflow.
    aliot_shadow_time_t time;

    aliot_update_ack_wait_list_t update_ack_wait_list[ALIOT_SHADOW_UPDATE_WAIT_ACK_LIST_NUM];
    //aliot_shadow_attr_list_pt atrr_list[ALIOT_SHADOW_UPDATE_ATTR_LIST_NUM];
    list_t *attr_list;

    char *ptopic_update;
    char *ptopic_get;

}aliot_inner_data_t, *aliot_inner_data_pt;;


typedef struct {

    aliot_mqtt_param_t mqtt;

}aliot_shadow_para_t, *aliot_shadow_para_pt;



//typedef struct aliot_shadow_st {
//
//    MQTTClient_t mqtt;
//    uint32_t token_num;
//    uint32_t version; //todo what will happen if overflow.
//    aliot_inner_data_t inner_data;
//    ALIYUN_IOT_MUTEX_S mutex;
//
//}aliot_shadow_t, *aliot_shadow_pt;


struct aliot_shadow_st{

    MQTTClient_t mqtt;

    aliot_inner_data_t inner_data;
    ALIYUN_IOT_MUTEX_S mutex;
};


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



/**
 * @brief Connect to the AWS IoT Thing Shadow service over MQTT
 *
 * This function does the TLSv1.2 handshake and establishes the MQTT connection
 *
 * @param pClient   MQTT Client used as the protocol layer
 * @param pParams   Shadow Conenction parameters like TLS cert location
 * @return An IoT Error Type defining successful/failed Connection
 */


/**
 * @brief Yield function to let the background tasks of MQTT and Shadow
 *
 * This function could be use in a separate thread waiting for the incoming messages, ensuring the connection is kept alive with the AWS Service.
 * It also ensures the expired requests of Shadow actions are cleared and Timeout callback is executed.
 * @note All callbacks ever used in the SDK will be executed in the context of this function.
 *
 * @param pClient   MQTT Client used as the protocol layer
 * @param timeout   in milliseconds, This is the maximum time the yield function will wait for a message and/or read the messages from the TLS buffer
 * @return An IoT Error Type defining successful/failed Yield
 */
void aliyun_iot_shadow_yield(aliot_shadow_pt pshadow, uint32_t timeout);



/**
 * @brief Disconnect from the AWS IoT Thing Shadow service over MQTT
 *
 * This will close the underlying TCP connection, MQTT connection will also be closed
 *
 * @param pClient   MQTT Client used as the protocol layer
 * @return An IoT Error Type defining successful/failed disconnect status
 */
aliot_err_t aliyun_iot_shadow_deconstruct(aliot_shadow_pt pshadow);



//aliot_
//NOTE: @attr must have enough long life.
//0, 【】针对云端主动下发时的回调
//1, register device attribute
//2,
aliot_err_t aliyun_iot_shadow_register_attribute(aliot_shadow_pt pshadow, aliot_shadow_attr_pt pattr);


//
aliot_err_t aliyun_iot_shadow_delete_attribute(aliot_shadow_pt pshadow, aliot_shadow_attr_pt pattr);


aliot_err_t aliyun_iot_shadow_update_format_init(format_data_pt pformat,
                char *buf,
                uint16_t size);

aliot_err_t aliyun_iot_shadow_update_format_add(format_data_pt pformat, aliot_shadow_attr_pt pattr);

//返回格式化后的数
aliot_err_t aliyun_iot_shadow_update_format_finalize(format_data_pt pformat);


//同步接口

//返回值:
//SHADOW_ACCEPTED, SHADOW_REJECTED
//

//将会清除
aliot_err_t aliyun_iot_shadow_update(
        aliot_shadow_pt pshadow,
        char *data,
        uint32_t data_len,
        uint16_t timeout_s);


//异步接口
aliot_err_t aliyun_iot_shadow_update_asyn(
        aliot_shadow_pt pshadow,
        char *data,
        size_t data_len,
        uint16_t timeout_s,
        aliot_update_cb_fpt cb_fpt);



//与云端设备影子同步数据
aliot_err_t aliyun_iot_shadow_sync( aliot_shadow_pt pshadow );

//TODO: 通过update({"method":"get"}), 触发GET-UPDATE-GET, 以完成全部操作
//流程:
//端(update:get)->云
//云(get:control)->端
//端(update:update)->云
//云(get:reply)->端


#endif /* INCLUDE_MQTT_ALIYUN_IOT_SHADOW_H_ */
