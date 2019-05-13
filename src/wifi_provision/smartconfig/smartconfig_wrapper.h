#include "infra_types.h"
#include "infra_defs.h"
#include "wrappers_defs.h"
/*************************************** common hals ***************************************/
void HAL_SleepMs(uint32_t ms);
void *HAL_Malloc(uint32_t size);
void HAL_MutexLock(void *mutex);
void HAL_MutexUnlock(void *mutex);
uint64_t HAL_UptimeMs(void);
void HAL_Free(void *ptr);
void *HAL_MutexCreate(void);
void HAL_MutexDestroy(void *mutex);

/*************************************** wifi provision frameworks hals ***************************************/
/* frameworks/awss.c*/
int HAL_Sys_Net_Is_Ready();
int HAL_Wifi_Get_Ap_Info(char ssid[HAL_MAX_SSID_LEN],char passwd[HAL_MAX_PASSWD_LEN],uint8_t bssid[ETH_ALEN]);
/* awss_crypt.c */
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN]);
int HAL_GetProductSecret(char *product_secret);
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN]);
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN]);
/* zconfig_vendor_common.c */
void HAL_Awss_Close_Monitor(void);
void HAL_Awss_Open_Monitor(_IN_ awss_recv_80211_frame_cb_t cb);
void HAL_Awss_Switch_Channel(char primary_channel, char secondary_channel, uint8_t bssid[ETH_ALEN]);
char *HAL_Wifi_Get_Mac(_OU_ char mac_str[HAL_MAC_LEN]);
int HAL_Awss_Connect_Ap(
            _IN_ uint32_t connection_timeout_ms,
            _IN_ char ssid[HAL_MAX_SSID_LEN],
            _IN_ char passwd[HAL_MAX_PASSWD_LEN],
            _IN_OPT_ enum AWSS_AUTH_TYPE auth,
            _IN_OPT_ enum AWSS_ENC_TYPE encry,
            _IN_OPT_ uint8_t bssid[ETH_ALEN],
            _IN_OPT_ uint8_t channel);

