#ifndef _DEV_RESET_WRAPPER_H_
#define _DEV_RESET_WRAPPER_H_

void HAL_Printf(const char *fmt, ...);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);
void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
void HAL_Printf(const char *fmt, ...);
int HAL_GetProductKey(char *product_key);
int HAL_GetDeviceName(char *device_name);
int HAL_Kv_Set(const char *key, const void *val, int len, int sync);
int HAL_Kv_Get(const char *key, void *val, int *buffer_len);
void *HAL_Timer_Create(const char *name, void (*func)(void *), void *user_data);
int HAL_Timer_Start(void *timer, int ms);
int HAL_Timer_Stop(void *timer);
int HAL_Timer_Delete(void *timer);

#endif