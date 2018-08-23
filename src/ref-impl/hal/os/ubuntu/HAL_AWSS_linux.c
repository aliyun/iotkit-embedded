/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <string.h>
#include "iot_import.h"
#include "ali_crypto.h"

    /**
     * @brief   启动一次Wi-Fi的空中扫描(Scan)
     *
     * @param[in] cb @n pass ssid info(scan result) to this callback one by one
     * @return 0 for wifi scan is done, otherwise return -1
     * @see None.
     * @note
     *      This API should NOT exit before the invoking for cb is finished.
     *      This rule is something like the following :
     *      HAL_Wifi_Scan() is invoked...
     *      ...
     *      for (ap = first_ap; ap <= last_ap; ap = next_ap){
     *        cb(ap)
     *      }
     *      ...
     *      HAL_Wifi_Scan() exit...
     */
    int HAL_Wifi_Scan(awss_wifi_scan_result_cb_t cb)
    {
        /*
        netmgr_register_wifi_scan_result_callback(
          (netmgr_wifi_scan_result_cb_t)cb);
        hal_wifi_start_scan_adv(NULL);

        while (netmgr_get_scan_cb_finished() != true) { // block
            aos_msleep(50);
        }
        */

        return 0;
    }

    /**
     * @brief   获取所连接的热点(Access Point)的信息
     *
     * @param[out] ssid: array to store ap ssid. It will be null if ssid is not
     required.
     * @param[out] passwd: array to store ap password. It will be null if ap
     password is not required.
     * @param[out] bssid: array to store ap bssid. It will be null if bssid is
     not required.
     * @return
       @verbatim
         = 0: succeeded
         = -1: failed
       @endverbatim
     * @see None.
     * @note None.
     */
    int HAL_Wifi_Get_Ap_Info(_OU_ char ssid[HAL_MAX_SSID_LEN],
                             _OU_ char passwd[HAL_MAX_PASSWD_LEN],
                             _OU_ uint8_t bssid[ETH_ALEN])
    {
        /*
        netmgr_ap_config_t config = { 0 };

        netmgr_get_ap_config(&config);
        if (ssid) {
            strncpy(ssid, config.ssid, HAL_MAX_SSID_LEN - 1);
        }
        if (passwd) {
            strncpy(passwd, config.pwd, HAL_MAX_PASSWD_LEN - 1);
        }
        if (bssid) {
            memcpy(bssid, config.bssid, ETH_ALEN);
        }
        */

        return 0;
    }

