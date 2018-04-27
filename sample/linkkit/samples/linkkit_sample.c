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
#if !(WIN32)
#include <unistd.h>
#endif

#include "linkkit_export.h"

#include "iot_import.h"

/*
 * example for product "鐏�-Demo"
 */


#ifdef ON_DAILY
#define DM_PRODUCT_KEY_1        "a1nmfrdo3MI"
#define DM_DEVICE_NAME_1        "light_demo_for_ilop_device_test"
#define DM_DEVICE_SECRET_1      "kobN5zg08IwlgbqSUeaxo0vbEsOiEI7b"
#else
#define DM_PRODUCT_KEY_1        "a1grYGVCPWl"
#define DM_DEVICE_NAME_1        "0402_09"
#define DM_DEVICE_SECRET_1      "DocxiD5wS6x9HNZrbC2CAoi9UnYP74Xt"
#endif
#define DM_DEVICE_ID_1          "IoTxHttpTestDev_001"


#define LINKKIT_PRINTF(...)  \
    do {                                                     \
        printf("\e[0;32m%s@line%d\t:", __FUNCTION__, __LINE__);  \
        printf(__VA_ARGS__);                                 \
        printf("\e[0m");                                   \
    } while (0)

/* identifier of property/service/event, users should modify this macros according to your own product TSL. */
#define EVENT_PROPERTY_POST_IDENTIFIER         "post"
#define EVENT_ERROR_IDENTIFIER                 "Error"
#define EVENT_ERROR_OUTPUT_INFO_IDENTIFIER     "ErrorCode"
#define EVENT_CUSTOM_IDENTIFIER                "Custom"

/* specify ota buffer size for ota service, ota service will use this buffer for bin download. */
#define OTA_BUFFER_SIZE                  (512+1)
/* PLEASE set RIGHT tsl string according to your product. */
#if !(WIN32)
const char TSL_STRING[] = "{\"schema\":\"http://aliyun/iot/thing/desc/schema\",\"profile\":{\"productKey\":\"a1saiXev7Q9\",\"deviceName\":\"q0LP4t1AWGnbp3uwF6Md\"},\"services\":[{\"outputData\":[],\"identifier\":\"set\",\"inputData\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\"},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\"},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\"},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\"},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\"},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\"},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\"},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\"},{\"identifier\":\"PropertyTime\",\"dataType\":{\"specs\":{},\"type\":\"date\"},\"name\":\"时间\"},{\"identifier\":\"Propertypoint\",\"dataType\":{\"specs\":{\"min\":\"-100\",\"max\":\"100\"},\"type\":\"double\"},\"name\":\"浮点型\"},{\"identifier\":\"PropertyCharacter\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"字符串\"}],\"method\":\"thing.service.property.set\",\"name\":\"set\",\"required\":true,\"callType\":\"sync\",\"desc\":\"属性设置\"},{\"outputData\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\"},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\"},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\"},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\"},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\"},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\"},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\"},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\"},{\"identifier\":\"PropertyTime\",\"dataType\":{\"specs\":{},\"type\":\"date\"},\"name\":\"时间\"},{\"identifier\":\"Propertypoint\",\"dataType\":{\"specs\":{\"min\":\"-100\",\"max\":\"100\"},\"type\":\"double\"},\"name\":\"浮点型\"},{\"identifier\":\"PropertyCharacter\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"字符串\"}],\"identifier\":\"get\",\"inputData\":[\"LightSwitch\",\"RGBColor\",\"NightLightSwitch\",\"WorkMode\",\"ColorTemperature\",\"Brightness\",\"HSLColor\",\"HSVColor\",\"PropertyTime\",\"Propertypoint\",\"PropertyCharacter\"],\"method\":\"thing.service.property.get\",\"name\":\"get\",\"required\":true,\"callType\":\"sync\",\"desc\":\"属性获取\"},{\"outputData\":[{\"identifier\":\"Contrastratio\",\"dataType\":{\"specs\":{\"min\":\"0\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"对比度\"}],\"identifier\":\"Custom\",\"inputData\":[{\"identifier\":\"transparency\",\"dataType\":{\"specs\":{\"min\":\"-100\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"透明度\"}],\"method\":\"thing.service.Custom\",\"name\":\"自定义服务\",\"required\":false,\"callType\":\"async\"}],\"properties\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\",\"accessMode\":\"rw\",\"required\":true},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"        },\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"PropertyTime\",\"dataType\":{\"specs\":{},\"type\":\"date\"},\"name\":\"时间\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"Propertypoint\",\"dataType\":{\"specs\":{\"min\":\"-100\",\"max\":\"100\"},\"type\":\"double\"},\"name\":\"浮点型\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"PropertyCharacter\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"字符串\",\"accessMode\":\"rw\",\"required\":false}],\"events\":[{\"outputData\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\"},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\"},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\"},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\"},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\"},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\"},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\"},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\"},{\"identifier\":\"PropertyTime\",\"dataType\":{\"specs\":{},\"type\":\"date\"},\"name\":\"时间\"},{\"identifier\":\"Propertypoint\",\"dataType\":{\"specs\":{\"min\":\"-100\",\"max\":\"100\"},\"type\":\"double\"},\"name\":\"浮点型\"},{\"identifier\":\"PropertyCharacter\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"字符串\"}],\"identifier\":\"post\",\"method\":\"thing.event.property.post\",\"name\":\"post\",\"type\":\"info\",\"required\":true,\"desc\":\"属性上报\"},{\"outputData\":[{\"identifier\":\"ErrorCode\",\"dataType\":{\"specs\":{\"0\":\"恢复正常\"},\"type\":\"enum\"},\"name\":\"故障代码\"}],\"identifier\":\"Error\",\"method\":\"thing.event.Error.post\",\"name\":\"故障上报\",\"type\":\"info\",\"required\":true}]}";
#else
const char TSL_STRING[] = "add tsl in there";
#endif

