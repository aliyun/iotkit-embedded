#ifndef __IOTX_MQTT_H__
#define __IOTX_MQTT_H__

#include "iotx_mqtt_config.h"

/* Information structure of mutli-subscribe */
typedef struct {
    const char                                    *topicFilter;
    iotx_mqtt_qos_t                                qos;
    iotx_mqtt_event_handle_func_fpt                messageHandler;
} iotx_mutli_sub_info_t, *iotx_mutli_sub_info_pt;

int iotx_mc_batchsub(void *handle, iotx_mutli_sub_info_pt *sub_list, int list_size, void *pcontext);

#endif  /* __IOTX_MQTT_H__ */
