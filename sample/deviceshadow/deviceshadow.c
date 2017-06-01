#include "aliyun_iot_platform_memory.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_mqtt_client.h"
#include "aliyun_iot_auth.h"
#include "aliyun_iot_device.h"
#include "aliyun_iot_shadow.h"


//The product and device information from IOT console
#define PRODUCT_KEY         "4eViBFJ2QGH"
#define DEVICE_NAME         "sh_xk_device_4"
#define DEVICE_ID           "gAPvCV7YhFccFMaHmAYh"
#define DEVICE_SECRET       "qkmOMDccik2HnyCamIqK5gcZnuuwNBXe"

#define MSG_LEN_MAX         (1024)


/**
 * @brief This is a callback function when a control value coming from server.
 *
 * @param [in] pattr: attribute structure pointer
 * @return none
 * @see none.
 * @note none.
 */
static void device_shadow_cb_light(aliot_shadow_attr_pt pattr)
{

    /*
     * ****** Your Code ******
     */


    ALIOT_LOG_DEBUG("attribute name=%s, attribute value=%d\r\n", pattr->pattr_name, *(int32_t *)pattr->pattr_data);
}


/* Device shadow demo entry */
int demo_device_shadow(unsigned char *msg_buf, unsigned char *msg_readbuf)
{

    char buf[1024];

    aliot_err_t rc;
    aliot_shadow_t shadow;
    aliot_shadow_para_t shadaw_para;


    /* Initialize the device info */
    aliyun_iot_device_init();

    if (0 != aliyun_iot_set_device_info(PRODUCT_KEY, DEVICE_NAME, DEVICE_ID, DEVICE_SECRET)) {
        ALIOT_LOG_DEBUG("run aliyun_iot_set_device_info() error!\n");
        return -1;
    }

    /* Device AUTH */
    rc = aliyun_iot_auth(aliyun_iot_get_device_info(), aliyun_iot_get_user_info());
    if (SUCCESS_RETURN != rc) {
        ALIOT_LOG_DEBUG("run aliyun_iot_auth() error!\n");
        return rc;
    }


    /* Construct a device shadow */
    memset(&shadow, 0, sizeof(aliot_shadow_t));
    memset(&shadaw_para, 0, sizeof(aliot_shadow_para_t));

    shadaw_para.mqtt.mqttCommandTimeout_ms = 10000;
    shadaw_para.mqtt.pReadBuf = msg_readbuf;
    shadaw_para.mqtt.readBufSize = MSG_LEN_MAX;
    shadaw_para.mqtt.pWriteBuf = msg_buf;
    shadaw_para.mqtt.writeBufSize = MSG_LEN_MAX;
    shadaw_para.mqtt.disconnectHandler = NULL;
    shadaw_para.mqtt.disconnectHandlerData = (void *)&shadow.mqtt;

    shadaw_para.mqtt.cleansession      = 0;
    shadaw_para.mqtt.MQTTVersion       = 4;
    shadaw_para.mqtt.keepAliveInterval = 60;
    shadaw_para.mqtt.willFlag          = 0;


    rc = aliyun_iot_shadow_construct(&shadow, &shadaw_para);
    if (SUCCESS_RETURN != rc) {
        ALIOT_LOG_DEBUG("run aliyun_iot_auth() error!\n");
        return rc;
    }


    /* Define and add two attribute */

    int32_t light = 1000, temperature = 1001;
    aliot_shadow_attr_t attr_light, attr_temperature;

    memset(&attr_light, 0, sizeof(aliot_shadow_attr_t));
    memset(&attr_temperature, 0, sizeof(aliot_shadow_attr_t));

    /* Initialize the @light attribute */
    attr_light.attr_type = ALIOT_SHADOW_INT32;
    attr_light.mode = ALIOT_SHADOW_RW;
    attr_light.pattr_name = "switch";
    attr_light.pattr_data = &light;
    attr_light.callback = device_shadow_cb_light;

    /* Initialize the @temperature attribute */
    attr_temperature.attr_type = ALIOT_SHADOW_INT32;
    attr_temperature.mode = ALIOT_SHADOW_READONLY;
    attr_temperature.pattr_name = "temperature";
    attr_temperature.pattr_data = &temperature;
    attr_temperature.callback = NULL;


    /* Register the attribute */
    /* Note that you must register the attribute you want to synchronize with cloud
     * before calling aliyun_iot_shadow_sync() */
    aliyun_iot_shadow_register_attribute(&shadow, &attr_light);
    aliyun_iot_shadow_register_attribute(&shadow, &attr_temperature);


    /* synchronize the device shadow with device shadow cloud */
    aliyun_iot_shadow_sync(&shadow);

    do {
        format_data_t format;

        /* Format the attribute data */
        aliyun_iot_shadow_update_format_init(&format, buf, 1024);
        aliyun_iot_shadow_update_format_add(&format, &attr_temperature);
        aliyun_iot_shadow_update_format_add(&format, &attr_light);
        aliyun_iot_shadow_update_format_finalize(&format);

        /* Update attribute data */
        aliyun_iot_shadow_update(&shadow, format.buf, format.offset, 10);

        /* Sleep 1000 ms */
        aliyun_iot_pthread_taskdelay(1000);
    } while (0);


    /* Delete the two attributes */
    //aliyun_iot_shadow_delete_attribute(&shadow, &attr_temperature);
    //aliyun_iot_shadow_delete_attribute(&shadow, &attr_light);

    aliyun_iot_shadow_deconstruct(&shadow);

    return 0;
}


int main()
{
    unsigned char *msg_buf = (unsigned char *)aliyun_iot_memory_malloc(MSG_LEN_MAX);
    unsigned char *msg_readbuf = (unsigned char *)aliyun_iot_memory_malloc(MSG_LEN_MAX);


    demo_device_shadow(msg_buf, msg_readbuf);

    aliyun_iot_memory_free(msg_buf);
    aliyun_iot_memory_free(msg_readbuf);

    ALIOT_LOG_DEBUG("out of demo!\n");

    return 0;
}

