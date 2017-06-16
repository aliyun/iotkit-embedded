/*********************************************************************************
 * 文件名称: aliot_log.c
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
#include "aliot_platform.h"
#include "aliot_error.h"
#include "aliot_log.h"


aliot_log_level_t g_iotLogLevel = ALIOT_LOG_LEVEL_DEBUG;


void aliot_log_set_level(aliot_log_level_t iotLogLevel)
{
    g_iotLogLevel = iotLogLevel;
}

aliot_log_level_t aliot_log_get_level()
{
    return g_iotLogLevel;
}


