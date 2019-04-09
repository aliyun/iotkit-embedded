#include "hal_awss.h"
#include "hal_common.h"
#include "ieee80211.h"
#include "smartconfig_ieee80211.h"

static int first_call_flag = 1;

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
    printf("/***********************************************/\n");
    printf("/*        Verify HAL_Awss_Open_Monitor         */\n");
    printf("/***********************************************/\n\n");
    if (first_call_flag) {
        printf("|          Frame Type         | Direction |  Packet Length  |\n");
        printf("|-----------------------------|-----------|-----------------|\n");
        first_call_flag = 0;
    }

    HAL_Awss_Open_Monitor(awss_80211_frame_handler);
}

void verify_awss_close_monitor()
{
    first_call_flag = 1;

    printf("/***********************************************/\n");
    printf("/*        Verify HAL_Awss_Close_Monitor        */\n");
    printf("/***********************************************/\n\n");
    HAL_Awss_Close_Monitor();
}

void verify_awss_preprocess(char channel)
{
    int timeout = 0;
    char mac_str[HAL_MAC_LEN] = {0};

    printf("\n\n");
    printf("/***********************************************/\n");
    printf("/*   Verify HAL_Awss_Get_Timeout_Interval_Ms   */\n");
    timeout = HAL_Awss_Get_Timeout_Interval_Ms();
    printf("/*   Awss Timeout Interval Ms: %10d ms   */\n",timeout);
    printf("/***********************************************/\n\n");

    printf("/***********************************************/\n");
    printf("/* Verify HAL_Awss_Get_Channelscan_Interval_Ms */\n");
    timeout = HAL_Awss_Get_Channelscan_Interval_Ms();
    printf("/* Awss Channel Scan Interval Ms: %9d ms */\n");
    printf("/***********************************************/\n\n");

    printf("/***********************************************/\n");
    printf("/*           Verify HAL_Wifi_Get_Mac           */\n");
    HAL_Wifi_Get_Mac(mac_str);
    printf("/*         Wifi Mac: %17s         */\n",mac_str);
    printf("/***********************************************/\n\n");

    printf("/***********************************************/\n");
    printf("/*       Verify HAL_Awss_Switch_Channel        */\n");
    HAL_Awss_Switch_Channel(channel,0,NULL);
    printf("/*            Wifi Channel Set: %3d            */\n",channel);
    printf("/***********************************************/\n\n");
    
}