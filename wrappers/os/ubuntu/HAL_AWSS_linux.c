/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 * This file provided HALs for smartconfig and phone ap wifi-provision,
 * which have been verified with wireless card Linksys思科wusb600n双频无线网卡
 * To buy this card, pls visit:
    https://item.taobao.com/item.htm?ut_sk=1.XGVDbbjwimoDAP/KhyE3F1/y_21380790_1557922170221.DingTalk.1&id=10929152134&sourceType=item&price=499&suid=E8AE39DE-B83A-466C-AC66-7E23B81B371F&un=a6384eb589484e4b132b0f6991806569&share_crt_v=1&cpp=1&shareurl=true&spm=a313p.22.7k.1033739374611&short_name=h.edXpfty&app=chrome
 */

#include "infra_config.h"

#if defined(WIFI_PROVISION_ENABLED) || defined(DEV_BIND_ENABLED)

#include <string.h>
#include <stdlib.h>
#include "infra_defs.h"
#include "wrappers.h"
#include "assert.h"
#include "stdio.h"
#include <net/if.h>
#include <ieee80211_radiotap.h>
#include <netinet/ip.h>
#include <sys/time.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include "wrappers_defs.h"

/* please use ifconfig to get your wireless card's name, and replace g_ifname */
char *g_ifname = "wlx00259ce04ceb";

char g_opened_ap[36] = {0};

int HAL_ThreadCreate(
            void **thread_handle,
            void *(*work_routine)(void *),
            void *arg,
            hal_os_thread_param_t *hal_os_thread_param,
            int *stack_used);

static int do_cmd_exec(char *cmd, char *result, int len)
{
    char buf[1024];
    FILE *filp;
    int ret = 0;
    filp = popen(cmd, "r");
    if (!filp) {
        printf("cmd: %s failed\n", cmd);
        return -1;
    }
    memset(buf, '\0', sizeof(buf));
    ret = fread(buf, sizeof(buf) - 1, 1, filp);

    /* add this code to pass white scan check */
    buf[1023] = '\0';

    pclose(filp);

    return snprintf(result, len, "%s", buf);
}

/**
 * @brief   获取Wi-Fi网口的MAC地址, 格式应当是"XX:XX:XX:XX:XX:XX"
 *
 * @param   mac_str : 用于存放MAC地址字符串的缓冲区数组
 * @return  指向缓冲区数组起始位置的字符指针
 */
char *HAL_Wifi_Get_Mac(_OU_ char mac_str[HAL_MAC_LEN])
{
    int fd = -1;
    int ret = -1;
    struct ifreq if_hwaddr;

    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("socket error: ");
        return NULL ;
    }

    memset(&if_hwaddr, 0, sizeof(if_hwaddr));
    strncpy(if_hwaddr.ifr_name, g_ifname, sizeof(if_hwaddr.ifr_name) - 1);

    ret = ioctl(fd, SIOCGIFHWADDR, &if_hwaddr);
    if (ret >= 0) {
        unsigned char *hwaddr = (unsigned char *)if_hwaddr.ifr_hwaddr.sa_data;
        sprintf(mac_str, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
                hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
    }

    close(fd);
    printf("mac_str: %s\n", mac_str);
    return mac_str;
}

/**
 * @brief   设置Wi-Fi网卡工作在监听(Monitor)模式, 并在收到802.11帧的时候调用被传入的回调函数
 *
 * @param[in] cb @n A function pointer, called back when wifi receive a frame.
 */
#define MAX_REV_BUFFER 8000