/* user sample context struct. */
typedef struct _sample_context {
    void* thing;

    int cloud_connected;
    int thing_enabled;

    int service_custom_input_transparency;
    int service_custom_output_contrastratio;
#ifdef SERVICE_OTA_ENABLED
    char ota_buffer[OTA_BUFFER_SIZE];
#endif /* SERVICE_OTA_ENABLED */
} sample_context_t;

sample_context_t g_sample_context;

#ifdef SERVICE_OTA_ENABLED
#ifndef SERVICE_COTA_ENABLED
/* callback function for fota service. */
static void fota_callback(service_fota_callback_type_t callback_type, const char* version)
{
    sample_context_t* sample;

    assert(callback_type < service_fota_callback_type_number);

    sample = &g_sample_context;

    /* temporarily disable thing when ota service invoked */
    sample->thing_enabled = 0;

    linkkit_invoke_fota_service(sample->ota_buffer, OTA_BUFFER_SIZE);

    sample->thing_enabled = 1;

    /* reboot the device... */
}

#else /* SERVICE_COTA_ENABLED */
static void cota_callback(service_cota_callback_type_t callback_type,
						const char* configid,
						uint32_t  configsize,
						const char* gettype,
						const char* sign,
						const char* signmethod,
						const char* cota_url)
{
    sample_context_t* sample;

    assert(callback_type < service_cota_callback_type_number);

    sample = &g_sample_context;

    /* temporarily disable thing when ota service invoked */
    sample->thing_enabled = 0;

    linkkit_invoke_cota_service(sample->ota_buffer, OTA_BUFFER_SIZE);

    sample->thing_enabled = 1;

    /* update config... */
}
#endif /* SERVICE_COTA_ENABLED */
#endif /* SERVICE_OTA_ENABLED */

static int on_connect(void* ctx)
{
    sample_context_t* sample = ctx;

    sample->cloud_connected = 1;

    LINKKIT_PRINTF("cloud is connected\n");

    return 0;
}

static int on_disconnect(void* ctx)
{
    sample_context_t* sample = ctx;

    sample->cloud_connected = 0;

    LINKKIT_PRINTF("cloud is disconnect\n");

    return 0;
}

static int raw_data_arrived(void* thing_id, void* data, int len, void* ctx)
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

static int thing_create(void* thing_id, void* ctx)
{
    sample_context_t* sample = ctx;

    LINKKIT_PRINTF("new thing@%p created.\n", thing_id);

    sample->thing = thing_id;

    return 0;
}

static int thing_enable(void* thing_id, void* ctx)
{
    sample_context_t* sample = ctx;

    sample->thing_enabled = 1;

    return 0;
}

static int thing_disable(void* thing, void* ctx)
{
    sample_context_t* sample = ctx;

    sample->thing_enabled = 0;

    return 0;
}
#ifdef RRPC_ENABLED
static int handle_service_custom(sample_context_t* sample, void* thing, char* service_identifier, int request_id, int rrpc)
#else
static int handle_service_custom(sample_context_t* sample, void* thing, char* service_identifier, int request_id)
#endif /* RRPC_ENABLED */
{
    char identifier[128] = {0};

    /*
     * get iutput value.
     */
    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "transparency");
    linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &sample->service_custom_input_transparency, NULL);

    /*
     * set output value according to user's process result.
     */

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "Contrastratio");

    sample->service_custom_output_contrastratio = sample->service_custom_input_transparency >= 0 ? sample->service_custom_input_transparency : sample->service_custom_input_transparency * -1;

    linkkit_set_value(linkkit_method_set_service_output_value, thing, identifier, &sample->service_custom_output_contrastratio, NULL);
