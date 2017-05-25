
#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_platform_memory.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_common_jsonparser.h"
#include "aliyun_iot_common_list.h"
#include "aliyun_iot_shadow_delta.h"

static aliot_err_t aliyun_iot_shadow_delta_response(aliot_shadow_pt pshadow)
{
#define ALIOT_SHADOW_DELTA_VERSION_LEN  (11)
#define ALIOT_SHADOW_DELTA_RESPONSE_FMT "{\"method\":\"update\",\"state\":{\"desired\":\"null\"},\"version\":%d}"

    aliot_err_t rc;
    char *resp_data;
    int len = sizeof(ALIOT_SHADOW_DELTA_RESPONSE_FMT) + ALIOT_SHADOW_DELTA_VERSION_LEN;

    resp_data = (char *)aliyun_iot_memory_malloc(len);
    if (NULL == resp_data) {
        return ERROR_NO_MEM;
    }

    rc = snprintf(resp_data,
            len,
            ALIOT_SHADOW_DELTA_RESPONSE_FMT,
            ads_common_get_version(pshadow));
    if ((rc < 0) || (rc >= len)) {
        return FAIL_RETURN;
    }

    rc = ads_common_publish2update(pshadow, resp_data, rc);

    return rc;
}


aliot_err_t aliyun_iot_shadow_delta_init(aliot_shadow_pt pshadow)
{
    return SUCCESS_RETURN;
}


static uint32_t aliyun_iot_shadow_get_timestamp(const char *pmetadata_desired,
                    size_t len_metadata_desired,
                    const char *pname)
{
    const char *pdata;
    int len;

    //attribute be matched, and then get timestamp
    pdata = json_get_value_by_fullname(pmetadata_desired,
                len_metadata_desired,
                pname,
                &len,
                NULL);

    if (NULL != pdata) {
        pdata = json_get_value_by_fullname(pdata,
                    len,
                    "timestamp",
                    &len,
                    NULL);

        if (NULL != pdata) {
            return atoi(pdata);
        }
    }

    ALIOT_LOG_ERROR("NOT timestamp in JSON doc");
    return 0;
}


static aliot_err_t aliyun_iot_shadow_delta_update_attr_value(
                        aliot_shadow_attr_pt pattr,
                        const char *pvalue,
                        size_t value_len)
{
    return ads_common_convert_string2data(pvalue, value_len, pattr->attr_type, pattr->pattr_data);
}

//handle response ACK of UPDATE
void aliyun_iot_shadow_delta_entry(
        aliot_shadow_pt pshadow,
        char *json_doc,
        size_t json_doc_len)
{
    const char *key_metadata;
    const char *pstate, *pmetadata, *pvalue;
    int len_state, len_metadata, len;
    aliot_shadow_attr_pt pattr;
    list_iterator_t *iter;
    list_node_t *node;


    if (NULL != (pstate = json_get_value_by_fullname(json_doc,
                                json_doc_len,
                                "payload.state.desired",
                                &len_state,
                                NULL))) {
        key_metadata = "payload.metadata.desired";
    } else {
        //if have not desired key, get reported key instead.
        key_metadata = "payload.metadata.reported";
        pstate = json_get_value_by_fullname(json_doc,
                        json_doc_len,
                        "payload.state.reported",
                        &len_state,
                        NULL);
    }

    pmetadata = json_get_value_by_fullname(json_doc,
                    json_doc_len,
                    key_metadata,
                    &len_metadata,
                    NULL);

    if ((NULL == pstate) || (NULL == pmetadata)) {
        ALIOT_LOG_ERROR("Invalid 'control' JSON Doc");
        return;
    }

    //Iterate the list and check JSON document according to list_node.val.pattr_name
    //If the attribute be found, call the function registered by calling aliyun_iot_shadow_delta_register_attr()

    iter = list_iterator_new(pshadow->inner_data.attr_list, LIST_TAIL);
    if (NULL == iter) {
        ALIOT_LOG_WARN("Allocate memory failed");
        return ;
    }

    while (node = list_iterator_next(iter), NULL != node) {
        pattr = (aliot_shadow_attr_pt)node->val;
        pvalue = json_get_value_by_fullname(pstate, len_state, pattr->pattr_name, &len, NULL);

        //check if match attribute or not be matched
        if (NULL != pvalue) { //attribute be matched
            //get timestamp
            pattr->timestamp = aliyun_iot_shadow_get_timestamp(
                                    pmetadata,
                                    len_metadata,
                                    pattr->pattr_name);

            //convert string of JSON value according to destination data type.
            if (SUCCESS_RETURN != aliyun_iot_shadow_delta_update_attr_value(pattr, pvalue, len)) {
                ALIOT_LOG_WARN("Update attribute value failed.");
            }

            if (NULL != pattr->callback) {
                //call related callback function
                pattr->callback(pattr);
            }
        }
    }

    list_iterator_destroy(iter);

    //generate ACK and publish to @update topic using QOS1
    aliyun_iot_shadow_delta_response(pshadow);
}