static int s_enable_sniffer = 1;
static int s_fd_socket = 0;
extern char *g_ifname;
static void *func_Sniffer(void *cb)
{
    int32_t raw_socket = 0;
    struct ifreq ifr;
    struct sockaddr_ll sll;
    char rev_buffer[MAX_REV_BUFFER];
    int skipLen = 26;/* radiotap 默认长度为26 */
    char *ifname = g_ifname;
    memset(&ifr, 0, sizeof(struct ifreq));
    memset(&sll, 0, sizeof(struct sockaddr_ll));
    /* ifr.ifr_name can take 16 chars at most*/
    if (strlen(ifname) > 15) {
        return NULL;
    }
    raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_socket < 0) {
        printf("raw socket error: ");
        return NULL ;
    }

    memcpy(ifr.ifr_name, ifname, strlen(ifname));
    if (ioctl(raw_socket, SIOCGIFINDEX, &ifr) < 0) {
        close(raw_socket);
        printf("SIOCGIFINDED error: ");
        return NULL ;
    }

    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);
    if (bind(raw_socket, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        close(raw_socket);
        printf("bind error: ");
        return NULL ;
    }

    if (ioctl(raw_socket, SIOCGIFFLAGS, &ifr) < 0) {
        close(raw_socket);
        printf("SIOCGIFFLAGS error: ");
        return NULL ;
    }
    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(raw_socket, SIOCSIFFLAGS, &ifr) < 0) {
        close(raw_socket);
        printf("SIOCSIFFLAGS error: ");
        return NULL ;
    }

    printf("Sniffer Thread Create\r\n");
    s_fd_socket = raw_socket;
    while ((1 == s_enable_sniffer)) {
        int rev_num = recvfrom(raw_socket, rev_buffer, MAX_REV_BUFFER, 0, NULL, NULL);
        struct ieee80211_radiotap_header *pHeader = (struct ieee80211_radiotap_header *)rev_buffer;
        skipLen = pHeader->it_len;

#ifdef WIFI_CHIP_7601
        skipLen = 144;
#endif
        if (skipLen >= MAX_REV_BUFFER) {
            continue;
        }
        if (0) {
            int index = 0;
            /* printf("skipLen:%d ", skipLen); */
            for (index = 0; index < 180; index++) {
                printf("%02X-", rev_buffer[index]);
            }
            printf("\r\n");
        }
        if (rev_num > skipLen) {
            /* TODO fix the link type, with fcs, rssi */
            ((awss_recv_80211_frame_cb_t)cb)(rev_buffer + skipLen, rev_num - skipLen, AWSS_LINK_TYPE_NONE, 0, 0);
        }
    }

    close(raw_socket);

    printf("Sniffer Proc Finish\r\n");
    return (void *)0;
}

void stop_sniff()
{
    s_enable_sniffer = 0;
    close(s_fd_socket);
}

void start_sniff(_IN_ awss_recv_80211_frame_cb_t cb)
{
    static void *g_sniff_thread = NULL;
    int stack_used;
    hal_os_thread_param_t task_parms = {0};
    HAL_ThreadCreate(&g_sniff_thread, func_Sniffer, (void *)cb, &task_parms, &stack_used);
}



void HAL_Awss_Open_Monitor(_IN_ awss_recv_80211_frame_cb_t cb)
{
    extern void start_sniff(_IN_ awss_recv_80211_frame_cb_t cb);
    char *ifname = g_ifname;
    char buffer[256] = {0};
    int ret = 0;

#ifdef __UBUNTU_SDK_DEMO__
#if defined(WIFI_PROVISION_ENABLED)
    /* reset network */
    char *wifi_name = "linkkit";
    memset(buffer, 0, 128);
    snprintf(buffer, 128, "nmcli connection down %s", wifi_name);
    ret = system(buffer);

    memset(buffer, 0, 128);
    snprintf(buffer, 128, "nmcli connection delete %s", wifi_name);
    ret = system(buffer);
    sleep(10);
#endif
#endif
    s_enable_sniffer = 1;
    memset(buffer, 0, 256);
    snprintf(buffer, 256, "ifconfig %s down", ifname);
    ret = system(buffer);
    printf("ret1 is %d\r\n", ret);

    memset(buffer, 0, 256);
    snprintf(buffer, 256, "iwconfig %s mode monitor", ifname);
    ret = system(buffer);
    printf("ret2 is %d\r\n", ret);

    memset(buffer, 0, 256);
    snprintf(buffer, 256, "ifconfig %s up", ifname);
    ret = system(buffer);

    printf("ret3 is %d\r\n", ret);

    start_sniff(cb);
}

