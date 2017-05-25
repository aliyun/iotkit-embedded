

#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_debug.h"
#include "aliyun_iot_common_timer.h"
#include "aliyun_iot_platform_pthread.h"
#include "aliyun_iot_platform_memory.h"

#include "aliyun_iot_shadow.h"
#include "aliyun_iot_shadow_common.h"

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


static aliot_shadow_pt paliot_device_shadow = NULL;
aliot_shadow_pt ads_common_get_ads(void)
{
    return paliot_device_shadow;
}

void ads_common_set_ads(aliot_shadow_pt pshadow)
{
    paliot_device_shadow = pshadow;
}

//return handle of format data.
aliot_err_t ads_common_format_init(format_data_pt pformat,
                char *buf,
                uint16_t size,
                const char *method,
                const char *head_str)
{
    int ret;
    uint32_t size_free_space;
    memset(pformat, 0, sizeof(format_data_t));

    pformat->buf = buf;
    pformat->buf_size = size;

    if (NULL == method) {
        return ERROR_SHADOW_NO_METHOD;
    }

    size_free_space = pformat->buf_size;

    ret = snprintf(pformat->buf,
            size_free_space,
            "{\"%s\":\"%s\"",
            "method",
            method);

    CHECK_SNPRINTF_RET(ret, size_free_space);
    pformat->offset = ret;


    //copy the JOSN head
    size_free_space = pformat->buf_size - pformat->offset;
    if (NULL != head_str) {
        ret = snprintf(pformat->buf + pformat->offset,
                    size_free_space,
                    ",%s",
                    head_str);
        CHECK_SNPRINTF_RET(ret, size_free_space);
        pformat->offset += ret;
    }

    pformat->flag_new = true;

    return SUCCESS_RETURN;
}


//加入需要上报的数据属性
aliot_err_t ads_common_format_add(format_data_pt pformat,
        const char *name,
        const void *pvalue,
        aliot_shadow_attr_datatype_t datatype)
{
    int ret;
    uint32_t size_free_space;

    if (pformat->flag_new) {
        pformat->flag_new = false;
    } else {
        //Add comma char.
        size_free_space = pformat->buf_size - pformat->offset;
        if (size_free_space > 1) { //there is enough space to accommodate ',' char.
            *(pformat->buf + pformat->offset) = ',';
            *(pformat->buf + pformat->offset + 1) = '\0';
            ++pformat->offset;
        } else {
            return FAIL_RETURN;
        }
    }

    size_free_space = pformat->buf_size - pformat->offset;

    //add the string: "${pattr->pattr_name}":"
    ret = snprintf(pformat->buf + pformat->offset,
            size_free_space,
            "\"%s\":",
            name);

    CHECK_SNPRINTF_RET(ret, size_free_space);

    pformat->offset += ret;
    size_free_space = pformat->buf_size - pformat->offset;

    //convert attribute data to JSON string, and add to buffer
    ret = ads_common_convert_data2string(pformat->buf + pformat->offset,
                size_free_space,
                datatype,
                pvalue);
    if (ret < 0) {
        return FAIL_RETURN;
    }

    pformat->offset += ret;



    return SUCCESS_RETURN;
}


//返回格式化后的数
aliot_err_t ads_common_format_finalize(format_data_pt pformat, const char *tail_str)
{
#define UPDATE_JSON_STR_END         ",\"clientToken\":\"%s-%d\",\"version\":%d}"

    int ret;
    uint16_t size_free_space = pformat->buf_size - pformat->offset;

    if (NULL != tail_str) {
        ret = snprintf(pformat->buf + pformat->offset, size_free_space, "%s", tail_str);
        CHECK_SNPRINTF_RET(ret, size_free_space);
        pformat->offset += ret;
    }

    size_free_space = pformat->buf_size - pformat->offset;

    ret = snprintf(pformat->buf + pformat->offset,
            size_free_space,
            UPDATE_JSON_STR_END,
            aliyun_iot_get_user_info()->client_id,
            ads_common_get_tokennum(ads_common_get_ads()),
            ads_common_get_version(ads_common_get_ads()));

    CHECK_SNPRINTF_RET(ret, size_free_space);
    pformat->offset += ret;

    return SUCCESS_RETURN;

#undef UPDATE_JSON_STR_END
}


