/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include "hal_common.h"
#include "hal_awss.h"
#include "ieee80211.h"
#include <stdio.h>
#include <string.h>
#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

/* format mac string uppercase */

int os_is_big_endian(void)
{
    uint32_t data = 0xFF000000;

    if (0xFF == *(uint8_t *) & data) {
        return 1;                                    /* big endian */
    }

    return 0;                                         /* little endian */
}

/* reverse byte order */
static uint16_t reverse_16bit(uint16_t data)
{
    return (data >> 8) | (data << 8);
}

/* host byte order to big endian */
uint16_t os_htobe16(uint16_t data)
{
    if (os_is_big_endian()) {
        return data;
    }

    return reverse_16bit(data);
}

/* host byte order to little endian */
uint16_t os_htole16(uint16_t data)
{
    if (os_is_big_endian()) {
        return reverse_16bit(data);
    }

    return data;
}

/* big endian to host byte order */
uint16_t os_be16toh(uint16_t data)
{
    return os_htobe16(data);
}

/* little endian to host byte order */
uint16_t os_le16toh(uint16_t data)
{
    return os_htole16(data);
}

/* get unaligned data in big endian. */
uint16_t os_get_unaligned_be16(uint8_t * ptr)
{
    uint16_t res;

    memcpy(&res, ptr, sizeof(uint16_t));

    return os_be16toh(res);
}

/* get unaligned data in little endian. */
uint16_t os_get_unaligned_le16(uint8_t * ptr)
{
    uint16_t res;

    memcpy(&res, ptr, sizeof(uint16_t));

    return os_le16toh(res);

}

/**
 * ieee80211_is_mgmt - check if type is IEEE80211_FTYPE_MGMT
 * @fc: frame control bytes in little-endian byteorder
 */

int is_invalid_pkg(void *pkt_data, uint32_t pkt_length)
{
#define MIN_PKG         (33)
#define MAX_PKG         (1480 + 56 + 200)
    if (pkt_length < MIN_PKG || pkt_length > MAX_PKG) {
        return 1;
    }
    return 0;
}

#if 0
int ieee80211_is_mgmt(uint16_t fc)
{
    return (fc & os_htole16(IEEE80211_FCTL_FTYPE)) ==
           os_htole16(IEEE80211_FTYPE_MGMT);
}
#endif

/**
 * ieee80211_is_ctl - check if type is IEEE80211_FTYPE_CTL
 * @fc: frame control bytes in little-endian byteorder
 */
int ieee80211_is_ctl(uint16_t fc)
{
    return (fc & os_htole16(IEEE80211_FCTL_FTYPE)) ==
           os_htole16(IEEE80211_FTYPE_CTL);
}

/**
 * ieee80211_is_data - check if type is IEEE80211_FTYPE_DATA
 * @fc: frame control bytes in little-endian byteorder
 */
int ieee80211_is_data(uint16_t fc)
{
    return (fc & os_htole16(IEEE80211_FCTL_FTYPE)) ==
           os_htole16(IEEE80211_FTYPE_DATA);
}


/**
 * ieee80211_has_tods - check if IEEE80211_FCTL_TODS is set
 * @fc: frame control bytes in little-endian byteorder
 */
int ieee80211_has_tods(uint16_t fc)
{
    return (fc & os_htole16(IEEE80211_FCTL_TODS)) != 0;
}

/**
 * ieee80211_has_fromds - check if IEEE80211_FCTL_FROMDS is set
 * @fc: frame control bytes in little-endian byteorder
 */
int ieee80211_has_fromds(uint16_t fc)
{
    return (fc & os_htole16(IEEE80211_FCTL_FROMDS)) != 0;
}

/**
 * ieee80211_has_a4 - check if IEEE80211_FCTL_TODS and IEEE80211_FCTL_FROMDS are set
 * @fc: frame control bytes in little-endian byteorder
 */
int ieee80211_has_a4(uint16_t fc)
{
    uint16_t tmp = os_htole16(IEEE80211_FCTL_TODS | IEEE80211_FCTL_FROMDS);
    return (fc & tmp) == tmp;
}

/**
 * ieee80211_has_order - check if IEEE80211_FCTL_ORDER is set
 * @fc: frame control bytes in little-endian byteorder
 */
int ieee80211_has_order(uint16_t fc)
{
    return (fc & os_htole16(IEEE80211_FCTL_ORDER)) != 0;
}

/**
 * ieee80211_has_protected - check if IEEE80211_FCTL_PROTECTED is set
 * @fc: frame control bytes in little-endian byteorder
 */
