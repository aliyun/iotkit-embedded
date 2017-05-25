
#include "aliyun_iot_common_debug.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_jsonparser.h"

#include "aliyun_iot_platform_pthread.h"
#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_platform_memory.h"

#include "aliyun_iot_mqtt_client.h"

#include "aliyun_iot_shadow.h"
#include "aliyun_iot_shadow_common.h"
#include "aliyun_iot_shadow_update.h"
#include "aliyun_iot_shadow_delta.h"


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


static void aliyun_iot_shadow_publish_success_cb(void* context, unsigned int msgId)
{
    ALIOT_LOG_DEBUG("publish message is arrived,id = %d\n", msgId);
}

static void aliyun_iot_shadow_subscribe_ack_timeout(SUBSCRIBE_INFO_S *subInfo)
{
    ALIOT_LOG_DEBUG("msgId = %d, sub ack is timeout\n", subInfo->msgId);
}

static void aliyun_iot_shadow_handle_expire(void)
{
    aliyun_iot_shadow_update_wait_ack_list_handle_expire( ads_common_get_ads() );
}


//当服务端通过/shadow/get下发时指令时，回调该函数。
static void aliyun_iot_shadow_callback_get(MessageData *msg)
{
    const char *pname;
    int val_len, val_type;
    aliot_shadow_pt pshadow;

    ALIOT_LOG_DEBUG("topic=%s", msg->topicName->cstring);
    ALIOT_LOG_DEBUG("data of topic=%-128.128s", (char *)msg->message->payload);

    if (NULL == (pshadow = ads_common_get_ads())) {
        ALIOT_LOG_ERROR("Error Flow! Please call 'aliyun_iot_shadow_construct()'");
        return;
    }

    //update time if there is 'timestamp' key in JSON string
    pname = json_get_value_by_name(msg->message->payload,
                    msg->message->payloadlen,
                    "timestamp",
                    &val_len,
                    &val_type);
    if (NULL != pname) {
        ads_common_update_time(pshadow, atoi(pname));
    }

    //update 'version' if there is 'version' key in JSON string
    pname = json_get_value_by_name(msg->message->payload,
                        msg->message->payloadlen,
                        "version",
                        &val_len,
                        &val_type);
    if (NULL != pname) {
        ads_common_update_version(pshadow, atoi(pname));
    }


    //get 'method'
    pname = json_get_value_by_name(msg->message->payload,
                    msg->message->payloadlen,
                    "method",
                    &val_len,
                    &val_type);
    if (NULL == pname) {
        ALIOT_LOG_ERROR("Invalid JSON document: not 'method' key");
    }

    if ((strlen("control") == val_len) && strcmp(pname, "control")) {
        //call delta handle function
        ALIOT_LOG_DEBUG("receive 'control' method");
        aliyun_iot_shadow_delta_entry(
                pshadow,
                msg->message->payload,
                msg->message->payloadlen);

    } else if ((strlen("reply") == val_len) && strcmp(pname, "reply")) {
        //call update ACK handle function.
        ALIOT_LOG_DEBUG("receive 'reply' method");
        aliyun_iot_shadow_update_wait_ack_list_handle_response(
                pshadow,
                msg->message->payload,
                msg->message->payloadlen);
    } else {
        ALIOT_LOG_ERROR("Invalid 'method' key");
    }

    ALIOT_LOG_DEBUG("End of method handle");
}

static aliot_err_t aliyun_iot_shadow_subcribe_get(aliot_shadow_pt pshadow)
{
    if (NULL == pshadow->inner_data.ptopic_get) {
        pshadow->inner_data.ptopic_get = ads_common_generate_topic_name(pshadow, "get");
        if (NULL == pshadow->inner_data.ptopic_get) {
            return FAIL_RETURN;
        }
    }

    return aliyun_iot_mqtt_subscribe(&pshadow->mqtt,
                pshadow->inner_data.ptopic_get,
                QOS1,
                aliyun_iot_shadow_callback_get);
}


//return handle of format data.
aliot_err_t aliyun_iot_shadow_update_format_init(format_data_pt pformat,
                char *buf,
                uint16_t size)
{
    return ads_common_format_init(pformat, buf, size, "update", ",\"state\":{\"reported\":{");
}


//加入需要上报的数据属性
aliot_err_t aliyun_iot_shadow_update_format_add(format_data_pt pformat, aliot_shadow_attr_pt pattr)
{
    return ads_common_format_add(pformat, pattr->pattr_name, pattr->pattr_data, pattr->attr_type);
}


//返回格式化后的数
aliot_err_t aliyun_iot_shadow_update_format_finalize(format_data_pt pformat)
{
    return ads_common_format_finalize(pformat, "}}");
}


aliot_err_t aliyun_iot_shadow_update_asyn(
                aliot_shadow_pt pshadow,
                char *data,
                size_t data_len,
                uint16_t timeout_s,
                aliot_update_cb_fpt cb_fpt)
{
    int rc = SUCCESS_RETURN;
    aliot_update_ack_wait_list_pt pelement;
    int val_len, val_type;
    const char *ptoken;

    if ((NULL == pshadow) || (NULL == data)) {
            return NULL_VALUE_ERROR;
    }

    if (!aliyun_iot_mqtt_is_connected(&pshadow->mqtt)) {
        ALIOT_LOG_ERROR("The MQTT connection must be established before UPDATE data.");
        return ERROR_SHADOW_INVALID_STATE;
    }

    /*Add to callback list */
    ptoken = json_get_value_by_name(data, data_len, "clientToken", &val_len, &val_type);
    ALIOT_ASSERT(NULL != ptoken, "Token should always exit.");

    pelement = aliyun_iot_shadow_update_wait_ack_list_add(pshadow, ptoken, val_len, cb_fpt, timeout_s);
    if (NULL == pelement) {
        return ERROR_SHADOW_WAIT_LIST_OVERFLOW;
    }

    if (SUCCESS_RETURN != (rc = ads_common_publish2update(pshadow, data, data_len))) {
        aliyun_iot_shadow_update_wait_ack_list_remove(pshadow, pelement);
        return rc;
    }

    return SUCCESS_RETURN;
}


static aliot_shadow_ack_code_t shadow_update_flag_ack = ALIOT_SHADOW_ACK_NONE;
static void aliot_update_ack_cb(
                aliot_shadow_ack_code_t ack_code,
                const char *ack_msg, //NOTE: NOT a string.
                uint32_t ack_msg_len)
{
    ALIOT_LOG_DEBUG("ack_code=%d, ack_msg=%-256.256s", ack_code, ack_msg);
    shadow_update_flag_ack = ack_code;
}


aliot_err_t aliyun_iot_shadow_update(
                aliot_shadow_pt pshadow,
                char *data,
                uint32_t data_len,
                uint16_t timeout_s)
{
    if ((NULL == pshadow) || (NULL == data)) {
        return NULL_VALUE_ERROR;
    }

    if (!aliyun_iot_mqtt_is_connected(&pshadow->mqtt)) {
        ALIOT_LOG_ERROR("The MQTT connection must be established before UPDATE data.");
        return ERROR_SHADOW_INVALID_STATE;
    }

    shadow_update_flag_ack = ALIOT_SHADOW_ACK_NONE;

    //update synchronously
    aliyun_iot_shadow_update_asyn(pshadow, data, data_len, timeout_s, aliot_update_ack_cb);

    //wait ACK
    //TODO #BUG It NOT support multiple call simultaneously.
    while(ALIOT_SHADOW_ACK_NONE == shadow_update_flag_ack) {
        aliyun_iot_shadow_yield(pshadow, 200);
    }

    if ((ALIOT_SHADOW_ACK_SUCCESS == shadow_update_flag_ack)
          || (ALIOT_SHADOW_ACK_ERR_SHADOW_DOCUMENT_IS_NULL == shadow_update_flag_ack)) {
        //It is not the error that device shadow document is null
        return SUCCESS_RETURN;
    } else if (ALIOT_SHADOW_ACK_TIMEOUT == shadow_update_flag_ack) {
        return ERROR_SHADOW_UPDATE_TIMEOUT;
    } else {
        return ERROR_SHADOW_UPDATE_NACK;
    }
}


aliot_err_t aliyun_iot_shadow_sync(aliot_shadow_pt pshadow)
{
#define SHADOW_SYNC_MSG_SIZE      (256)

    aliot_err_t ret;
    format_data_t format;

    ALIOT_LOG_INFO("Device Shadow sync start.");

    format.buf = aliyun_iot_memory_malloc(SHADOW_SYNC_MSG_SIZE);
    if (NULL == format.buf) {
        ALIOT_LOG_ERROR("Device Shadow sync failed");
        return ERROR_NO_MEM;
    }

    ads_common_format_init(&format, format.buf, SHADOW_SYNC_MSG_SIZE, "get", NULL);
    ads_common_format_finalize(&format, NULL);

    ret = aliyun_iot_shadow_update(pshadow, format.buf, format.offset, 10);

    aliyun_iot_memory_free(format.buf);

    ALIOT_LOG_INFO("Device Shadow sync success.");

    return ret;

#undef SHADOW_SYNC_MSG_SIZE
}


