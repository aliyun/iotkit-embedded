
#ifndef SRC_CLOUD_CONNECTION_COAP_H_
#define SRC_CLOUD_CONNECTION_COAP_H_


#include "iot_export_cloud_connection.h"


void* iotx_cloud_connection_coap_init(void *param, void* handle);

int iotx_cloud_connection_coap_subscribe(void *handle,
                        const char *topic_filter,
                        iotx_message_qos_t qos);

int iotx_cloud_connection_coap_unsubscribe(void *handle,
                        const char *topic_filter);

int iotx_cloud_connection_coap_publish(void *handle,
                        iotx_cloud_connection_msg_pt message);

int iotx_cloud_connection_coap_deinit(void *handle);

int iotx_cloud_connection_coap_yield(void *handle, int timeout_ms);

#endif /* SRC_CLOUD_CONNECTION_UTIL_H_ */
