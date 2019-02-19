#include "infra_types.h"
#include "infra_defs.h"
#include "wrappers_defs.h"

int HAL_Awss_Get_Conn_Encrypt_Type(void);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN]);
int HAL_GetProductSecret(char *product_secret);
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN]);
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN]);
void *HAL_Timer_Create(const char *name, void (*func)(void *), void *user_data);
int HAL_Timer_Stop(void *timer);
int HAL_Timer_Start(void *timer, int ms);
int HAL_Timer_Delete(void *timer);
char *HAL_Wifi_Get_Mac(char mac_str[HAL_MAC_LEN]);
void HAL_Srandom(uint32_t seed);
uint32_t HAL_Random(uint32_t region);
void HAL_Reboot();
void *HAL_MutexCreate(void);
void HAL_SleepMs(uint32_t ms);
void HAL_MutexDestroy(void *mutex);
void HAL_MutexLock(void *mutex);
void HAL_MutexUnlock(void *mutex);
void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
int HAL_Sys_Net_Is_Ready();
uint64_t HAL_UptimeMs(void);
uint32_t HAL_Wifi_Get_IP(char ip_str[NETWORK_ADDR_LEN], const char *ifname);
