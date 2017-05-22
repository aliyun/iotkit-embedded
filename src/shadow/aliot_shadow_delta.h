
#ifndef _ALIOT_SHADOW_DELTA_H_
#define _ALIOT_SHADOW_DELTA_H_

#include "aliot_platform.h"
#include "aliot_shadow.h"
#include "aliot_shadow_config.h"
#include "aliot_shadow_common.h"
#include "aliot_shadow_update.h"


bool aliot_shadow_delta_check_existence(aliot_shadow_pt pshadow, const char *attr_name);

void aliot_shadow_delta_entry(
            aliot_shadow_pt pshadow,
            const char *json_doc,
            size_t json_doc_len);

aliot_err_t aliot_shadow_delta_register_attr(
                aliot_shadow_pt pshadow,
                aliot_shadow_attr_pt pattr);

#endif /* _ALIOT_SHADOW_DELTA_H_ */
