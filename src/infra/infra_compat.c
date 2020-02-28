#include "infra_config.h"

#ifdef INFRA_COMPAT
#include <string.h>
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_state.h"
#include "infra_compat.h"
#if defined(INFRA_LOG)
    #include "infra_log.h"
#endif
#if defined(INFRA_MEM_STATS)
    #include "infra_mem_stats.h"
#endif

#include "wrappers.h"

/*sdk_impl_ctx_t g_sdk_impl_ctx = {0};*/

#if !defined(INFRA_LOG)
void IOT_SetLogLevel(IOT_LogLevel level) {}
#endif

#if defined(WIFI_PROVISION_ENABLED)
    extern void awss_set_press_timeout_ms(unsigned int timeout_ms);
    extern void awss_set_channel_scan_interval_ms(uint32_t timeout_ms);
#endif

static char http_custom_domain[IOTX_DOMAIN_MAX_LEN + 1] = {0};
static iotx_dev_meta_info_t  g_dev_meta_info_t = {0};
/* global variable for mqtt construction */
static iotx_conn_info_t g_iotx_conn_info = {0};
static char g_empty_string[1] = "";

int IOT_SetupConnInfo(const char *product_key,
                      const char *device_name,
                      const char *device_secret,
                      void **info_ptr)
{
    if (product_key == NULL || device_name == NULL || device_secret == NULL ||
        strlen(product_key) > IOTX_PRODUCT_KEY_LEN ||
        strlen(device_name) > IOTX_DEVICE_NAME_LEN ||
        strlen(device_secret) > IOTX_DEVICE_SECRET_LEN) {
        return NULL_VALUE_ERROR;
    }

    if (info_ptr) {
        memset(&g_iotx_conn_info, 0, sizeof(iotx_conn_info_t));
        g_iotx_conn_info.host_name = g_empty_string;
        g_iotx_conn_info.client_id = g_empty_string;
        g_iotx_conn_info.username = g_empty_string;
        g_iotx_conn_info.password = g_empty_string;
        g_iotx_conn_info.pub_key = g_empty_string;

        *info_ptr = &g_iotx_conn_info;
    }
    return SUCCESS_RETURN;
}

int IOCTL_FUNC(IOTX_IOCTL_SET_REGION, void *);
int IOCTL_FUNC(IOTX_IOCTL_GET_REGION, void *);
int IOCTL_FUNC(IOTX_IOCTL_SET_MQTT_DOMAIN, void *data);
int IOCTL_FUNC(IOTX_IOCTL_SET_HTTP_DOMAIN, void *data);
#if defined(DYNAMIC_REGISTER)
    int IOCTL_FUNC(IOTX_IOCTL_SET_DYNAMIC_REGISTER, void *data);
    int IOCTL_FUNC(IOTX_IOCTL_GET_DYNAMIC_REGISTER, void *);
#endif
int IOCTL_FUNC(IOTX_IOCTL_FOTA_TIMEOUT_MS, void *data);
#if defined(DEVICE_MODEL_ENABLED)
    int IOCTL_FUNC(IOTX_IOCTL_SET_CUSTOMIZE_INFO, void *data);
    int IOCTL_FUNC(IOTX_IOCTL_SET_PROXY_REGISTER, void *);
    int IOCTL_FUNC(IOTX_IOCTL_SET_OTA_DEV_ID, void *);
    int IOCTL_FUNC(IOTX_IOCTL_QUERY_DEVID, void *data);
    int IOCTL_FUNC(IOTX_IOCTL_SET_MQTT_PORT, void *);
#endif
int IOCTL_FUNC(IOTX_IOCTL_SET_AWSS_ENABLE_INTERVAL, void *data);
int IOCTL_FUNC(IOTX_IOCTL_SET_AWSS_CHANNEL_SCAN_INTERVAL, void *data);
int IOCTL_FUNC(IOTX_IOCTL_SUB_USER_TOPIC, void *);
int IOCTL_FUNC(IOTX_IOCTL_RECV_PROP_REPLY, void *);
int IOCTL_FUNC(IOTX_IOCTL_SEND_PROP_SET_REPLY, void *);

