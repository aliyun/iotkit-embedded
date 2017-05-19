#include "aliyun_iot_platform_memory.h"


void* aliyun_iot_memory_malloc(INT32 size)
{
    return mem_alloc(size);
}

void* aliyun_iot_memory_calloc(UINT32 n, UINT32 size)
{
	return mem_calloc(n, size);
}

void aliyun_iot_memory_free(void* ptr)
{
    mem_free(ptr);
    return;
}
