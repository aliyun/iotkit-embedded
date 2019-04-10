#ifndef _HAL_AWSS_H_
#define _HAL_AWSS_H_

#include "hal_common.h"

void verify_awss_preprocess();
void verify_awss_open_monitor();
void verify_awss_close_monitor();
void verify_awss_switch_channel(char channel);
void verfiy_awss_connect_ap(uint32_t connection_timeout_ms,
            char ssid[HAL_MAX_SSID_LEN],
            char passwd[HAL_MAX_PASSWD_LEN],
            enum AWSS_AUTH_TYPE auth,
            enum AWSS_ENC_TYPE encry,
            uint8_t bssid[ETH_ALEN],
            uint8_t channel);
void verify_awss_get_ap_info(void);
void verify_awss_net_is_ready(void);

#endif