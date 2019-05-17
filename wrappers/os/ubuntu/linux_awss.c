#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/ioctl.h>

#include <net/if.h>
#if 1
    #include <ieee80211_radiotap.h>
#endif
#include "wrappers_defs.h"

#include <netinet/ip.h>
#include <sys/time.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include "iot_import_awss.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#if 0
    #include "aiot_errno.h"
    #include "aiot_wp_api.h"
    #include "ubuntu_sys.h"
#endif

#define WLAN_DEV "wlan0"
#define MAX_REV_BUFFER 8000

int HAL_ThreadCreate(
    void **thread_handle,
    void *(*work_routine)(void *),
    void *arg,
    hal_os_thread_param_t *hal_os_thread_param,
    int *stack_used);
#if 0
typedef int (*awss_recv_80211_frame_cb_t)(char *buf, int length,
        enum AWSS_LINK_TYPE link_type, int with_fcs, signed char rssi);
#endif

static int s_enable_sniffer = 1;
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
    raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_socket < 0) {
        perror("raw socket error: ");
        return NULL ;
    }

    memcpy(ifr.ifr_name, ifname , strlen(ifname));
    if (ioctl(raw_socket, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDED error: ");
        return NULL ;
    }

    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);
    if (bind(raw_socket, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        perror("bind error: ");
        return NULL ;
    }

    if (ioctl(raw_socket, SIOCGIFFLAGS, &ifr) < 0) {
        perror("SIOCGIFFLAGS error: ");
        return NULL ;
    }
    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(raw_socket, SIOCSIFFLAGS, &ifr) < 0) {
        perror("SIOCSIFFLAGS error: ");
        return NULL ;
    }

    printf("Sniffer Thread Create\r\n");
    if (raw_socket < 0) {
        perror("Sniffer Socket Alloc failed");
        return (void *)0;
    }


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
#if 0
            ((awss_recv_80211_frame_cb_t)cb)(rev_buffer + skipLen, rev_num - skipLen, AWSS_LINK_TYPE_80211_RADIO, 1, 0);
#endif
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
}

void start_sniff(_IN_ awss_recv_80211_frame_cb_t cb)
{
    static void *g_sniff_thread = NULL;
    int stack_used;
    hal_os_thread_param_t task_parms = {0};
    HAL_ThreadCreate(&g_sniff_thread, func_Sniffer, (void *)cb, &task_parms, &stack_used);
}