int ieee80211_has_protected(uint16_t fc)
{
    return (fc & os_htole16(IEEE80211_FCTL_PROTECTED)) != 0;
}

/**
 * ieee80211_is_data_qos - check if type is IEEE80211_FTYPE_DATA and IEEE80211_STYPE_QOS_DATA is set
 * @fc: frame control bytes in little-endian byteorder
 */
int ieee80211_is_data_qos(uint16_t fc)
{
    /*
     * mask with QOS_DATA rather than IEEE80211_FCTL_STYPE as we just need
     * to check the one bit
     */
    return (fc & os_htole16(IEEE80211_FCTL_FTYPE | IEEE80211_STYPE_QOS_DATA)) ==
           os_htole16(IEEE80211_FTYPE_DATA | IEEE80211_STYPE_QOS_DATA);
}

/**
 * ieee80211_is_data_present - check if type is IEEE80211_FTYPE_DATA and has data
 * @fc: frame control bytes in little-endian byteorder
 */
#if 0
int ieee80211_is_data_present(uint16_t fc)
{
    /*
     * mask with 0x40 and test that that bit is clear to only return true
     * for the data-containing substypes.
     */
    return (fc & os_htole16(IEEE80211_FCTL_FTYPE | 0x40)) ==
           os_htole16(IEEE80211_FTYPE_DATA);
}
#endif

/**
 * ieee80211_is_data_present - check if type is IEEE80211_FTYPE_DATA and only data
 * @fc: frame control bytes in little-endian byteorder
 */
int ieee80211_is_data_exact(uint16_t fc)
{
    uint16_t tmp = fc & os_htole16(IEEE80211_FCTL_FTYPE | IEEE80211_FCTL_STYPE);

    return (tmp == os_htole16(IEEE80211_FTYPE_DATA)) ||
           (tmp == os_htole16(IEEE80211_FTYPE_DATA | IEEE80211_STYPE_QOS_DATA));
}

/**
 * ieee80211_is_beacon - check if IEEE80211_FTYPE_MGMT && IEEE80211_STYPE_BEACON
 * @fc: frame control bytes in little-endian byteorder
 */
int ieee80211_is_beacon(uint16_t fc)
{
    return (fc & os_htole16(IEEE80211_FCTL_FTYPE | IEEE80211_FCTL_STYPE)) ==
           os_htole16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_BEACON);
}

/**
 * ieee80211_is_action - check if IEEE80211_FTYPE_MGMT && IEEE80211_STYPE_ACTION
 * @fc: frame control bytes in little-endian byteorder
 */
#if 0
int ieee80211_is_action(uint16_t fc)
{
    return (fc & os_htole16(IEEE80211_FCTL_FTYPE | IEEE80211_FCTL_STYPE)) ==
           os_htole16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
}
#endif

/**
 * ieee80211_is_probe_req - check if IEEE80211_FTYPE_MGMT && IEEE80211_STYPE_PROBE_REQ
 * @fc: frame control bytes in little-endian byteorder
 */
int ieee80211_is_probe_req(uint16_t fc)
{
    return (fc & os_htole16(IEEE80211_FCTL_FTYPE | IEEE80211_FCTL_STYPE)) ==
           os_htole16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_REQ);
}

/**
 * ieee80211_is_probe_resp - check if IEEE80211_FTYPE_MGMT && IEEE80211_STYPE_PROBE_RESP
 * @fc: frame control bytes in little-endian byteorder
 */
int ieee80211_is_probe_resp(uint16_t fc)
{
    return (fc & os_htole16(IEEE80211_FCTL_FTYPE | IEEE80211_FCTL_STYPE)) ==
           os_htole16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_RESP);
}


/**
 * ieee80211_get_SA - get pointer to SA
 * @hdr: the frame
 *
 * Given an 802.11 frame, this function returns the offset
 * to the source address (SA). It does not verify that the
 * header is long enough to contain the address, and the
 * header must be long enough to contain the frame control
 * field.
 */
uint8_t *ieee80211_get_SA(struct ieee80211_hdr *hdr)
{
    if (ieee80211_has_a4(hdr->frame_control))
        return hdr->addr4;
    if (ieee80211_has_fromds(hdr->frame_control))
        return hdr->addr3;
    return hdr->addr2;
}

/**
 * ieee80211_get_DA - get pointer to DA
 * @hdr: the frame
 *
 * Given an 802.11 frame, this function returns the offset
 * to the destination address (DA). It does not verify that
 * the header is long enough to contain the address, and the
 * header must be long enough to contain the frame control
 * field.
 */