int ads_common_convert_data2string(
                char *buf,
                size_t buf_len,
                aliot_shadow_attr_datatype_t type,
                const void *pData) {

    int ret = -1;

    if ((NULL == buf) || (buf_len == 0)
        || ((ALIOT_SHADOW_NULL != type) && (NULL == pData))) {
        return ERROR_NULL_VALUE;
    }

    if (ALIOT_SHADOW_INT32 == type) {
        ret = snprintf(buf, buf_len, "%" PRIi32, *(int32_t *)(pData));
    } else if (ALIOT_SHADOW_STRING == type) {
        ret = snprintf(buf, buf_len, "\"%s\"", (char *)(pData));
    } else if (ALIOT_SHADOW_NULL == type) {
        ret = snprintf(buf, buf_len, "%s", "\"null\"");
    } else {
        ALIOT_LOG_ERROR("Error data type");
        ret = -1;
    }

    if ((ret < 0) || (ret >= buf_len)) {
        return -1;
    }

    return ret;
}


aliot_err_t ads_common_convert_string2data(
                const char *buf,
                size_t buf_len,
                aliot_shadow_attr_datatype_t type,
                void *pdata)
{
    if ((NULL == buf) || (buf_len == 0) || (NULL == pdata)) {
        return ERROR_NULL_VALUE;
    }

    if (type == ALIOT_SHADOW_INT32) {
        if (0 == strcmp(pdata, "true")) {
            *((int32_t *)pdata) = 1;
        } else if (0 == strcmp(pdata, "false")) {
            *((int32_t *)pdata) = 0;
        } else if (0 == strcmp(pdata, "null")) {
            *((int32_t *)pdata) = 0;
        } else {
            *((int32_t *)pdata) = atoi(buf);
        }
    } else if (type == ALIOT_SHADOW_STRING) {
        memcpy(pdata, buf, buf_len);
    } else {
        ALIOT_LOG_ERROR("Error data type");
        return ERROR_SHADOW_UNDEF_TYPE;
    }

    return SUCCESS_RETURN;
}


void ads_common_update_time(aliot_shadow_pt pshadow, uint32_t new_timestamp)
{
    aliyun_iot_mutex_lock(&pshadow->mutex);
    pshadow->inner_data.time.base_system_time = aliot_time_get_s();
    pshadow->inner_data.time.epoch_time = new_timestamp;
    aliyun_iot_mutex_unlock(&pshadow->mutex);

    ALIOT_LOG_INFO("update system time");
}


bool ads_common_check_attr_existence(
            aliot_shadow_pt pshadow,
            aliot_shadow_attr_pt pattr)
{
    list_node_t *node;

    aliyun_iot_mutex_lock(&pshadow->mutex);
    node = list_find(pshadow->inner_data.attr_list, pattr);
    aliyun_iot_mutex_unlock(&pshadow->mutex);

    return (NULL != node);
}


//register attribute to list
aliot_err_t ads_common_register_attr (
                aliot_shadow_pt pshadow,
                aliot_shadow_attr_pt pattr)
{
    list_node_t *node = list_node_new( pattr );
    if (NULL == node) {
        return ERROR_NO_MEM;
    }

    aliyun_iot_mutex_lock(&pshadow->mutex);
    list_lpush(pshadow->inner_data.attr_list, node);
    aliyun_iot_mutex_unlock(&pshadow->mutex);

    return SUCCESS_RETURN;
}


