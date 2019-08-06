#include "iot_export.h"

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
    {ITE_SERVICE_REQUST,       NULL},
    {ITE_PROPERTY_SET,         NULL},
    {ITE_PROPERTY_GET,         NULL},
    {ITE_REPORT_REPLY,         NULL},
    {ITE_TRIGGER_EVENT_REPLY,  NULL},
    {ITE_TIMESTAMP_REPLY,      NULL},
    {ITE_TOPOLIST_REPLY,       NULL},
    {ITE_PERMIT_JOIN,          NULL},
    {ITE_INITIALIZE_COMPLETED, NULL},
    {ITE_FOTA,                 NULL},
    {ITE_COTA,                 NULL},
    {ITE_MQTT_CONNECT_SUCC,    NULL},
    {ITE_STATE_EVERYTHING,     NULL},
    {ITE_STATE_USER_INPUT,     NULL},
    {ITE_STATE_SYS_DEPEND,     NULL},
    {ITE_STATE_MQTT_COMM,      NULL},
    {ITE_STATE_WIFI_PROV,      NULL},
    {ITE_STATE_COAP_LOCAL,     NULL},
    {ITE_STATE_HTTP_COMM,      NULL},
    {ITE_STATE_OTA,            NULL},
    {ITE_STATE_DEV_BIND,       NULL},
    {ITE_STATE_SUB_DEVICE,     NULL},
    {ITE_STATE_DEV_MODEL,      NULL}        /* DEV_MODEL must be last entry */
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
DEFINE_EVENT_CALLBACK(ITE_SERVICE_REQUST,       int (*callback)(const int, const char *, const int, const char *,
                      const int, char **, int *))
DEFINE_EVENT_CALLBACK(ITE_PROPERTY_SET,         int (*callback)(const int, const char *, const int))
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
DEFINE_EVENT_CALLBACK(ITE_COTA,                 int (*callback)(const int, const char *, int, const char *,
                      const char *, const char *, const char *))
DEFINE_EVENT_CALLBACK(ITE_MQTT_CONNECT_SUCC,    int (*callback)(void))

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
DEFINE_EVENT_CALLBACK(ITE_STATE_SUB_DEVICE, state_handler_t callback)
DEFINE_EVENT_CALLBACK(ITE_STATE_DEV_MODEL,  state_handler_t callback)

#define IOTX_STATE_EVENT_MESSAGE_MAXLEN (64)
int iotx_state_event(const int event, const int state_code, const char *state_message)
{
    char message[IOTX_STATE_EVENT_MESSAGE_MAXLEN + 1] = {0};
    void *everything_state_handler = iotx_event_callback(ITE_STATE_EVERYTHING);
    void *state_handler = iotx_event_callback(event);

    if (state_handler == NULL) {
        return -1;
    }

    if (state_message != NULL) {
        if (strlen(state_message) > IOTX_STATE_EVENT_MESSAGE_MAXLEN) {
            memcpy(message, state_message, IOTX_STATE_EVENT_MESSAGE_MAXLEN);
        } else {
            memcpy(message, state_message, strlen(state_message));
        }
    }

    ((state_handler_t)state_handler)(state_code, message);

    if (everything_state_handler && everything_state_handler != state_handler) {
        ((state_handler_t)everything_state_handler)(state_code, message);
    }

    return 0;
}
