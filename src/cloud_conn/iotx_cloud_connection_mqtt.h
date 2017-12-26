
#ifndef SRC_CLOUD_CONNECTION_MQTT_H_
#define SRC_CLOUD_CONNECTION_MQTT_H_

#include "iot_export_cloud_connection.h"


#define MQTT_MSGLEN             (1024 * 4)


void* iotx_cloud_connection_mqtt_init(void *param, void* handle);

int iotx_cloud_connection_mqtt_subscribe(void *handle,
                        const char *topic_filter,
                        iotx_message_qos_t qos);

int iotx_cloud_connection_mqtt_unsubscribe(void *handle,
                        const char *topic_filter);

int iotx_cloud_connection_mqtt_publish(void *handle,
                        iotx_cloud_connection_msg_pt message);

int iotx_cloud_connection_mqtt_deinit(void *handle);

int iotx_cloud_connection_mqtt_yield(void *handle, int timeout_ms);

#endif /* SRC_CLOUD_CONNECTION_MQTT_H_ */