uint8_t *ieee80211_get_DA(struct ieee80211_hdr *hdr)
{
    if (ieee80211_has_tods(hdr->frame_control))
        return hdr->addr3;
    else
        return hdr->addr1;
}

uint8_t *ieee80211_get_BSSID(struct ieee80211_hdr *hdr)
{
    if (ieee80211_has_tods(hdr->frame_control)) {
        if (!ieee80211_has_fromds(hdr->frame_control))
            return hdr->addr1;
        else
            return NULL;
    } else {
        if (ieee80211_has_fromds(hdr->frame_control))
            return hdr->addr2;
        else
            return hdr->addr3;
    }
}

int ieee80211_get_bssid(uint8_t *in, uint8_t *mac)
{
    uint8_t *bssid = ieee80211_get_BSSID((struct ieee80211_hdr *)in);

    if (bssid)
        memcpy(mac, bssid, ETH_ALEN);
    else
        return -1;

    return 0;
}

int ieee80211_has_frags(uint16_t fc)
{
    uint16_t tmp = fc & os_htole16(IEEE80211_FCTL_MOREFRAGS | IEEE80211_FCTL_ORDER);

    return !!tmp;
}

/* DATA:        24B */
/* QOS-DATA:    26B */
int ieee80211_hdrlen(uint16_t fc)
{
    uint32_t hdrlen = 24;

    if (ieee80211_is_data(fc)) {
        if (ieee80211_has_a4(fc))
            hdrlen = 30;
        if (ieee80211_is_data_qos(fc)) {
            hdrlen += IEEE80211_QOS_CTL_LEN;
            if (ieee80211_has_order(fc))
                hdrlen += IEEE80211_HT_CTL_LEN;
        }
        goto out;
    }

    if (ieee80211_is_ctl(fc)) {
        /*
         * ACK and CTS are 10 bytes, all others 16. To see how
         * to get this condition consider
         *   subtype mask:   0b0000000011110000 (0x00F0)
         *   ACK subtype:    0b0000000011010000 (0x00D0)
         *   CTS subtype:    0b0000000011000000 (0x00C0)
         *   bits that matter:         ^^^      (0x00E0)
         *   value of those: 0b0000000011000000 (0x00C0)
         */
        if ((fc & os_htole16(0x00E0)) == os_htole16(0x00C0))
            hdrlen = 10;
        else
            hdrlen = 16;
    }

out:
    return hdrlen;
}

/* helpers */
int ieee80211_get_radiotap_len(uint8_t *data)
{
    struct ieee80211_radiotap_header *hdr =
            (struct ieee80211_radiotap_header *)data;

    return os_get_unaligned_le16((uint8_t *)&hdr->it_len);
}

const uint8_t *cfg80211_find_ie(uint8_t eid, const uint8_t *ies, int len)
{
    while (len > 2 && ies[0] != eid) {
        len -= ies[1] + 2;
            ies += ies[1] + 2;
    }
    if (len < 2)
        return NULL;
    if (len < 2 + ies[1])
        return NULL;
    return ies;
}

/**
 * cfg80211_find_vendor_ie - find vendor specific information element in data
 *
 * @oui: vendor OUI
 * @oui_type: vendor-specific OUI type
 * @ies: data consisting of IEs
 * @len: length of data
 *
 * Return: %NULL if the vendor specific element ID could not be found or if the
 * element is invalid (claims to be longer than the given data), or a pointer to
 * the first byte of the requested element, that is the byte containing the
 * element ID.
 *
 * Note: There are no checks on the element length other than having to fit into
 * the given data.
 */
const uint8_t *cfg80211_find_vendor_ie(uint32_t oui, uint8_t oui_type, const uint8_t *ies, int len)
{
    struct ieee80211_vendor_ie *ie;
    const uint8_t *pos = ies, *end = ies + len;
    int ie_oui;

    while (pos < end) {
        pos = cfg80211_find_ie(WLAN_EID_VENDOR_SPECIFIC, pos,
                               end - pos);
        if (!pos)
            return NULL;

        ie = (struct ieee80211_vendor_ie *)pos;

        /* make sure we can access ie->len */
        /* BUILD_BUG_ON(offsetof(struct ieee80211_vendor_ie, len) != 1); */

        if (ie->len < sizeof(*ie))
            goto cont;

        ie_oui = ie->oui[0] << 16 | ie->oui[1] << 8 | ie->oui[2];
        /* awss_trace("oui=%x, type=%x, len=%d\r\n", ie_oui, oui_type, ie->len); */
        if (ie_oui == oui && ie->oui_type == oui_type)
            return pos;
cont:
        pos += 2 + ie->len;
    }
    return NULL;
}

