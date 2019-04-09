#include "hal_awss.h"
#include "hal_common.h"
#include "ieee80211.h"
#include "smartconfig_ieee80211.h"

uint8_t br_mac[ETH_ALEN];
extern int g_channel;

const uint8_t zconfig_fixed_offset[ZC_ENC_TYPE_MAX + 1][2] = {
    {  /* open, none, ip(20) + udp(8) + 8(LLC) */
        36, 36
    },
    {  /* wep, + iv(4) + data + ICV(4) */
        44, 44  /* feixun, wep64(10byte), wep128(26byte) */
    },
    {  /* tkip, + iv/keyID(4) + Ext IV(4) + data + MIC(8) + ICV(4) */
        56, 56  /* tkip(10byte, 20byte), wpa2+tkip(20byte) */
    },
    {  /* aes, + ccmp header(8) + data + MIC(8) + ICV(4) */
        52, 52
    },
    {  /* tkip-aes */
        56, 52  /* fromDs==tkip,toDs==aes */
    }
};

/* following is broadcast protocol related code */
uint8_t is_start_frame(uint16_t len)
{
    return (len == START_FRAME);
}

uint8_t is_group_frame(uint16_t len)
{
    /* is group frame? */
    return (len > GROUP_FRAME && len <= GROUP_FRAME_END);
}

uint8_t is_data_frame(uint16_t len)
{
    uint8_t group_frame, index;
    /* is start frame */
    if (is_start_frame(len)) {
        return 0;
    }

    /* is group frame? */
    group_frame = is_group_frame(len);
    if (group_frame) {
        return 0;
    }

    index = (len >> PAYLOAD_BITS_CNT) & 0xF;
    return (index >= ZC_GRP_PKT_IDX_START && index <= ZC_GRP_PKT_IDX_END);
}

int awss_ieee80211_smartconfig_process(uint8_t *ieee80211, int len, int link_type, struct parser_res *res, char rssi)
{
    int hdrlen, fc, seq_ctrl;
    struct ieee80211_hdr *hdr;
    uint8_t *data, *bssid_mac, *dst_mac;
    uint8_t encry = ZC_ENC_TYPE_INVALID;
    uint8_t tods;
    uint8_t index = 0;

    if (ieee80211 == NULL) {
        return ALINK_INVALID;
    }

    hdr = (struct ieee80211_hdr *)ieee80211;
    fc = hdr->frame_control;
    seq_ctrl = hdr->seq_ctrl;

    /*
     * for smartconfig with bcast of data
     */
    if (!ieee80211_is_data_exact(fc)) {
        return ALINK_INVALID;
    }

    /* tods = 1, fromds = 0 || tods = 0, fromds = 1 */
    if (ieee80211_has_tods(fc) == ieee80211_has_fromds(fc)) {
        return ALINK_INVALID;
    }
    /* drop frag, more, order*/
    if (ieee80211_has_frags(fc)) {
        return ALINK_INVALID;
    }

    memset(br_mac, 0xFF, ETH_ALEN);
    dst_mac = (uint8_t *)ieee80211_get_DA(hdr);
    if (memcmp(dst_mac, br_mac, ETH_ALEN)) {
        return ALINK_INVALID;    /* only handle br frame */
    }

    bssid_mac = (uint8_t *)ieee80211_get_BSSID(hdr);

    /*
     * payload len = frame.len - (radio_header + wlan_hdr)
     */
    hdrlen = ieee80211_hdrlen(fc);
    if (hdrlen > len) {
        return ALINK_INVALID;
    }

#ifdef _PLATFORM_QCOM_
    /* Note: http://stackoverflow.com/questions/17688710/802-11-qos-data-frames */
    hdrlen = (hdrlen + 3) & 0xFC;/* align header to 32bit boundary */
#endif

    res->u.br.data_len = len - hdrlen;       /* eating the hdr */
    res->u.br.sn = IEEE80211_SEQ_TO_SN(os_le16toh(seq_ctrl));

    data = ieee80211 + hdrlen;               /* eating the hdr */
    tods = ieee80211_has_tods(fc);

    do {
#ifdef AWSS_SUPPORT_APLIST
        struct ap_info *ap_info;
        ap_info = zconfig_get_apinfo(bssid_mac);
        if (ap_info && ZC_ENC_TYPE_INVALID != ap_info->encry[tods]) {
            encry = ap_info->encry[tods];
        } else
#endif
        {
            if (!ieee80211_has_protected(fc)) {
                encry = ZC_ENC_TYPE_NONE;
            } else {
                /* Note: avoid empty null data */
                if (len < 8) {      /* IV + ICV + DATA >= 8 */
                    return ALINK_INVALID;
                }
                if (!(ieee80211[3] & 0x3F)) {
                    encry = ZC_ENC_TYPE_WEP;
                } else if (data[3] & (1 << 5)) {/* Extended IV */
                    if (data[1] == ((data[0] | 0x20) & 0x7F)) { /* tkip, WEPSeed  = (TSC1 | 0x20 ) & 0x7F */
                        encry = ZC_ENC_TYPE_TKIP;
                    }
                    if (data[2] == 0 && (!(data[3] & 0x0F))) {
                        encry = ZC_ENC_TYPE_AES;
                    }

                    /*
                     * Note: above code use if(tkip) and if(ase)
                     * instead of if(tkip) else if(aes)
                     * beacause two condition may bother match.
                     */
                }
            }
        }
    } while (0);

    if (encry == ZC_ENC_TYPE_INVALID) {
        printf("invalid encry type!\r\n");
    }
    res->u.br.encry_type = encry;

    /* convert IEEE 802.11 header + possible LLC headers into Ethernet header
     * IEEE 802.11 address fields:
     * ToDS FromDS Addr1 Addr2 Addr3 Addr4
     *   0     0   DA    SA    BSSID n/a
     *   0     1   DA    BSSID SA    n/a
     *   1     0   BSSID SA    DA    n/a
     *   1     1   RA    TA    DA    SA
     */
    res->src = ieee80211_get_SA(hdr);
    res->dst = ieee80211_get_DA(hdr);
    res->bssid = ieee80211_get_BSSID(hdr);
    res->tods = ieee80211_has_tods(fc);

    res->u.br.data_len -= zconfig_fixed_offset[encry][0];
    /* printf("data_len: 0x%04X\n",res->u.br.data_len); */

    /* 
     * |    Smart Config Frame Type   | From DS | To DS | Length |
     * 
     */
    
    if (is_start_frame(res->u.br.data_len) || is_group_frame(res->u.br.data_len) || is_data_frame(res->u.br.data_len)) {
        char *frame_type = NULL;
        char *direction = NULL;
        if (is_start_frame(res->u.br.data_len)) {
            frame_type = "SmartConfig Start Frame";
        }else if (is_group_frame(res->u.br.data_len)) {
            frame_type = "SmartConfig Group Frame";
        }else if (is_data_frame(res->u.br.data_len)) {
            frame_type = "SmartConfig  Data Frame";
        }

        if (ieee80211_has_fromds(fc)) {
            direction = "FromDS";
        }else{
            direction = "ToDS";
        }

        printf("|   %23s   |   %6s  |  %04d (0x%04X)  |    %2d   |\n",
                frame_type,direction,res->u.br.data_len,res->u.br.data_len,g_channel);
    }

    return ALINK_BROADCAST;
}
