#ifndef _INFRA_CLASSIC_H_
#define _INFRA_CLASSIC_H_

#ifndef VERSION_NUM_SIZE
    #define VERSION_NUM_SIZE    4
#endif

#ifndef RANDOM_NUM_SIZE
    #define RANDOM_NUM_SIZE     4
#endif

#ifndef MAC_ADDRESS_SIZE
    #define MAC_ADDRESS_SIZE    8
#endif

#ifndef CHIP_CODE_SIZE
    #define CHIP_CODE_SIZE      4
#endif

#define AOS_ACTIVE_INFO_LEN (81)
/**
 * @brief event list used for iotx_regist_event_monitor_cb
 */
enum iotx_event_t {
    IOTX_AWSS_START = 0x1000,       /* AWSS start without enbale, just supports device discover */
    IOTX_AWSS_ENABLE,               /* AWSS enable */
    IOTX_AWSS_LOCK_CHAN,            /* AWSS lock channel(Got AWSS sync packet) */
    IOTX_AWSS_CS_ERR,               /* AWSS AWSS checksum is error */
    IOTX_AWSS_PASSWD_ERR,           /* AWSS decrypt passwd error */
    IOTX_AWSS_GOT_SSID_PASSWD,      /* AWSS parse ssid and passwd successfully */
    IOTX_AWSS_CONNECT_ADHA,         /* AWSS try to connnect adha (device discover, router solution) */
    IOTX_AWSS_CONNECT_ADHA_FAIL,    /* AWSS fails to connect adha */
    IOTX_AWSS_CONNECT_AHA,          /* AWSS try to connect aha (AP solution) */
    IOTX_AWSS_CONNECT_AHA_FAIL,     /* AWSS fails to connect aha */
    IOTX_AWSS_SETUP_NOTIFY,         /* AWSS sends out device setup information (AP and router solution) */
    IOTX_AWSS_CONNECT_ROUTER,       /* AWSS try to connect destination router */
    IOTX_AWSS_CONNECT_ROUTER_FAIL,  /* AWSS fails to connect destination router. */
    IOTX_AWSS_GOT_IP,               /* AWSS connects destination successfully and got ip address */
    IOTX_AWSS_SUC_NOTIFY,           /* AWSS sends out success notify (AWSS sucess) */
    IOTX_AWSS_BIND_NOTIFY,          /* AWSS sends out bind notify information to support bind between user and device */
    IOTX_AWSS_ENABLE_TIMEOUT,       /* AWSS enable timeout(user needs to call awss_config_press again to enable awss) */
    IOTX_CONN_CLOUD = 0x2000,       /* Device try to connect cloud */
    IOTX_CONN_CLOUD_FAIL,           /* Device fails to connect cloud, refer to net_sockets.h for error code */
    IOTX_CONN_CLOUD_SUC,            /* Device connects cloud successfully */
    IOTX_RESET = 0x3000,            /* Linkkit reset success (just got reset response from cloud without any other operation) */
};

/**
 * @brief register callback to monitor all event from system.
 *
 * @param callback, when some event occurs, the system will trigger callback to user.
 *                  refer to enum iotx_event_t for event list supported.
 *
 * @return 0 when success, -1 when fail.
 * @note: user should make sure that callback is not block and runs to complete fast.
 */
int iotx_event_regist_cb(void (*monitor_cb)(int event));

/**
 * @brief post event to trigger callback resitered by iotx_event_regist_cb
 *
 * @param event, event id, refer to iotx_event_t
 *
 * @return 0 when success, -1 when fail.
 */
int iotx_event_post(int event);

#ifndef BUILD_AOS
unsigned int aos_get_version_info(unsigned char version_num[VERSION_NUM_SIZE],
                                  unsigned char random_num[RANDOM_NUM_SIZE], unsigned char mac_address[MAC_ADDRESS_SIZE],
                                  unsigned char chip_code[CHIP_CODE_SIZE], unsigned char *output_buffer, unsigned int output_buffer_size);
