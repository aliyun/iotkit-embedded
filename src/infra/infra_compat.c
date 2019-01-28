#ifdef INFRA_COMPAT
#include <string.h>
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"

#ifdef MQTT_COMM_ENABLED
#include "dev_sign_api.h"
#include "mqtt_api.h"
iotx_conn_info_t g_iotx_conn_info;
extern iotx_sign_mqtt_t g_sign_mqtt;
int IOT_SetupConnInfo(const char *product_key,
                      const char *device_name,
                      const char *device_secret,
                      void **info_ptr)
{
    int res = FAIL_RETURN;
    iotx_dev_meta_info_t meta_data;

    if (product_key == NULL || device_name == NULL || device_secret == NULL ||
        strlen(product_key) >= IOTX_PRODUCT_KEY_LEN ||
        strlen(device_name) >= IOTX_DEVICE_NAME_LEN ||
        strlen(device_secret) >= IOTX_DEVICE_SECRET_LEN) {
        return NULL_VALUE_ERROR;
    }

    memset(&g_sign_mqtt, 0, sizeof(iotx_sign_mqtt_t));
    memset(&g_iotx_conn_info,0,sizeof(iotx_conn_info_t));

    memset(&meta_data, 0, sizeof(iotx_dev_meta_info_t));
    memcpy(meta_data.product_key, product_key, strlen(product_key));
    memcpy(meta_data.device_name, device_name, strlen(device_name));
    memcpy(meta_data.device_secret, device_secret, strlen(device_secret));

    /* just connect shanghai region */
    res = IOT_Sign_MQTT(IOTX_CLOUD_REGION_SHANGHAI, &meta_data, &g_sign_mqtt);
    if (res < SUCCESS_RETURN) {
        return res;
    }

    g_iotx_conn_info.port = g_sign_mqtt.port;
    g_iotx_conn_info.host_name = g_sign_mqtt.hostname;
    g_iotx_conn_info.client_id = g_sign_mqtt.clientid;
    g_iotx_conn_info.username = g_sign_mqtt.username;
    g_iotx_conn_info.password = g_sign_mqtt.password;
#ifdef SUPPORT_TLS
        {
            extern const char *iotx_ca_crt;
            g_iotx_conn_info.pub_key = iotx_ca_crt;
        }
#endif
    *info_ptr = (void *)&g_iotx_conn_info;
    return SUCCESS_RETURN;
}
#endif

#endif