/**
     * @brief   在当前信道(channel)上以基本数据速率(1Mbps)发送裸的802.11帧(raw
     802.11 frame)
     *
     * @param[in] type @n see enum HAL_Awss_frame_type, currently only
     FRAME_BEACON
     *                      FRAME_PROBE_REQ is used
     * @param[in] buffer @n 80211 raw frame, include complete mac header & FCS
     field
     * @param[in] len @n 80211 raw frame length
     * @return
       @verbatim
       =  0, send success.
       = -1, send failure.
       = -2, unsupported.
       @endverbatim
     * @see None.
     * @note awss use this API send raw frame in wifi monitor mode & station
     mode
     */
    int HAL_Wifi_Send_80211_Raw_Frame(_IN_ enum HAL_Awss_Frame_Type type,
                                      _IN_ uint8_t *buffer, _IN_ int len)
    {
   //     return hal_wlan_send_80211_raw_frame(NULL, buffer, len);
        return 0;
    }

    /**
     * @brief   在站点(Station)模式下使能或禁用对管理帧的过滤
     *
     * @param[in] filter_mask @n see mask macro in enum HAL_Awss_frame_type,
     *                      currently only FRAME_PROBE_REQ_MASK &
     FRAME_BEACON_MASK is used
     * @param[in] vendor_oui @n oui can be used for precise frame match,
     optional
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
     * @note awss use this API to filter specific mgnt frame in wifi station
     mode
     */
    int HAL_Wifi_Enable_Mgmt_Frame_Filter(
      _IN_ uint32_t filter_mask, _IN_OPT_ uint8_t vendor_oui[3],
      _IN_ awss_wifi_mgmt_frame_cb_t callback)
    {
        /*
        monitor_cb = callback;

        if (callback != NULL) {
            hal_wlan_register_mgnt_monitor_cb(NULL, mgnt_rx_cb);
        } else {
            hal_wlan_register_mgnt_monitor_cb(NULL, NULL);
        }
        */
        return 0;
    }

    /**
     * @brief   获取配网服务(`AWSS`)的超时时间长度, 单位是毫秒
     *
     * @return  超时时长, 单位是毫秒
     * @note    推荐时长是60,0000毫秒
     */
    int HAL_Awss_Get_Timeout_Interval_Ms(void)
    {
        return 3 * 60 * 1000;
    }

    /**
     * @brief   获取配网服务(`AWSS`)超时时长到达之后,
     * 去连接默认SSID时的超时时长, 单位是毫秒
     *
     * @return  超时时长, 单位是毫秒
     * @note    推荐时长是0毫秒, 含义是永远持续
     */
    int HAL_Awss_Get_Connect_Default_Ssid_Timeout_Interval_Ms(void)
    {
        return 0;
    }

    /**
     * @brief   获取在每个信道(`channel`)上扫描的时间长度, 单位是毫秒
     *
     * @return  时间长度, 单位是毫秒
     * @note    推荐时长是200毫秒到400毫秒
     */
    int HAL_Awss_Get_Channelscan_Interval_Ms(void)
    {
        return 200;
    }

    /**
     * @brief   802.11帧的处理函数, 可以将802.11 Frame传递给这个函数
     *
     * @param[in] buf @n 80211 frame buffer, or pointer to struct ht40_ctrl
     * @param[in] length @n 80211 frame buffer length
     * @param[in] link_type @n AWSS_LINK_TYPE_NONE for most rtos HAL,
     *              and for linux HAL, do the following step to check
     *              which header type the driver supported.
       @verbatim
                   a) iwconfig wlan0 mode monitor    #open monitor mode
                   b) iwconfig wlan0 channel 6    #switch channel 6
                   c) tcpdump -i wlan0 -s0 -w file.pacp    #capture 80211 frame
     & save d) open file.pacp with wireshark or omnipeek check the link header
     type and fcs included or not
       @endverbatim
     * @param[in] with_fcs @n 80211 frame buffer include fcs(4 byte) or not
     * @param[in] rssi @n rssi of packet
     */
    /*
    typedef int (*awss_recv_80211_frame_cb_t)(char *buf, int length,
                                              enum AWSS_LINK_TYPE link_type,
                                              int with_fcs, signed char rssi);

    awss_recv_80211_frame_cb_t g_ieee80211_handler;

    static void monitor_data_handler(uint8_t *buf, int len,
                                     hal_wifi_link_info_t *info)
    {
        int with_fcs  = 0;
        int link_type = AWSS_LINK_TYPE_NONE;

        (*g_ieee80211_handler)((char *)buf, len, link_type, with_fcs,
                               info->rssi);
    }
    */

    /**
     * @brief   设置Wi-Fi网卡工作在监听(Monitor)模式,
     * 并在收到802.11帧的时候调用被传入的回调函数
     *
     * @param[in] cb @n A function pointer, called back when wifi receive a
     * frame.
     */
    void HAL_Awss_Open_Monitor(_IN_ awss_recv_80211_frame_cb_t cb)
    {
        /*
        hal_wifi_module_t *module = hal_wifi_get_default_module();

        if (module == NULL) {
            return;
        }

        g_ieee80211_handler = cb;
        hal_wifi_register_monitor_cb(module, monitor_data_handler);
        hal_wifi_start_wifi_monitor(module);
        HAL_Awss_Switch_Channel(6, 0, NULL);
        */
    }

    /**
     * @brief   设置Wi-Fi网卡离开监听(Monitor)模式,
     * 并开始以站点(Station)模式工作
     */
    void HAL_Awss_Close_Monitor(void)
    {
        /*
        hal_wifi_module_t *module;

        module = hal_wifi_get_default_module();
        if (module == NULL) {
            return;
        }

        hal_wifi_register_monitor_cb(module, NULL);
        hal_wifi_stop_wifi_monitor(module);
        */
    }

    /**
     * @brief   设置Wi-Fi网卡切换到指定的信道(channel)上
     *
     * @param[in] primary_channel @n Primary channel.
     * @param[in] secondary_channel @n Auxiliary channel if 40Mhz channel is
     * supported, currently this param is always 0.
     * @param[in] bssid @n A pointer to wifi BSSID on which awss lock the
     * channel, most HAL may ignore it.
     */
    void HAL_Awss_Switch_Channel(_IN_ char     primary_channel,
                                 _IN_OPT_ char secondary_channel,
                                 _IN_OPT_ uint8_t bssid[ETH_ALEN])
    {
        /*
        hal_wifi_module_t *module;

        module = hal_wifi_get_default_module();
        if (module == NULL) {
            return;
        }

        hal_wifi_set_channel(module, (int)primary_channel);
        */
    }

    /**
     * @brief   要求Wi-Fi网卡连接指定热点(Access Point)的函数
     *
     * @param[in] connection_timeout_ms @n AP connection timeout in ms or
     HAL_WAIT_INFINITE
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
     *      If the STA connects the old AP, HAL should disconnect from the old
     AP firstly.
     */
    int HAL_Awss_Connect_Ap(_IN_ uint32_t connection_timeout_ms,
                            _IN_ char     ssid[HAL_MAX_SSID_LEN],
                            _IN_ char     passwd[HAL_MAX_PASSWD_LEN],
                            _IN_OPT_ enum AWSS_AUTH_TYPE auth,
                            _IN_OPT_ enum AWSS_ENC_TYPE  encry,
                            _IN_OPT_ uint8_t bssid[ETH_ALEN],
                            _IN_OPT_ uint8_t channel)
    {
        /*
        int                ms_cnt = 0;
        netmgr_ap_config_t config = { 0 };
        if (ssid != NULL) {
            strncpy(config.ssid, ssid, sizeof(config.ssid) - 1);
        }
        if (passwd != NULL) {
            strncpy(config.pwd, passwd, sizeof(config.pwd) - 1);
        }
        if (bssid != NULL) {
            memcpy(config.bssid, bssid, ETH_ALEN);
        }
        netmgr_set_ap_config(&config);
#ifndef ESP8266_CONFIG
        printf("------------------------suspend station\n");
        hal_wifi_suspend_station(NULL);
#endif
        // LOGI("aos_awss", "Will reconnect wifi: %s %s", ssid, passwd);
        netmgr_reconnect_wifi();

        while (ms_cnt < connection_timeout_ms) {
            if (netmgr_get_ip_state() == false) {
                LOGD("[waitConnAP]", "waiting for connecting AP");
                aos_msleep(500);
                ms_cnt += 500;
            } else {
                LOGI("[waitConnAP]", "AP connected");
                return 0;
            }
        }
        */

        return -1;
    }

    /**
     * @brief   获取`smartconfig`服务的安全等级
     *
     * @param None.
     * @return The security level:
     @verbatim
        0: open (no encrypt)
        1: aes256cfb with default aes-key and aes-iv
        2: aes128cfb with default aes-key and aes-iv
        3: aes128cfb with aes-key per product and aes-iv = 0
        4: aes128cfb with aes-key per device and aes-iv = 0
        5: aes128cfb with aes-key per manufacture and aes-iv = 0
        others: invalid
    @endverbatim
    * @see None.
    */
    int HAL_Awss_Get_Encrypt_Type()
    {
        return 3;
    }

    /**
     * @brief    Get Security level for wifi configuration with connection.
     *           Used for AP solution of router and App.
     *
     * @param None.
     * @return The security level:
     @verbatim
        3: aes128cfb with aes-key per product and aes-iv = random
        4: aes128cfb with aes-key per device and aes-iv = random
        5: aes128cfb with aes-key per manufacture and aes-iv = random
        others: invalid
    @endverbatim
    * @see None.
    */
    int HAL_Awss_Get_Conn_Encrypt_Type()
    {
        return 4;
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
 //       return netmgr_get_ip_state() == true ? 1 : 0;
        return 0;
    }

#ifdef __cplusplus
}
#endif
