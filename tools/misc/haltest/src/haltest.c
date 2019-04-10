#include <stdio.h>
#include <unistd.h>
#include "hal_common.h"

#define TEST_SWITCH_CHANNEL_INTERNVAL_MS (250)

int switch_channel()
{
    static int index = 0;
    int channels[] = {1,2,3,4,5,6,7,8,9,10,11,12,13};

    if (index == sizeof(channels)/sizeof(int)) {
        index = 1;
    }else{
        index++;
    }

    return index;
}

int main(int argc, char *argv[])
{
    int time_passed = 0;
    uint8_t bssid[ETH_ALEN] = {0x11,0x22,0x33,0x44,0x55,0x66};

    verify_awss_preprocess();
    verify_awss_close_monitor();
    verify_awss_open_monitor();

     while(1) {
         if (time_passed > 10 *1000) {
             break;
         }
         verify_awss_switch_channel(switch_channel());
         usleep(TEST_SWITCH_CHANNEL_INTERNVAL_MS*1000);
         time_passed += TEST_SWITCH_CHANNEL_INTERNVAL_MS;
    }

    verfiy_awss_connect_ap(5000, "ssid", "passwd", AWSS_AUTH_TYPE_WPAPSKWPA2PSK, AWSS_ENC_TYPE_AES, bssid, 6);
    verify_awss_get_ap_info();
    verify_awss_net_is_ready();

    return 0;
}