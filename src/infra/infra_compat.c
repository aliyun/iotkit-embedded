#include "infra_config.h"

#ifdef INFRA_COMPAT
#include <string.h>
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"

#ifdef MQTT_COMM_ENABLED
#include "dev_sign_api.h"
#include "mqtt_api.h"

#ifdef DYNAMIC_REGISTER
#include "dynreg_api.h"
#endif

#ifdef INFRA_LOG
    #include "infra_log.h"
    #define sdk_err(...)       log_err("infra_compat", __VA_ARGS__)
    #define sdk_info(...)      log_info("infra_compat", __VA_ARGS__)
#else
    #define sdk_err(...)
    #define sdk_info(...)
#endif

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);

#ifdef DYNAMIC_REGISTER
void HAL_Printf(const char *fmt, ...);
int HAL_SetDeviceSecret(char *device_secret);
int HAL_GetProductSecret(char *product_secret);
int HAL_Kv_Set(const char *key, const void *val, int len, int sync);
int HAL_Kv_Get(const char *key, void *val, int *buffer_len);
#define KV_KEY_DEVICE_SECRET            "DyncRegDeviceSecret"
#endif

static iotx_conn_info_t g_iotx_conn_info = {0};
static sdk_impl_ctx_t g_sdk_impl_ctx = {0};

extern iotx_sign_mqtt_t g_sign_mqtt;
int IOT_SetupConnInfo(const char *product_key,
                      const char *device_name,
                      const char *device_secret,
                      void **info_ptr)
{
    int res = FAIL_RETURN;
    iotx_dev_meta_info_t meta_data;
#ifdef DYNAMIC_REGISTER
    sdk_impl_ctx_t     *ctx = &g_sdk_impl_ctx;
#endif

    if (product_key == NULL || device_name == NULL || device_secret == NULL ||
        strlen(product_key) >= IOTX_PRODUCT_KEY_LEN ||
        strlen(device_name) >= IOTX_DEVICE_NAME_LEN ||
        strlen(device_secret) >= IOTX_DEVICE_SECRET_LEN) {
        return NULL_VALUE_ERROR;
    }

    memset(&g_sign_mqtt, 0, sizeof(iotx_sign_mqtt_t));
    memset(&g_iotx_conn_info, 0, sizeof(iotx_conn_info_t));

    memset(&meta_data, 0, sizeof(iotx_dev_meta_info_t));
    memcpy(meta_data.product_key, product_key, strlen(product_key));
    memcpy(meta_data.device_name, device_name, strlen(device_name));
    memcpy(meta_data.device_secret, device_secret, strlen(device_secret));

#ifdef DYNAMIC_REGISTER
    if (ctx->dynamic_register) {
        char device_secret_actual[IOTX_DEVICE_SECRET_LEN + 1] = {0};
        int device_secret_len = IOTX_DEVICE_SECRET_LEN;

        /* Check if Device Secret exit in KV */
        if (HAL_Kv_Get(KV_KEY_DEVICE_SECRET, device_secret_actual, &device_secret_len) == 0) {
            sdk_info("Get DeviceSecret from KV succeed");

            *(device_secret_actual + device_secret_len) = 0;
            HAL_SetDeviceSecret(device_secret_actual);
            memset(meta_data.device_secret,0,IOTX_DEVICE_SECRET_LEN + 1);
            memcpy(meta_data.device_secret,device_secret_actual,strlen(device_secret_actual));
        } else {
            char product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = {0};

            /* KV not exit, goto dynamic register */
            sdk_info("DeviceSecret KV not exist, Now We Need Dynamic Register...");

            /* Check If Product Secret Exist */
            HAL_GetProductSecret(product_secret);
            if (strlen(product_secret) == 0) {
                sdk_err("Product Secret Is Not Exist");
                return FAIL_RETURN;
            }

            if (strlen(product_secret) == 0) {
                return FAIL_RETURN;
            }
            memcpy(meta_data.product_secret, product_secret, strlen(product_secret));

            res = IOT_Dynamic_Register(ctx->domain_type,&meta_data);
            if (res != SUCCESS_RETURN) {
                sdk_err("Dynamic Register Failed");
                return FAIL_RETURN;
            }

            device_secret_len = strlen(meta_data.device_secret);
            if (HAL_Kv_Set(KV_KEY_DEVICE_SECRET, meta_data.device_secret, device_secret_len, 1) != 0) {
                sdk_err("Save Device Secret to KV Failed");
                return FAIL_RETURN;
            }

            HAL_SetDeviceSecret(meta_data.device_secret);
        }
    }
#endif
    /* just connect shanghai region */
    res = IOT_Sign_MQTT(g_sdk_impl_ctx.domain_type, &meta_data, &g_sign_mqtt);
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
    if (info_ptr) {
        *info_ptr = (void *)&g_iotx_conn_info;
    }
    return SUCCESS_RETURN;
}
#endif