/**
 * @brief   设置Wi-Fi网卡离开监听(Monitor)模式, 并开始以站点(Station)模式工作
 */
void HAL_Awss_Close_Monitor(void)
{

    int ret = -1;
    char buffer[256] = {0};
    char *ifname = g_ifname;
    stop_sniff();
    memset(buffer, 0, 256);
    snprintf(buffer, 256, "ifconfig %s down", ifname);
    ret = system(buffer);

    memset(buffer, 0, 256);
    snprintf(buffer, 256, "iwconfig %s mode managed", ifname);
    ret =  system(buffer);

    memset(buffer, 0, 256);
    snprintf(buffer, 256, "ifconfig %s up", ifname);
    ret = system(buffer);
}

/**
 * @brief   设置Wi-Fi网卡切换到指定的信道(channel)上
 *
 * @param[in] primary_channel @n Primary channel.
 * @param[in] secondary_channel @n Auxiliary channel if 40Mhz channel is supported, currently
 *              this param is always 0.
 * @param[in] bssid @n A pointer to wifi BSSID on which awss lock the channel, most HAL
 *              may ignore it.
 */
void HAL_Awss_Switch_Channel(
            _IN_ char primary_channel,
            _IN_OPT_ char secondary_channel,
            _IN_OPT_ uint8_t bssid[ETH_ALEN])
{
    char cmd[255] = {0};
    int ret = -1;
    snprintf(cmd, 255, "iwconfig %s channel %d", g_ifname, primary_channel);
    ret = system(cmd);
    printf("switch:%s, ret is %d\n", cmd, ret);
    assert(0 == ret);
}

/**
 * @brief   要求Wi-Fi网卡连接指定热点(Access Point)的函数
 *
 * @param[in] connection_timeout_ms @n AP connection timeout in ms or HAL_WAIT_INFINITE
 * @param[in] ssid @n AP ssid
 * @param[in] passwd @n AP passwd
 * @param[in] auth @n optional(AWSS_AUTH_TYPE_INVALID), AP auth info
 * @param[in] encry @n optional(AWSS_ENC_TYPE_INVALID), AP encry info
 * @param[in] bssid @n optional(NULL or zero mac address), AP bssid info
 * @param[in] channel @n optional, AP channel info
 * @return
   @verbatim
     = 0: connect AP & DHCP success
     = -1: connect AP or DHCP fail/timeout
   @endverbatim
 * @see None.
 * @note
 *      If the STA connects the old AP, HAL should disconnect from the old AP firstly.
 *      If bssid specifies the dest AP, HAL should use bssid to connect dest AP.
 */

