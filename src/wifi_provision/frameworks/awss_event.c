/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "infra_log.h"
#include "infra_classic.h"

extern int iotx_event_post(int event);

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

int awss_event_post(int event)
{
    int ret = 0;
    void *cb = NULL;

    ret = iotx_event_post(event);

    cb = (void *)iotx_event_callback(ITE_AWSS_STATUS);
    if (cb)
        ret = ((int (*)(int))cb)(event);

    return ret;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif
