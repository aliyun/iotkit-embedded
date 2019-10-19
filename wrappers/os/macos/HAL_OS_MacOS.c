/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */





#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>
//#include <sys/prctl.h>
#include <sys/time.h>
#include <semaphore.h>
#include <errno.h>
#include <assert.h>
#include <net/if.h>       // struct ifreq
//inet_ntoa
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "wrappers_defs.h"
#include "wrappers_os.h"

#ifdef DYNAMIC_REGISTER
    char _product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1ZETBPbycq";
    char _product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "L68wCVXYUaNg1Ey9";
    char _device_name[IOTX_DEVICE_NAME_LEN + 1]       = "example1";
    char _device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "";
#else
    #ifdef DEVICE_MODEL_ENABLED
        char _product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1RIsMLz2BJ";
        char _product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "fSAF0hle6xL0oRWd";
        char _device_name[IOTX_DEVICE_NAME_LEN + 1]       = "example1";
        char _device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "RDXf67itLqZCwdMCRrw0N5FHbv5D7jrE";
    #else
        char _product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1MZxOdcBnO";
        char _product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "h4I4dneEFp7EImTv";
        char _device_name[IOTX_DEVICE_NAME_LEN + 1]       = "test_01";
        char _device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "t9GmMf2jb3LgWfXBaZD2r3aJrfVWBv56";
    #endif
#endif

void *HAL_MutexCreate(void)
{
    int err_num;
    pthread_mutex_t *mutex = (pthread_mutex_t *)HAL_Malloc(sizeof(pthread_mutex_t));
    if (NULL == mutex) {
        return NULL;
    }

    if (0 != (err_num = pthread_mutex_init(mutex, NULL))) {
        printf("create mutex failed\n");
        HAL_Free(mutex);
        return NULL;
    }

    return mutex;
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_destroy((pthread_mutex_t *)mutex))) {
        printf("destroy mutex failed\n");
    }

    HAL_Free(mutex);
}

void HAL_MutexLock(_IN_ void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_lock((pthread_mutex_t *)mutex))) {
        printf("lock mutex failed\n");
    }
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_unlock((pthread_mutex_t *)mutex))) {
        printf("unlock mutex failed\n");
    }
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    return malloc(size);
}

void HAL_Free(_IN_ void *ptr)
{
    free(ptr);
}

#if defined(__MACH__) && !defined(CLOCK_REALTIME)

#include <sys/time.h>
#define CLOCK_REALTIME 0
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 0

// clock_gettime is not implemented on older versions of OS X (< 10.12).
// If implemented, CLOCK_REALTIME will have already been defined.

int clock_gettime(int clk_id, struct timespec *t)
{
    struct timeval now;
    int rv = gettimeofday(&now, NULL);
    if (rv) {
        return rv;
    }
    t->tv_sec  = now.tv_sec;
    t->tv_nsec = now.tv_usec * 1000;
    return 0;
}
#endif

uint64_t HAL_UptimeMs(void)
{
    uint64_t            time_ms;
    struct timespec     ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    time_ms = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000 / 1000);

    return time_ms;
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
    usleep(1000 * ms);
}

void HAL_Srandom(uint32_t seed)
{
    srandom(seed);
}

uint32_t HAL_Random(uint32_t region)
{
    return (region > 0) ? (random() % region) : (uint32_t)random();
}

int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *format, va_list ap)
{
    return vsnprintf(str, len, format, ap);
}

int HAL_GetFirmwareVersion(_OU_ char version[IOTX_FIRMWARE_VER_LEN + 1])
{
    memset(version, 0x0, IOTX_FIRMWARE_VER_LEN + 1);
    strncpy(version, "1.0", IOTX_FIRMWARE_VER_LEN + 1);
    version[IOTX_FIRMWARE_VER_LEN] = '\0';
    return strlen(version);
}

typedef struct {
    int count;
    pthread_cond_t cond;
    pthread_mutex_t lock;
} os_sem_t;

