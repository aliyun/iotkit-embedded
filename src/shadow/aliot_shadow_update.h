
#ifndef _ALIOT_SHADOW_UPDATE_H_
#define _ALIOT_SHADOW_UPDATE_H_

#include "aliot_platform.h"
#include "aliot_error.h"

#include "aliot_shadow.h"
#include "aliot_shadow_config.h"
#include "aliot_shadow_common.h"


aliot_update_ack_wait_list_pt aliot_shadow_update_wait_ack_list_add(
            aliot_shadow_pt pshadow,
            const char *token,
            size_t token_len,
            aliot_update_cb_fpt cb,
            void *pcontext,
            uint32_t timeout);

void aliot_shadow_update_wait_ack_list_remove(aliot_shadow_pt pshadow, aliot_update_ack_wait_list_pt element);

void ads_update_wait_ack_list_handle_expire(aliot_shadow_pt pshadow);

void ads_update_wait_ack_list_handle_response(
            aliot_shadow_pt pshadow,
            const char *json_doc,
            size_t json_doc_len);


#endif /* _ALIOT_SHADOW_UPDATE_H_ */
