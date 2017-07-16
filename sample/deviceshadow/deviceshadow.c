#include "aliot_platform.h"
#include "lite/lite-log.h"
#include "lite/lite-utils.h"
#include "aliot_mqtt_client.h"
#include "aliot_auth.h"
#include "aliot_device.h"
#include "aliot_shadow.h"


//The product and device information from IOT console
//#define PRODUCT_KEY         "OvNmiEYRDSY"
//#define DEVICE_NAME         "sh_online_sample_shadow"
//#define DEVICE_SECRET       "RcS3af0lHnpzNkfcVB1RKc4kSoR84D2n"

#ifndef DIRECT_MQTT
#define PRODUCT_KEY         "6RcIOUafDOm"
#define DEVICE_NAME         "sh_pre_sample_shadow"
#define DEVICE_SECRET       "DLpwSvgsyjD2jPDusSSjucmVGm9UJCt7"
#else
#define PRODUCT_KEY         "jRCMjOhnScj"
#define DEVICE_NAME         "dns_test"
#define DEVICE_SECRET       "OJurfzWl9SsyL6eaxBkMvmHW15KMyn3C"
#endif

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


    log_debug("attribute name=%s, attribute value=%d\r\n", pattr->pattr_name, *(int32_t *)pattr->pattr_data);
}


/* Device shadow demo entry */
int demo_device_shadow(char *msg_buf, char *msg_readbuf)
{
    char buf[1024];
    aliot_err_t rc;
    aliot_user_info_pt puser_info;
    void *h_shadow;
    aliot_shadow_para_t shadaw_para;


    /* Initialize the device info */
    aliot_device_init();

    if (0 != aliot_set_device_info(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET)) {
        log_debug("run aliot_set_device_info() error!\n");
        return -1;
    }

    /* Device AUTH */
    rc = aliot_auth(aliot_get_device_info(), aliot_get_user_info());
    if (SUCCESS_RETURN != rc) {
        log_err("rc = aliot_auth() = %d", rc);
        return rc;
    }

    puser_info = aliot_get_user_info();

    /* Construct a device shadow */
    memset(&shadaw_para, 0, sizeof(aliot_shadow_para_t));

    shadaw_para.mqtt.port = puser_info->port;
    shadaw_para.mqtt.host = puser_info->host_name;
    shadaw_para.mqtt.client_id = puser_info->client_id;
    shadaw_para.mqtt.user_name = puser_info->user_name;
    shadaw_para.mqtt.password = puser_info->password;
    shadaw_para.mqtt.pub_key = puser_info->pubKey;

    shadaw_para.mqtt.request_timeout_ms = 2000;
    shadaw_para.mqtt.clean_session = 0;
    shadaw_para.mqtt.keepalive_interval_ms = 60000;
    shadaw_para.mqtt.pread_buf = msg_readbuf;
    shadaw_para.mqtt.read_buf_size = MSG_LEN_MAX;
    shadaw_para.mqtt.pwrite_buf = msg_buf;
    shadaw_para.mqtt.write_buf_size = MSG_LEN_MAX;

    shadaw_para.mqtt.handle_event.h_fp = NULL;
    shadaw_para.mqtt.handle_event.pcontext = NULL;

    h_shadow = aliot_shadow_construct(&shadaw_para);
    if (NULL == h_shadow) {
        log_debug("construct device shadow failed!");
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
     * before calling aliot_shadow_sync() */
    aliot_shadow_register_attribute(h_shadow, &attr_light);
    aliot_shadow_register_attribute(h_shadow, &attr_temperature);


    /* synchronize the device shadow with device shadow cloud */
    aliot_shadow_sync(h_shadow);

    do {
        format_data_t format;

        /* Format the attribute data */
        aliot_shadow_update_format_init(h_shadow, &format, buf, 1024);
        aliot_shadow_update_format_add(h_shadow, &format, &attr_temperature);
        aliot_shadow_update_format_add(h_shadow, &format, &attr_light);
        aliot_shadow_update_format_finalize(h_shadow, &format);

        /* Update attribute data */
        aliot_shadow_update(h_shadow, format.buf, format.offset, 10);

        /* Sleep 1000 ms */
        aliot_platform_msleep(1000);
    } while (0);


    /* Delete the two attributes */
    aliot_shadow_delete_attribute(h_shadow, &attr_temperature);
    aliot_shadow_delete_attribute(h_shadow, &attr_light);

    aliot_shadow_deconstruct(h_shadow);

    return 0;
}


int main()
{
    LITE_openlog("shadow");
    LITE_set_loglevel(LOG_DEBUG_LEVEL);

    char *msg_buf = (char *)aliot_platform_malloc(MSG_LEN_MAX);
    char *msg_readbuf = (char *)aliot_platform_malloc(MSG_LEN_MAX);

    demo_device_shadow(msg_buf, msg_readbuf);

    aliot_platform_free(msg_buf);
    aliot_platform_free(msg_readbuf);

    log_debug("out of demo!");
    LITE_dump_malloc_free_stats(LOG_DEBUG_LEVEL);
    LITE_closelog();

    return 0;
}

