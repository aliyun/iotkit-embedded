/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <stdarg.h>
#include <stdio.h>
#include "infra_config.h"
#include "infra_defs.h"

#if defined(WIFI_PROVISION_ENABLED)

#define  HAL_DEBUG_OUT 1

/*This variable is to define the name of the network interface, you must change it to yours*/
static wchar_t *gIfName = L"\\DEVICE\\TCPIP_{C9E93150-C6B1-4B15-8A2E-C08747261CD2}";

/**
 * @brief   设置Wi-Fi网卡工作在监听(Monitor)模式, 并在收到802.11帧的时候调用被传入的回调函数
 *
 * @param[in] cb @n A function pointer, called back when wifi receive a frame.
 */
void HAL_Awss_Open_Monitor(_IN_ awss_recv_80211_frame_cb_t cb)
{
}

/**
 * @brief   设置Wi-Fi网卡离开监听(Monitor)模式, 并开始以站点(Station)模式工作
 */
void HAL_Awss_Close_Monitor(void)
{
}

/**
 * @brief   设置Wi-Fi网卡切换到指定的信道(channel)上
 *
 * @param[in] primary_channel @n Primary channel.
 * @param[in] secondary_channel @n Auxiliary channel if 40Mhz channel is supported, currently
 *              this param is always 0.
 * @param[in] bssid @n A pointer to wifi BSSID on which awss lock the channel, most HAL
 *              may ignore it.
 */
void HAL_Awss_Switch_Channel(
            _IN_ char primary_channel,
            _IN_OPT_ char secondary_channel,
            _IN_OPT_ uint8_t bssid[ETH_ALEN])
{
}

/**
 * @brief   要求Wi-Fi网卡连接指定热点(Access Point)的函数
 *
 * @param[in] connection_timeout_ms @n AP connection timeout in ms or HAL_WAIT_INFINITE
 * @param[in] ssid @n AP ssid
 * @param[in] passwd @n AP passwd
 * @param[in] auth @n optional(AWSS_AUTH_TYPE_INVALID), AP auth info
 * @param[in] encry @n optional(AWSS_ENC_TYPE_INVALID), AP encry info
 * @param[in] bssid @n optional(NULL or zero mac address), AP bssid info
 * @param[in] channel @n optional, AP channel info
 * @return
   @verbatim
     = 0: connect AP & DHCP success
     = -1: connect AP or DHCP fail/timeout
   @endverbatim
 * @see None.
 * @note
 *      If the STA connects the old AP, HAL should disconnect from the old AP firstly.
 *      If bssid specifies the dest AP, HAL should use bssid to connect dest AP.
 */
int HAL_Awss_Connect_Ap(
            _IN_ uint32_t connection_timeout_ms,
            _IN_ char ssid[HAL_MAX_SSID_LEN],
            _IN_ char passwd[HAL_MAX_PASSWD_LEN],
            _IN_OPT_ enum AWSS_AUTH_TYPE auth,
            _IN_OPT_ enum AWSS_ENC_TYPE encry,
            _IN_OPT_ uint8_t bssid[ETH_ALEN],
            _IN_OPT_ uint8_t channel)
{
    return 0;
}

/**
 * @brief check system network is ready(get ip address) or not.
 *
 * @param None.
 * @return 0, net is not ready; 1, net is ready.
 * @see None.
 * @note None.
 */
int HAL_Sys_Net_Is_Ready()
{
    return 0;
}

/**
 * @brief   在当前信道(channel)上以基本数据速率(1Mbps)发送裸的802.11帧(raw 802.11 frame)
 *
 * @param[in] type @n see enum HAL_Awss_frame_type, currently only FRAME_BEACON
 *                      FRAME_PROBE_REQ is used
 * @param[in] buffer @n 80211 raw frame, include complete mac header & FCS field
 * @param[in] len @n 80211 raw frame length
 * @return
   @verbatim
   =  0, send success.
   = -1, send failure.
   = -2, unsupported.
   @endverbatim
 * @see None.
 * @note awss use this API send raw frame in wifi monitor mode & station mode
 */
int HAL_Wifi_Send_80211_Raw_Frame(_IN_ enum HAL_Awss_Frame_Type type,
                                  _IN_ uint8_t *buffer, _IN_ int len)
{
    return 0;
}


