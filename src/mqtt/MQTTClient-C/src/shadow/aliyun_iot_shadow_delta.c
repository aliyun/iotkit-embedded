/*
 * aliyun_iot_shadow_delta.c
 *
 *  Created on: May 17, 2017
 *      Author: qibiao.wqb
 */

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_jsonparser.h"
#include "aliyun_iot_common_list.h"
#include "shadow/aliyun_iot_shadow_delta.h"

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

    ads_common_increase_version(pshadow);

    return ads_common_publish2update(pshadow, resp_data, rc);
}


aliot_err_t aliyun_iot_shadow_delta_init(aliot_shadow_pt pshadow)
{
    return SUCCESS_RETURN;
}



static uint32_t aliyun_iot_shadow_get_timestamp(char *pmetadata_desired,
                size_t len_metadata_desired,
                const char *pname)
{
    char *pdata;
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

    WRITE_IOT_ERROR_LOG("NOT timestamp in JSON doc");
    return 0;
}


static aliot_err_t aliyun_iot_shadow_delta_update_attr_value(aliot_shadow_attr_pt pattr, char *pvalue, size_t value_len)
{
    return ads_common_convert_string2data(pvalue, value_len, pattr->attr_type, pattr->pattr_data);
}

//handle response ACK of UPDATE
void aliyun_iot_shadow_delta_entry(
        aliot_shadow_pt pshadow,
        char *json_doc,
        size_t json_doc_len)
{
    int len_state_desired, len_metadata_desired, len;
    char *pstate_desired, *pmetadata_desired, *pvalue;
    aliot_shadow_attr_pt pattr;

    list_iterator_t *iter;
    list_node_t *node;

    //TODO
//    pdata = json_get_value_by_fullname(json_doc,
//                json_doc_len,
//                "payload.status",
//                &len,
//                NULL);
//    if ((NULL == pdata) || (0 != strncmp("success", pdata, len))){
//
//        return ;
//    }

    pstate_desired = json_get_value_by_fullname(json_doc,
                        json_doc_len,
                        "payload.state.desired",
                        &len_state_desired,
                        NULL);

    pmetadata_desired = json_get_value_by_fullname(json_doc,
                        json_doc_len,
                        "payload.state.metadata",
                        &len_metadata_desired,
                        NULL);

    if ((NULL == pstate_desired) || (NULL == pmetadata_desired)) {
        WRITE_IOT_ERROR_LOG("Invalid 'control' JSON Doc");
        return;
    }

    //Iterate the list and check JSON document according to list_node.val.pattr_name
    //If the attribute be found, call the function registered by calling aliyun_iot_shadow_delta_register_attr()

    iter = list_iterator_new(pshadow->inner_data.attr_list, LIST_TAIL);
    if (NULL == iter) {
        WRITE_IOT_WARNING_LOG("Allocate memory failed");
        return ;
    }

    while (node = list_iterator_next(iter), NULL != node) {
        pattr = (aliot_shadow_attr_pt)node->val;
        pvalue = json_get_value_by_fullname(pstate_desired, len_state_desired, pattr->pattr_name, &len, NULL);

        //check if macch attribute or not be matched
        if (NULL != pvalue) { //attribute be matched
            //get timestamp
            pattr->timestamp = aliyun_iot_shadow_get_timestamp(
                                    pmetadata_desired,
                                    len_metadata_desired,
                                    pattr->pattr_name);

            //convert string of JSON value according to destination data type.
            if (SUCCESS_RETURN != aliyun_iot_shadow_delta_update_attr_value(pattr, pvalue, len)) {
                WRITE_IOT_WARNING_LOG("Update attribute value failed.");
            }

            //call related callback function
            pattr->callback(pattr);
        }
    }

    list_iterator_destroy(iter);

    //generate ACK and publish to @update topic using QOS1
    aliyun_iot_shadow_delta_response(pshadow);
}

