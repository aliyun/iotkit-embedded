#ifndef __DEV_RESET_API_H__
#define __DEV_RESET_API_H__

#include "infra_types.h"
#include "infra_defs.h"
#include "mqtt_api.h"

/**
 * @brief   report reset message to cloud.
 *
 * @param meta_info. device meta info, only product_key and device_name needed.
 * @param extended. reserved.
 * 
 * @retval  -1 : failure
 * @retval  0 : sucess
 */
int IOT_DevReset_Report(iotx_dev_meta_info_t *meta_info, iotx_mqtt_event_handle_func_fpt handle, void *extended);

#endif

