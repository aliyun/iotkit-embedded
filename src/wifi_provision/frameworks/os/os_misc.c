/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdlib.h>
#include "os.h"
#include "awss_log.h"

/****** Convert values between host and big-/little-endian byte order ******/

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

#if 0
//reverse byte order
static inline uint32_t reverse_32bit(uint32_t data)
{
    data = (data >> 16) | (data << 16);
    return ((data & 0xff00ff00UL) >> 8) | ((data & 0x00ff00ffUL) << 8);
}

//host byte order to big endian
static uint32_t os_htobe32(uint32_t data)
{
    if (os_is_big_endian()) {
        return data;
    }

    return reverse_32bit(data);
}

//host byte order to little endian
static uint32_t os_htole32(uint32_t data)
{
    if (os_is_big_endian()) {
        return reverse_32bit(data);
    }

    return data;
}

//big endian to host byte order
static uint32_t os_be32toh(uint32_t data)
{
    return os_htobe32(data);
}

//little endian to host byte order
uint32_t os_le32toh(uint32_t data)
{
    return os_htole32(data);
}

//reverse byte order
static inline uint64_t reverse_64bit(uint64_t data)
{
    data = (data >> 32) | (data << 32);
    data = ((data & 0xff00ff00ff00ff00ULL) >> 8) | ((data & 0x00ff00ff00ff00ffULL) << 8);

    return ((data & 0xffff0000ffff0000ULL) >> 16) | ((data & 0x0000ffff0000ffffULL) << 16);
}

//host to big endian
uint64_t os_htobe64(uint64_t data)
{
    if (os_is_big_endian()) {
        return data;
    }

    return reverse_64bit(data);
}

//host to little endian
uint64_t os_htole64(uint64_t data)
{
    if (os_is_big_endian()) {
        return reverse_64bit(data);
    }

    return data;
}

//big endian to host
uint64_t os_be64toh(uint64_t data)
{
    return os_htobe64(data);
}

//little endian to host
uint64_t os_le64toh(uint64_t data)
{
    return os_htole64(data);
}

uint32_t os_get_unaligned_be32(uint8_t * ptr)
{
    uint32_t res;

    memcpy(&res, ptr, sizeof(uint32_t));

    return os_be32toh(res);
}

//get unaligned data in little endian.
uint32_t os_get_unaligned_le32(uint8_t * ptr)
{
    uint32_t res;

    memcpy(&res, ptr, sizeof(uint32_t));

    return os_le32toh(res);

}

uint16_t os_htons(uint16_t n)
{
    return os_htobe16(n);
}

uint16_t os_ntohs(uint16_t n)
{
    return os_be16toh(n);
}

uint32_t os_htonl(uint32_t n)
{
    return os_htobe32(n);
}

uint32_t os_ntohl(uint32_t n)
{
    return os_be32toh(n);
}

char *os_ntoa(const uint32_t ip, char buf[OS_IP_LEN])
{
    char inv[3];
    char *rp;
    uint8_t *ap;
    uint8_t rem;
    uint8_t n;
    uint8_t i;

    rp = buf;
    ap = (uint8_t *) & ip;

    for (n = 0; n < 4; n++) {
        i = 0;
        do {
            rem = *ap % (uint8_t) 10;
            *ap /= (uint8_t) 10;
            inv[i++] = '0' + rem;
        } while (*ap);

        while (i--) {
            *rp++ = inv[i];
        }

        *rp++ = '.';
        ap++;
    }

    *--rp = 0;
    return buf;
}
#endif
/* format mac string uppercase */
char *os_wifi_get_mac_str(char mac_str[OS_MAC_LEN])
{
    char *str;
    int colon_num = 0, i;

    str = platform_wifi_get_mac(mac_str);

    /* sanity check */
    while (str) {
        str = strchr(str, ':');
        if (str) {
            colon_num ++;
            str ++; /* eating char ':' */
        }
    }

    /* convert to capital letter */
    for (i = 0; i < OS_MAC_LEN && mac_str[i]; i ++) {
        if ('a' <= mac_str[i] && mac_str[i] <= 'z') {
            mac_str[i] -= 'a' - 'A';
        }
    }

    return mac_str;
}

