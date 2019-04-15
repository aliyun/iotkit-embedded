/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include "wifi_provision_internal.h"


#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

int awss_connect(char ssid[HAL_MAX_SSID_LEN], char passwd[HAL_MAX_PASSWD_LEN], uint8_t bssid[ETH_ALEN], 
                 uint8_t token[AWSS_TOKEN_LEN] )
{

    if(token != NULL) {    
        awss_set_token(token);
    }
    return HAL_Awss_Connect_Ap(WLAN_CONNECTION_TIMEOUT_MS, ssid, passwd, 0, 0, (uint8_t *)bssid, 0);

}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif
