#include "infra_types.h"
#include "infra_defs.h"

uint64_t HAL_UptimeMs(void)
{
    unsigned int tick, cur_time;
    kal_get_time(&tick);
    cur_time = kal_ticks_to_milli_secs(tick);
    return (uint64_t)cur_time;
}

int HAL_Snprintf(char *str, const int len, const char *fmt, ...)
{
	return (int)1;
}

void HAL_Printf(const char *fmt, ...)
{
	return;
}

void *HAL_Malloc(uint32_t size)
{
	return med_alloc_ext_mem(size);
}

void HAL_Free(void *ptr)
{
	med_free_ext_mem(ptr);
}

int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN])
{
	return (int)1;
}

int HAL_GetFirmwareVersion(char *version)
{
	return (int)1;
}

int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN])
{
	return (int)1;
}

int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN])
{
	return (int)1;
}