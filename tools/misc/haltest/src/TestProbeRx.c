#include "hal_common.h"
#include "hal_awss.h"
#include "zconfig_protocol.h"
#include "ieee80211.h"

 int is_invalid_pkg(void *pkt_data, uint32_t pkt_length);
/*
 *  * make sure 80211 frame is word align, otherwise struct ieee80211_hdr will bug
 *   * TODO: code refactor, avoid using memmove
 *    */
#define demo_check_ieee80211_buf_alignment(buf_addr, len) \
do {\
    if (((unsigned long)(buf_addr) & 0x1) && len > 0) {\
        uint8_t *word_align_addr = (uint8_t *)((unsigned long)(buf_addr) & ~0x1);\
        memmove(word_align_addr, buf_addr, len);\
        buf_addr = word_align_addr;\
    }\
} while (0)

uint8_t *demo_zconfig_remove_link_header(uint8_t **in, int *len, int link_type)
{
    int lt_len = 0;

    switch (link_type) {
    case AWSS_LINK_TYPE_NONE:
        break;
    case AWSS_LINK_TYPE_PRISM:
#define PRISM_HDR_LEN           144
        *in += PRISM_HDR_LEN;
        *len -= PRISM_HDR_LEN;
        /* 144, no need to check buf aligment */
        break;
    case AWSS_LINK_TYPE_80211_RADIO:
        lt_len = ieee80211_get_radiotap_len(*in);
        *in += lt_len;
        *len -= lt_len;
        demo_check_ieee80211_buf_alignment(*in, *len);
        break;
    case AWSS_LINK_TYPE_80211_RADIO_AVS:
#define WLANCAP_MAGIC_COOKIE_V1 0x80211001
#define WLANCAP_MAGIC_COOKIE_V2 0x80211002
        lt_len = *(uint32_t *)(*in + 4);/* first 4 byte is magic code */
        *in += lt_len;
        *len -= lt_len;
        demo_check_ieee80211_buf_alignment(*in, *len);
        break;
    default:
        printf("un-supported link type!\r\n");
        break;
    }

    return *in;
}

int verify_awss_ieee80211_zconfig_process(uint8_t *mgmt_header, int len, int link_type, struct parser_res *res, signed char rssi)
{
    const uint8_t *registrar_ie = NULL;
    struct ieee80211_hdr *hdr;
    uint16_t ieoffset;
    int fc;

    /*
 *      * when device try to connect current router (include adha and aha)
 *           * skip the new aha and process the new aha in the next scope.
 *                */
    if (mgmt_header == NULL)
        return ALINK_INVALID;
    /*
 *      * we don't process zconfig used by enrollee until user press configure button
 *           */

    hdr = (struct ieee80211_hdr *)mgmt_header;
    fc = hdr->frame_control;

    if (!ieee80211_is_probe_req(fc) && !ieee80211_is_probe_resp(fc))
        return ALINK_INVALID;

    ieoffset = offset_of(struct ieee80211_mgmt, u.probe_resp.variable);
    if (ieoffset > len)
        return ALINK_INVALID;

    registrar_ie = (const uint8_t *)cfg80211_find_vendor_ie(WLAN_OUI_ALIBABA,
            WLAN_OUI_TYPE_REGISTRAR, mgmt_header + ieoffset, len - ieoffset);
    if (registrar_ie == NULL)
        return ALINK_INVALID;

    res->u.ie.alink_ie_len = len - (registrar_ie - mgmt_header);
    res->u.ie.alink_ie = (uint8_t *)registrar_ie;
    printf("verify got zeroconfig reply\n");

    return ALINK_ZERO_CONFIG;
}

int demo_ieee80211_data_extract(uint8_t *in, int len, int link_type, struct parser_res *res, signed char rssi)
{
    struct ieee80211_hdr *hdr;
    int alink_type = ALINK_INVALID;
    int i, fc;

    hdr = (struct ieee80211_hdr *)demo_zconfig_remove_link_header(&in, &len, link_type);
    if (len <= 0) {
        return -1;
    }
    fc = hdr->frame_control;
    verify_awss_ieee80211_zconfig_process((uint8_t *)hdr, len, link_type, res, rssi);
    return 0;
}

int demo_zconfig_recv_callback(void *pkt_data, uint32_t pkt_length, uint8_t channel,
                          int link_type, int with_fcs, signed char rssi)
{
    struct parser_res res;
    memset(&res, 0, sizeof(res));

    /* remove FCS filed */
    if (with_fcs) {
        pkt_length -= 4;
    }

    /* useless, will be removed */
    if (is_invalid_pkg(pkt_data, pkt_length)) {
        return -1;
    }

    res.channel = channel;

    demo_ieee80211_data_extract(pkt_data, pkt_length, link_type, &res, rssi);

    return 0;
}

int demo_aws_80211_frame_handler(char *buf, int length, aws_link_type_t link_type, int with_fcs, signed char rssi) {
    return demo_zconfig_recv_callback(buf, length, 6, AWSS_LINK_TYPE_80211_RADIO, with_fcs, rssi);
}



extern void HAL_Awss_Open_Monitor(awss_recv_80211_frame_cb_t cb);

void verify_rx_mng_raw_frame() {
    HAL_Awss_Open_Monitor(demo_aws_80211_frame_handler);
}