/**
 * extract ssid from beacon frame or probe resp frame
 *
 * @beacon_frame: [IN] original 80211 beacon frame
 * @frame_len: [IN] len of beacon frame
 * @ssid: [OUT] null-terminated string, max len 32 bytes
 *
 * Return:
 *     0/success, -1/failed
 */
int ieee80211_get_ssid(uint8_t *beacon_frame, uint16_t frame_len, uint8_t *ssid)
{
    uint16_t ieoffset = offset_of(struct ieee80211_mgmt, u.beacon.variable);/* same as u.probe_resp.variable */
    const uint8_t *ptr = cfg80211_find_ie(WLAN_EID_SSID,
                                          beacon_frame + ieoffset, frame_len - ieoffset);
    if (ptr) {
        uint8_t ssid_len = ptr[1];
        if (ssid_len <= 32) {    /* ssid 32 octets at most */
            memcpy(ssid, ptr + 2, ssid_len);/* eating EID & len */
            ssid[ssid_len] = '\0';
            return 0;
        }
    }

    return -1;
}

/**
 * extract channel from beacon frame or probe resp frame
 *
 * @beacon_frame: [IN] original 80211 beacon frame
 * @frame_len: [IN] len of beacon frame
 *
 * Return:
 *     bss channel 1-13, 0--means invalid channel
 */
int cfg80211_get_bss_channel(uint8_t *beacon_frame, uint16_t frame_len)
{
    uint16_t ieoffset = offsetof(struct ieee80211_mgmt, u.beacon.variable);/* same as u.probe_resp.variable */
    const uint8_t *ie = beacon_frame + ieoffset;
    uint16_t ielen = frame_len - ieoffset;

    const uint8_t *tmp;
    int channel_number = 0;

    tmp = cfg80211_find_ie(WLAN_EID_DS_PARAMS, ie, ielen);
    if (tmp && tmp[1] == 1) {
        channel_number = tmp[2];
    } else {
        tmp = cfg80211_find_ie(WLAN_EID_HT_OPERATION, ie, ielen);
        if (tmp && tmp[1] >= sizeof(struct ieee80211_ht_operation)) {
            struct ieee80211_ht_operation *htop = (void *)(tmp + 2);

            channel_number = htop->primary_chan;
        }
    }

    return channel_number;
}

static const uint8_t WPA_OUI23A_TYPE[] =            {0x00, 0x50, 0xf2, 0x01};
static const uint8_t RSN_SUITE_1X[] =               {0x00, 0x0f, 0xac, 0x01};

static const uint8_t WPA_CIPHER_SUITE_NONE23A[] =   {0x00, 0x50, 0xf2, 0x00};
static const uint8_t WPA_CIPHER_SUITE_WEP4023A[] =  {0x00, 0x50, 0xf2, 0x01};
static const uint8_t WPA_CIPHER_SUITE_TKIP23A[] =   {0x00, 0x50, 0xf2, 0x02};
/* static const uint8_t WPA_CIPHER_SUITE_WRAP23A[] = {0x00, 0x50, 0xf2, 0x03}; */
static const uint8_t WPA_CIPHER_SUITE_CCMP23A[] =   {0x00, 0x50, 0xf2, 0x04};
static const uint8_t WPA_CIPHER_SUITE_WEP10423A[] = {0x00, 0x50, 0xf2, 0x05};

static const uint8_t RSN_CIPHER_SUITE_NONE23A[] =   {0x00, 0x0f, 0xac, 0x00};
static const uint8_t RSN_CIPHER_SUITE_WEP4023A[] =  {0x00, 0x0f, 0xac, 0x01};
static const uint8_t RSN_CIPHER_SUITE_TKIP23A[] =   {0x00, 0x0f, 0xac, 0x02};
/* static const uint8_t RSN_CIPHER_SUITE_WRAP23A[] = {0x00, 0x0f, 0xac, 0x03}; */
static const uint8_t RSN_CIPHER_SUITE_CCMP23A[] =   {0x00, 0x0f, 0xac, 0x04};
static const uint8_t RSN_CIPHER_SUITE_WEP10423A[] = {0x00, 0x0f, 0xac, 0x05};

#define WPA_SELECTOR_LEN        (4)
#define RSN_SELECTOR_LEN        (4)

#define BIT(x)                  (1 << (x))
#define WPA_CIPHER_NONE         BIT(0)
#define WPA_CIPHER_WEP40        BIT(1)
#define WPA_CIPHER_WEP104       BIT(2)
#define WPA_CIPHER_TKIP         BIT(3)
#define WPA_CIPHER_CCMP         BIT(4)

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif