#include "infra_types.h"
#include "infra_defs.h"

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
void HAL_Printf(const char *fmt, ...);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);
uint64_t HAL_UptimeMs(void);
void HAL_SleepMs(uint32_t ms);

void *HAL_MutexCreate(void);
void HAL_MutexDestroy(void *mutex);
void HAL_MutexLock(void *mutex);
void HAL_MutexUnlock(void *mutex);

int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN]);
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN]);
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN]);
int HAL_GetFirmwareVersion(char *version);

uintptr_t HAL_TCP_Establish(const char *host, uint16_t port);
int HAL_TCP_Destroy(uintptr_t fd);
int32_t HAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms);
int32_t HAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms);

/* mqtt protocol wrapper */
void *wrapper_mqtt_init(iotx_mqtt_param_t *mqtt_params);
int wrapper_mqtt_connect(void *client);
int wrapper_mqtt_yield(void *client, int timeout_ms);
int wrapper_mqtt_check_state(void *client);
int wrapper_mqtt_subscribe(void *client,
                           const char *topicFilter,
                           iotx_mqtt_qos_t qos,
                           iotx_mqtt_event_handle_func_fpt topic_handle_func,
                           void *pcontext);
int wrapper_mqtt_subscribe_sync(void *client,
                                const char *topic_filter,
                                iotx_mqtt_qos_t qos,
                                iotx_mqtt_event_handle_func_fpt topic_handle_func,
                                void *pcontext,
                                int timeout_ms);
int wrapper_mqtt_unsubscribe(void *client, const char *topicFilter);
int wrapper_mqtt_publish(void *client, const char *topicName, iotx_mqtt_topic_info_pt topic_msg);
int wrapper_mqtt_release(void **pclient);


