#include "aliyun_iot_platform_memory.h"


void* aliyun_iot_memory_malloc(INT32 size)
{
    return OSMMalloc(size);
}


void aliyun_iot_memory_free(void* ptr)
{
    OSMFree(ptr);
    return;
}
