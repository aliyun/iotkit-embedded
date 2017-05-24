/*********************************************************************************
 * 文件名称: aliyun_iot_common_log.c
 * 作       者:
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 其       它:
 * 历       史:
 **********************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "aliyun_iot_platform_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"

#include "aliyun_iot_platform_pthread.h"
#include "aliyun_iot_platform_timer.h"

aliot_log_level_t g_iotLogLevel = ALIOT_LOG_LEVEL_DEBUG;


void aliyun_iot_common_log_set_level(aliot_log_level_t iotLogLevel)
{
    g_iotLogLevel = iotLogLevel;
}

aliot_log_level_t aliyun_iot_common_log_get_level()
{
    return g_iotLogLevel;
}