aliot_err_t aliyun_iot_shadow_construct(aliot_shadow_pt pshadow, aliot_shadow_para_pt pparams)
{
    int rc = 0;
    aliot_user_info_pt puser_info = aliyun_iot_get_user_info();

    //initialize shadow data
    memset(pshadow, 0x0, sizeof(aliot_shadow_t));

    if (0 != aliyun_iot_mutex_init(&(pshadow->mutex))){
        return FAIL_RETURN;
    }

    pparams->mqtt.deliveryCompleteFun = aliyun_iot_shadow_publish_success_cb;
    pparams->mqtt.subAckTimeOutFun = aliyun_iot_shadow_subscribe_ack_timeout;

    //initialize MQTT
    rc = aliyun_iot_mqtt_init(&pshadow->mqtt, &pparams->mqtt, puser_info);
    if (SUCCESS_RETURN != rc)
    {
        ALIOT_LOG_ERROR("aliyun_iot_mqtt_init failed ret = %d", rc);
        return rc;
    }

    rc = aliyun_iot_mqtt_connect(&pshadow->mqtt);
    if (SUCCESS_RETURN != rc)
    {
        aliyun_iot_mqtt_release(&pshadow->mqtt);
        ALIOT_LOG_ERROR("ali_iot_mqtt_connect failed ret = %d", rc);
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

    pshadow->inner_data.attr_list = list_new();
    if (NULL == pshadow->inner_data.attr_list) {
        return ERROR_NO_MEM;
    }

    //WRITE_IOT_DEBUG_LOG("list = %p", pshadow->inner_data.attr_list);
    //WRITE_IOT_DEBUG_LOG("Device shadow construct successfully");

    //TODO if failed, release resource.

    //restore shadow variable
    ads_common_set_ads(pshadow);

    return SUCCESS_RETURN;
}


void aliyun_iot_shadow_yield(aliot_shadow_pt pshadow, uint32_t timeout)
{
    aliyun_iot_mqtt_yield(&pshadow->mqtt, timeout);
    aliyun_iot_shadow_handle_expire();
}


aliot_err_t aliyun_iot_shadow_deconstruct(aliot_shadow_pt pshadow)
{

    if (NULL != pshadow->inner_data.ptopic_get) {
        aliyun_iot_memory_free(pshadow->inner_data.ptopic_get);
    }

    if (NULL != pshadow->inner_data.ptopic_update) {
        aliyun_iot_memory_free(pshadow->inner_data.ptopic_update);
    }

    aliyun_iot_mqtt_release(&pshadow->mqtt);

    ads_common_set_ads(NULL);

    memset(pshadow, 0, sizeof(aliot_shadow_t));

    return SUCCESS_RETURN;
}


aliot_err_t aliyun_iot_shadow_register_attribute(aliot_shadow_pt pshadow, aliot_shadow_attr_pt pattr)
{
    //check if already registered
    if (ads_common_check_attr_existence(pshadow, pattr)) {
        return ERROR_SHADOW_ATTR_EXIST;
    }

    if (SUCCESS_RETURN != ads_common_register_attr(pshadow, pattr)) {
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}


//Remove attribute from Device Shadow in cloud by delete method.
aliot_err_t aliyun_iot_shadow_delete_attribute(aliot_shadow_pt pshadow, aliot_shadow_attr_pt pattr)
{
#define SHADOW_DELETE_MSG_SIZE      (256)

    aliot_err_t ret;
    format_data_t format;

    if (!ads_common_check_attr_existence(pshadow, pattr)) {
        return ERROR_SHADOW_ATTR_NO_EXIST;
    }

    format.buf = aliyun_iot_memory_malloc(SHADOW_DELETE_MSG_SIZE);
    if (NULL == format.buf) {
        return ERROR_NO_MEM;
    }

    ads_common_format_init(&format, format.buf, SHADOW_DELETE_MSG_SIZE, "delete", ",\"state\":{\"reported\":{");
    ads_common_format_add(&format, pattr->pattr_name, NULL, ALIOT_SHADOW_NULL);
    ads_common_format_finalize(&format, "}}");

    ret = aliyun_iot_shadow_update(pshadow, format.buf, format.offset, 10);
    if (SUCCESS_RETURN != ret) {
        aliyun_iot_memory_free(format.buf);
        return ret;
    }

    aliyun_iot_memory_free(format.buf);

    return ads_common_remove_attr(pshadow, pattr);

#undef SHADOW_DELETE_MSG_SIZE
}
