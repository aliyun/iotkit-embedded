/** USER NOTIFICATION
 *  this sample code is only used for evaluation or test of the iLop project.
 *  Users should modify this sample code freely according to the product/device TSL, like
 *  property/event/service identifiers, and the item value type(type, length, etc...).
 *  Create user's own execution logic for specific products.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>

#include "linkkit_export.h"

#include "iot_import.h"

/*
 * example for share business
 */


#define LINKKIT_PRINTF(...)  \
    do {                                                     \
        printf("\e[0;32m%s@line%d:\t", __FUNCTION__, __LINE__);  \
        printf(__VA_ARGS__);                                 \
        printf("\e[0m");                                   \
    } while (0)

/* identifier of property/service/event, users should modify this macros according to your own product TSL. */
#define EVENT_ERROR_IDENTIFIER                 "Error"
#define EVENT_ERROR_OUTPUT_INFO_IDENTIFIER     "ErrorCode"
#define EVENT_SHORTAGE_IDENTIFIER              "Shortage"


#define SERVICE_DELIVERCOMMODITY_IDENTIFIER    "deliverCommodity"
#define SERVICE_ADDINVENTORY_IDENTIFIER        "addInventory"
#define SERVICE_GETINVENTORY_IDENTIFIER        "getInventory"
#define SERVICE_LISTINVENTORY_IDENTIFIER       "listInventory"

#define PROPERTY_SWITCH_IDENTIFIER             "switch"

/* specify ota buffer size for ota service, ota service will use this buffer for bin download. */
#define OTA_BUFFER_SIZE                  (512+1)
/* PLEASE set RIGHT tsl string according to your product. */

const char TSL_STRING[] = "{\"schema\":\"https://iot-tsl.oss-cn-shanghai.aliyuncs.com/schema.json\",\"profile\":{\"productKey\":\"a1fG355fLM5\",\"deviceName\":\"led_light_test_dev1\"},\"link\":\"/sys/a1fG355fLM5/led_light_test_dev1/thing/\",\"services\":[{\"outputData\":[],\"identifier\":\"set\",\"inputData\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\"},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\"},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\"},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\"},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\"},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\"},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\"},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\"}],\"method\":\"thing.service.property.set\",\"name\":\"set\",\"required\":true,\"callType\":\"sync\",\"desc\":\"属性设置\"},{\"outputData\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\"},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\"},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\"},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\"},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\"},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\"},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\"},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\"}],\"identifier\":\"get\",\"inputData\":[\"LightSwitch\",\"RGBColor\",\"HSVColor\",\"NightLightSwitch\",\"WorkMode\",\"ColorTemperature\",\"HSLColor\",\"Brightness\"],\"method\":\"thing.service.property.get\",\"name\":\"get\",\"required\":true,\"callType\":\"sync\",\"desc\":\"属性获取\"}],\"properties\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\",\"accessMode\":\"rw\",\"required\":true},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\",\"accessMode\":\"rw\",\"required\":false}],\"events\":[{\"outputData\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\"},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\"},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\"},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\"},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\"},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\"},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\"},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\"}],\"identifier\":\"post\",\"method\":\"thing.event.property.post\",\"name\":\"post\",\"type\":\"info\",\"required\":true,\"desc\":\"属性上报\"},{\"outputData\":[{\"identifier\":\"ErrorCode\",\"dataType\":{\"specs\":{\"0\":\"正常\"},\"type\":\"enum\"},\"name\":\"故障代码\"}],\"identifier\":\"Error\",\"method\":\"thing.event.Error.post\",\"name\":\"故障上报\",\"type\":\"info\",\"required\":true}]}";
/* user sample context struct. */

typedef struct _shared_business_device {

    int   inventory[5][5];

    int   prop_switch;

    char* service_deliverCommodity_result;
    int   service_deliverCommodity_code;

    char* service_addInventory_result;
    int   service_addInventory_code;

    int   service_getInventory_r;
    int   service_getInventory_c;

} shared_business_device_t;

typedef struct _sample_context {
    const void* thing;

    int cloud_connected;
    int thing_enabled;

    int service_custom_input_transparency;
    int service_custom_output_contrastratio;
#ifdef SERVICE_OTA_ENABLED
    char ota_buffer[OTA_BUFFER_SIZE];
#endif /* SERVICE_OTA_ENABLED */

    shared_business_device_t shared_business_device;
} sample_context_t;

sample_context_t g_sample_context;

#ifdef SERVICE_OTA_ENABLED
/* callback function for fota service. */
static void fota_callback(service_fota_callback_type_t callback_type, const char* version)
{
    sample_context_t* sample_ctx;

    assert(callback_type < service_fota_callback_type_number);

    sample_ctx = &g_sample_context;

    /* temporarily disable thing when ota service invoked */
    sample_ctx->thing_enabled = 0;

    linkkit_invoke_ota_service(sample_ctx->ota_buffer, OTA_BUFFER_SIZE);

    sample_ctx->thing_enabled = 1;

    /* reboot the device... */
}
#endif /* SERVICE_OTA_ENABLED */
#ifdef LOCAL_CONN_ENABLE
static int on_connect(void* ctx, int cloud)
#else
static int on_connect(void* ctx)
#endif
{
    sample_context_t* sample_ctx = ctx;

    sample_ctx->cloud_connected = 1;
#ifdef LOCAL_CONN_ENABLE
    LINKKIT_PRINTF("%s is connected\n", cloud ? "cloud" : "local");
#else
    LINKKIT_PRINTF("%s is connected\n", "cloud");
#endif

    return 0;
}

#ifdef LOCAL_CONN_ENABLE
static int on_disconnect(void* ctx, int cloud)
#else
static int on_disconnect(void* ctx)
#endif
{
    sample_context_t* sample_ctx = ctx;

    sample_ctx->cloud_connected = 0;

#ifdef LOCAL_CONN_ENABLE
    LINKKIT_PRINTF("%s is disconnect\n", cloud ? "cloud" : "local");
#else
    LINKKIT_PRINTF("%s is disconnect\n", "cloud");
#endif
    return 0;
}

static int raw_data_arrived(const void* thing_id, const void* data, int len, void* ctx)
{
    char raw_data[128] = {0};

    LINKKIT_PRINTF("raw data arrived,len:%d\n", len);

    /* do user's raw data process logical here. */

    /* ............................... */

    /* user's raw data process logical complete */

    snprintf(raw_data, sizeof(raw_data), "test down raw reply data %lld", (long long)HAL_UptimeMs());

    linkkit_invoke_raw_service(thing_id, 0, raw_data, strlen(raw_data));

    return 0;
}

static int thing_create(const void* thing_id, void* ctx)
{
    sample_context_t* sample_ctx = ctx;

    LINKKIT_PRINTF("new thing@%p created.\n", thing_id);

    sample_ctx->thing = thing_id;

    return 0;
}

static int thing_enable(const void* thing_id, void* ctx)
{
    sample_context_t* sample_ctx = ctx;

    sample_ctx->thing_enabled = 1;

    return 0;
}

static int thing_disable(const void* thing, void* ctx)
{
    sample_context_t* sample_ctx = ctx;

    sample_ctx->thing_enabled = 0;

    return 0;
}
#ifdef RRPC_ENABLED
static int handle_service_deliverCommodity(sample_context_t* _sample_ctx, const void* thing, const char* service_identifier, int request_id, int rrpc)
#else
static int handle_service_deliverCommodity(sample_context_t* _sample_ctx, const void* thing, const char* service_identifier, int request_id)
#endif /* RRPC_ENABLED */
{
    sample_context_t* sample_ctx = _sample_ctx;
    shared_business_device_t* shared_business_device = &sample_ctx->shared_business_device;
    char identifier[128] = {0};
    int x, y;

    /*
     * get iutput value.
     */
    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "x");
    linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &x, NULL);

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "y");
    linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &y, NULL);

    /*
     * set output value according to user's process result.
     */

    shared_business_device->inventory[x][y] -= 1;
    if (shared_business_device->inventory[x][y] < 0) {
        shared_business_device->service_deliverCommodity_result = "deliver comm fail";
        shared_business_device->service_deliverCommodity_code = 4;
        shared_business_device->inventory[x][y] = 0;
    } else {
        shared_business_device->service_deliverCommodity_result = "deliver comm success";
        shared_business_device->service_deliverCommodity_code = 0;
    }

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "result");
    linkkit_set_value(linkkit_method_set_service_output_value, thing, identifier, shared_business_device->service_deliverCommodity_result, NULL);

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "code");
    linkkit_set_value(linkkit_method_set_service_output_value, thing, identifier, &shared_business_device->service_deliverCommodity_code, NULL);
