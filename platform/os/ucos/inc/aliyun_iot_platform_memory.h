#ifndef ALIYUN_IOT_PLATFORM_MEMORY_H
#define ALIYUN_IOT_PLATFORM_MEMORY_H

//#include <stdlib.h>
#include "os_cpu.h"
#include "os_dmem.h"

#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"

void* aliyun_iot_memory_malloc(INT32 size);

void aliyun_iot_memory_free(void* ptr);

#endif