void *HAL_SemaphoreCreate(void)
{
    os_sem_t *sem = malloc(sizeof(os_sem_t));
    if (!sem) {
        return NULL;
    }
    memset(sem, 0, sizeof(os_sem_t));

    sem->count = 0;
    pthread_cond_init(&sem->cond, NULL);
    pthread_mutex_init(&sem->lock, NULL);

    return sem;
}

void HAL_SemaphoreDestroy(_IN_ void *_sem)
{
    os_sem_t *sem = _sem;
    if (sem) {
        pthread_cond_destroy(&sem->cond);
        pthread_mutex_destroy(&sem->lock);
        free(sem);
    }
}

void HAL_SemaphorePost(_IN_ void *_sem)
{
    os_sem_t *sem = _sem;
    if (!sem) {
        return;
    }
    pthread_mutex_lock(&sem->lock);
    sem->count++;
    pthread_cond_signal(&sem->cond);
    pthread_mutex_unlock(&sem->lock);
}

int HAL_SemaphoreWait(_IN_ void *_sem, _IN_ uint32_t timeout_ms)
{
    os_sem_t *sem = _sem;
    if (!sem) {
        return -1;
    }

    struct timeval tv;
    if (gettimeofday(&tv, NULL) < 0) {
        return -1;
    }

    struct timespec ts;
    ts.tv_sec  = tv.tv_sec + timeout_ms / 1000;
    ts.tv_nsec = tv.tv_usec * 1000 + timeout_ms % 1000;

    pthread_mutex_lock(&sem->lock);
    while (sem->count <= 0) {
        int ret = pthread_cond_timedwait(&sem->cond, &sem->lock, &ts);
        if (ret != 0) {
            if (ret != EAGAIN) {
                pthread_mutex_unlock(&sem->lock);
                return -1;
            }
        } else if (ret == 0) {
            break;
        }
    }
    sem->count--;
    pthread_mutex_unlock(&sem->lock);

    return 0;
}

int HAL_ThreadCreate(
            _OU_ void **thread_handle,
            _IN_ void *(*work_routine)(void *),
            _IN_ void *arg,
            _IN_ hal_os_thread_param_t *hal_os_thread_param,
            _OU_ int *stack_used)
{
    int ret = -1;
    if (stack_used) {
        *stack_used = 0;
    }

    ret = pthread_create((pthread_t *)thread_handle, NULL, work_routine, arg);

    return ret;
}

static FILE *fp;

#define otafilename "/tmp/alinkota.bin"
void HAL_Firmware_Persistence_Start(void)
{
    fp = fopen(otafilename, "w");
    assert(fp);
    return;
}

int HAL_Firmware_Persistence_Write(_IN_ char *buffer, _IN_ uint32_t length)
{
    unsigned int written_len = 0;
    written_len = fwrite(buffer, 1, length, fp);

    if (written_len != length) {
        return -1;
    }
    return 0;
}

int HAL_Firmware_Persistence_Stop(void)
{
    if (fp != NULL) {
        fclose(fp);
    }

    return 0;
}

int HAL_Config_Write(const char *buffer, int length)
{
    FILE *fp;
    size_t written_len;
    char filepath[128] = {0};

    if (!buffer || length <= 0) {
        return -1;
    }

    snprintf(filepath, sizeof(filepath), "./%s", "alinkconf");
    fp = fopen(filepath, "w");
    if (!fp) {
        return -1;
    }

    written_len = fwrite(buffer, 1, length, fp);

    fclose(fp);

    return ((written_len != length) ? -1 : 0);
}

int HAL_Config_Read(char *buffer, int length)
{
    FILE *fp;
    size_t read_len;
    char filepath[128] = {0};

    if (!buffer || length <= 0) {
        return -1;
    }

    snprintf(filepath, sizeof(filepath), "./%s", "alinkconf");
    fp = fopen(filepath, "r");
    if (!fp) {
        return -1;
    }

    read_len = fread(buffer, 1, length, fp);
    fclose(fp);

    return ((read_len != length) ? -1 : 0);
}