//remove attribute to list
aliot_err_t ads_common_remove_attr (
                aliot_shadow_pt pshadow,
                aliot_shadow_attr_pt pattr)
{
    aliot_err_t rc = SUCCESS_RETURN;
    list_node_t *node;

    aliyun_iot_mutex_lock(&pshadow->mutex);
    node = list_find(pshadow->inner_data.attr_list, pattr);
    if (NULL == node) {
        rc = ERROR_SHADOW_NO_ATTRIBUTE;
        ALIOT_LOG_ERROR("Try to remove a non-existent attribute.");
    } else {
        list_remove(pshadow->inner_data.attr_list, node);
    }
    aliyun_iot_mutex_unlock(&pshadow->mutex);

    return rc;
}


void ads_common_update_version(aliot_shadow_pt pshadow, uint32_t version)
{
    aliyun_iot_mutex_lock(&pshadow->mutex);

    //version number always grow up
    if (version > pshadow->inner_data.version) {
         pshadow->inner_data.version = version;
    }
    aliyun_iot_mutex_unlock(&pshadow->mutex);

    ALIOT_LOG_INFO("update shadow version");
}



uint32_t ads_common_get_version(aliot_shadow_pt pshadow)
{
    uint32_t ver;
    aliyun_iot_mutex_lock(&pshadow->mutex);
    ++pshadow->inner_data.version;
    ver = pshadow->inner_data.version;
    ++pshadow->inner_data.version;
    aliyun_iot_mutex_unlock(&pshadow->mutex);
    return ver;
}


uint32_t ads_common_get_tokennum(aliot_shadow_pt pshadow)
{
    uint32_t ver;
    aliyun_iot_mutex_lock(&pshadow->mutex);
    ++pshadow->inner_data.token_num;
    ver = pshadow->inner_data.token_num;
    ++pshadow->inner_data.token_num;
    aliyun_iot_mutex_unlock(&pshadow->mutex);
    return ver;
}


char *ads_common_generate_topic_name(aliot_shadow_pt pshadow, const char *topic)
{
#define SHADOW_TOPIC_FMT      "/shadow/%s/%s/%s"
#define SHADOW_TOPIC_LEN      (PRODUCT_KEY_LEN + DEVICE_NAME_LEN)

    int len, ret;
    char *topic_full = NULL;
    aliot_device_info_pt pdevice_info = aliyun_iot_get_device_info( );

    len = SHADOW_TOPIC_LEN + sizeof(SHADOW_TOPIC_FMT);

    topic_full = aliyun_iot_memory_malloc(len + 1);
    if (NULL == topic_full) {
        return NULL;
    }

    ret = snprintf(topic_full,
                len,
                SHADOW_TOPIC_FMT,
                topic,
                pdevice_info->product_key,
                pdevice_info->device_name);
    if (ret < 0) {
        aliyun_iot_memory_free(topic_full);
        return NULL;
    }

    ALIOT_ASSERT(ret < len, "Memory should always enough.");

    return topic_full;
}


aliot_err_t ads_common_publish2update(aliot_shadow_pt pshadow, char *data, uint32_t data_len)
{
    MQTTMessage topic_msg;

    //check if topic name have been generated or not
    if (NULL == pshadow->inner_data.ptopic_update) {
        //Have NOT update topic name, generate it.
        pshadow->inner_data.ptopic_update = ads_common_generate_topic_name(pshadow, "update");
        if (NULL == pshadow->inner_data.ptopic_update) {
            return FAIL_RETURN;
        }
    }

    ALIOT_LOG_DEBUG("publish msg = %s", data);

    topic_msg.qos        = QOS1;
    topic_msg.retained   = FALSE_IOT;
    topic_msg.dup        = FALSE_IOT;
    topic_msg.payload    = (void *)data;
    topic_msg.payloadlen = data_len;
    topic_msg.id         = 0;

    return aliyun_iot_mqtt_publish(&pshadow->mqtt, pshadow->inner_data.ptopic_update, &topic_msg);
}
