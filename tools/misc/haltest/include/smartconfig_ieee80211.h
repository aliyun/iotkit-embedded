#ifndef _SMARTCONFIG_IEEE80211_H_
#define _SMARTCONFIG_IEEE80211_H_

#include "hal_common.h"

#define PAYLOAD_BITS_CNT    (7)

enum _ZC_ENC_TYPE_ {
    ZC_ENC_TYPE_NONE,
    ZC_ENC_TYPE_WEP,
    ZC_ENC_TYPE_TKIP,
    ZC_ENC_TYPE_AES,
    ZC_ENC_TYPE_TKIPAES,
    ZC_ENC_TYPE_MAX = ZC_ENC_TYPE_TKIPAES,
    ZC_ENC_TYPE_INVALID = 0xff,
};

int awss_ieee80211_smartconfig_process(uint8_t *ieee80211, int len, int link_type, struct parser_res *res, char rssi);

void verify_smartconfig_raw_frame();
#endif