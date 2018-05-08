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
#if !defined(_WIN32)
#include <unistd.h>
#endif

#include "linkkit_export.h"

#include "iot_import.h"

/*
 * example for product "灯-Demo"
 */
#if defined(SUPPORT_SINGAPORE_DOMAIN)
    #define IOTX_PRODUCT_KEY        "zSqW3X4e0lJ"
    #define IOTX_DEVICE_NAME        "azhan_sg"
    #define IOTX_DEVICE_SECRET      "qUMFGaNQGjoTdDkxKppfVzQwtpnMxRBx"
#elif defined(ON_DAILY)
    #define IOTX_PRODUCT_KEY        "a1nmfrdo3MI"
    #define IOTX_DEVICE_NAME        "light_demo_for_ilop_device_test"
    #define IOTX_DEVICE_SECRET      "kobN5zg08IwlgbqSUeaxo0vbEsOiEI7b"
#elif defined(ON_PRE)
    #define IOTX_PRODUCT_KEY        "a1u36e7PPs1"
    #define IOTX_DEVICE_NAME        "MxFRySXtCYbcR9vsYDyd"
    #define IOTX_DEVICE_SECRET      "5U0GSuYZHsfINMB3n9bxUT2Yo1fJyo9S"
#else
    #define IOTX_PRODUCT_KEY        "a1AzoSi5TMc"
    #define IOTX_DEVICE_NAME        "test_light_for_dm_cmp_2"
    #define IOTX_DEVICE_SECRET      "XjdICFKdWrm8pDughFg2cAImO6O0aDhE"
#endif
#define LINKKIT_PRINTF(...)  \
    do {                                                     \
        printf("\e[0;32m%s@line%d:\t", __FUNCTION__, __LINE__);  \
        printf(__VA_ARGS__);                                 \
        printf("\e[0m");                                   \
    } while (0)

/* identifier of property/service/event, users should modify this macros according to your own product TSL. */
#define EVENT_ERROR_IDENTIFIER                 "Error"
#define EVENT_ERROR_OUTPUT_INFO_IDENTIFIER     "ErrorCode"
#define EVENT_CUSTOM_IDENTIFIER                "Custom"

/* specify ota buffer size for ota service, ota service will use this buffer for bin download. */
#define OTA_BUFFER_SIZE                  (512+1)
/* PLEASE set RIGHT tsl string according to your product. */

//const char TSL_STRING[] = "{\"schema\":\"http://aliyun/iot/thing/desc/schema\",\"profile\":{\"productKey\":\"a1AzoSi5TMc\",\"deviceName\":\"test_light_for_dm_cmp_2\"},\"services\":[{\"outputData\":[],\"identifier\":\"set\",\"inputData\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\"},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\"},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\"},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\"},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\"},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\"},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\"},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\"},{\"identifier\":\"PropertyTime\",\"dataType\":{\"specs\":{},\"type\":\"date\"},\"name\":\"时间\"},{\"identifier\":\"Propertypoint\",\"dataType\":{\"specs\":{\"min\":\"-100\",\"max\":\"100\"},\"type\":\"double\"},\"name\":\"浮点型\"},{\"identifier\":\"PropertyCharacter\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"字符串\"}],\"method\":\"thing.service.property.set\",\"name\":\"set\",\"required\":true,\"callType\":\"sync\",\"desc\":\"属性设置\"},{\"outputData\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\"},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\"},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\"},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\"},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\"},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\"},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\"},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\"},{\"identifier\":\"PropertyTime\",\"dataType\":{\"specs\":{},\"type\":\"date\"},\"name\":\"时间\"},{\"identifier\":\"Propertypoint\",\"dataType\":{\"specs\":{\"min\":\"-100\",\"max\":\"100\"},\"type\":\"double\"},\"name\":\"浮点型\"},{\"identifier\":\"PropertyCharacter\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"字符串\"}],\"identifier\":\"get\",\"inputData\":[\"LightSwitch\",\"RGBColor\",\"NightLightSwitch\",\"WorkMode\",\"ColorTemperature\",\"Brightness\",\"HSLColor\",\"HSVColor\",\"PropertyTime\",\"Propertypoint\",\"PropertyCharacter\"],\"method\":\"thing.service.property.get\",\"name\":\"get\",\"required\":true,\"callType\":\"sync\",\"desc\":\"属性获取\"},{\"outputData\":[{\"identifier\":\"Contrastratio\",\"dataType\":{\"specs\":{\"min\":\"0\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"对比度\"}],\"identifier\":\"Custom\",\"inputData\":[{\"identifier\":\"transparency\",\"dataType\":{\"specs\":{\"min\":\"-100\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"透明度\"}],\"method\":\"thing.service.Custom\",\"name\":\"自定义服务\",\"required\":false,\"callType\":\"async\"}],\"properties\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\",\"accessMode\":\"rw\",\"required\":true},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"        },\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"PropertyTime\",\"dataType\":{\"specs\":{},\"type\":\"date\"},\"name\":\"时间\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"Propertypoint\",\"dataType\":{\"specs\":{\"min\":\"-100\",\"max\":\"100\"},\"type\":\"double\"},\"name\":\"浮点型\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"PropertyCharacter\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"字符串\",\"accessMode\":\"rw\",\"required\":false}],\"events\":[{\"outputData\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\"},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\"},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\"},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\"},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\"},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\"},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\"},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\"},{\"identifier\":\"PropertyTime\",\"dataType\":{\"specs\":{},\"type\":\"date\"},\"name\":\"时间\"},{\"identifier\":\"Propertypoint\",\"dataType\":{\"specs\":{\"min\":\"-100\",\"max\":\"100\"},\"type\":\"double\"},\"name\":\"浮点型\"},{\"identifier\":\"PropertyCharacter\",\"dataType\":{\"specs\":{\"length\":\"255\"},\"type\":\"text\"},\"name\":\"字符串\"}],\"identifier\":\"post\",\"method\":\"thing.event.property.post\",\"name\":\"post\",\"type\":\"info\",\"required\":true,\"desc\":\"属性上报\"},{\"outputData\":[{\"identifier\":\"ErrorCode\",\"dataType\":{\"specs\":{\"0\":\"恢复正常\"},\"type\":\"enum\"},\"name\":\"故障代码\"}],\"identifier\":\"Error\",\"method\":\"thing.event.Error.post\",\"name\":\"故障上报\",\"type\":\"info\",\"required\":true}]}";
static const char TSL_STRING[] = "{\"schema\":\"https://iot-tsl.oss-cn-shanghai.aliyuncs.com/schema.json\",\"profile\":{\"productKey\":\"a1fG355fLM5\",\"deviceName\":\"led_light_test_dev1\"},\"link\":\"/sys/a1fG355fLM5/led_light_test_dev1/thing/\",\"services\":[{\"outputData\":[],\"identifier\":\"set\",\"inputData\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\"},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\"},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\"},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\"},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\"},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\"},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\"},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\"}],\"method\":\"thing.service.property.set\",\"name\":\"set\",\"required\":true,\"callType\":\"sync\",\"desc\":\"属性设置\"},{\"outputData\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\"},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\"},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\"},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\"},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\"},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\"},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\"},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\"}],\"identifier\":\"get\",\"inputData\":[\"LightSwitch\",\"RGBColor\",\"HSVColor\",\"NightLightSwitch\",\"WorkMode\",\"ColorTemperature\",\"HSLColor\",\"Brightness\"],\"method\":\"thing.service.property.get\",\"name\":\"get\",\"required\":true,\"callType\":\"sync\",\"desc\":\"属性获取\"}],\"properties\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\",\"accessMode\":\"rw\",\"required\":true},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\",\"accessMode\":\"rw\",\"required\":false},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\",\"accessMode\":\"rw\",\"required\":false}],\"events\":[{\"outputData\":[{\"identifier\":\"LightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"主灯开关\"},{\"identifier\":\"RGBColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Red\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"红色\"},{\"identifier\":\"Green\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"绿色\"},{\"identifier\":\"Blue\",\"dataType\":{\"specs\":{\"min\":\"0\",\"unitName\":\"无\",\"max\":\"255\"},\"type\":\"int\"},\"name\":\"蓝色\"}],\"type\":\"struct\"},\"name\":\"RGB调色\"},{\"identifier\":\"HSVColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Value\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明度\"}],\"type\":\"struct\"},\"name\":\"HSV调色\"},{\"identifier\":\"NightLightSwitch\",\"dataType\":{\"specs\":{\"0\":\"关闭\",\"1\":\"开启\"},\"type\":\"bool\"},\"name\":\"夜灯开关\"},{\"identifier\":\"WorkMode\",\"dataType\":{\"specs\":{\"0\":\"手动\",\"1\":\"阅读\",\"2\":\"影院\",\"3\":\"夜灯\",\"4\":\"生活\",\"5\":\"柔和\"},\"type\":\"enum\"},\"name\":\"工作模式\"},{\"identifier\":\"ColorTemperature\",\"dataType\":{\"specs\":{\"unit\":\"K\",\"min\":\"2700\",\"unitName\":\"开尔文\",\"max\":\"6500\"},\"type\":\"int\"},\"name\":\"冷暖色温\"},{\"identifier\":\"HSLColor\",\"dataType\":{\"specs\":[{\"identifier\":\"Hue\",\"dataType\":{\"specs\":{\"unit\":\"°\",\"min\":\"0\",\"unitName\":\"度\",\"max\":\"360\"},\"type\":\"int\"},\"name\":\"色调\"},{\"identifier\":\"Saturation\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"饱和度\"},{\"identifier\":\"Lightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"亮度\"}],\"type\":\"struct\"},\"name\":\"HSL调色\"},{\"identifier\":\"Brightness\",\"dataType\":{\"specs\":{\"unit\":\"%\",\"min\":\"0\",\"unitName\":\"百分比\",\"max\":\"100\"},\"type\":\"int\"},\"name\":\"明暗度\"}],\"identifier\":\"post\",\"method\":\"thing.event.property.post\",\"name\":\"post\",\"type\":\"info\",\"required\":true,\"desc\":\"属性上报\"},{\"outputData\":[{\"identifier\":\"ErrorCode\",\"dataType\":{\"specs\":{\"0\":\"正常\"},\"type\":\"enum\"},\"name\":\"故障代码\"}],\"identifier\":\"Error\",\"method\":\"thing.event.Error.post\",\"name\":\"故障上报\",\"type\":\"info\",\"required\":true}]}";
/* user sample context struct. */
typedef struct _sample_context {
    const void*   thing;
    int           cloud_connected;
#ifdef LOCAL_CONN_ENABLE
    int           local_connected;
#endif
    int           thing_enabled;

    int           service_custom_input_transparency;
    int           service_custom_output_contrastratio;
#ifdef SERVICE_OTA_ENABLED
    char          ota_buffer[OTA_BUFFER_SIZE];
#endif /* SERVICE_OTA_ENABLED */
} sample_context_t;

sample_context_t g_sample_context;

void post_property_cb(const void* thing_id, int respons_id, int code, const char* response_message, void* ctx)
{
    LINKKIT_PRINTF("thing@%p: response arrived:\nid:%d\tcode:%d\tmessage:%s\n", thing_id, respons_id, code, response_message == NULL ? "NULL" : response_message);
}

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

#ifdef LOCAL_CONN_ENABLE
    if (cloud) {
        sample_ctx->cloud_connected = 1;
    } else {
        sample_ctx->local_connected = 1;
    }
    LINKKIT_PRINTF("%s is connected\n", cloud ? "cloud" : "local");
#else
    sample_ctx->cloud_connected = 1;
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

#ifdef LOCAL_CONN_ENABLE
    if (cloud) {
        sample_ctx->cloud_connected = 0;
    } else {
        sample_ctx->local_connected = 0;
    }
    LINKKIT_PRINTF("%s is disconnect\n", cloud ? "cloud" : "local");
#else
    sample_ctx->cloud_connected = 0;
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

static int thing_disable(const void* thing_id, void* ctx)
{
    sample_context_t* sample_ctx = ctx;

    sample_ctx->thing_enabled = 0;

    return 0;
}
#ifdef RRPC_ENABLED
static int handle_service_custom(sample_context_t* _sample_ctx, const void* thing, const char* service_identifier, int request_id, int rrpc)
#else
static int handle_service_custom(sample_context_t* _sample_ctx, const void* thing, const char* service_identifier, int request_id)
#endif /* RRPC_ENABLED */
{
    char identifier[128] = {0};
    sample_context_t* sample_ctx = _sample_ctx;

    /*
     * get iutput value.
     */
    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "transparency");
    linkkit_get_value(linkkit_method_get_service_input_value, thing, identifier, &sample_ctx->service_custom_input_transparency, NULL);

    /*
     * set output value according to user's process result.
     */

    snprintf(identifier, sizeof(identifier), "%s.%s", service_identifier, "Contrastratio");

    sample_ctx->service_custom_output_contrastratio = sample_ctx->service_custom_input_transparency >= 0 ? sample_ctx->service_custom_input_transparency : sample_ctx->service_custom_input_transparency * -1;

    linkkit_set_value(linkkit_method_set_service_output_value, thing, identifier, &sample_ctx->service_custom_output_contrastratio, NULL);
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

    if (strcmp(service, "Custom") == 0) {
#ifdef RRPC_ENABLED
        handle_service_custom(sample_ctx, thing_id, service, request_id, rrpc);
#else
        handle_service_custom(sample_ctx, thing_id, service, request_id);
#endif /* RRPC_ENABLED */
    }

    return 0;
}

