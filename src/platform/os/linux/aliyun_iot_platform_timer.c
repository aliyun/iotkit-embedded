/*********************************************************************************
 * 文件名称: aliyun_iot_platform_timer.c
 * 作       者:
 * 版       本:
 * 日       期: 2016-05-30
 * 描       述:
 * 其       它:
 * 历       史:
 **********************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include "aliyun_iot_platform_timer.h"


uint32_t aliot_platform_time_get_ms(void)
{
    struct timeval now;

    gettimeofday(&now, NULL);

    return (uint32_t)((now.tv_sec)*1000 + (now.tv_usec)/1000);
}
