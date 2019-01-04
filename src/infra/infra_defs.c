
#include "infra_config.h"
#include "infra_defs.h"

iotx_region_item_t g_infra_mqtt_domain[IOTX_MQTT_DOMAIN_NUMBER] = {
#if defined(ON_DAILY)
    {"10.125.3.189",    1883},   /* Shanghai */
#elif defined(ON_PRE)
    {"100.67.80.75",    80},   /* Shanghai */
#else
    {"iot-as-mqtt.cn-shanghai.aliyuncs.com",    1883},   /* Shanghai */
#endif
    {"iot-as-mqtt.ap-southeast-1.aliyuncs.com", 1883},   /* Singapore */
    {"iot-as-mqtt.ap-northeast-1.aliyuncs.com", 1883},   /* Japan */
    {"iot-as-mqtt.us-west-1.aliyuncs.com",      1883},   /* America */
    {"iot-as-mqtt.eu-central-1.aliyuncs.com",   1883}   /* Germany */
};

const char *g_infra_http_domain[IOTX_HTTP_DOMAIN_NUMBER] = {
    "iot-auth.cn-shanghai.aliyuncs.com",         /* Shanghai */
    "iot-auth.ap-southeast-1.aliyuncs.com",      /* Singapore */
    "iot-auth.ap-northeast-1.aliyuncs.com",      /* Japan */
    "iot-auth.us-west-1.aliyuncs.com",           /* America */
    "iot-auth.eu-central-1.aliyuncs.com"         /* Germany */
};