static int thing_prop_changed(const void* thing_id, const char* property, void* ctx)
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
    } else {
        linkkit_get_value(linkkit_method_get_property_value, thing_id, property, NULL, &value_str);

        LINKKIT_PRINTF("property(%s) new value set: %s\n", property, value_str);
    }

    /* do user's process logical here. */
    linkkit_post_property(thing_id, property, post_property_cb);
    return 0;
}

static int linkit_data_arrived(const void* thing_id, const void* params, int len, void* ctx)
{
    LINKKIT_PRINTF("thing@%p: masterdev_linkkit_data(%d byte): %s\n", thing_id, len, (const char*)params);
    return 0;
}

static linkkit_ops_t alink_ops = {
    .on_connect           = on_connect,
    .on_disconnect        = on_disconnect,
    .raw_data_arrived     = raw_data_arrived,
    .thing_create         = thing_create,
    .thing_enable         = thing_enable,
    .thing_disable        = thing_disable,
    .thing_call_service   = thing_call_service,
    .thing_prop_changed   = thing_prop_changed,
    .linkit_data_arrived  = linkit_data_arrived,
};

static unsigned long long uptime_sec(void)
{
    static unsigned long long start_time = 0;

    if (start_time == 0) {
        start_time = HAL_UptimeMs();
    }

    return (HAL_UptimeMs() - start_time) / 1000;
}


static int post_all_prop(sample_context_t* sample)
{
    return linkkit_post_property(sample->thing, NULL, post_property_cb);
}

static int is_active(sample_context_t* sample_ctx)
{
#ifdef LOCAL_CONN_ENABLE
    return (sample_ctx->cloud_connected && sample_ctx->thing_enabled) || (sample_ctx->local_connected && sample_ctx->thing_enabled);
#else
    return sample_ctx->cloud_connected && sample_ctx->thing_enabled;
#endif
}

#ifdef POST_WIFI_STATUS
static int post_property_wifi_status_once(sample_context_t* sample_ctx)
{
    int ret = -1;
    int i = 0;
    static int is_post = 0;
    char val_buf[32];
    char ssid[HAL_MAX_SSID_LEN];
    char passwd[HAL_MAX_PASSWD_LEN];
    uint8_t bssid[ETH_ALEN];
    hal_wireless_info_t wireless_info;

    char* band = NULL;
    int channel = 0;
    int rssi = 0;
    int snr = 0;
    int tx_rate = 0;
    int rx_rate = 0;

    if(is_active(sample_ctx) && 0 == is_post) {
        HAL_GetWirelessInfo(&wireless_info);
        HAL_Wifi_Get_Ap_Info(ssid, passwd, bssid);

        band = wireless_info.band == 0 ? "2.4G" : "5G";
        channel = wireless_info.channel;
        rssi = wireless_info.rssi;
        snr = wireless_info.snr;
        tx_rate = wireless_info.tx_rate;
        rx_rate = wireless_info.rx_rate;

        linkkit_set_value(linkkit_method_set_property_value, sample_ctx->thing, "WIFI_Band", band, NULL);
        linkkit_set_value(linkkit_method_set_property_value, sample_ctx->thing, "WIFI_Channel", &channel, NULL);
        linkkit_set_value(linkkit_method_set_property_value, sample_ctx->thing, "WiFI_RSSI", &rssi, NULL);
        linkkit_set_value(linkkit_method_set_property_value, sample_ctx->thing, "WiFI_SNR", &snr, NULL);

        memset(val_buf, 0, sizeof(val_buf));
        for(i = 0; i < ETH_ALEN; i++) {
            snprintf(val_buf + strlen(val_buf), sizeof(val_buf) - strlen(val_buf), "%c:", bssid[i]);
        }
        if(strlen(val_buf) > 0 && val_buf[strlen(val_buf) - 1] == ':') val_buf[strlen(val_buf) - 1] = '\0';
        linkkit_set_value(linkkit_method_set_property_value, sample_ctx->thing, "WIFI_AP_BSSID", val_buf, NULL);

        linkkit_set_value(linkkit_method_set_property_value, sample_ctx->thing, "WIFI_Tx_Rate", &tx_rate, NULL);
        linkkit_set_value(linkkit_method_set_property_value, sample_ctx->thing, "WIFI_Rx_Rate", &rx_rate, NULL);

        is_post = 1;
        ret = 0;
    }
    return ret;
}
#endif

int main(int argc, char* argv[])
{
    sample_context_t* sample_ctx = &g_sample_context;
    int execution_time = 0;
    int get_tsl_from_cloud = 0;
    int exit = 0;
    unsigned long long now = 0;
    unsigned long long prev_sec = 0;

#if !defined(_WIN32)
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
    HAL_SetProductKey(IOTX_PRODUCT_KEY);
    HAL_SetDeviceName(IOTX_DEVICE_NAME);
    HAL_SetDeviceSecret(IOTX_DEVICE_SECRET);

    memset(sample_ctx, 0, sizeof(sample_context_t));
    sample_ctx->thing_enabled = 1;

    linkkit_start(8, get_tsl_from_cloud, linkkit_loglevel_debug, &alink_ops, linkkit_cloud_domain_shanghai, sample_ctx);
    if (!get_tsl_from_cloud) {
        linkkit_set_tsl(TSL_STRING, strlen(TSL_STRING));
    }

#ifdef SERVICE_OTA_ENABLED
    linkkit_ota_init(fota_callback);
#endif /* SERVICE_OTA_ENABLED */
    while (1) {
#ifndef CM_SUPPORT_MULTI_THREAD
        linkkit_dispatch();
#endif
        now = uptime_sec();
        if (prev_sec == now) {
#ifdef CM_SUPPORT_MULTI_THREAD
            HAL_SleepMs(100);
#else
            linkkit_yield(100);
#endif /* CM_SUPPORT_MULTI_THREAD */
            continue;
        }

        /* about 30 seconds, assume trigger post property event about every 30s. */

#ifdef POST_WIFI_STATUS
        if(now % 10 == 0) {
            post_property_wifi_status_once(sample_ctx);
        }
#endif
        if (now % 30 == 0 && is_active(sample_ctx)) {
            post_all_prop(sample_ctx);
        }

        if (exit) break;

        /* after all, this is an sample, give a chance to return... */
        /* modify this value for this sample executaion time period */
        if (now > 60 * execution_time) exit = 1;

        prev_sec = now;
    }

    linkkit_end();
    LINKKIT_PRINTF("out of sample!\n");
    return 0;
}