#endif

typedef enum {
    ITE_AWSS_STATUS,
    ITE_CONNECT_SUCC,
    ITE_CONNECT_FAIL,
    ITE_DISCONNECTED,
    ITE_RAWDATA_ARRIVED,
    ITE_SERVICE_REQUEST,
    ITE_PROPERTY_SET,
    ITE_PROPERTY_GET,
#ifdef DEVICE_MODEL_SHADOW
    ITE_PROPERTY_DESIRED_GET_REPLY,
#endif
    ITE_REPORT_REPLY,
    ITE_TRIGGER_EVENT_REPLY,
    ITE_TIMESTAMP_REPLY,
    ITE_TOPOLIST_REPLY,
    ITE_PERMIT_JOIN,
    ITE_INITIALIZE_COMPLETED,
    ITE_FOTA,
    ITE_COTA,
    ITE_MQTT_CONNECT_SUCC
} iotx_ioctl_event_t;

#define IOT_RegisterCallback(evt, cb)           iotx_register_for_##evt(cb);
#define DECLARE_EVENT_CALLBACK(evt, cb)         DLL_IOT_API int iotx_register_for_##evt(cb);
#define DEFINE_EVENT_CALLBACK(evt, cb)          DLL_IOT_API int iotx_register_for_##evt(cb) { \
        if (evt < 0 || evt >= sizeof(g_impl_event_map)/sizeof(impl_event_map_t)) {return -1;} \
        g_impl_event_map[evt].callback = (void *)callback;return 0;}

DECLARE_EVENT_CALLBACK(ITE_AWSS_STATUS,          int (*cb)(int))
DECLARE_EVENT_CALLBACK(ITE_CONNECT_SUCC,         int (*cb)(void))
DECLARE_EVENT_CALLBACK(ITE_CONNECT_FAIL,         int (*cb)(void))
DECLARE_EVENT_CALLBACK(ITE_DISCONNECTED,         int (*cb)(void))
DECLARE_EVENT_CALLBACK(ITE_RAWDATA_ARRIVED,      int (*cb)(const int, const unsigned char *, const int))
DECLARE_EVENT_CALLBACK(ITE_SERVICE_REQUEST,       int (*cb)(const int, const char *, const int, const char *, const int,
                       char **, int *))
DECLARE_EVENT_CALLBACK(ITE_PROPERTY_SET,         int (*cb)(const int, const char *, const int))
DECLARE_EVENT_CALLBACK(ITE_PROPERTY_DESIRED_GET_REPLY,         int (*cb)(const char *, const int))
DECLARE_EVENT_CALLBACK(ITE_PROPERTY_GET,         int (*cb)(const int, const char *, const int, char **, int *))
DECLARE_EVENT_CALLBACK(ITE_REPORT_REPLY,         int (*cb)(const int, const int, const int, const char *, const int))
DECLARE_EVENT_CALLBACK(ITE_TRIGGER_EVENT_REPLY,  int (*cb)(const int, const int, const int, const char *, const int,
                       const char *, const int))
DECLARE_EVENT_CALLBACK(ITE_TIMESTAMP_REPLY,      int (*cb)(const char *))
DECLARE_EVENT_CALLBACK(ITE_TOPOLIST_REPLY,       int (*cb)(const int, const int, const int, const char *, const int))
DECLARE_EVENT_CALLBACK(ITE_PERMIT_JOIN,          int (*cb)(const char *, const int))
DECLARE_EVENT_CALLBACK(ITE_INITIALIZE_COMPLETED, int (*cb)(const int))
DECLARE_EVENT_CALLBACK(ITE_FOTA,                 int (*cb)(const int, const char *))
DECLARE_EVENT_CALLBACK(ITE_COTA,                 int (*cb)(const int, const char *, int, const char *, const char *,
                       const char *, const char *))
DECLARE_EVENT_CALLBACK(ITE_MQTT_CONNECT_SUCC,    int (*cb)(void))

void *iotx_event_callback(int evt);

#endif