/*********************************************************************************
 * 文件名称: aliyun_iot_platform_timer.c
 * 作       者:
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 其       它:
 * 历       史:
 **********************************************************************************/

#include "aliyun_iot_platform_timer.h"

#include <stdlib.h>
#include <windows.h>


uint32_t aliot_platform_time_get_ms(void)
{
    return (uint32_t)(GetTickCount());
}