/**
 * @brief   在站点(Station)模式下使能或禁用对管理帧的过滤
 *
 * @param[in] filter_mask @n see mask macro in enum HAL_Awss_frame_type,
 *                      currently only FRAME_PROBE_REQ_MASK & FRAME_BEACON_MASK is used
 * @param[in] vendor_oui @n oui can be used for precise frame match, optional
 * @param[in] callback @n see awss_wifi_mgmt_frame_cb_t, passing 80211
 *                      frame or ie to callback. when callback is NULL
 *                      disable sniffer feature, otherwise enable it.
 * @return
   @verbatim
   =  0, success
   = -1, fail
   = -2, unsupported.
   @endverbatim
 * @see None.
 * @note awss use this API to filter specific mgnt frame in wifi station mode
 */
int HAL_Wifi_Enable_Mgmt_Frame_Filter(
            _IN_ uint32_t filter_mask,
            _IN_OPT_ uint8_t vendor_oui[3],
            _IN_ awss_wifi_mgmt_frame_cb_t callback)
{
    return 0;
}

/**
 * @brief   获取所连接的热点(Access Point)的信息
 *
 * @param[out] ssid: array to store ap ssid. It will be null if ssid is not required.
 * @param[out] passwd: array to store ap password. It will be null if ap password is not required.
 * @param[out] bssid: array to store ap bssid. It will be null if bssid is not required.
 * @return
   @verbatim
     = 0: succeeded
     = -1: failed
   @endverbatim
 * @see None.
 * @note
 *     If the STA dosen't connect AP successfully, HAL should return -1 and not touch the ssid/passwd/bssid buffer.
 */
int HAL_Wifi_Get_Ap_Info(
            _OU_ char ssid[HAL_MAX_SSID_LEN],
            _OU_ char passwd[HAL_MAX_PASSWD_LEN],
            _OU_ uint8_t bssid[ETH_ALEN])
{
    return 0;
}

/* @brief   打开当前设备热点，并把设备由SoftAP模式切换到AP模式
 */
int HAL_Awss_Open_Ap(const char *ssid, const char *passwd, int beacon_interval, int hide)
{
    return 0;
}

/* @brief   关闭当前设备热点，并把设备由SoftAP模式切换到Station模式
*/
int HAL_Awss_Close_Ap()
{
    return 0;
}

#endif  /* #if defined(HAL_AWSS) */

#ifdef DEV_BIND_ENABLED

/**
 * @brief   获取Wi-Fi网口的MAC地址, 格式应当是"XX:XX:XX:XX:XX:XX"
 *
 * @param   mac_str : 用于存放MAC地址字符串的缓冲区数组
 * @return  指向缓冲区数组起始位置的字符指针
 */
char *HAL_Wifi_Get_Mac(char mac_str[HAL_MAC_LEN])
{
    MIB_IFTABLE *pIfTable = NULL;
    MIB_IFROW *pIfRow = NULL;
    DWORD dwSize = 0;
    int i;

    mac_str[0] = 0;
    if (GetIfTable((MIB_IFTABLE *)&dwSize, &dwSize, 0) != ERROR_INSUFFICIENT_BUFFER) {
        printf("Failed to get if table at step 1 in HAL_Wifi_Get_Mac\n\r");
        return (NULL);
    }
    pIfTable = (MIB_IFTABLE *) malloc(dwSize);
    if (pIfTable == NULL) {
        return (NULL);
    }
    if (GetIfTable((MIB_IFTABLE *)pIfTable, &dwSize, 0) != NO_ERROR) {
        free(pIfTable);
        printf("Failed to get if table (HAL_Wifi_Get_Mac)\n\r");
        return (NULL);
    }

    for (i = 0; i < pIfTable->dwNumEntries; i++) {
        pIfRow = (MIB_IFROW *) & pIfTable->table[i];
        if (wcscmp(pIfRow->wszName, gIfName) == 0) {
            break;
        }
    }
    if (pIfRow == NULL) {
        free(pIfTable);
        return NULL;
    }

    if (i < pIfTable->dwNumEntries) {
        snprintf(mac_str, HAL_MAC_LEN, "%02X:%02X:%02X:%02X:%02X:%02X",
                 pIfRow->bPhysAddr[0], pIfRow->bPhysAddr[1], pIfRow->bPhysAddr[2],
                 pIfRow->bPhysAddr[3], pIfRow->bPhysAddr[4], pIfRow->bPhysAddr[5]);
        if (HAL_DEBUG_OUT) {
            printf("WIFI MAC is %s\n\r", mac_str);
        }
    }

    free(pIfTable);
    return (mac_str);
}

#endif