#ifdef RRPC_ENABLED
    linkkit_answer_service(thing, service_identifier, request_id, 200, rrpc);
#else
    linkkit_answer_service(thing, service_identifier, request_id, 200);
#endif /* RRPC_ENABLED */

    return 0;
}

#ifdef RRPC_ENABLED
static int handle_service_addInventory(sample_context_t* _sample_ctx, const void* thing, const char* service_identifier, int request_id, int rrpc)
#else
static int handle_service_addInventory(sample_context_t* _sample_ctx, const void* thing, const char* service_identifier, int request_id)
#endif /* RRPC_ENABLED */
{
    sample_context_t* sample_ctx = _sample_ctx;
    shared_business_device_t* shared_business_device = &sample_ctx->shared_business_device;
    char identifier[128] = {0};
    int x, y, c;

    /*
     * get iutput value.
     */
    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "x");
    linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &x, NULL);

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "y");
    linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &y, NULL);

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "c");
    linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &c, NULL);

    /*
     * set output value according to user's process result.
     */
    assert(x < 5 && y < 5);
    shared_business_device->inventory[x][y] += c;

    shared_business_device->service_addInventory_result = "add inventory success";
    shared_business_device->service_addInventory_code = 0;

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "result");
    linkkit_set_value(linkkit_method_set_service_output_value, thing, identifier, shared_business_device->service_addInventory_result, NULL);

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "code");
    linkkit_set_value(linkkit_method_set_service_output_value, thing, identifier, &shared_business_device->service_addInventory_code, NULL);
#ifdef RRPC_ENABLED
    linkkit_answer_service(thing, service_identifier, request_id, 200, rrpc);
#else
    linkkit_answer_service(thing, service_identifier, request_id, 200);
#endif /* RRPC_ENABLED */

    return 0;
}

#ifdef RRPC_ENABLED
static int handle_service_getInventory(sample_context_t* _sample_ctx, const void* thing, const char* service_identifier, int request_id, int rrpc)
#else
static int handle_service_getInventory(sample_context_t* _sample_ctx, const void* thing, const char* service_identifier, int request_id)
#endif /* RRPC_ENABLED */
{
    sample_context_t* sample_ctx = _sample_ctx;
    shared_business_device_t* shared_business_device = &sample_ctx->shared_business_device;
    char identifier[128] = {0};
    int inventory;
    int x, y;

    /*
     * get iutput value.
     */
    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "x");
    linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &x, NULL);

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "y");
    linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &y, NULL);

    /*
     * set output value according to user's process result.
     */
    assert(x < 5 && y < 5);
    inventory = shared_business_device->inventory[x][y];

    shared_business_device->service_getInventory_c = inventory;
    shared_business_device->service_getInventory_r = inventory ? 1 : 0;

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "x");
    linkkit_set_value(linkkit_method_set_service_output_value, thing, identifier, &x, NULL);

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "y");
    linkkit_set_value(linkkit_method_set_service_output_value, thing, identifier, &y, NULL);

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "r");
    linkkit_set_value(linkkit_method_set_service_output_value, thing, identifier, &shared_business_device->service_getInventory_r, NULL);

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "c");
    linkkit_set_value(linkkit_method_set_service_output_value, thing, identifier, &shared_business_device->service_getInventory_c, NULL);
#ifdef RRPC_ENABLED
    linkkit_answer_service(thing, service_identifier, request_id, 200, rrpc);
#else
    linkkit_answer_service(thing, service_identifier, request_id, 200);
#endif /* RRPC_ENABLED */

    return 0;
}

#ifdef RRPC_ENABLED
static int handle_service_listInventory(sample_context_t* _sample_ctx, const void* thing, const char* service_identifier, int request_id, int rrpc)
#else
static int handle_service_listInventory(sample_context_t* _sample_ctx, const void* thing, const char* service_identifier, int request_id)
#endif /* RRPC_ENABLED */
{
    sample_context_t* sample_ctx = _sample_ctx;
    shared_business_device_t* shared_business_device = &sample_ctx->shared_business_device;
    char identifier[128] = {0};
    int x, y;
    int inventory;
    char output[32] = {0};

    /*
     * set output value according to user's process result.
     */

    for (x = 0; x < 5; ++x) {
        for (y = 0; y < 5; ++y) {
            inventory = shared_business_device->inventory[x][y];
            snprintf(identifier, sizeof(identifier), "%s.%s[%d]", service_identifier, "inventoryDetail", x*5+y);
            snprintf(output, sizeof(output), "x:%d,y:%d,c:%d", x, y, inventory);
            linkkit_set_value(linkkit_method_set_service_output_value, thing, identifier, output, NULL);
        }
    }

#ifdef RRPC_ENABLED
    linkkit_answer_service(thing, service_identifier, request_id, 200, rrpc);
#else
    linkkit_answer_service(thing, service_identifier, request_id, 200);
#endif /* RRPC_ENABLED */

    return 0;
}

