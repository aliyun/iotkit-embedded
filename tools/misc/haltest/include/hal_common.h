#ifndef _HAL_COMMON_H_
#define _HAL_COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* link type */
typedef enum {
    AWSS_LINK_TYPE_NONE,
    AWSS_LINK_TYPE_PRISM,
    AWSS_LINK_TYPE_80211_RADIO,
    AWSS_LINK_TYPE_80211_RADIO_AVS
} aws_link_type_t;

typedef int (*awss_recv_80211_frame_cb_t)(char *buf, int length,
        aws_link_type_t link_type, int with_fcs, signed char rssi);

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
    int                   stack_size;   /* stack size requirements in bytes; 0 is default stack size */
    int                      detach_state; /* 0: not detached state; otherwise: detached state. */
    char                    *name;         /* thread name. */
} hal_os_thread_param_t;

int HAL_ThreadCreate(
            void **thread_handle,
            void *(*work_routine)(void *),
            void *arg,
            hal_os_thread_param_t *hal_os_thread_param,
            int *stack_used);

#endif