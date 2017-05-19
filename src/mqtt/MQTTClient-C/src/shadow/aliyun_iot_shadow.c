/*
 * aliyun_iot_shadow.c
 *
 *  Created on: May 15, 2017
 *      Author: qibiao.wqb
 */

#include "aliyun_iot_common_debug.h"
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_jsonparser.h"

#include "aliyun_iot_platform_pthread.h"

#include "shadow/aliyun_iot_shadow.h"
#include "shadow/aliyun_iot_shadow_update.h"
#include "shadow/aliyun_iot_shadow_delta.h"


#define UPDATE_JSON_STR_HEAD         "{\"method\":\"update\",\"state\":{\"reported\":{"
#define UPDATE_JSON_STR_TAIL         "}},\"clientToken\":\"%s-%d\",\"version\":%d}"

//check return code
#define CHECK_RETURN_CODE(ret_code) \
    do{ \
        if (ret_code < 0) { \
            return ret_code; \
        } \
    }while(0);


//check return code of snprintf
#define CHECK_SNPRINTF_RET(ret_code, buf_len) \
    do{ \
        if ((ret_code) < 0) { \
            return FAIL_RETURN; \
        } else if ((ret_code) >= (buf_len)) { \
            return ERROR_NO_ENOUGH_MEM; \
        } \
    }while(0);


aliot_shadow_pt pshadow_g = NULL;


static aliot_err_t convert_data2string(
        char *buf,
        size_t buf_len,
        aliot_shadow_datatype type,
        void *pData) {

    int32_t ret = 0;

    if((NULL == buf) || (buf_len == 0)) {
        return ERROR_NULL_VALUE;
    }

    if(type == ALIOT_SHADOW_INT32) {
        ret = snprintf(buf, buf_len, "%" PRIi32",", *(int32_t *)(pData));
    } else if(type == ALIOT_SHADOW_INT16) {
        ret = snprintf(buf, buf_len, "%" PRIi16",", *(int16_t *)(pData));
    } else if(type == ALIOT_SHADOW_INT8) {
        ret = snprintf(buf, buf_len, "%" PRIi8",", *(int8_t *)(pData));
    } else if(type == ALIOT_SHADOW_UINT32) {
        ret = snprintf(buf, buf_len, "%" PRIu32",", *(uint32_t *)(pData));
    } else if(type == ALIOT_SHADOW_UINT16) {
        ret = snprintf(buf, buf_len, "%" PRIu16",", *(uint16_t *)(pData));
    } else if(type == ALIOT_SHADOW_UINT8) {
        ret = snprintf(buf, buf_len, "%" PRIu8",", *(uint8_t *)(pData));
    } else if(type == ALIOT_SHADOW_DOUBLE) {
        ret = snprintf(buf, buf_len, "%f,", *(double *)(pData));
    } else if(type == ALIOT_SHADOW_FLOAT) {
        ret = snprintf(buf, buf_len, "%f,", *(float *)(pData));
    } else if(type == ALIOT_SHADOW_BOOL) {
        ret = snprintf(buf, buf_len, "%s,", *(bool *)(pData) ? "true" : "false");
    } else if(type == ALIOT_SHADOW_STRING) {
        ret = snprintf(buf, buf_len, "\"%s\",", (char *)(pData));
    }

    CHECK_SNPRINTF_RET(ret, buf_len);

    return ret;
}


static void aliyun_iot_shadow_handle_expire(void)
{

    aliyun_iot_shadow_update_wait_ack_list_handle_expire(pshadow_g);

}





//当服务端通过/shadow/get下发时指令时，回调该函数。
static void aliyun_iot_shadow_callback_get(MessageData *msg)
{
    char *pmethod;
    int val_len, val_type;

    WRITE_IOT_DEBUG_LOG("topic=%s", msg->topicName->cstring);
    WRITE_IOT_DEBUG_LOG("data of topic=%s", msg->message->payload);

    if (pshadow_g == NULL) {
        WRITE_IOT_ERROR_LOG("Error Flow! Please call 'aliyun_iot_shadow_construct()'");
        return;
    }

    pmethod = json_get_value_by_name(msg->message->payload,
                    msg->message->payloadlen,
                    "method",
                    &val_len, &val_type);

    if (NULL == pmethod) {
        WRITE_IOT_ERROR_LOG("Invalid JSON document: not 'method' key");
    }

    if ((strlen("control") == val_len) && strcmp(pmethod, "control")) {
        //call delta handle function
        aliyun_iot_shadow_delta_entry(
                pshadow_g,
                msg->message->payload,
                msg->message->payloadlen);

    } else if ((strlen("reply") == val_len) && strcmp(pmethod, "reply")) {
        //call update ACK handle function.
        aliyun_iot_shadow_update_wait_ack_list_handle_response(
                pshadow_g,
                msg->message->payload,
                msg->message->payloadlen);
    } else {
        WRITE_IOT_ERROR_LOG("Invalid 'method' key");
    }
}


static aliot_err_t aliyun_iot_shadow_subcribe_get(aliot_shadow_pt pshadow)
{
#define SHADOW_TOPIC_GET_FMT         "/shadow/get/%s/%s"
///< The fixed length of GET topic
#define SHADOW_TOPIC_GET_LEN         (PRODUCT_KEY_LEN + DEVICE_NAME_LEN + 14)

    int rc;
    char *shadow_get_topic = NULL;
    aliot_device_info_pt pdevice_info = aliyun_iot_get_device_info();
    aliot_user_info_pt puser_info = aliyun_iot_get_user_info();


    shadow_get_topic = aliyun_iot_memory_malloc(SHADOW_TOPIC_GET_LEN + 1);
    if (NULL == shadow_get_topic) {
        return ERROR_NO_MEM;
    }

    rc = snprintf(shadow_get_topic,
            SHADOW_TOPIC_GET_LEN + 1,
            SHADOW_TOPIC_GET_FMT,
            pdevice_info->product_key,
            pdevice_info->device_name );
    if (rc < 0) {
        rc = FAIL_RETURN;
        goto do_exit;
    } else if (rc >= SHADOW_TOPIC_GET_LEN + 1) {
        rc = ERROR_NO_ENOUGH_MEM;
        goto do_exit;
    }

    rc = aliyun_iot_mqtt_subscribe(&pshadow->mqtt, shadow_get_topic, QOS1, aliyun_iot_shadow_callback_get);
    if (SUCCESS_RETURN != rc) {
        goto do_exit;
    }


do_exit:
    if (NULL == shadow_get_topic) {
        aliyun_iot_memory_free(shadow_get_topic);
    }

    return SUCCESS_RETURN;
}

//return handle of format data.
int aliyun_iot_shadow_format_init(format_data_pt pformat, aliot_shadow_pt pshadow, char *buf, uint16_t size)
{
    int ret;
    memset(pformat, 0, sizeof(format_data_t));

    pformat->pshadow = pshadow;
    pformat->buf = buf;
    pformat->buf_size = size;

    //copy the JOSN head
    ret = snprintf(pformat->buf, pformat->buf_size, "%s", UPDATE_JSON_STR_HEAD);
    CHECK_SNPRINTF_RET(ret, pformat->buf_size);

    pformat->offset = ret;
    return 0;
}


//加入需要上报的数据属性
int aliyun_iot_shadow_format_add(format_data_pt pformat, aliot_shadow_attr_pt pattr)
{
    int ret;
    uint16_t size_free_space = pformat->buf_size - pformat->offset;

    //add the string: "${pattr->pattr_name}":"
    ret = snprintf(pformat->buf + pformat->offset,
            size_free_space,
            "\"%s\":\"",
            pattr->pattr_name);

    CHECK_SNPRINTF_RET(ret, size_free_space);

    pformat->offset += ret;
    size_free_space = pformat->buf_size - pformat->offset;

    //convert attribute data to JSON string, and add to buffer
    ret = convert_data2string(pformat->buf + pformat->offset,
            size_free_space,
            pattr->attr_type,
            pattr->pattr_data);
    if (ret < 0) {
        return ret;
    }
    pformat->offset += ret;

    size_free_space = pformat->buf_size - pformat->offset;

    if (ret <= 0) {
        return ERROR_NO_ENOUGH_MEM;
    }

    //add the last character: "
    pformat->buf[pformat->offset] = '\"';
    ++pformat->offset;

    return 0;
}


//返回格式化后的数
aliot_err_t aliyun_iot_shadow_format_finalize(format_data_pt pformat)
{
    int ret;
    uint16_t size_free_space = pformat->buf_size - pformat->offset;

    ret = snprintf(pformat->buf + pformat->offset,
            size_free_space,
            UPDATE_JSON_STR_TAIL,
            aliyun_iot_get_user_info()->client_id,
            pformat->pshadow->token_num,
            pformat->pshadow->version);

    CHECK_SNPRINTF_RET(ret, size_free_space);

    return SUCCESS_RETURN;
}