#if defined(DEVICE_MODEL_CLASSIC) && defined(DEVICE_MODEL_ENABLED)
    #include "iotx_dm.h"
#endif

int IOT_Ioctl(int option, void *data)
{
    int                 res = SUCCESS_RETURN;
    sdk_impl_ctx_t     *ctx = NULL;

    ctx = &g_sdk_impl_ctx;

    if (option < 0 || data == NULL) {
        return FAIL_RETURN;
    }

    switch (option) {
        case IOTX_IOCTL_SET_REGION: {
            ctx->domain_type = *(int *)data;
            /* iotx_guider_set_region(*(int *)data); */

            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_GET_REGION: {
            *(int *)data = ctx->domain_type;

            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_SET_MQTT_DOMAIN: {
            ctx->domain_type = IOTX_CLOUD_REGION_CUSTOM;

            if (ctx->cloud_custom_domain) {
                HAL_Free(ctx->cloud_custom_domain);
                ctx->cloud_custom_domain = NULL;
                g_infra_mqtt_domain[IOTX_CLOUD_REGION_CUSTOM] = NULL;
            }
            ctx->cloud_custom_domain = HAL_Malloc(strlen((char *)data) + 1);
            if (ctx->cloud_custom_domain == NULL) {
                return FAIL_RETURN;
            }
            memset(ctx->cloud_custom_domain, 0, strlen((char *)data) + 1);
            memcpy(ctx->cloud_custom_domain, data, strlen((char *)data));
            g_infra_mqtt_domain[IOTX_CLOUD_REGION_CUSTOM] = (const char *)ctx->cloud_custom_domain;
            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_SET_HTTP_DOMAIN: {
            ctx->domain_type = IOTX_HTTP_REGION_CUSTOM;

            if (ctx->http_custom_domain) {
                HAL_Free(ctx->http_custom_domain);
                ctx->http_custom_domain = NULL;
                g_infra_http_domain[IOTX_CLOUD_REGION_CUSTOM] = NULL;
            }
            ctx->http_custom_domain = HAL_Malloc(strlen((char *)data) + 1);
            if (ctx->http_custom_domain == NULL) {
                return FAIL_RETURN;
            }
            memset(ctx->http_custom_domain, 0, strlen((char *)data) + 1);
            memcpy(ctx->http_custom_domain, data, strlen((char *)data));
            g_infra_http_domain[IOTX_CLOUD_REGION_CUSTOM] = (const char *)ctx->http_custom_domain;
            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_SET_DYNAMIC_REGISTER: {
            ctx->dynamic_register = *(int *)data;

            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_GET_DYNAMIC_REGISTER: {
            *(int *)data = ctx->dynamic_register;

            res = SUCCESS_RETURN;
        }
        break;
#if defined(DEVICE_MODEL_CLASSIC) && defined(DEVICE_MODEL_ENABLED) && !defined(DEPRECATED_LINKKIT)
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
        case IOTX_IOCTL_RECV_EVENT_REPLY:
        case IOTX_IOCTL_RECV_PROP_REPLY: {
            res = iotx_dm_set_opt(IMPL_LINKKIT_IOCTL_SWITCH_EVENT_POST_REPLY, data);
        }
        break;
        case IOTX_IOCTL_SEND_PROP_SET_REPLY : {
            res = iotx_dm_set_opt(IMPL_LINKKIT_IOCTL_SWITCH_PROPERTY_SET_REPLY, data);
        }
        break;
#endif
        case IOTX_IOCTL_SET_SUBDEV_SIGN: {
            /* todo */
        }
        break;
        case IOTX_IOCTL_GET_SUBDEV_LOGIN: {
            /* todo */
        }
        break;
#if defined(DEVICE_MODEL_CLASSIC) && defined(DEVICE_MODEL_GATEWAY)
#ifdef DEVICE_MODEL_SUBDEV_OTA
        case IOTX_IOCTL_SET_OTA_DEV_ID: {
            int devid = *(int *)(data);
            res = iotx_dm_ota_switch_device(devid);
        }
        break;
#endif
#endif
#else
        case IOTX_IOCTL_RECV_EVENT_REPLY:
        case IOTX_IOCTL_RECV_PROP_REPLY:
        case IOTX_IOCTL_SEND_PROP_SET_REPLY:
        case IOTX_IOCTL_GET_SUBDEV_LOGIN: {
            res = SUCCESS_RETURN;
        }
        break;
#endif
        default: {
            res = FAIL_RETURN;
        }
        break;
    }

    return res;
}

void IOT_DumpMemoryStats(IOT_LogLevel level)
{
#ifdef INFRA_MEM_STATS
    int             lvl = (int)level;

    if (lvl > LOG_DEBUG_LEVEL) {
        lvl = LOG_DEBUG_LEVEL;
        HAL_Printf("Invalid input level, using default: %d => %d", level, lvl);
    }

    LITE_dump_malloc_free_stats(lvl);
#endif
}

#endif