#define REBOOT_CMD "reboot"
void HAL_Reboot(void)
{
    if (system(REBOOT_CMD)) {
        perror("HAL_Reboot failed");
    }
}

#define ROUTER_INFO_PATH        "/proc/net/route"
#define ROUTER_RECORD_SIZE      256

char *_get_default_routing_ifname(char *ifname, int ifname_size)
{
    FILE *fp = NULL;
    char line[ROUTER_RECORD_SIZE] = {0};
    char iface[IFNAMSIZ] = {0};
    char *result = NULL;
    unsigned int destination, gateway, flags, mask;
    unsigned int refCnt, use, metric, mtu, window, irtt;

    fp = fopen(ROUTER_INFO_PATH, "r");
    if (fp == NULL) {
        perror("fopen");
        return result;
    }

    char *buff = fgets(line, sizeof(line), fp);
    if (buff == NULL) {
        perror("fgets");
        goto out;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (11 !=
            sscanf(line, "%s %08x %08x %x %d %d %d %08x %d %d %d",
                   iface, &destination, &gateway, &flags, &refCnt, &use,
                   &metric, &mask, &mtu, &window, &irtt)) {
            perror("sscanf");
            continue;
        }

        /*default route */
        if ((destination == 0) && (mask == 0)) {
            strncpy(ifname, iface, ifname_size - 1);
            result = ifname;
            break;
        }
    }

out:
    if (fp) {
        fclose(fp);
    }

    return result;
}

char *HAL_Wifi_Get_Mac(char mac_str[HAL_MAC_LEN])
{
#if 0
    struct ifreq ifr;
    int sock = -1;

    memset(mac_str, 0, HAL_MAC_LEN);

    char ifname_buff[IFNAMSIZ] = {0};
    char *ifname = _get_default_routing_ifname(ifname_buff, sizeof(ifname_buff));
    if (!ifname) {
        perror("get default routeing ifname");
        goto fail;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        goto fail;
    }

    ifr.ifr_addr.sa_family = AF_INET; //ipv4 address
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        close(sock);
        perror("ioctl");
        goto fail;
    }

    close(sock);

    char *ptr = mac_str;
    char *end = mac_str + HAL_MAC_LEN;

    int i;
    for (i = 0; i < 6; i++) {
        if (i == 5) {
            ptr += snprintf(ptr, end - ptr, "%02x", (uint8_t)ifr.ifr_hwaddr.sa_data[i]);
        } else {
            ptr += snprintf(ptr, end - ptr, "%02x:", (uint8_t)ifr.ifr_hwaddr.sa_data[i]);
        }
    }

    return mac_str;

fail:
    strncpy(mac_str, "de:ad:be:ef:00:00", HAL_MAC_LEN - 1);
#endif
    return mac_str;
}

uint32_t HAL_Wifi_Get_IP(char ip_str[NETWORK_ADDR_LEN], const char *ifname)
{
    struct ifreq ifreq;
    int sock = -1;
    char ifname_buff[IFNAMSIZ] = {0};

    if ((NULL == ifname || strlen(ifname) == 0) &&
        NULL == (ifname = _get_default_routing_ifname(ifname_buff, sizeof(ifname_buff)))) {
        perror("get default routeing ifname");
        return -1;
    }

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    ifreq.ifr_addr.sa_family = AF_INET; //ipv4 address
    strncpy(ifreq.ifr_name, ifname, IFNAMSIZ - 1);

    if (ioctl(sock, SIOCGIFADDR, &ifreq) < 0) {
        close(sock);
        perror("ioctl");
        return -1;
    }

    close(sock);

    strncpy(ip_str,
            inet_ntoa(((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr),
            NETWORK_ADDR_LEN);

    return ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr;
}

int HAL_Kv_Set(const char *key, const void *val, int len, int sync)
{
    return 0;
}

int HAL_Kv_Get(const char *key, void *buffer, int *buffer_len)
{
    return 0;
}

int HAL_Kv_Del(const char *key)
{
    return 0;
}

