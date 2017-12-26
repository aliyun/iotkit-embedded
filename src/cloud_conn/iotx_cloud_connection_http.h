
#ifndef SRC_CLOUD_CONNECTION_HTTP_H_
#define SRC_CLOUD_CONNECTION_HTTP_H_


#include "iot_export_cloud_connection.h"


void* iotx_cloud_connection_http_init(void *param, void* handle);

int iotx_cloud_connection_http_subscribe(void *handle,
                        const char *topic_filter,
                        iotx_message_qos_t qos);

int iotx_cloud_connection_http_unsubscribe(void *handle,
                        const char *topic_filter);

int iotx_cloud_connection_http_publish(void *handle,
                        iotx_cloud_connection_msg_pt message);

int iotx_cloud_connection_http_deinit(void *handle);

int iotx_cloud_connection_http_yield(void *handle, int timeout_ms);


#endif /* SRC_CLOUD_CONNECTION_HTTP_H_ */
