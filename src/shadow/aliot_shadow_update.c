
#include "aliot_platform.h"

#include "lite/lite-log.h"
#include "lite/lite-utils.h"
#include "aliot_device.h"

#include "aliot_shadow_update.h"


//add a new wait element
//return: NULL, failed; others, pointer of element.
aliot_update_ack_wait_list_pt aliot_shadow_update_wait_ack_list_add(
            aliot_shadow_pt pshadow,
            const char *ptoken, //NOTE: this is NOT a string.
            size_t token_len,
            aliot_update_cb_fpt cb,
            void *pcontext,
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
    list[i].pcontext = pcontext;

    if (token_len >= ADS_TOKEN_LEN) {
        log_warning("token is too long.");
        token_len = ADS_TOKEN_LEN - 1;
    }
    memcpy(list[i].token, ptoken, token_len);
    list[i].token[token_len] = '\0';

    aliot_time_init(&list[i].timer);
    utils_time_cutdown(&list[i].timer, timeout);

    log_debug("Add update ACK list");

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
            if (utils_time_is_expired(&pelement[i].timer)) {
                if (NULL != pelement[i].callback) {
                    pelement[i].callback(pelement[i].pcontext, ALIOT_SHADOW_ACK_TIMEOUT, NULL, 0);
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
    int i;
    const char *pdata, *ppayload, *pToken;
    aliot_update_ack_wait_list_pt pelement = pshadow->inner_data.update_ack_wait_list;

    //get token
    pdata = LITE_json_value_of("clientToken", json_doc);
    if (NULL == pdata) {
        log_warning("Invalid JSON document: not 'clientToken' key");
        return;
    }
    pToken = pdata;

    ppayload = LITE_json_value_of("payload", json_doc);
    if (NULL == ppayload) {
        log_warning("Invalid JSON document: not 'payload' key");
        LITE_free(pdata);
        return;
    } else {
        log_debug("ppayload = %s", ppayload);
    }

    aliot_platform_mutex_lock(pshadow->mutex);
    for (i = 0; i < ADS_UPDATE_WAIT_ACK_LIST_NUM; ++i) {
        if (0 != pelement[i].flag_busy) {
            //check the related
            if (0 == memcmp(pdata, pelement[i].token, strlen(pelement[i].token))) {
                LITE_free(pdata);
                aliot_platform_mutex_unlock(pshadow->mutex);
                log_debug("token=%s", pelement[i].token);
                do {
                    pdata = LITE_json_value_of("status", ppayload);
                    if (NULL == pdata) {
                        log_warning("Invalid JSON document: not 'payload.status' key");
                        break;
                    }

                    if (0 == strncmp(pdata, "success", strlen(pdata))) {
                        char    *temp = NULL;

                        //If have 'state' keyword in @json_shadow.payload, attribute value should be updated.
                        temp = LITE_json_value_of("state", ppayload);
                        if (NULL != temp) {
                            aliot_shadow_delta_entry(pshadow, json_doc, json_doc_len); //update attribute
                            LITE_free(temp);
                        }

                        pelement[i].callback(pelement[i].pcontext, ALIOT_SHADOW_ACK_SUCCESS, NULL, 0);
                    } else if (0 == strncmp(pdata, "error", strlen(pdata))) {
                        aliot_shadow_ack_code_t ack_code;

                        pdata = LITE_json_value_of("content.errorcode", ppayload);
                        if (NULL == pdata) {
                            log_warning("Invalid JSON document: not 'content.errorcode' key");
                            break;
                        }
                        ack_code = atoi(pdata);
                        LITE_free(pdata);

                        pdata = LITE_json_value_of("content.errormessage", ppayload);
                        if (NULL == pdata) {
                            log_warning("Invalid JSON document: not 'content.errormessage' key");
                            break;
                        }

                        pelement[i].callback(pelement[i].pcontext, ack_code, pdata, strlen(pdata));
                        LITE_free(pdata);
                    } else {
                        log_warning("Invalid JSON document: value of 'status' key is invalid.");
                        LITE_free(pdata);
                    }

                    LITE_free(pdata);
                    LITE_free(ppayload);
                } while (0);

                aliot_platform_mutex_lock(pshadow->mutex);
                memset(&pelement[i], 0, sizeof(aliot_update_ack_wait_list_t));
                aliot_platform_mutex_unlock(pshadow->mutex);
                return;
            }
        }
    }

    LITE_free(pToken);
    LITE_free(ppayload);
    aliot_platform_mutex_unlock(pshadow->mutex);
    log_warning("Not match any wait element in list.");
}
