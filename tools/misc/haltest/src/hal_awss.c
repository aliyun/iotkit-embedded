#include "hal_awss.h"
#include "hal_common.h"
#include "zconfig_ieee80211.h"
#include "smartconfig_ieee80211.h"

int smartconfig_80211_frame_handler(char *buf, int length, aws_link_type_t link_type, int with_fcs, signed char rssi)
{
    struct parser_res res;
    static int first_call_flag = 1;
    memset(&res, 0, sizeof(res));

    /* remove FCS filed */
    if (with_fcs) {
        length -= 4;
    }

    /* useless, will be removed */
    if (is_invalid_pkg(buf, length)) {
        return -1;
    }

    if (first_call_flag) {
        printf("\n\n|          Frame Type         | From DS |  Packet Length  |\n");
        first_call_flag = 0;
    }
    awss_ieee80211_smartconfig_process(buf, length, AWSS_LINK_TYPE_80211_RADIO, &res, rssi);
}

void verify_awss_rx_frame(void)
{
    HAL_Awss_Open_Monitor(smartconfig_80211_frame_handler);
}