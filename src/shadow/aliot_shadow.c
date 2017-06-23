#include "aliot_platform.h"

#include "aliot_list.h"
#include "aliot_debug.h"
#include "aliot_error.h"
#include "aliot_jsonparser.h"

#include "aliot_mqtt_client.h"

#include "aliot_shadow.h"
#include "aliot_shadow_common.h"
#include "aliot_shadow_update.h"
#include "aliot_shadow_delta.h"


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


static void ads_handle_expire(aliot_shadow_pt pshadow)
{
    ads_update_wait_ack_list_handle_expire(pshadow);
}


//当服务端通过/shadow/get下发时指令时，回调该函数。
static void aliot_shadow_callback_get(aliot_shadow_pt pshadow, void *pclient, aliot_mqtt_event_msg_pt msg)
{
    const char *pname;
    int val_len, val_type;

    aliot_mqtt_topic_info_pt topic_info = (aliot_mqtt_topic_info_pt)msg->msg;

    ALIOT_LOG_DEBUG("topic=%.*s", topic_info->topic_len, topic_info->ptopic);
    ALIOT_LOG_DEBUG("data of topic=%.*s", topic_info->payload_len, (char *)topic_info->payload);

    //update time if there is 'timestamp' key in JSON string
    pname = json_get_value_by_name(topic_info->payload,
                                   topic_info->payload_len,
                                   "timestamp",
                                   &val_len,
                                   &val_type);
    if (NULL != pname) {
        ads_common_update_time(pshadow, atoi(pname));
    }

    //update 'version' if there is 'version' key in JSON string
    pname = json_get_value_by_name(topic_info->payload,
                                   topic_info->payload_len,
                                   "version",
                                   &val_len,
                                   &val_type);
    if (NULL != pname) {
        ads_common_update_version(pshadow, atoi(pname));
    }


    //get 'method'
    pname = json_get_value_by_name(topic_info->payload,
                                   topic_info->payload_len,
                                   "method",
                                   &val_len,
                                   &val_type);
    if (NULL == pname) {
        ALIOT_LOG_ERROR("Invalid JSON document: not 'method' key");
    } else if ((strlen("control") == val_len) && strcmp(pname, "control")) {
        //call delta handle function
        ALIOT_LOG_DEBUG("receive 'control' method");

        aliot_shadow_delta_entry(
                    pshadow,
                    topic_info->payload,
                    topic_info->payload_len);

    } else if ((strlen("reply") == val_len) && strcmp(pname, "reply")) {
        //call update ACK handle function.
        ALIOT_LOG_DEBUG("receive 'reply' method");
        ads_update_wait_ack_list_handle_response(
                    pshadow,
                    topic_info->payload,
                    topic_info->payload_len);
    } else {
        ALIOT_LOG_ERROR("Invalid 'method' key");
    }

    ALIOT_LOG_DEBUG("End of method handle");
}

static aliot_err_t aliot_shadow_subcribe_get(aliot_shadow_pt pshadow)
{
    if (NULL == pshadow->inner_data.ptopic_get) {
        pshadow->inner_data.ptopic_get = ads_common_generate_topic_name(pshadow, "get");
        if (NULL == pshadow->inner_data.ptopic_get) {
            return FAIL_RETURN;
        }
    }

    return aliot_mqtt_subscribe(pshadow->mqtt,
                                 pshadow->inner_data.ptopic_get,
                                 ALIOT_MQTT_QOS1,
                                 (aliot_mqtt_event_handle_func_fpt)aliot_shadow_callback_get,
                                 pshadow);
}


aliot_err_t aliot_shadow_update_format_init(void *pshadow,
                format_data_pt pformat,
                char *buf,
                uint16_t size)
{
    return ads_common_format_init((aliot_shadow_pt)pshadow, pformat, buf, size, "update", "\"state\":{\"reported\":{");
}


aliot_err_t aliot_shadow_update_format_add(void *pshadow,
                format_data_pt pformat,
                aliot_shadow_attr_pt pattr)
{
    return ads_common_format_add((aliot_shadow_pt)pshadow, pformat, pattr->pattr_name, pattr->pattr_data, pattr->attr_type);
}


aliot_err_t aliot_shadow_update_format_finalize(void *pshadow, format_data_pt pformat)
{
    return ads_common_format_finalize((aliot_shadow_pt)pshadow, pformat, "}}");
}


