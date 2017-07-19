
#include "aliot_platform.h"

#include "lite/lite-log.h"
#include "lite/lite-utils.h"
#include "utils_list.h"
#include "aliot_shadow_delta.h"

static aliot_err_t aliot_shadow_delta_response(aliot_shadow_pt pshadow)
{
#define ALIOT_SHADOW_DELTA_RESPONSE_LEN     (256)

    aliot_err_t rc;
    void *buf;
    format_data_t format;

    buf = aliot_platform_malloc(ALIOT_SHADOW_DELTA_RESPONSE_LEN);
    if (NULL == buf) {
        return ERROR_NO_MEM;
    }

    ads_common_format_init(pshadow, &format, buf, ALIOT_SHADOW_DELTA_RESPONSE_LEN, "update", "\"state\":{\"desired\":\"null\"");
    ads_common_format_finalize(pshadow, &format, "}");

    rc = ads_common_publish2update(pshadow, format.buf, format.offset);

    aliot_platform_free(buf);

    return (rc >= 0) ? SUCCESS_RETURN : rc;
}



static uint32_t aliot_shadow_get_timestamp(const char *pmetadata_desired,
        size_t len_metadata_desired,
        const char *pname)
{
    const char *pdata;

    //attribute be matched, and then get timestamp

    pdata = LITE_json_value_of(pname, pmetadata_desired);

    if (NULL != pdata) {
        pdata = LITE_json_value_of("timestamp", pdata);
        if (NULL != pdata) {
            return atoi(pdata);
        }
    }

    log_err("NOT timestamp in JSON doc");
    return 0;
}


static aliot_err_t aliot_shadow_delta_update_attr_value(
            aliot_shadow_attr_pt pattr,
            const char *pvalue,
            size_t value_len)
{
    return ads_common_convert_string2data(pvalue, value_len, pattr->attr_type, pattr->pattr_data);
}


static void aliot_shadow_delta_update_attr(aliot_shadow_pt pshadow,
                const char *json_doc_attr,
                uint32_t json_doc_attr_len,
                const char *json_doc_metadata,
                uint32_t json_doc_metadata_len)
{
    const char *pvalue;
    aliot_shadow_attr_pt pattr;
    list_iterator_t *iter;
    list_node_t *node;

    //Iterate the list and check JSON document according to list_node.val.pattr_name
    //If the attribute be found, call the function registered by calling aliot_shadow_delta_register_attr()

    aliot_platform_mutex_lock(pshadow->mutex);
    iter = list_iterator_new(pshadow->inner_data.attr_list, LIST_TAIL);
    if (NULL == iter) {
        aliot_platform_mutex_unlock(pshadow->mutex);
        log_warning("Allocate memory failed");
        return ;
    }

    while (node = list_iterator_next(iter), NULL != node) {
        pattr = (aliot_shadow_attr_pt)node->val;
        pvalue = LITE_json_value_of(pattr->pattr_name, json_doc_attr);

        //check if match attribute or not be matched
        if (NULL != pvalue) { //attribute be matched
            //get timestamp
            pattr->timestamp = aliot_shadow_get_timestamp(
                                    json_doc_metadata,
                                    json_doc_metadata_len,
                                    pattr->pattr_name);

            //convert string of JSON value according to destination data type.
            if (SUCCESS_RETURN != aliot_shadow_delta_update_attr_value(pattr, pvalue, strlen(pvalue))) {
                log_warning("Update attribute value failed.");
            }

            if (NULL != pattr->callback) {
                aliot_platform_mutex_unlock(pshadow->mutex);
                //call related callback function
                pattr->callback(pattr);
                aliot_platform_mutex_lock(pshadow->mutex);
            }
        }
    }

    list_iterator_destroy(iter);
    aliot_platform_mutex_unlock(pshadow->mutex);
}

//handle response ACK of UPDATE
void aliot_shadow_delta_entry(
            aliot_shadow_pt pshadow,
            const char *json_doc,
            size_t json_doc_len)
{
    const char *key_metadata;
    const char *pstate, *pmetadata, *pvalue;

    pstate = LITE_json_value_of("payload.state.desired", json_doc);
    if (NULL != pstate) {
        key_metadata = "payload.metadata.desired";
    } else {
        //if have not desired key, get reported key instead.
        key_metadata = "payload.metadata.reported";
        pstate = LITE_json_value_of("payload.state.reported", json_doc);
    }

    pmetadata = LITE_json_value_of(key_metadata, json_doc);

    if ((NULL == pstate) || (NULL == pmetadata)) {
        log_err("Invalid JSON Doc");
        return;
    }

    aliot_shadow_delta_update_attr(pshadow,
                pstate,
                strlen(pstate),
                pmetadata,
                strlen(pmetadata));

    LITE_free(pstate);
    LITE_free(pmetadata);

    //generate ACK and publish to @update topic using QOS1
    aliot_shadow_delta_response(pshadow);
}

