
#include "aliot_platform.h"

#include "aliot_log.h"
#include "aliot_jsonparser.h"
#include "aliot_device.h"

#include "aliot_shadow_update.h"


//add a new wait element
//return: NULL, failed; others, pointer of element.
aliot_update_ack_wait_list_pt aliot_shadow_update_wait_ack_list_add(
            aliot_shadow_pt pshadow,
            const char *ptoken, //NOTE: this is NOT a string.
            size_t token_len,
            aliot_update_cb_fpt cb,
            uint32_t timeout)
{
    int i;
    aliot_update_ack_wait_list_pt list = pshadow->inner_data.update_ack_wait_list;

    aliot_platform_mutex_lock(pshadow->mutex);

    for (i = 0; i < ADS_UPDATE_WAIT_ACK_LIST_NUM; ++i) {
        if (0 == list[i].flag_busy) {
            list[i].flag_busy = 1;
            break;
        }
    }

    aliot_platform_mutex_unlock(pshadow->mutex);

    if (i >= ADS_UPDATE_WAIT_ACK_LIST_NUM) {
        return NULL;
    }

    list[i].callback = cb;

    if (token_len >= ADS_TOKEN_LEN) {
        ALIOT_LOG_WARN("token is too long.");
        token_len = ADS_TOKEN_LEN - 1;
    }
    memcpy(list[i].token, ptoken, token_len);
    list[i].token[token_len] = '\0';

    aliot_time_init(&list[i].timer);
    aliot_time_cutdown(&list[i].timer, timeout);

    ALIOT_LOG_DEBUG("Add update ACK list");

    return &list[i];
}


void aliot_shadow_update_wait_ack_list_remove(aliot_shadow_pt pshadow, aliot_update_ack_wait_list_pt element)
{
    aliot_platform_mutex_lock(pshadow->mutex);
    element->flag_busy = 0;
    memset(element, 0, sizeof(aliot_update_ack_wait_list_t));
    aliot_platform_mutex_unlock(pshadow->mutex);
}


void ads_update_wait_ack_list_handle_expire(aliot_shadow_pt pshadow)
{
    size_t i;

    aliot_update_ack_wait_list_pt pelement = pshadow->inner_data.update_ack_wait_list;

    aliot_platform_mutex_lock(pshadow->mutex);

    for (i = 0; i < ADS_UPDATE_WAIT_ACK_LIST_NUM; ++i) {
        if (0 != pelement[i].flag_busy) {
            if (aliot_time_is_expired(&pelement[i].timer)) {
                if (NULL != pelement[i].callback) {
                    pelement[i].callback(ALIOT_SHADOW_ACK_TIMEOUT, NULL, 0);
                }
                //free it.
                memset(&pelement[i], 0, sizeof(aliot_update_ack_wait_list_t));
            }
        }
    }

    aliot_platform_mutex_unlock(pshadow->mutex);
}


//handle response ACK of UPDATE
void ads_update_wait_ack_list_handle_response(
            aliot_shadow_pt pshadow,
            const char *json_doc,
            size_t json_doc_len)
{
    int data_len, payload_len, i;
    const char *pdata, *ppayload;
    aliot_update_ack_wait_list_pt pelement = pshadow->inner_data.update_ack_wait_list;

    //get token
    pdata = json_get_value_by_name(json_doc, (int)json_doc_len, "clientToken", &data_len, NULL);
    if (NULL == pdata) {
        ALIOT_LOG_WARN("Invalid JSON document: not 'clientToken' key");
        return;
    }

    ppayload = json_get_value_by_fullname(json_doc, (int)json_doc_len, "payload", &payload_len, NULL);
    if (NULL == ppayload) {
        ALIOT_LOG_WARN("Invalid JSON document: not 'payload' key");
        return;
    }

    aliot_platform_mutex_lock(pshadow->mutex);
    for (i = 0; i < ADS_UPDATE_WAIT_ACK_LIST_NUM; ++i) {

        if (0 != pelement[i].flag_busy) {
            //check the related
            if (((NULL == pdata) && (0 == strcmp("get", pelement[i].token)))
                 || (0 == memcmp(pdata, pelement[i].token, strlen(pelement[i].token)))) {
                aliot_platform_mutex_unlock(pshadow->mutex);
                ALIOT_LOG_DEBUG("token=%s", pelement[i].token);
                do {
                    pdata = json_get_value_by_fullname(ppayload, payload_len, "status", &data_len, NULL);
                    if (NULL == pdata) {
                        ALIOT_LOG_WARN("Invalid JSON document: not 'payload.status' key");
                        break;
                    }

                    if (0 == strncmp(pdata, "success", data_len)) {
                        pelement[i].callback(ALIOT_SHADOW_ACK_SUCCESS, NULL, 0);
                    } else if (0 == strncmp(pdata, "error", data_len)) {
                        aliot_shadow_ack_code_t ack_code;

                        pdata = json_get_value_by_fullname(ppayload, payload_len, "content.errorcode", &data_len, NULL);
                        if (NULL == pdata) {
                            ALIOT_LOG_WARN("Invalid JSON document: not 'content.errorcode' key");
                            break;
                        }
                        ack_code = atoi(pdata);

                        pdata = json_get_value_by_fullname(ppayload, payload_len, "content.errormessage", &data_len, NULL);
                        if (NULL == pdata) {
                            ALIOT_LOG_WARN("Invalid JSON document: not 'content.errormessage' key");
                            break;
                        }

                        pelement[i].callback(ack_code, pdata, data_len);
                    } else {
                        ALIOT_LOG_WARN("Invalid JSON document: value of 'status' key is invalid.");
                    }
                } while (0);

                aliot_platform_mutex_lock(pshadow->mutex);
                memset(&pelement[i], 0, sizeof(aliot_update_ack_wait_list_t));
                aliot_platform_mutex_unlock(pshadow->mutex);
                return;
            }
        }
    }

    aliot_platform_mutex_unlock(pshadow->mutex);
    ALIOT_LOG_WARN("Not match any wait element in list.");
}
