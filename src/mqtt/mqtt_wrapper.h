#include "infra_types.h"
#include "infra_defs.h"

int8_t HAL_GetPartnerID(char *pid_str);
int8_t HAL_GetModuleID(char *mid_str);
int HAL_SetProductKey(char *product_key);
int HAL_SetDeviceName(char *device_name);
int HAL_SetProductSecret(char *product_secret);
int HAL_SetDeviceSecret(char *device_secret);

int8_t HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN]);
int8_t HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN]);
int8_t HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN]);

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
void HAL_Printf(const char *fmt, ...);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);
uint64_t HAL_UptimeMs(void);
void HAL_Srandom(uint32_t seed);
void *HAL_MutexCreate(void);
void HAL_MutexDestroy(void *mutex);
void HAL_MutexLock(void *mutex);
void HAL_MutexUnlock(void *mutex);
void HAL_SleepMs(uint32_t ms);
uint32_t HAL_Random(uint32_t region);
int HAL_GetFirmwareVersion(char *version);

uint8_t algo_sha256_wrapper(uint8_t *input, uint16_t input_len, uint8_t output[32]);
//mqtt protocol wrapper
void *mqtt_init_wrapper(iotx_mqtt_param_t *mqtt_params);
int mqtt_connect_wrapper(void *client);
int mqtt_yield_wrapper(void *client, int timeout_ms);
int mqtt_check_state_wrapper(void *client);
int mqtt_subscribe_wrapper(void *client,
                           const char *topicFilter,
                           iotx_mqtt_qos_t qos,
                           iotx_mqtt_event_handle_func_fpt topic_handle_func,
                           void *pcontext);
int mqtt_subscribe_sync_wrapper(void *client,
                                const char *topic_filter,
                                iotx_mqtt_qos_t qos,
                                iotx_mqtt_event_handle_func_fpt topic_handle_func,
                                void *pcontext,
                                int timeout_ms);
int mqtt_unsubscribe_wrapper(void *client, const char *topicFilter);
int mqtt_publish_wrapper(void *client, const char *topicName, iotx_mqtt_topic_info_pt topic_msg);
int mqtt_release_wrapper(void **pclient);
