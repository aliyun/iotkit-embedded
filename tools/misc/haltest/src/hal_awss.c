#include "hal_awss.h"
#include "hal_common.h"
#include "ieee80211.h"
#include "smartconfig_ieee80211.h"

static int g_first_call_flag = 1;
int g_channel = 1;

int awss_80211_frame_handler(char *buf, int length, aws_link_type_t link_type, int with_fcs, signed char rssi)
{
    struct parser_res res;
    memset(&res, 0, sizeof(res));

    /* remove FCS filed */
    if (with_fcs) {
        length -= 4;
    }

    /* useless, will be removed */
    if (is_invalid_pkg(buf, length)) {
        return -1;
    }

    awss_ieee80211_smartconfig_process(buf, length, AWSS_LINK_TYPE_80211_RADIO, &res, rssi);
}

void verify_awss_open_monitor()
{
    printf("\n/***********************************************/\n");
    printf("/*        Verify HAL_Awss_Open_Monitor         */\n");
    printf("/***********************************************/\n");
    if (g_first_call_flag) {
        printf("\n|---------------------------------------------------------------------|\n");
        printf("|          Frame Type         | Direction |  Packet Length  | Channel |\n");
        printf("|-----------------------------|-----------|-----------------|---------|\n");
        g_first_call_flag = 0;
    }

    HAL_Awss_Open_Monitor(awss_80211_frame_handler);
}

void verify_awss_close_monitor()
{
    g_first_call_flag = 1;

    printf("\n/***********************************************/\n");
    printf("/*        Verify HAL_Awss_Close_Monitor        */\n");
    printf("/***********************************************/\n");
    HAL_Awss_Close_Monitor();
}

void verify_awss_preprocess()
{
    int timeout = 0;
    char mac_str[HAL_MAC_LEN] = {0};

    printf("\n/***********************************************/\n");
    printf("/*   Verify HAL_Awss_Get_Timeout_Interval_Ms   */\n");
    timeout = HAL_Awss_Get_Timeout_Interval_Ms();
    printf("/*   Awss Timeout Interval Ms: %10d ms   */\n",timeout);
    printf("/***********************************************/\n");

    printf("\n/***********************************************/\n");
    printf("/* Verify HAL_Awss_Get_Channelscan_Interval_Ms */\n");
    timeout = HAL_Awss_Get_Channelscan_Interval_Ms();
    printf("/* Awss Channel Scan Interval Ms: %9d ms */\n",timeout);
    printf("/***********************************************/\n");

    printf("\n/***********************************************/\n");
    printf("/*           Verify HAL_Wifi_Get_Mac           */\n");
    HAL_Wifi_Get_Mac(mac_str);
    printf("/*         Wifi Mac: %17s         */\n",mac_str);
    printf("/***********************************************/\n");

    printf("\n/***********************************************/\n");
    printf("/*   Verify HAL_Awss_Switch_Channel In Loop    */\n");
    printf("/***********************************************/\n");
    
}

void verify_awss_switch_channel(char channel)
{
    HAL_Awss_Switch_Channel(channel,0,NULL);
    g_channel = channel;
}

void verfiy_awss_connect_ap(uint32_t connection_timeout_ms,
            char ssid[HAL_MAX_SSID_LEN],
            char passwd[HAL_MAX_PASSWD_LEN],
            enum AWSS_AUTH_TYPE auth,
            enum AWSS_ENC_TYPE encry,
            uint8_t bssid[ETH_ALEN],
            uint8_t channel)
{
    int res = 0;

    printf("\n/***********************************************/\n");
    printf("/*          Verify HAL_Awss_Connect_Ap         */\n");
    res = HAL_Awss_Connect_Ap(connection_timeout_ms, ssid, passwd, auth, encry, bssid, channel);
    printf("/*                  Result: %2d                 */\n",res);
    printf("/***********************************************/\n");
}

void verify_awss_get_ap_info(void)
{
    int res = 0;
    uint8_t ssid[HAL_MAX_SSID_LEN] = {0};
    uint8_t passwd[HAL_MAX_PASSWD_LEN] = {0};
    uint8_t bssid[ETH_ALEN] = {0x11,0x22,0x33,0x44,0x55,0x66};

    printf("\n/***********************************************/\n");
    printf("/*         Verify HAL_Wifi_Get_Ap_Info         */\n");
    res = HAL_Wifi_Get_Ap_Info(ssid, passwd,bssid);
    printf("/*                  Result: %2d                 */\n",res);
    printf("/*           SSID: %15s             */\n",ssid);
    printf("/*           PASSWD: %15s           */\n",passwd);
    printf("/*           BSSID: %02X:%02X:%02X:%02X:%02X:%02X          */\n",
            bssid[0],bssid[1],bssid[2],bssid[3],bssid[4],bssid[5]);
    printf("/***********************************************/\n");
}

void verify_awss_net_is_ready(void)
{
    int res = 0;

    printf("\n/***********************************************/\n");
    printf("/*         Verify HAL_Sys_Net_Is_Ready         */\n");
    res = HAL_Sys_Net_Is_Ready();
    printf("/*                  Result: %2d                 */\n",res);
    printf("/***********************************************/\n");
}