#ifdef RRPC_ENABLED
static int thing_call_service(const void* thing_id, const char* service, int request_id, int rrpc, void* ctx)
#else
static int thing_call_service(const void* thing_id, const char* service, int request_id, void* ctx)
#endif /* RRPC_ENABLED */
{
    sample_context_t* sample_ctx = ctx;

    LINKKIT_PRINTF("service(%s) requested, id: thing@%p, request id:%d\n",
                   service, thing_id, request_id);

    if (strcmp(service, SERVICE_DELIVERCOMMODITY_IDENTIFIER) == 0) {
#ifdef RRPC_ENABLED
        handle_service_deliverCommodity(sample_ctx, thing_id, service, request_id, rrpc);
#else
        handle_service_deliverCommodity(sample_ctx, thing_id, service, request_id);
#endif /* RRPC_ENABLED */
    } else if (strcmp(service, SERVICE_ADDINVENTORY_IDENTIFIER) == 0) {
#ifdef RRPC_ENABLED
        handle_service_addInventory(sample_ctx, thing_id, service, request_id, rrpc);
#else
        handle_service_addInventory(sample_ctx, thing_id, service, request_id);
#endif /* RRPC_ENABLED */
    } else if (strcmp(service, SERVICE_GETINVENTORY_IDENTIFIER) == 0) {
#ifdef RRPC_ENABLED
        handle_service_getInventory(sample_ctx, thing_id, service, request_id, rrpc);
#else
        handle_service_getInventory(sample_ctx, thing_id, service, request_id);
#endif /* RRPC_ENABLED */
    } else if (strcmp(service, SERVICE_LISTINVENTORY_IDENTIFIER) == 0) {
#ifdef RRPC_ENABLED
        handle_service_listInventory(sample_ctx, thing_id, service, request_id, rrpc);
#else
        handle_service_listInventory(sample_ctx, thing_id, service, request_id);
#endif /* RRPC_ENABLED */
    }

    return 0;
}

void post_property_cb(const void* thing_id, int respons_id, int code, const char* response_message, void* ctx)
{
    LINKKIT_PRINTF("thing@%p: response arrived:\nid:%d\tcode:%d\tmessage:%s\n", thing_id, respons_id, code, response_message == NULL ? "NULL" : response_message);
}

static int thing_prop_changed(const void* thing_id, const char* property, void* ctx)
{
    char* value_str = NULL;
    char property_buf[64] = {0};
    sample_context_t* sample_ctx = ctx;
    shared_business_device_t* shared_business_device = &sample_ctx->shared_business_device;

    /* get new property value */
    if (strstr(property, PROPERTY_SWITCH_IDENTIFIER) != 0) {
        snprintf(property_buf, sizeof(property_buf), "%s", property);
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &shared_business_device->prop_switch, &value_str);

        LINKKIT_PRINTF("property(%s), Value:%s\n", property, shared_business_device->prop_switch ? "on" : "off");
        /* XXX: do user's process logical here. */
    }  else {
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property, NULL, &value_str);

        LINKKIT_PRINTF("#### property(%s) new value set: %s ####\n", property, value_str);
    }

    /* do user's process logical here. */
    linkkit_post_property(thing_id, property, post_property_cb);

    return 0;
}

int masterdev_linkit_data_arrived(const void* thing_id, const void* data, int len, void* ctx)
{
    LINKKIT_PRINTF("#### masterdev_linkkit_data(%d byte): %s####\n", len, (char*)data);
    return 0;
}

static linkkit_ops_t alinkops = {
    .on_connect         = on_connect,
    .on_disconnect      = on_disconnect,
    .raw_data_arrived   = raw_data_arrived,
    .thing_create       = thing_create,
    .thing_enable       = thing_enable,
    .thing_disable      = thing_disable,
    .thing_call_service = thing_call_service,
    .thing_prop_changed = thing_prop_changed,
    .linkit_data_arrived = masterdev_linkit_data_arrived,
};