int IOCTL_FUNC(IOTX_IOCTL_SET_HTTP_DOMAIN, void *data)
{
    if (data == NULL) {
        return FAIL_RETURN;
    }
    memset(http_custom_domain, 0, strlen((char *)data) + 1);
    memcpy(http_custom_domain, data, strlen((char *)data));
    g_infra_http_domain[IOTX_CLOUD_REGION_CUSTOM] = (const char *)http_custom_domain;
    return SUCCESS_RETURN;
}

int IOT_Ioctl(int  option, void *data)
{
    int                 res = FAIL_RETURN;
    iotx_dev_meta_info_t     *meta = NULL;

    meta = &g_dev_meta_info_t;

    if (option < 0 || data == NULL) {
        return FAIL_RETURN;
    }
    switch (option) {
        case IOTX_IOCTL_GET_MODULE: {
            if (NULL == data) {
                res = FAIL_RETURN;
                break;
            }
            memcpy(data, meta->module, strlen(meta->module));
            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_SET_MODULE:
            if ((data != NULL) && (strlen(data) <= IOTX_MODULE_LEN)) {
                memset(meta->module, 0, IOTX_MODULE_LEN + 1);
                memcpy(meta->module, data, strlen(data));
                res = SUCCESS_RETURN;
            } else {
                res = FAIL_RETURN;
            }
            break;
        case IOTX_IOCTL_SET_REGION:
            res = IOCTL_FUNC(IOTX_IOCTL_SET_REGION, data);
            break;
        case IOTX_IOCTL_GET_REGION:
            res = IOCTL_FUNC(IOTX_IOCTL_GET_REGION, data);
            break;

        case IOTX_IOCTL_SET_MQTT_DOMAIN: {
            int domain_type = IOTX_CLOUD_REGION_CUSTOM;
            IOCTL_FUNC(IOTX_IOCTL_SET_REGION, &domain_type);
            res = IOCTL_FUNC(IOTX_IOCTL_SET_MQTT_DOMAIN, data);
        }
        break;

        case IOTX_IOCTL_SET_HTTP_DOMAIN: {
            int region = IOTX_HTTP_REGION_CUSTOM;
            IOCTL_FUNC(IOTX_IOCTL_SET_REGION, &region);
            res = IOCTL_FUNC(IOTX_IOCTL_SET_HTTP_DOMAIN, data);
        }
        break;
#if defined(DYNAMIC_REGISTER)
        case IOTX_IOCTL_SET_DYNAMIC_REGISTER:
            res = IOCTL_FUNC(IOTX_IOCTL_SET_DYNAMIC_REGISTER, data);
            break;
        case IOTX_IOCTL_GET_DYNAMIC_REGISTER:
            res = IOCTL_FUNC(IOTX_IOCTL_GET_DYNAMIC_REGISTER, data);
            break;
#endif

            break;
        case IOTX_IOCTL_SET_PRODUCT_KEY: {
            if ((data != NULL) && (strlen(data) <= IOTX_PRODUCT_KEY_LEN)) {
                memset(meta->product_key, 0, IOTX_PRODUCT_KEY_LEN + 1);
                memcpy(meta->product_key, data, strlen(data));
                res = SUCCESS_RETURN;
            } else {
                res = FAIL_RETURN;
            }
        }
        break;
        case IOTX_IOCTL_GET_PRODUCT_KEY: {
            memcpy(data, meta->product_key, strlen(meta->product_key));
            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_SET_PRODUCT_SECRET: {
            if ((data != NULL) && (strlen(data) <= IOTX_PRODUCT_SECRET_LEN)) {
                memset(meta->product_secret, 0, IOTX_PRODUCT_SECRET_LEN + 1);
                memcpy(meta->product_secret, data, strlen(data));
                res = SUCCESS_RETURN;
            } else {
                res = FAIL_RETURN;
            }
        }
        break;
        case IOTX_IOCTL_GET_PRODUCT_SECRET: {
            memcpy(data, meta->product_secret, strlen(meta->product_secret));
            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_SET_DEVICE_NAME: {
            if ((data != NULL) && (strlen(data) <= IOTX_DEVICE_NAME_LEN)) {
                memset(meta->device_name, 0, IOTX_DEVICE_NAME_LEN + 1);
                memcpy(meta->device_name, data, strlen(data));
                res = SUCCESS_RETURN;
            } else {
                res = FAIL_RETURN;
            }
        }
        break;
        case IOTX_IOCTL_GET_DEVICE_NAME: {
            memcpy(data, meta->device_name, strlen(meta->device_name));
            res = SUCCESS_RETURN;
        }
        break;
        case IOTX_IOCTL_SET_DEVICE_SECRET: {
            if ((data != NULL) && (strlen(data) <= IOTX_DEVICE_SECRET_LEN)) {
                memset(meta->device_secret, 0, IOTX_DEVICE_SECRET_LEN + 1);
                memcpy(meta->device_secret, data, strlen(data));
                res = SUCCESS_RETURN;
            } else {
                res = FAIL_RETURN;
            }
        }
        break;
        case IOTX_IOCTL_GET_DEVICE_SECRET: {
            memcpy(data, meta->device_secret, strlen(meta->device_secret));
            res = SUCCESS_RETURN;
        }

#if defined(DEVICE_MODEL_ENABLED) && !defined(DEPRECATED_LINKKIT)
#if !defined(DEVICE_MODEL_RAWDATA_SOLO)
        case IOTX_IOCTL_RECV_EVENT_REPLY:
        case IOTX_IOCTL_RECV_PROP_REPLY: {
            res = IOCTL_FUNC(IOTX_IOCTL_RECV_PROP_REPLY, data);
        }
        break;
        case IOTX_IOCTL_SEND_PROP_SET_REPLY : {
            res = IOCTL_FUNC(IOTX_IOCTL_SEND_PROP_SET_REPLY, data);
        }
        break;
#endif
#if defined(DEVICE_MODEL_GATEWAY)
#ifdef DEVICE_MODEL_SUBDEV_OTA
        case IOTX_IOCTL_SET_OTA_DEV_ID: {
            res = IOCTL_FUNC(IOTX_IOCTL_SET_OTA_DEV_ID, data);
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
#if defined(DEVICE_MODEL_ENABLED)
        case IOTX_IOCTL_FOTA_TIMEOUT_MS: {
            res = IOCTL_FUNC(IOTX_IOCTL_FOTA_TIMEOUT_MS, data);
        }
        break;

        case IOTX_IOCTL_SUB_USER_TOPIC: {
            res = IOCTL_FUNC(IOTX_IOCTL_SUB_USER_TOPIC, data);
        }
        break;
        case IOTX_IOCTL_SET_CUSTOMIZE_INFO: {
            res = IOCTL_FUNC(IOTX_IOCTL_SET_CUSTOMIZE_INFO, data);
        }
        break;
        case IOTX_IOCTL_SET_MQTT_PORT: {
            res = IOCTL_FUNC(IOTX_IOCTL_SET_MQTT_PORT, data);
        }
#endif

#if defined(DEVICE_MODEL_GATEWAY) && !defined(DEPRECATED_LINKKIT)
        case IOTX_IOCTL_SET_PROXY_REGISTER: {
            res = IOCTL_FUNC(IOTX_IOCTL_SET_PROXY_REGISTER, data);
        }
        break;
        case IOTX_IOCTL_QUERY_DEVID: {
            res = IOCTL_FUNC(IOTX_IOCTL_QUERY_DEVID, data);
        }
        break;
        case IOTX_IOCTL_SET_SUBDEV_SIGN: {
            /* todo */
        }
        break;
        case IOTX_IOCTL_GET_SUBDEV_LOGIN: {
            /* todo */
        }
        break;
#endif

#if defined(WIFI_PROVISION_ENABLED)
        case IOTX_IOCTL_SET_AWSS_ENABLE_INTERVAL: {
            res = IOCTL_FUNC(IOTX_IOCTL_SET_AWSS_ENABLE_INTERVAL, data);
        }
        break;
        case IOTX_IOCTL_SET_AWSS_CHANNEL_SCAN_INTERVAL: {
            res = IOCTL_FUNC(IOTX_IOCTL_SET_AWSS_CHANNEL_SCAN_INTERVAL, data);
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

static void *g_event_monitor = NULL;

int iotx_event_regist_cb(void (*monitor_cb)(int event))
{
    g_event_monitor = (void *)monitor_cb;
    return 0;
}

int iotx_event_post(int event)
{
    if (g_event_monitor == NULL) {
        return -1;
    }
    ((void (*)(int))g_event_monitor)(event);
    return 0;
}

typedef struct {
    int eventid;
    void *callback;
} impl_event_map_t;

static impl_event_map_t g_impl_event_map[] = {
    {ITE_AWSS_STATUS,          NULL},
    {ITE_CONNECT_SUCC,         NULL},
    {ITE_CONNECT_FAIL,         NULL},
    {ITE_DISCONNECTED,         NULL},
    {ITE_RAWDATA_ARRIVED,      NULL},
    {ITE_SERVICE_REQUEST,      NULL},
    {ITE_SERVICE_REQUEST_EXT,  NULL},
    {ITE_PROPERTY_SET,         NULL},
    {ITE_PROPERTY_GET,         NULL},
#ifdef DEVICE_MODEL_SHADOW
    {ITE_PROPERTY_DESIRED_GET_REPLY,         NULL},
#endif
    {ITE_REPORT_REPLY,         NULL},
    {ITE_TRIGGER_EVENT_REPLY,  NULL},
    {ITE_TIMESTAMP_REPLY,      NULL},
    {ITE_TOPOLIST_REPLY,       NULL},
    {ITE_PERMIT_JOIN,          NULL},
    {ITE_INITIALIZE_COMPLETED, NULL},
    {ITE_FOTA,                 NULL},
    {ITE_FOTA_MODULE,          NULL},
    {ITE_COTA,                 NULL},
    {ITE_MQTT_CONNECT_SUCC,    NULL},
    {ITE_CLOUD_ERROR,          NULL},
    {ITE_DYNREG_DEVICE_SECRET, NULL},
    {ITE_IDENTITY_RESPONSE,    NULL},
    {ITE_BIND_EVENT,           NULL},
    {ITE_STATE_EVERYTHING,     NULL},
    {ITE_STATE_USER_INPUT,     NULL},
    {ITE_STATE_SYS_DEPEND,     NULL},
    {ITE_STATE_MQTT_COMM,      NULL},
    {ITE_STATE_WIFI_PROV,      NULL},
    {ITE_STATE_COAP_LOCAL,     NULL},
    {ITE_STATE_HTTP_COMM,      NULL},
    {ITE_STATE_OTA,            NULL},
    {ITE_STATE_DEV_BIND,       NULL},
    {ITE_STATE_DEV_MODEL,      NULL}
};

void *iotx_event_callback(int evt)
{
    if (evt < 0 || evt >= sizeof(g_impl_event_map) / sizeof(impl_event_map_t)) {
        return NULL;
    }
    return g_impl_event_map[evt].callback;
}

DEFINE_EVENT_CALLBACK(ITE_AWSS_STATUS,          int (*callback)(int))
DEFINE_EVENT_CALLBACK(ITE_CONNECT_SUCC,         int (*callback)(void))
DEFINE_EVENT_CALLBACK(ITE_CONNECT_FAIL,         int (*callback)(void))
DEFINE_EVENT_CALLBACK(ITE_DISCONNECTED,         int (*callback)(void))
DEFINE_EVENT_CALLBACK(ITE_RAWDATA_ARRIVED,      int (*callback)(const int, const unsigned char *, const int))
DEFINE_EVENT_CALLBACK(ITE_SERVICE_REQUEST,      int (*callback)(const int, const char *, const int, const char *,
                      const int, char **, int *))
DEFINE_EVENT_CALLBACK(ITE_SERVICE_REQUEST_EXT,  int (*callback)(int, const char *, int, const char *, int, const char *,
                      int, void *))
DEFINE_EVENT_CALLBACK(ITE_PROPERTY_SET,         int (*callback)(const int, const char *, const int))
#ifdef DEVICE_MODEL_SHADOW
    DEFINE_EVENT_CALLBACK(ITE_PROPERTY_DESIRED_GET_REPLY,         int (*callback)(const char *, const int))
#endif
DEFINE_EVENT_CALLBACK(ITE_PROPERTY_GET,         int (*callback)(const int, const char *, const int, char **, int *))
DEFINE_EVENT_CALLBACK(ITE_REPORT_REPLY,         int (*callback)(const int, const int, const int, const char *,
                      const int))
DEFINE_EVENT_CALLBACK(ITE_TRIGGER_EVENT_REPLY,  int (*callback)(const int, const int, const int, const char *,
                      const int, const char *, const int))
DEFINE_EVENT_CALLBACK(ITE_TIMESTAMP_REPLY,      int (*callback)(const char *))
DEFINE_EVENT_CALLBACK(ITE_TOPOLIST_REPLY,       int (*callback)(const int, const int, const int, const char *,
                      const int))
DEFINE_EVENT_CALLBACK(ITE_PERMIT_JOIN,          int (*callback)(const char *, int))
DEFINE_EVENT_CALLBACK(ITE_INITIALIZE_COMPLETED, int (*callback)(const int))
DEFINE_EVENT_CALLBACK(ITE_FOTA,                 int (*callback)(const int, const char *))
DEFINE_EVENT_CALLBACK(ITE_FOTA_MODULE,          int (*callback)(const int, const char *, const char *))
DEFINE_EVENT_CALLBACK(ITE_COTA,                 int (*callback)(const int, const char *, int, const char *,
                      const char *, const char *, const char *))
DEFINE_EVENT_CALLBACK(ITE_MQTT_CONNECT_SUCC,    int (*callback)(void))
DEFINE_EVENT_CALLBACK(ITE_CLOUD_ERROR,          int (*callback)(const int, const char *, const char *))
DEFINE_EVENT_CALLBACK(ITE_DYNREG_DEVICE_SECRET, int (*callback)(const char *))
DEFINE_EVENT_CALLBACK(ITE_IDENTITY_RESPONSE,    int (*callback)(const char *))
DEFINE_EVENT_CALLBACK(ITE_BIND_EVENT,           int (*callback)(const char *))
int iotx_register_for_ITE_STATE_EVERYTHING(state_handler_t callback)
{
    int idx = 0;

    for (idx = ITE_STATE_EVERYTHING; idx <= ITE_STATE_DEV_MODEL; idx++) {
        g_impl_event_map[idx].callback = (void *)callback;
    }

    return 0;
}

DEFINE_EVENT_CALLBACK(ITE_STATE_USER_INPUT, state_handler_t callback)
DEFINE_EVENT_CALLBACK(ITE_STATE_SYS_DEPEND, state_handler_t callback)
DEFINE_EVENT_CALLBACK(ITE_STATE_MQTT_COMM,  state_handler_t callback)
DEFINE_EVENT_CALLBACK(ITE_STATE_WIFI_PROV,  state_handler_t callback)
DEFINE_EVENT_CALLBACK(ITE_STATE_COAP_LOCAL, state_handler_t callback)
DEFINE_EVENT_CALLBACK(ITE_STATE_HTTP_COMM,  state_handler_t callback)
DEFINE_EVENT_CALLBACK(ITE_STATE_OTA,        state_handler_t callback)
DEFINE_EVENT_CALLBACK(ITE_STATE_DEV_BIND,   state_handler_t callback)
DEFINE_EVENT_CALLBACK(ITE_STATE_DEV_MODEL,  state_handler_t callback)

#if !defined(__APPLE__)
    extern int vsnprintf(char *str, size_t size, const char *format, va_list ap);
#endif

#define IOTX_STATE_EVENT_MESSAGE_MAXLEN (64)
int iotx_state_event(const int event, const int code, const char *msg_format, ...)
{
    char        message[IOTX_STATE_EVENT_MESSAGE_MAXLEN + 1] = {0};
    void       *everything_state_handler = iotx_event_callback(ITE_STATE_EVERYTHING);
    void       *state_handler = iotx_event_callback(event);
    va_list     args;

    if (state_handler == NULL) {
        return -1;
    }

    if (msg_format != NULL) {
        memset(message, 0, sizeof(message));
        va_start(args, msg_format);
        vsnprintf(message, IOTX_STATE_EVENT_MESSAGE_MAXLEN, msg_format, args);
        va_end(args);
    }

    ((state_handler_t)state_handler)(code, message);

    if (everything_state_handler && everything_state_handler != state_handler) {
        ((state_handler_t)everything_state_handler)(code, message);
    }

    return 0;
}

#endif
