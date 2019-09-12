/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include "wifi_provision_internal.h"

#ifdef DEV_BIND_ENABLED
    #include "bind_api.h"
#endif

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif


int awss_connect(char ssid[HAL_MAX_SSID_LEN], char passwd[HAL_MAX_PASSWD_LEN], uint8_t *bssid, uint8_t bssid_len,
                 uint8_t *token, uint8_t token_len)
{
    unsigned char final_token[AWSS_TOKEN_LEN] = {0};
    unsigned char final_bssid[6] = {0};

    uint8_t has_bssid = 1;
    int ret;

    /*need to complete the token*/
#ifdef DEV_BIND_ENABLED
    ret = IOT_Bind_SetToken_Ext(token, token_len, passwd, bssid, bssid_len);
#endif
    /*
        ret = awss_complete_token(passwd, bssid, bssid_len, token, token_len, final_token);

        if(ret == 0) {
            awss_set_token(final_token);
        }
    */
    /*need to complete the bssid */
    if (bssid_len > 0 && bssid_len < 6 && bssid != NULL) {
        if (zc_bssid != NULL) {
            memcpy(final_bssid, zc_bssid, 6);
        } else {
            has_bssid = 0;
        }
    } else if (bssid_len == 6 && bssid != NULL) {
        memcpy(final_bssid, bssid, 6);
    } else {
        has_bssid = 0;
    }

    return HAL_Awss_Connect_Ap(WLAN_CONNECTION_TIMEOUT_MS, ssid, passwd, 0, 0, has_bssid ? final_bssid : NULL, 0);
}


#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif
