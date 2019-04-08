#include "hal_common.h"
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <netinet/ip.h>
#include <sys/time.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
#include "ieee80211_radiotap.h"

#define WLAN_DEV "wlan0"
#define MAX_REV_BUFFER 8000

static int s_enable_sniffer = 1;
static void * func_Sniffer(void *cb)
{
    int sock = socket(PF_PACKET, SOCK_RAW, htons(0x03)); /* ETH_P_ALL */
    char rev_buffer[MAX_REV_BUFFER];

    int skipLen = 26;/* radiotap 默认长度为26 */
    /* printf("Sniffer Thread Create\r\n"); */
    if(sock < 0)
    {
        printf("Sniffer Socket Alloc Fails %d \r\n", sock);
        perror("Sniffer Socket Alloc");
        return (void *)0;
    }

    {/* 强制绑定到wlan0 上。后续可以考虑去掉 */
        struct ifreq ifr;
        memset(&ifr, 0x00, sizeof(ifr));
        strncpy(ifr.ifr_name, WLAN_DEV , strlen(WLAN_DEV) + 1);
        setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifr, sizeof(ifr));
    }

    while((1 == s_enable_sniffer))
    {
        int rev_num = recvfrom(sock, rev_buffer, MAX_REV_BUFFER, 0, NULL, NULL);
        struct ieee80211_radiotap_header *pHeader = (struct ieee80211_radiotap_header *)rev_buffer;
        skipLen = pHeader->it_len;

#ifdef WIFI_CHIP_7601
        skipLen = 144;
#endif
        if(skipLen >= MAX_REV_BUFFER)
        {/* 有出现过header全ff的情况，这里直接丢掉这个包 */
            /* printf("Sniffer skip len > MAX_REV_BUFFER\n"); */
            continue;
        }

        if(0)
        {
            int index = 0;
            /* printf("skipLen:%d ", skipLen); */
            for(index = 0; index < 180; index++)
            {
                printf("%02X-", rev_buffer[index]);
            }
            printf("\r\n");
        }
        if(rev_num > skipLen)
        {
            /* TODO fix the link type, with fcs, rssi */
            ((awss_recv_80211_frame_cb_t)cb)(rev_buffer + skipLen, rev_num - skipLen, AWSS_LINK_TYPE_80211_RADIO, 1, 0);
        }
    }

    close(sock);

    printf("Sniffer Proc Finish\r\n");
    return (void *)0;
}

void start_sniff(awss_recv_80211_frame_cb_t cb){
    static void *g_sniff_thread = NULL;
    int stack_used;
    hal_os_thread_param_t task_parms = {0};
    HAL_ThreadCreate(&g_sniff_thread, func_Sniffer, (void *)cb, &task_parms, &stack_used);
}

void HAL_Awss_Open_Monitor(awss_recv_80211_frame_cb_t cb)
{
    extern void start_sniff(awss_recv_80211_frame_cb_t cb);
    int ret = system("ifconfig wlan0 down; ifconfig wlan0 up");
    /* printf("wlan0 up ret is %d", ret); */
    assert(0 == ret);
    ret = system("iwconfig wlan0 mode monitor");
    /* printf("wlan0 mode monitor ret is %d", ret); */
    assert(0 == ret);
    ret = system("echo \"10 1 1\" > /proc/net/rtl8188fu/wlan0/monitor");
    /* printf("wlan0 echo data to monitor ret is  %d", ret); */
    assert(0 == ret);
    start_sniff(cb);
}

int HAL_ThreadCreate(
            void **thread_handle,
            void *(*work_routine)(void *),
            void *arg,
            hal_os_thread_param_t *hal_os_thread_param,
            int *stack_used)
{
    int ret = -1;

    if (stack_used) {
        *stack_used = 0;
    }

    ret = pthread_create((pthread_t *)thread_handle, NULL, work_routine, arg);

    return ret;
}