static unsigned long long uptime_sec(void)
{
    static unsigned long long start_time = 0;

    if (start_time == 0) {
        start_time = HAL_UptimeMs();
    }

    return (HAL_UptimeMs() - start_time) / 1000;
}

static int is_active(sample_context_t* sample_ctx)
{
    return sample_ctx->cloud_connected && sample_ctx->thing_enabled;
}

int main(int argc, char* argv[])
{
    sample_context_t* sample_ctx = &g_sample_context;
    shared_business_device_t* shared_business_device = &sample_ctx->shared_business_device;
    int i, j, inventory;

    int execution_time = 0;
    int get_tsl_from_cloud = 0;
    int exit = 0;

    int id_send = 0;
    int ret;

    unsigned long long now = 0;
    unsigned long long prev_sec = 0;

    int opt;
    while ((opt = getopt(argc, argv, "t:g:h")) != -1) {
        switch (opt) {
        case 't':
            execution_time = atoi(optarg);
            break;
        case 'g':
            get_tsl_from_cloud = atoi(optarg);
            break;
        case 'h':
            LINKKIT_PRINTF("-t to specify sample execution time period(minutes); -g to specify if get tsl from cloud(0: not, !0: yes).\n");
            return 0;
            break;
        default:
            break;
        }
    }

    execution_time = execution_time < 1 ? 1 : execution_time;
    LINKKIT_PRINTF("sample execution time: %d minutes\n", execution_time);
    LINKKIT_PRINTF("%s tsl from cloud\n", get_tsl_from_cloud == 0 ? "Not get" : "get");

    memset(sample_ctx, 0, sizeof(sample_context_t));
    sample_ctx->thing_enabled = 1;

    linkkit_start(8, get_tsl_from_cloud, linkkit_loglevel_debug, &alinkops, linkkit_cloud_domain_shanghai, sample_ctx);
    if (!get_tsl_from_cloud) {
        linkkit_set_tsl(TSL_STRING, strlen(TSL_STRING));
    }

#ifdef SERVICE_OTA_ENABLED
    linkkit_ota_init(fota_callback);
#endif /* SERVICE_OTA_ENABLED */

    for (i = 0; i < 5; ++i) {
        for (j = 0; j < 5; ++j) {
            shared_business_device->inventory[i][j] = 10;
        }
    }

    shared_business_device->prop_switch = 1;

    while (1) {
#ifndef CM_SUPPORT_MULTI_THREAD
        linkkit_dispatch();
#endif
        now = uptime_sec();
        if (prev_sec == now) {
#ifdef CM_SUPPORT_MULTI_THREAD
            HAL_SleepMs(100);
#else
            linkkit_yield(50);
#endif /* CM_SUPPORT_MULTI_THREAD */
            continue;
        }

        /* about 60 seconds, assume trigger post property event about every 60s. */
        if (now % 600 == 0 && is_active(sample_ctx)) {
            char identifier[128];
            for (i = 0; i < 5; ++i) {
                for (j = 0; j < 5; ++j) {
                    inventory = shared_business_device->inventory[i][j];

                    snprintf(identifier, sizeof(identifier), "%s.%s", EVENT_SHORTAGE_IDENTIFIER, "x");
                    linkkit_set_value(linkkit_method_set_event_output_value, sample_ctx->thing, identifier, &i, NULL);

                    snprintf(identifier, sizeof(identifier), "%s.%s", EVENT_SHORTAGE_IDENTIFIER, "y");
                    linkkit_set_value(linkkit_method_set_event_output_value, sample_ctx->thing, identifier, &j, NULL);

                    snprintf(identifier, sizeof(identifier), "%s.%s", EVENT_SHORTAGE_IDENTIFIER, "inventory");
                    linkkit_set_value(linkkit_method_set_event_output_value, sample_ctx->thing, identifier, &inventory, NULL);

                    ret = linkkit_trigger_event(sample_ctx->thing, EVENT_SHORTAGE_IDENTIFIER, post_property_cb);
                    if (ret > 0) {
                        id_send = ret;
                        LINKKIT_PRINTF("send id:%d\n", id_send);
                    }
                }
            }
        }

        if (exit) break;

        /* after all, this is an sample, give a chance to return... */
        /* modify this value for this sample executaion time period */
        if (now > 60 * execution_time) exit = 1;

        prev_sec = now;
    }

    linkkit_end();

    return 0;
}