#ifdef RRPC_ENABLED
    linkkit_answer_service(thing, service_identifier, request_id, 200, rrpc);
#else
    linkkit_answer_service(thing, service_identifier, request_id, 200);
#endif /* RRPC_ENABLED */

    return 0;
}
#ifdef RRPC_ENABLED
static int thing_call_service(void* thing_id, char* service, int request_id, int rrpc, void* ctx)
#else
static int thing_call_service(void* thing_id, char* service, int request_id, void* ctx)
#endif /* RRPC_ENABLED */
{
    sample_context_t* sample = ctx;

    LINKKIT_PRINTF("service(%s) requested, id: thing@%p, request id:%d\n",
                   service ? service : "NULL", thing_id, request_id);

    if (strcmp(service, "Custom") == 0) {
#ifdef RRPC_ENABLED
        handle_service_custom(sample, thing_id, service, request_id, rrpc);
#else
        handle_service_custom(sample, thing_id, service, request_id);
#endif /* RRPC_ENABLED */
    }

    return 0;
}

static int thing_prop_changed(void* thing_id, char* property, void* ctx)
{
    char* value_str = NULL;
    char property_buf[64] = {0};

    /* get new property value */
    if (strstr(property, "HSVColor") != 0) {
        int hue, saturation, value;

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Hue");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &hue, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Saturation");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &saturation, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Value");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &value, &value_str);

        LINKKIT_PRINTF("property(%s), Hue:%d, Saturation:%d, Value:%d\n", property, hue, saturation, value);

        /* XXX: do user's process logical here. */
    } else if (strstr(property, "HSLColor") != 0) {
        int hue, saturation, lightness;

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Hue");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &hue, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Saturation");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &saturation, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Lightness");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &lightness, &value_str);

        LINKKIT_PRINTF("property(%s), Hue:%d, Saturation:%d, Lightness:%d\n", property, hue, saturation, lightness);
        /* XXX: do user's process logical here. */
    }  else if (strstr(property, "RGBColor") != 0) {
        int red, green, blue;

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Red");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &red, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Green");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &green, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "Blue");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &blue, &value_str);

        LINKKIT_PRINTF("property(%s), Red:%d, Green:%d, Blue:%d\n", property, red, green, blue);
        /* XXX: do user's process logical here. */
    } else if (strstr(property, "structProperty") != 0) {
        int childINTName = 0;
        int childFLOATName = 0;
        int childDATEName = 0;
        int childTEXTName = 0;
        int childBOOLName = 0;
        int childENUMName = 0;
        int childDOUBLEName = 0;

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "structchildINTd1e197e");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &childINTName, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "structchildFLOATcdef1f2");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &childFLOATName, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "structchildDATE228857b");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &childDATEName, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "structchildTEXT5ad129f");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &childTEXTName, &value_str);
        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "structchildBOOLea5c7ba");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &childBOOLName, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "structchildENUMf600fe3");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &childENUMName, &value_str);

        snprintf(property_buf, sizeof(property_buf), "%s.%s", property, "structchildDOUBLEff6bba6");
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property_buf, &childDOUBLEName, &value_str);

        LINKKIT_PRINTF("property(%s), childINTName:%d, childFLOATName:%d, childDATEName:%d, childTEXTName:%d, childBOOLName:%d, childENUMName:%d, childDOUBLEName:%d\n",\
         property, childINTName, childFLOATName, childDATEName, childTEXTName, childBOOLName, childENUMName, childDOUBLEName);

    } else {
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property, NULL, &value_str);

        LINKKIT_PRINTF("#### property(%s) new value set: %s ####\n", property ? property : "NULL", value_str ? value_str : "NULL");
    }

    /* do user's process logical here. */
    linkkit_post_property(thing_id, property);
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
};