char *os_wifi_str2mac(char mac_str[OS_MAC_LEN], char mac[OS_ETH_ALEN])
{
    int i = 0;
    char *ptr = mac_str;
    char mac_addr[OS_ETH_ALEN] = {0};

    if (ptr == NULL)
        return NULL;

    while (isxdigit(*ptr) && i < OS_ETH_ALEN) {
        mac_addr[i ++] = (uint8_t)strtol(ptr, &ptr, 16);
        ++ ptr;
    }

    if (i < OS_ETH_ALEN)  /* don't touch mac when fail */
        return NULL;

    if (mac) memcpy(mac, mac_addr, OS_ETH_ALEN);

    return mac;
}

uint8_t *os_wifi_get_mac(uint8_t mac[OS_ETH_ALEN])
{
    char mac_str[OS_MAC_LEN] = {0};

    os_wifi_get_mac_str(mac_str);

    return (uint8_t *)os_wifi_str2mac(mac_str, (char *)mac);
}

int os_device_get_name(char key_str[OS_DEVICE_NAME_LEN])
{
    return device_get_name(key_str);
}

int os_device_get_secret(char secret_str[OS_DEVICE_SECRET_LEN])
{
    return device_get_secret(secret_str);
}

int os_product_get_secret(char secret_str[OS_PRODUCT_SECRET_LEN])
{
    return product_get_secret(secret_str);
}

int os_product_get_key(char key_str[OS_PRODUCT_KEY_LEN])
{
    return product_get_key(key_str);
}

int os_wifi_get_ap_info(
            _OUT_ char ssid[PLATFORM_MAX_SSID_LEN],
            _OUT_ char passwd[PLATFORM_MAX_PASSWD_LEN],
            _OUT_ uint8_t bssid[ETH_ALEN])
{
    return platform_wifi_get_ap_info(ssid, passwd, bssid);
}

int os_aes128_cfb_decrypt(
            _IN_ p_aes128_t aes,
            _IN_ const void *src,
            _IN_ size_t blockNum,
            _OUT_ void *dst)
{
    return platform_aes128_cfb_decrypt(aes, src, blockNum, dst);
}

int os_aes128_cfb_encrypt(
            _IN_ p_aes128_t aes,
            _IN_ const void *src,
            _IN_ size_t blockNum,
            _OUT_ void *dst)
{
    return platform_aes128_cfb_encrypt(aes, src, blockNum, dst);
}

int os_aes128_cbc_encrypt(
            _IN_ p_aes128_t aes,
            _IN_ const void *src,
            _IN_ size_t blockNum,
            _OUT_ void *dst)
{
    return platform_aes128_cbc_encrypt(aes, src, blockNum, dst);
}

p_aes128_t os_aes128_init(
            _IN_ const uint8_t *key,
            _IN_ const uint8_t *iv,
            _IN_ AES_DIR_t dir)
{
    return platform_aes128_init(key, iv, dir);
}

int os_aes128_destroy(
            _IN_ p_aes128_t aes)
{
    return platform_aes128_destroy(aes);
}

int os_wifi_scan(_IN_ platform_wifi_scan_result_cb_t cb)
{
    return platform_wifi_scan(cb);
}

int os_wifi_enable_mgnt_frame_filter(
            _IN_ uint32_t filter_mask,
            _IN_OPT_ uint8_t vendor_oui[3],
            _IN_ platform_wifi_mgnt_frame_cb_t callback)
{
    return platform_wifi_enable_mgnt_frame_filter(filter_mask, vendor_oui, callback);
}


int os_wifi_send_80211_raw_frame(_IN_ enum platform_awss_frame_type type,
        _IN_ uint8_t *buffer, _IN_ int len)
{
    return platform_wifi_send_80211_raw_frame(type, buffer, len);
}

