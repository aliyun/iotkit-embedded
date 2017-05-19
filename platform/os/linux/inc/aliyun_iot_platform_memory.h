#ifndef ALIYUN_IOT_PLATFORM_MEMORY_H
#define ALIYUN_IOT_PLATFORM_MEMORY_H

#include <stdlib.h>
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"

/***********************************************************
* 函数名称: aliyun_iot_memory_malloc
* 描       述: 动态开辟内存函数
* 输入参数: INT32 size
* 输出参数:
* 返 回  值: void*
* 说       明: 用所在系统的malloc函数实现此接口
************************************************************/
void* aliyun_iot_memory_malloc(INT32 size);

/***********************************************************
* 函数名称: aliyun_iot_memory_free
* 描       述: 释放动态内存函数
* 输入参数: void* ptr
* 输出参数:
* 返 回  值: void
* 说       明: 用所在系统的free函数实现此接口
************************************************************/
void aliyun_iot_memory_free(void* ptr);

#endif
