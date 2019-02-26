#include <stdio.h>
#include <string.h>
#include "mqtt_api.h"
#include "dev_reset_api.h"

void HAL_Printf(const char *fmt, ...);
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN]);
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN]);

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

static int reset_mqtt_packet_id = 0;
static int reset_reply_received = 0;

void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    EXAMPLE_TRACE("msg->event_type : %d", msg->event_type);
}

void example_sub_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    switch (msg->event_type)
    {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED: {
            iotx_mqtt_topic_info_t *packet_info = (iotx_mqtt_topic_info_t *)msg->msg;
            if (packet_info->packet_id != reset_mqtt_packet_id) {
                return;
            }
            EXAMPLE_TRACE("Receive MQTT Package");
            EXAMPLE_TRACE("Package ID: %d", packet_info->packet_id);
            EXAMPLE_TRACE("Topic: %.*s", packet_info->topic_len, packet_info->ptopic);
            EXAMPLE_TRACE("Payload: %.*s", packet_info->payload_len, packet_info->payload);
        }
        break;

        default:
            break;
    }
    reset_reply_received = 1;
}

int main(int argc, char *argv[])
{
    int res = 0;
    void                   *pclient = NULL;
    iotx_dev_meta_info_t    meta_info;
    iotx_mqtt_param_t       mqtt_params;

    memset(&mqtt_params, 0, sizeof(iotx_mqtt_param_t));
    memset(&meta_info, 0, sizeof(iotx_dev_meta_info_t));

    HAL_GetProductKey(meta_info.product_key);
    HAL_GetDeviceName(meta_info.device_name);

    mqtt_params.handle_event.h_fp = example_event_handle;

    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        return -1;
    }

    res = IOT_DevReset_Report(&meta_info, example_sub_handle, NULL);
    if (res < 0) {
        return -1;
    }
    reset_mqtt_packet_id = res;

    while (!reset_reply_received) {

        IOT_MQTT_Yield(pclient, 200);
    }

    EXAMPLE_TRACE("Example Execute Success, Now Exit...");

    return 0;
}