static int g_connect_status = 0;
int HAL_Awss_Connect_Ap(
            _IN_ uint32_t connection_timeout_ms,
            _IN_ char ssid[HAL_MAX_SSID_LEN],
            _IN_ char passwd[HAL_MAX_PASSWD_LEN],
            _IN_OPT_ enum AWSS_AUTH_TYPE auth,
            _IN_OPT_ enum AWSS_ENC_TYPE encry,
            _IN_OPT_ uint8_t bssid[ETH_ALEN],
            _IN_OPT_ uint8_t channel)
{
    char buffer[128] = {0};
    char *wifi_name = "linkkit";
    int ret = -1;

    printf("ssid  : %s\n", ssid);

    /**
     * using ubuntu network manager for connecting ap
     * reference:
     * https://developer.gnome.org/NetworkManager/stable/nmcli.html
     * https://www.96boards.org/documentation/consumer/guides/wifi_commandline.md.html     */
    memset(buffer, 0, 128);
    snprintf(buffer, 128, "nmcli connection down %s", wifi_name);
    ret = system(buffer);

    memset(buffer, 0, 128);
    snprintf(buffer, 128, "nmcli connection delete %s", wifi_name);
    ret = system(buffer);

    memset(buffer, 0, 128);
    snprintf(buffer, 128, "nmcli connection add con-name %s ifname %s type wifi ssid '%s'", wifi_name, g_ifname,
             ssid);
    ret = system(buffer);


    /**
     * security reference:
     * https://developer.gnome.org/NetworkManager/stable/settings-802-11-wireless-security.html
     */
    if (strlen(passwd) == 0) {
        memset(buffer, 0, 128);
        snprintf(buffer, 128, "nmcli connection modify %s wifi-sec.key-mgmt %s", "none", wifi_name);
        ret = system(buffer);
    } else {
        memset(buffer, 0, 128);
        snprintf(buffer, 128, "nmcli connection modify %s wifi-sec.key-mgmt %s", wifi_name, "wpa-psk");
        ret = system(buffer);
        memset(buffer, 0, 128);
        snprintf(buffer, 128, "nmcli connection modify %s wifi-sec.psk %s", wifi_name, passwd);
        ret = system(buffer);
    }

    memset(buffer, 0, 128);
    snprintf(buffer, 128, "nmcli connection up %s", wifi_name);
    ret = system(buffer);
    g_connect_status = (ret == 0 ? 0 : -1);
    return ret == 0 ? 0 : -1;
}

/**
 * @brief check system network is ready(get ip address) or not.
 *
 * @param None.
 * @return 0, net is not ready; 1, net is ready.
 * @see None.
 * @note None.
 */
int HAL_Sys_Net_Is_Ready()
{
    char result_buf[1024] = {0};

    if (g_connect_status < 0) {
        printf("error return\n");
        return 0;
    }

    do_cmd_exec("ifconfig", result_buf, sizeof(result_buf));
    if (strstr(result_buf, "inet addr")) {
        return 1;
    }
    return 0;
}

/**
 * @brief   在当前信道(channel)上以基本数据速率(1Mbps)发送裸的802.11帧(raw 802.11 frame)
 *
 * @param[in] type @n see enum HAL_Awss_frame_type, currently only FRAME_BEACON
 *                      FRAME_PROBE_REQ is used
 * @param[in] buffer @n 80211 raw frame, include complete mac header & FCS field
 * @param[in] len @n 80211 raw frame length
 * @return
   @verbatim
   =  0, send success.
   = -1, send failure.
   = -2, unsupported.
   @endverbatim
 * @see None.
 * @note awss use this API send raw frame in wifi monitor mode & station mode
 */


int HAL_Wifi_Send_80211_Raw_Frame(_IN_ enum HAL_Awss_Frame_Type type,
                                  _IN_ uint8_t *buffer, _IN_ int len)
{
    /* new raw socket */
    uint8_t t_buffer[512];
    struct ifreq t_ifr;
    struct sockaddr_ll t_sll;
    struct packet_mreq t_mr;
    int32_t t_size;

    uint8_t t_radiotap[] = {0x00, 0x00, 0x0d, 0x00, 0x04, 0x80, 0x02, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00};
    static int32_t t_socket = -1;

    if (t_socket < 0) {
        t_socket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

        if (t_socket < 0) {
            printf("<create_raw_socket> socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL)) failed!");
            return -1;
        }
        /* get the index of the interface */
        memset(&t_ifr, 0, sizeof(t_ifr));
        strncpy(t_ifr.ifr_name, g_ifname, sizeof(t_ifr.ifr_name) - 1);
        if (ioctl(t_socket, SIOCGIFINDEX, &t_ifr) < 0) {
            printf("<create_raw_socket> ioctl(SIOCGIFINDEX) failed!");
            close(t_socket);
            t_socket = -1;
            return -1;
        }
        /* bind the raw socket to the interface */

        memset(&t_sll, 0, sizeof(t_sll));
        t_sll.sll_family = AF_PACKET;
        t_sll.sll_ifindex = t_ifr.ifr_ifindex;
        t_sll.sll_protocol = htons(ETH_P_ALL);
        if (bind(t_socket, (struct sockaddr *)&t_sll, sizeof(t_sll)) < 0) {
            printf("<create_raw_socket> bind(ETH_P_ALL) failed!");
            close(t_socket);
            t_socket = -1;
            return -1;
        }
        /* open promisc */
        memset(&t_mr, 0, sizeof(t_mr));
        t_mr.mr_ifindex = t_sll.sll_ifindex;
        t_mr.mr_type = PACKET_MR_PROMISC;
        if (setsockopt(t_socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &t_mr, sizeof(t_mr)) < 0) {
            printf("<create_raw_socket> setsockopt(PACKET_MR_PROMISC) failed!");
            close(t_socket);
            t_socket = -1;
            return -1;
        }
    }
    //return t_socket;

    memcpy(t_buffer, t_radiotap, 13);
    len -= 4;
    memcpy(t_buffer + 13, buffer, len);
    len += 13;
    t_size = write(t_socket, t_buffer, len);
    if (t_size < 0) {
        printf("<send_80211_frame> write() failed!");
        close(t_socket);
        t_socket = -1;
        return -1;
    }
    return t_size;

}


/**
 * @brief   在站点(Station)模式下使能或禁用对管理帧的过滤
 *
 * @param[in] filter_mask @n see mask macro in enum HAL_Awss_frame_type,
 *                      currently only FRAME_PROBE_REQ_MASK & FRAME_BEACON_MASK is used
 * @param[in] vendor_oui @n oui can be used for precise frame match, optional
 * @param[in] callback @n see awss_wifi_mgmt_frame_cb_t, passing 80211
 *                      frame or ie to callback. when callback is NULL
 *                      disable sniffer feature, otherwise enable it.
 * @return
   @verbatim
   =  0, success
   = -1, fail
   = -2, unsupported.
   @endverbatim
 * @see None.
 * @note awss use this API to filter specific mgnt frame in wifi station mode
 */
int HAL_Wifi_Enable_Mgmt_Frame_Filter(
            _IN_ uint32_t filter_mask,
            _IN_OPT_ uint8_t vendor_oui[3],
            _IN_ awss_wifi_mgmt_frame_cb_t callback)
{
    return 0;
}

/**
 * @brief   获取所连接的热点(Access Point)的信息
 *
 * @param[out] ssid: array to store ap ssid. It will be null if ssid is not required.
 * @param[out] passwd: array to store ap password. It will be null if ap password is not required.
 * @param[out] bssid: array to store ap bssid. It will be null if bssid is not required.
 * @return
   @verbatim
     = 0: succeeded
     = -1: failed
   @endverbatim
 * @see None.
 * @note
 *     If the STA dosen't connect AP successfully, HAL should return -1 and not touch the ssid/passwd/bssid buffer.
 */

static void read_string_from_file(char *dst, const char *file, int dst_max_len)
{
    FILE *fp;
    int ret = 0;
    int lSize;
    char buffer[256] = {0};
    fp = fopen(file, "r");
    if (NULL == fp) {
        return;
    }
    fseek(fp, 0, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);
    if (lSize > 1 && lSize <= dst_max_len) {
        unsigned char pos = lSize - 1;
        ret = fread(dst, 1, lSize, fp);
        dst[pos] = 0;
    } else {
        /* the ssid has only one char, or illeagal */
        /*  put terminator at the first position */
        dst[0] = 0;
    }
    fclose(fp);
    snprintf(buffer, 256, "rm %s -f", file);
    ret = system(buffer);
    if (ret != 0) {
        printf("delete file %s error", file);
    }
}


