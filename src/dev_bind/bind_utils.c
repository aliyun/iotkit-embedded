/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */
#include <memory.h>
#include "coap_api.h"
#include "wrappers.h"
#include "infra_state.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

void *bind_zalloc(uint32_t size)
{
#ifdef INFRA_MEM_STATS
    void *ptr = LITE_malloc(size, MEM_MAGIC, "bind");
#else
    void *ptr = HAL_Malloc(size);
#endif
    if (ptr != NULL) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void bind_free(void *ptr)
{
#ifdef INFRA_MEM_STATS
    LITE_free(ptr);
#else
    HAL_Free((void *)ptr);
#endif
}

uint32_t bind_time_is_expired(uint32_t time)
{
    uint32_t cur_time;

    cur_time = HAL_UptimeMs();
    /*
     *  WARNING: Do NOT change the following code until you know exactly what it do!
     *
     *  check whether it reach destination time or not.
     */
    if ((cur_time - time) < (UINT32_MAX / 2)) {
        return 1;
    } else {
        return 0;
    }
}

int bind_time_countdown_ms(uint32_t *time, uint32_t millisecond)
{
    if (time == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }
    *time = HAL_UptimeMs() + millisecond;
    return 0;
}

uint32_t bind_time_left(uint32_t time)
{
    uint32_t now, res;

    if (bind_time_is_expired(time)) {
        return 0;
    }

    now = HAL_UptimeMs();
    res = time - now;
    return res;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif
