/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef _SAL_WRAPPER_H_
#define _SAL_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>

#ifndef NULL
#define NULL (void *)0
#endif

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
void HAL_Printf(const char *fmt, ...);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);
uint64_t HAL_UptimeMs(void);
void *HAL_MutexCreate(void);
void HAL_MutexDestroy(void *mutex);
void HAL_MutexLock(void *mutex);
void HAL_MutexUnlock(void *mutex);
void HAL_SleepMs(uint32_t ms);
void *HAL_SemaphoreCreate(void);
void HAL_SemaphoreDestroy(void *sem);
void HAL_SemaphorePost(void *sem);
int HAL_SemaphoreWait(void *sem, uint32_t timeout_ms);

#define PLATFORM_WAIT_INFINITE (~0)

typedef enum {
    os_thread_priority_idle = -3,        /* priority: idle (lowest) */
    os_thread_priority_low = -2,         /* priority: low */
    os_thread_priority_belowNormal = -1, /* priority: below normal */
    os_thread_priority_normal = 0,       /* priority: normal (default) */
    os_thread_priority_aboveNormal = 1,  /* priority: above normal */
    os_thread_priority_high = 2,         /* priority: high */
    os_thread_priority_realtime = 3,     /* priority: realtime (highest) */
    os_thread_priority_error = 0x84,     /* system cannot determine priority or thread has illegal priority */
} hal_os_thread_priority_t;

typedef struct _hal_os_thread {
    hal_os_thread_priority_t priority;     /*initial thread priority */
    void                    *stack_addr;   /* thread stack address malloced by caller, use system stack by . */
    size_t                   stack_size;   /* stack size requirements in bytes; 0 is default stack size */
    int                      detach_state; /* 0: not detached state; otherwise: detached state. */
    char                    *name;         /* thread name. */
} hal_os_thread_param_t;

int HAL_ThreadCreate(
                     void **thread_handle,
                     void *(*work_routine)(void *),
                     void *arg,
                     hal_os_thread_param_t *hal_os_thread_param,
                     int *stack_used);

typedef enum {
    /* WiFi */
    TCP_SERVER,
    TCP_CLIENT,
    SSL_CLIENT,
    UDP_BROADCAST,
    UDP_UNICAST,
    /*WiFi end */
    /* Add others hereafter */
} CONN_TYPE;

/* Fill necessary fileds according to the socket type. */
typedef struct {
    int fd; /* fd that are used in socket level */
    CONN_TYPE type;
    char *addr; /* remote ip or domain */
    int32_t r_port; /* remote port (set to -1 if not used) */
    int32_t l_port; /* local port (set to -1 if not used) */
    uint32_t tcp_keep_alive; /* tcp keep alive value (set to 0 if not used) */
} sal_conn_t;

/* Socket data state indicator. */
typedef enum netconn_evt {
    NETCONN_EVT_RCVPLUS,
    NETCONN_EVT_RCVMINUS,
    NETCONN_EVT_SENDPLUS,
    NETCONN_EVT_SENDMINUS,
    NETCONN_EVT_ERROR
} netconn_evt_t;

typedef int (*netconn_data_input_cb_t)(int fd, void *data, size_t len, char remote_ip[16], uint16_t remote_port);

/**
 * Module low level init so that it's ready to setup socket connection.
 *
 * @return  0 - success, -1 - failure
 */
int HAL_SAL_Init(void);


/**
 * Start a socket connection via module.
 *
 * @param[in]  conn - connect parameters which are used to setup
 *                 the socket connection.
 *
 * @return  0 - success, -1 - failure
 */
int HAL_SAL_Start(sal_conn_t *conn);


/**
 * Send data via module.
 * This function does not return until all data sent.
 *
 * @param[in]  fd - the file descripter to operate on.
 * @param[in]  data - pointer to data to send.
 * @param[in]  len - length of the data.
 * @param[in]  remote_ip - remote port number (optional).
 * @param[in]  remote_port - remote port number (optional).
 *
 * @return  0 - success, -1 - failure
 */
int HAL_SAL_Send(int fd, uint8_t *data, uint32_t len, char remote_ip[16],
                int32_t remote_port, int32_t timeout);


/**
 * Get IP information of the corresponding domain.
 * Currently only one IP string is returned (even when the domain
 * coresponses to mutliple IPs). Note: only IPv4 is supported.
 *
 * @param[in]   domain - the domain string.
 * @param[out]  ip - the place to hold the dot-formatted ip string.
 *
 * @return  0 - success, -1 - failure
 */
int HAL_SAL_DomainToIp(char *domain, char ip[16]);


/**
 * Close the socket connection.
 *
 * @param[in]  fd - the file descripter to operate on.
 * @param[in]  remote_port - remote port number (optional).
 *
 * @return  0 - success, -1 - failure
 */
int HAL_SAL_Close(int fd, int32_t remote_port);


/**
 * Destroy SAL or exit low level state if necessary.
 *
 * @return  0 - success, -1 - failure
 */
int HAL_SAL_Deinit(void);


/**
 * Register network connection data input function
 * Input data from module.
 * This callback should be called when the data is received from the module
 * It should tell the sal where the data comes from.
 * @param[in]  fd - the file descripter to operate on.
 * @param[in]  data - the received data.
 * @param[in]  len - expected length of the data when IN,
 *                    and real read len when OUT.
 * @param[in]  addr - remote ip address. Caller manages the
                            memory (optional).
 * @param[in]  port - remote port number (optional).
 *
 * @return  0 - success, -1 - failure
 */
int HAL_SAL_RegisterNetconnDataInputCb(netconn_data_input_cb_t cb);



#ifdef __cplusplus
}
#endif

#endif

