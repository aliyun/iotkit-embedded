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

void aos_get_version_hex(unsigned char version[VERSION_NUM_SIZE]);

#ifndef BUILD_AOS
unsigned int aos_get_version_info(unsigned char version_num[VERSION_NUM_SIZE],
                                  unsigned char random_num[RANDOM_NUM_SIZE], unsigned char mac_address[MAC_ADDRESS_SIZE],
                                  unsigned char chip_code[CHIP_CODE_SIZE], unsigned char *output_buffer, unsigned int output_buffer_size);
#endif
#endif