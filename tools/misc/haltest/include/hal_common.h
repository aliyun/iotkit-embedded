#ifndef _HAL_COMMON_H_
#define _HAL_COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define HAL_MAC_LEN (17 + 1) /* XX:XX:XX:XX:XX:XX */
#define ETH_ALEN (6)
#define NETWORK_ADDR_LEN        (16)
#define HAL_MAC_LEN             (17 + 1)    /* MAC地址的长度 */
#define STR_SHORT_LEN                   (32)
#define HAL_MAX_SSID_LEN                (32 + 1)    /* ssid: 32 octets at most, include the NULL-terminated */
#define HAL_MAX_PASSWD_LEN              (64 + 1)    /* password: 8-63 ascii */
#define WLAN_CONNECTION_TIMEOUT_MS      (30 * 1000)
#define ZC_MAX_SSID_LEN HAL_MAX_SSID_LEN
#define ZC_MAX_PASSWD_LEN HAL_MAX_PASSWD_LEN

enum AWSS_AUTH_TYPE {
    AWSS_AUTH_TYPE_OPEN,
    AWSS_AUTH_TYPE_SHARED,
    AWSS_AUTH_TYPE_WPAPSK,
    AWSS_AUTH_TYPE_WPA8021X,
    AWSS_AUTH_TYPE_WPA2PSK,
    AWSS_AUTH_TYPE_WPA28021X,
    AWSS_AUTH_TYPE_WPAPSKWPA2PSK,
    AWSS_AUTH_TYPE_MAX = AWSS_AUTH_TYPE_WPAPSKWPA2PSK,
    AWSS_AUTH_TYPE_INVALID = 0xff,
};

enum AWSS_ENC_TYPE {
    AWSS_ENC_TYPE_NONE,
    AWSS_ENC_TYPE_WEP,
    AWSS_ENC_TYPE_TKIP,
    AWSS_ENC_TYPE_AES,
    AWSS_ENC_TYPE_TKIPAES,
    AWSS_ENC_TYPE_MAX = AWSS_ENC_TYPE_TKIPAES,
    AWSS_ENC_TYPE_INVALID = 0xff,
};

/* link type */
typedef enum {
    AWSS_LINK_TYPE_NONE,
    AWSS_LINK_TYPE_PRISM,
    AWSS_LINK_TYPE_80211_RADIO,
    AWSS_LINK_TYPE_80211_RADIO_AVS
} aws_link_type_t;

typedef int (*awss_recv_80211_frame_cb_t)(char *buf, int length,
        aws_link_type_t link_type, int with_fcs, signed char rssi);

typedef enum {
    os_thread_priority_idle = -3,        /* priority: idle (lowest) */
    os_thread_priority_low = -2,         /* priority: low */
    os_thread_priority_belowNormal = -1, /* priority: below normal */
    os_thread_priority_normal = 0,       /* priority: normal (default) */
    os_thread_priority_aboveNormal = 1,  /* priority: above normal */
    os_thread_priority_high = 2,         /* priority: high */
    os_thread_priority_realtime = 3,     /* priority: realtime (highest) */
    os_thread_priority_error = 0x84,     /* system cannot determine priority or thread has illegal priority */
} hal_os_thread_priority_t;

typedef struct _hal_os_thread {
    hal_os_thread_priority_t priority;     /*initial thread priority */
    void                    *stack_addr;   /* thread stack address malloced by caller, use system stack by . */
    int                   stack_size;   /* stack size requirements in bytes; 0 is default stack size */
    int                      detach_state; /* 0: not detached state; otherwise: detached state. */
    char                    *name;         /* thread name. */
} hal_os_thread_param_t;

int HAL_ThreadCreate(
            void **thread_handle,
            void *(*work_routine)(void *),
            void *arg,
            hal_os_thread_param_t *hal_os_thread_param,
            int *stack_used);

void HAL_Awss_Open_Monitor(awss_recv_80211_frame_cb_t cb);
void HAL_Awss_Close_Monitor(void);
int HAL_Awss_Get_Timeout_Interval_Ms(void);
int HAL_Awss_Get_Channelscan_Interval_Ms(void);
char *HAL_Wifi_Get_Mac(char mac_str[HAL_MAC_LEN]);
void HAL_Awss_Switch_Channel(char primary_channel,char secondary_channel,uint8_t bssid[ETH_ALEN]);
int HAL_Awss_Connect_Ap(
            uint32_t connection_timeout_ms,
            char ssid[HAL_MAX_SSID_LEN],
            char passwd[HAL_MAX_PASSWD_LEN],
            enum AWSS_AUTH_TYPE auth,
            enum AWSS_ENC_TYPE encry,
            uint8_t bssid[ETH_ALEN],
            uint8_t channel);
int HAL_Wifi_Get_Ap_Info(
            char ssid[HAL_MAX_SSID_LEN],
            char passwd[HAL_MAX_PASSWD_LEN],
            uint8_t bssid[ETH_ALEN]);
int HAL_Sys_Net_Is_Ready(void);

#endif