static unsigned long long uptime_sec(void)
{
    static unsigned long long start_time = 0;

    if (start_time == 0) {
        start_time = HAL_UptimeMs();
    }

    return (HAL_UptimeMs() - start_time) / 1000;
}
#if 1
static int post_all_prop(sample_context_t* sample)
{
    return linkkit_post_property(sample->thing, NULL);
}
#endif
static int post_event_error(sample_context_t* sample)
{
    char event_output_identifier[64];
    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s", EVENT_ERROR_IDENTIFIER, EVENT_ERROR_OUTPUT_INFO_IDENTIFIER);

    int errorCode = 0;
    linkkit_set_value(linkkit_method_set_event_output_value,
                      sample->thing,
                      event_output_identifier,
                      &errorCode, NULL);

    return linkkit_trigger_event(sample->thing, EVENT_ERROR_IDENTIFIER);
}

#if 0
static int post_event_fault_alert(sample_context_t* sample)
{
    char event_output_identifier[64];
    snprintf(event_output_identifier, sizeof(event_output_identifier), "%s.%s", EVENT_ERROR_IDENTIFIER, EVENT_ERROR_OUTPUT_INFO_IDENTIFIER);

    int errorCode = 0;
    linkkit_set_value(linkkit_method_set_event_output_value,
                      sample->thing,
                      event_output_identifier,
                      &errorCode, NULL);

    return linkkit_trigger_event(sample->thing, EVENT_ERROR_IDENTIFIER);
}

static int upload_raw_data(sample_context_t* sample)
{
    char raw_data[128] = {0};

    snprintf(raw_data, sizeof(raw_data), "test up raw data %lld", HAL_UptimeMs());

    return linkkit_invoke_raw_service(sample->thing, 1, raw_data, strlen(raw_data));
}
#endif

static int is_active(sample_context_t* sample)
{
    return sample->cloud_connected && sample->thing_enabled;
}


#ifdef MQTT_ID2_AUTH
#include "tfs.h"
#endif

int main(int argc, char* argv[])
{
    sample_context_t* sample_ctx = &g_sample_context;
    int execution_time = 0;
    int get_tsl_from_cloud = 0;
    int exit = 0;

    unsigned long long now = 0;
    unsigned long long prev_sec = 0;

    HAL_SetProductKey(DM_PRODUCT_KEY_1);    
    
#ifdef MQTT_ID2_AUTH
    char __device_id2[TFS_ID2_LEN + 1];
    HAL_GetID2(__device_id2);
    HAL_SetDeviceName(__device_id2);
#else
    HAL_SetDeviceName(DM_DEVICE_NAME_1);
#endif

    HAL_SetDeviceSecret(DM_DEVICE_SECRET_1);

#if !(WIN32)
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
#endif

    execution_time = execution_time < 1 ? 1 : execution_time;
    LINKKIT_PRINTF("sample execution time: %d minutes\n", execution_time);
    LINKKIT_PRINTF("%s tsl from cloud\n", get_tsl_from_cloud == 0 ? "Not get" : "get");

    memset(sample_ctx, 0, sizeof(sample_context_t));
    sample_ctx->thing_enabled = 1;

    linkkit_start(8, get_tsl_from_cloud, linkkit_loglevel_debug, &alinkops, linkkit_cloud_domain_sh, sample_ctx);
    if (!get_tsl_from_cloud) {
        linkkit_set_tsl(TSL_STRING, strlen(TSL_STRING));
    }
#ifdef SERVICE_OTA_ENABLED
#ifndef SERVICE_COTA_ENABLED
    LINKKIT_PRINTF("FOTA start\n");
    linkkit_fota_init(fota_callback);
#else /* SERVICE_COTA_ENABLED */
    LINKKIT_PRINTF("COTA_start\n");
    linkkit_cota_init(cota_callback);
    linkkit_invoke_cota_get_config("product","file","",NULL);
#endif /* SERVICE_COTA_ENABLED */
#endif /* SERVICE_OTA_ENABLED */
    while (1) {
        linkkit_dispatch();

        now = uptime_sec();
        if (prev_sec == now) {
#ifdef CMP_SUPPORT_MULTI_THREAD
            HAL_SleepMs(100);
#else
            linkkit_yield(100);
#endif
            continue;
        }

        /* about 30 seconds, assume trigger post property event about every 30s. */
        if (now % 30 == 0 && is_active(sample_ctx)) {
            post_all_prop(sample_ctx);
        }
#if 0

        /* about 31 seconds, assume invoke raw up service about every 31s. */
        if (now % 31 == 0 && is_active(sample_ctx)) {
            upload_raw_data(sample_ctx);
        }

        /* about 60 seconds, assume trigger event about every 60s. */
        if (now % 60 == 0 && is_active(sample_ctx)) {
            post_event_fault_alert(sample_ctx);
        }
#endif
        /* about 60 seconds, assume trigger event about every 60s. */
        if (now % 60 == 0 && is_active(sample_ctx)) {
            post_event_error(sample_ctx);
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
