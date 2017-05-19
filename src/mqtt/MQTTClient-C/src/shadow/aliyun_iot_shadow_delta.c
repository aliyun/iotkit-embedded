/*
 * aliyun_iot_shadow_delta.c
 *
 *  Created on: May 17, 2017
 *      Author: qibiao.wqb
 */

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_list.h"
#include "shadow/aliyun_iot_shadow_delta.h"


aliot_err_t aliyun_iot_shadow_delta_init(aliot_shadow_pt pshadow)
{
    pshadow->inner_data.attr_list = list_new();
    if (NULL == pshadow->inner_data.attr_list) {
        return ERROR_NO_MEM;
    }

    return SUCCESS_RETURN;
}


bool aliyun_iot_shadow_delta_check_existence(const char *attr_name)
{
    return 1;
}


//register attribute to list
int aliyun_iot_shadow_delta_register_attr(
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


//handle response ACK of UPDATE
void aliyun_iot_shadow_delta_entry(
        aliot_shadow_pt pshadow,
        char *json_doc,
        size_t json_doc_len)
{
    //Iterate the list and check JSON document according to list_node.val.pattr_name
    //If the attribute be found, call the function registered by calling aliyun_iot_shadow_delta_register_attr()


    //generate ACK and publish to @update topic using QOS1


}