int os_awss_connect_ap(
            _IN_ uint32_t connection_timeout_ms,
            _IN_ char ssid[OS_MAX_SSID_LEN],
            _IN_ char passwd[OS_MAX_PASSWD_LEN],
            _IN_OPT_ enum AWSS_AUTH_TYPE auth,
            _IN_OPT_ enum AWSS_ENC_TYPE encry,
            _IN_OPT_ uint8_t bssid[ETH_ALEN],
            _IN_OPT_ uint8_t channel)
{
    return platform_awss_connect_ap(connection_timeout_ms, ssid, passwd,
                                    auth, encry, bssid, channel);
}

void os_awss_switch_channel(char primary_channel, char secondary_channel, uint8_t bssid[OS_ETH_ALEN])
{
    platform_awss_switch_channel(primary_channel, secondary_channel, bssid);
}


int os_awss_open_ap(const char *ssid, const char *passwd, int beacon_interval, int hide)
{
    return platform_awss_open_ap(ssid, passwd, beacon_interval, hide);
}

void os_awss_close_monitor(void)
{
    platform_awss_close_monitor();
}

void os_awss_open_monitor(os_awss_recv_80211_frame_cb_t cb)
{
    platform_awss_open_monitor(cb);
}

uint32_t os_wifi_get_ip(char ip_str[OS_IP_LEN], const char *ifname)
{
    return platform_wifi_get_ip(ip_str, ifname);
}

int os_get_conn_encrypt_type(void)
{
    return platform_get_conn_encrypt_type();
}

int os_get_encrypt_type(void)
{
    return platform_get_encrypt_type();
}

void os_msleep(_IN_ uint32_t ms)
{
    platform_msleep(ms);
}

void os_reboot()
{
    platform_reboot();
}


void *os_mutex_init(void)
{
    return platform_mutex_init();
}

void os_mutex_destroy(_IN_ void *mutex)
{
    platform_mutex_destroy(mutex);
}

void os_mutex_unlock(_IN_ void *mutex)
{
    platform_mutex_unlock(mutex);
}
#if 0
void os_semaphore_destroy(_IN_ void *sem)
{
    platform_semaphore_destroy(sem);
}

int os_semaphore_wait(_IN_ void *sem, _IN_ uint32_t timeout_ms)
{
    return platform_semaphore_wait(sem, timeout_ms);
}

void os_semaphore_post(_IN_ void *sem)
{
    platform_semaphore_post(sem);
}
#endif

void *os_zalloc(uint32_t size)
{
    void *ptr = os_malloc(size);
    if (ptr != NULL) {
        memset(ptr, 0, size);
    }
    return ptr;
}

uint32_t os_get_time_ms(void)
{
    static uint32_t fixed_delta;

    if (!fixed_delta) {
        fixed_delta = platform_get_time_ms() - 0xFFFF0000;
    }

    /* add a big offset, for easier caught time overflow bug */
    return platform_get_time_ms() - fixed_delta;
}

uint32_t time_elapsed_ms_since(uint32_t start_timestamp)
{
    uint32_t now = os_get_time_ms();
    return now - start_timestamp;
}

int os_is_big_endian(void)
{
    uint32_t data = 0xFF000000;

    if (0xFF == *(uint8_t *) & data) {
        return 1;                                    /* big endian */
    }

    return 0;                                         /* little endian */
}

void os_mutex_lock(_IN_ void *mutex)
{
    platform_mutex_lock(mutex);
}

int os_awss_get_timeout_interval_ms(void)
{
    return platform_awss_get_timeout_interval_ms();
}

int os_aes128_cbc_decrypt(
            _IN_ p_aes128_t aes,
            _IN_ const void *src,
            _IN_ size_t blockNum,
            _OUT_ void *dst)
{
    return platform_aes128_cbc_decrypt(aes, src, blockNum, dst);
}

int os_awss_close_ap()
{
    return platform_awss_close_ap();
}