int HAL_Wifi_Get_Ap_Info(
            _OU_ char ssid[HAL_MAX_SSID_LEN],
            _OU_ char passwd[HAL_MAX_PASSWD_LEN],
            _OU_ uint8_t bssid[ETH_ALEN])
{
#define MAXLINE 256
    char buffer[256] = {0};
    int ret = 0;
    char *data;

    if (g_connect_status < 0) {
        printf("error return in Apinfo\n");
        return 0;
    }

    if (NULL != ssid) {
        ret = system("wpa_cli status | grep ^ssid | sed 's/^ssid=//g' > /tmp/ssid");
        read_string_from_file(ssid, "/tmp/ssid", HAL_MAX_SSID_LEN);
    }

    if (NULL != passwd) {
        ret = system("wpa_cli status | grep ^passphrase | sed 's/^passphrase=//g' > /tmp/passphrase");
        read_string_from_file(passwd, "/tmp/passphrase", HAL_MAX_PASSWD_LEN);
    }

    if (NULL != bssid) {
        ret = system("wpa_cli status | grep ^bssid | sed 's/^bssid=//g' > /tmp/bssid");
        read_string_from_file((char *)bssid, "/tmp/bssid", ETH_ALEN);
    }

    return 0;
}

/* @brief   打开当前设备热点，并把设备由SoftAP模式切换到AP模式
 */

int HAL_Awss_Open_Ap(const char *ssid, const char *passwd, int beacon_interval, int hide)
{
    char buffer[256] = {0};
    char *sta_ssid = "linkkit";
    const char *ap_ssid = ssid; /*  "linkkit-ap"; */
    char *ap_passwd = "";
    int ret = -1;
    /**
     * using ubuntu16.04 network manager to create wireless access point
     * reference:
     * https://developer.gnome.org/NetworkManager/stable/nmcli.html
     * https://unix.stackexchange.com/questions/234552/create-wireless-access-point-and-share-internet-connection-with-nmcli
     */
    memset(buffer, 0, 256);
    snprintf(buffer, 256, "ifconfig %s down", g_ifname);
    ret = system(buffer);

    memset(buffer, 0, 256);
    snprintf(buffer, 256, "nmcli connection down %s", ap_ssid);
    ret = system(buffer);

    memset(buffer, 0, 256);
    snprintf(buffer, 256, "nmcli connection delete %s", sta_ssid);
    ret = system(buffer);

    memset(buffer, 0, 256);
    snprintf(buffer, 256, "nmcli connection delete %s", ap_ssid);
    ret = system(buffer);

    memset(buffer, 0, 256);
    snprintf(buffer, 256, "nmcli connection add con-name %s type wifi ifname %s autoconnect yes ssid %s mode ap", ap_ssid,
             g_ifname, ap_ssid);
    ret = system(buffer);

    if (strlen(ap_passwd) == 0) {
        memset(buffer, 0, 256);
        snprintf(buffer, 256, "nmcli connection modify %s 802-11-wireless.mode ap ipv4.method shared", ap_ssid);
        ret = system(buffer);
    } else {
        memset(buffer, 0, 256);
        snprintf(buffer, 256,
                 "nmcli connection modify %s 802-11-wireless.mode ap 802-11-wireless-security.key-mgmt wpa-psk ipv4.method shared 802-11-wireless-security.psk %s",
                 ap_ssid, ap_passwd);
        ret = system(buffer);
    }

    memset(buffer, 0, 256);
    snprintf(buffer, 256, "nmcli connection up %s", ap_ssid);
    ret = system(buffer);
    if (strlen(ap_ssid) < sizeof(g_opened_ap)) {
        memcpy(g_opened_ap, ap_ssid, strlen(ap_ssid));
    }
    return 0;
}

/* @brief   关闭当前设备热点，并把设备由SoftAP模式切换到Station模式
*/
int HAL_Awss_Close_Ap()
{
    char buffer[256] = {0};
    int ret = 0;
    /* use nmcli commands to close the previous open Ap */
    snprintf(buffer, 256, "nmcli connection down %s", g_opened_ap);
    ret = system(buffer);
    memset(g_opened_ap, 0, sizeof(g_opened_ap));
    return 0;
}

#endif  /* #if defined(HAL_AWSS) */