aliot_err_t aliot_shadow_update_asyn(
                void *handle,
                char *data,
                size_t data_len,
                uint16_t timeout_s,
                aliot_update_cb_fpt cb_fpt)
{
    int rc = SUCCESS_RETURN;
    aliot_update_ack_wait_list_pt pelement;
    int val_len, val_type;
    const char *ptoken, *pmethod;
    aliot_shadow_pt pshadow = (aliot_shadow_pt)handle;

    if ((NULL == handle) || (NULL == data)) {
        return NULL_VALUE_ERROR;
    }

    if (!aliot_mqtt_check_state_normal(pshadow->mqtt)) {
        ALIOT_LOG_ERROR("The MQTT connection must be established before UPDATE data.");
        return ERROR_SHADOW_INVALID_STATE;
    }

    /*Add to callback list */
    pmethod = json_get_value_by_name(data, data_len, "method", &val_len, &val_type);
    ptoken = json_get_value_by_name(data, data_len, "clientToken", &val_len, &val_type);
    ALIOT_ASSERT(NULL != ptoken, "Token should always exist.");

    pelement = aliot_shadow_update_wait_ack_list_add(pshadow, ptoken, val_len, cb_fpt, timeout_s);
    if (NULL == pelement) {
        return ERROR_SHADOW_WAIT_LIST_OVERFLOW;
    }

    if ((rc = ads_common_publish2update(pshadow, data, data_len)) < 0) {
        aliot_shadow_update_wait_ack_list_remove(pshadow, pelement);
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
    ALIOT_LOG_DEBUG("ack_code=%d, ack_msg=%.*s", ack_code, ack_msg_len, ack_msg);
    shadow_update_flag_ack = ack_code;
}


aliot_err_t aliot_shadow_update(
            void *handle,
            char *data,
            uint32_t data_len,
            uint16_t timeout_s)
{
    aliot_shadow_pt pshadow = (aliot_shadow_pt)handle;

    if ((NULL == pshadow) || (NULL == data)) {
        return NULL_VALUE_ERROR;
    }

    if (!aliot_mqtt_check_state_normal(pshadow->mqtt)) {
        ALIOT_LOG_ERROR("The MQTT connection must be established before UPDATE data.");
        return ERROR_SHADOW_INVALID_STATE;
    }

    shadow_update_flag_ack = ALIOT_SHADOW_ACK_NONE;

    //update asynchronously
    aliot_shadow_update_asyn(pshadow, data, data_len, timeout_s, aliot_update_ack_cb);

    //wait ACK
    //TODO #BUG It NOT support multiple call simultaneously.
    while (ALIOT_SHADOW_ACK_NONE == shadow_update_flag_ack) {
        aliot_shadow_yield(pshadow, 200);
    }

    if ((ALIOT_SHADOW_ACK_SUCCESS == shadow_update_flag_ack)
        || (ALIOT_SHADOW_ACK_ERR_SHADOW_DOCUMENT_IS_NULL == shadow_update_flag_ack)) {
        //It is not the error that device shadow document is null
        ALIOT_LOG_INFO("update success.");
        return SUCCESS_RETURN;
    } else if (ALIOT_SHADOW_ACK_TIMEOUT == shadow_update_flag_ack) {
        ALIOT_LOG_INFO("update timeout.");
        return ERROR_SHADOW_UPDATE_TIMEOUT;
    } else {
        ALIOT_LOG_INFO("update negative ack.");
        return ERROR_SHADOW_UPDATE_NACK;
    }
}


aliot_err_t aliot_shadow_sync(void *handle)
{
#define SHADOW_SYNC_MSG_SIZE      (256)

    aliot_err_t ret;
    void *buf;
    format_data_t format;
    aliot_shadow_pt pshadow = (aliot_shadow_pt)handle;

    ALIOT_LOG_INFO("Device Shadow sync start.");

    buf = aliot_platform_malloc(SHADOW_SYNC_MSG_SIZE);
    if (NULL == buf) {
        ALIOT_LOG_ERROR("Device Shadow sync failed");
        return ERROR_NO_MEM;
    }

    ads_common_format_init(pshadow, &format, buf, SHADOW_SYNC_MSG_SIZE, "get", NULL);
    ads_common_format_finalize(pshadow, &format, NULL);

    ret = aliot_shadow_update(pshadow, format.buf, format.offset, 10);
    if (SUCCESS_RETURN == ret) {
        ALIOT_LOG_INFO("Device Shadow sync success.");
    } else {
        ALIOT_LOG_INFO("Device Shadow sync failed.");
    }

    aliot_platform_free(buf);
    aliot_platform_msleep(1000);

    return ret;

#undef SHADOW_SYNC_MSG_SIZE
}


void *aliot_shadow_construct(aliot_shadow_para_pt pparams)
{
    int rc = 0;
    aliot_shadow_pt pshadow = NULL;

    //initialize shadow
    if (NULL == (pshadow = aliot_platform_malloc(sizeof(aliot_shadow_t)))) {
        ALIOT_LOG_ERROR("Not enough memory");
        return NULL;
    }
    memset(pshadow, 0x0, sizeof(aliot_shadow_t));

    if (NULL == (pshadow->mutex = aliot_platform_mutex_create())) {
        ALIOT_LOG_ERROR("create mutex failed");
        goto do_exit;
    }

    //TODO
    //pparams->mqtt.handle_event

    //construct MQTT client
    if (NULL == (pshadow->mqtt = aliot_mqtt_construct(&pparams->mqtt))) {
        ALIOT_LOG_ERROR("construct MQTT failed");
        goto do_exit;
    }

    rc = aliot_shadow_subcribe_get(pshadow);
    if (rc < 0) {
        ALIOT_LOG_ERROR("subscribe 'get' topic fialed, rc=%d", rc);
        goto do_exit;
    }

    //TODO
    aliot_platform_msleep(2000);



    pshadow->inner_data.attr_list = list_new();
    if (NULL == pshadow->inner_data.attr_list) {
        ALIOT_LOG_ERROR("new list failed");
        goto do_exit;
    }

    return pshadow;

do_exit:
    aliot_shadow_deconstruct(pshadow);

    return NULL;
}


void aliot_shadow_yield(void *handle, uint32_t timeout)
{
    aliot_shadow_pt pshadow = (aliot_shadow_pt)handle;
    aliot_mqtt_yield(pshadow->mqtt, timeout);
    ads_handle_expire(pshadow);
}


aliot_err_t aliot_shadow_deconstruct(void *handle)
{
    aliot_shadow_pt pshadow = (aliot_shadow_pt) handle;

    if (NULL != pshadow->mqtt) {
        if (NULL != pshadow->inner_data.ptopic_get) {
            aliot_mqtt_unsubscribe(pshadow->mqtt, pshadow->inner_data.ptopic_get);
        }

        aliot_platform_msleep(2000);
        aliot_mqtt_deconstruct(pshadow->mqtt);
    }

    if (NULL != pshadow->inner_data.ptopic_get) {
        aliot_platform_free(pshadow->inner_data.ptopic_get);
    }

    if (NULL != pshadow->inner_data.ptopic_update) {
        aliot_platform_free(pshadow->inner_data.ptopic_update);
    }

    if (NULL != pshadow->inner_data.attr_list) {
        list_destroy(pshadow->inner_data.attr_list);
    }

    if (NULL != pshadow->mutex) {
        aliot_platform_mutex_destroy(pshadow->mutex);
    }

    aliot_platform_free(handle);

    return SUCCESS_RETURN;
}


aliot_err_t aliot_shadow_register_attribute(void *handle, aliot_shadow_attr_pt pattr)
{
    //check if already registered
    if (ads_common_check_attr_existence((aliot_shadow_pt)handle, pattr)) {
        return ERROR_SHADOW_ATTR_EXIST;
    }

    if (SUCCESS_RETURN != ads_common_register_attr((aliot_shadow_pt)handle, pattr)) {
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}


//Remove attribute from Device Shadow in cloud by delete method.
aliot_err_t aliot_shadow_delete_attribute(void *handle, aliot_shadow_attr_pt pattr)
{
#define SHADOW_DELETE_MSG_SIZE      (256)

    aliot_err_t ret;
    void *buf;
    format_data_t format;
    aliot_shadow_pt pshadow = (aliot_shadow_pt) handle;

    if (!ads_common_check_attr_existence(pshadow, pattr)) {
        return ERROR_SHADOW_ATTR_NO_EXIST;
    }

    buf = aliot_platform_malloc(SHADOW_DELETE_MSG_SIZE);
    if (NULL == buf) {
        return ERROR_NO_MEM;
    }

    ads_common_format_init(pshadow, &format, buf, SHADOW_DELETE_MSG_SIZE, "delete", ",\"state\":{\"reported\":{");
    ads_common_format_add(pshadow, &format, pattr->pattr_name, NULL, ALIOT_SHADOW_NULL);
    ads_common_format_finalize(pshadow, &format, "}}");

    ret = aliot_shadow_update(pshadow, format.buf, format.offset, 10);
    if (SUCCESS_RETURN != ret) {
        aliot_platform_free(buf);
        return ret;
    }

    aliot_platform_free(buf);

    return ads_common_remove_attr(pshadow, pattr);

#undef SHADOW_DELETE_MSG_SIZE
}