aliot_err_t aliyun_iot_shadow_update_asyn(
                aliot_shadow_pt pshadow,
                char *data,
                uint16_t timeout_s,
                aliot_update_cb_fpt cb_fpt)
{
    int rc = SUCCESS_RETURN;
    aliot_update_ack_wait_list_pt pelement;
    char *name;
    int val_len, val_type;
    char *ptoken;

    if ((NULL == pshadow) || (NULL == data)) {
            return NULL_VALUE_ERROR;
    }

    if (aliyun_iot_mqtt_is_connected(&pshadow->mqtt)) {
        WRITE_IOT_ERROR_LOG("The MQTT connection must be established before UPDATE data.");
        return ERROR_SHADOW_INVALID_STATE;
    }

    /*Add to callback list */
    ptoken = json_get_value_by_name((char *)data, strlen((char *)data), "clientToken", &val_len, &val_type);
    ALIOT_ASSERT(NULL != ptoken, NULL, NULL);

    pelement = aliyun_iot_shadow_update_wait_ack_list_add(pshadow, ptoken, val_len, cb_fpt, timeout_s);
    if (NULL == pelement) {
        return ERROR_SHADOW_WAIT_LIST_OVERFLOW;
    }

    if (SUCCESS_RETURN != (rc = aliyun_iot_shadow_publish2update(pshadow, data))) {
        aliyun_iot_shadow_update_wait_ack_list_remove(pshadow, pelement);
        return rc;
    }

    return SUCCESS_RETURN;
}


aliot_err_t aliyun_iot_shadow_update(
                aliot_shadow_pt pshadow,
                char *data,
                uint16_t timeout_s)
{
    if ((NULL == pshadow) || (NULL == data)) {
        return NULL_VALUE_ERROR;
    }

    if (aliyun_iot_mqtt_is_connected(&pshadow->mqtt)) {
        WRITE_IOT_ERROR_LOG("The MQTT connection must be established before UPDATE data.");
        return ERROR_SHADOW_INVALID_STATE;
    }

    //TODO

    return SUCCESS_RETURN;
}


aliot_err_t aliyun_iot_shadow_sync( aliot_shadow_pt pshadow )
{
    return SUCCESS_RETURN;
}




aliot_err_t aliyun_iot_shadow_construct(aliot_shadow_pt pshadow, aliot_shadow_para_pt pparams)
{
    int rc = 0;
    aliot_device_info_pt pdevice_info = aliyun_iot_get_device_info();
    aliot_user_info_pt puser_info = aliyun_iot_get_user_info();

    if (SUCCESS_RETURN != aliyun_iot_auth(pdevice_info, puser_info))
    {
        printf("run aliyun_iot_auth() error!\n");
        return FAIL_RETURN;
    }


    //initialize shadow data
    memset(&pshadow, 0x0, sizeof(aliot_shadow_pt));

    if (0 != aliyun_iot_mutex_init(&(pshadow->mutex))){
        return FAIL_RETURN;
    }


    //initialize MQTT
    rc = aliyun_iot_mqtt_init(&pshadow->mqtt, &pparams->mqtt, puser_info);
    if (SUCCESS_RETURN != rc)
    {
        printf("aliyun_iot_mqtt_init failed ret = %d\n", rc);
        return rc;
    }

    rc = aliyun_iot_mqtt_connect(&pshadow->mqtt);
    if (SUCCESS_RETURN != rc)
    {
        aliyun_iot_mqtt_release(&pshadow->mqtt);
        printf("ali_iot_mqtt_connect failed ret = %d\n", rc);
        return rc;
    }

    rc = aliyun_iot_shadow_subcribe_get(pshadow);
    if (SUCCESS_RETURN != rc) {
        return rc;
    }

    rc = aliyun_iot_shadow_delta_init(pshadow);
    if (SUCCESS_RETURN != rc) {
        return rc;
    }

    //Render the global
    pshadow_g = pshadow;

    return SUCCESS_RETURN;
}


void aliyun_iot_shadow_yield(aliot_shadow_pt pshadow, uint32_t timeout)
{

    aliyun_iot_mqtt_yield(&pshadow->mqtt, timeout);
    aliyun_iot_shadow_handle_expire();

}


aliot_err_t aliyun_iot_shadow_deconstruct(aliot_shadow_pt *pClient)
{
    return SUCCESS_RETURN;
}


aliot_err_t aliyun_iot_shadow_register_delta(aliot_shadow_pt pshadow, aliot_shadow_attr_pt attr)
{
    //TODO check if already registered

    if (aliyun_iot_shadow_delta_check_existence(attr->pattr_name))
    {
        return ERROR_SHADOW_DELTA_REPEAT_ATTR;
    }



    return SUCCESS_RETURN;
}

