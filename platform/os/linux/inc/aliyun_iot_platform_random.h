#ifndef ALIYUN_IOT_PLATFORM_RANDOM_H
#define ALIYUN_IOT_PLATFORM_RANDOM_H

#include <stdio.h>

/***********************************************************
* 函数名称: aliyun_iot_gen_random
* 描       述: 获取随机数
* 输入参数: unsigned int len 读取随机数大小
* 输出参数: unsigned char *random 随机数的值
* 返 回  值: 0：成功，非零：失败
* 说       明: linux系统下随机数获取，通过读取设备获取随机值
*           mbedtls中使用
************************************************************/
int aliyun_iot_gen_random(unsigned char *random, unsigned int len);

#endif

