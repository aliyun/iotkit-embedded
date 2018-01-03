

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iot_import.h"
#include "iot_export.h"


extern void utils_hmac_sha1(const char *msg, int msg_len, char *digest, const char *key, int key_len);
extern void utils_hmac_md5(const char *msg, int msg_len, char *digest, const char *key, int key_len);

#define PRODUCT_KEY                    "a1AF9yh6PYx"
#define DEVICE_NAME                    "gatway_wifi_01"
#define DEVICE_SECRET                  "x9GlUdrfxTRPrzosi8pFPNqr3ORr7Xf2"

// static register
#define SUB_1_PRODUCT_KEY              "a18DAcXIew7"
#define SUB_1_DEVICE_NAME              "subdev_wifi_11"//"subdev_wifi_01"
#define SUB_1_DEVICE_SECRET            "9hhJDUpKVDrlg2kjbQIV96qfBqWS07ku"

// dynamie register
#define SUB_2_PRODUCT_KEY              "a18DAcXIew7"
#define SUB_2_DEVICE_NAME              "subdev_wifi_02"
#define SUB_2_DEVICE_SECRET            "fXgcjkeSDJYIaFKVWwGLyTjzksXpFjwN"

#define PAYLOAD_LEN_MAX                (1024)
#define MSG_LEN_MAX                    (1024 * 2)

static int _calc_sign(const char* product_key, 
        const char* device_name,
        const char* device_secret,
        char* hmac_sigbuf,
        const int hmac_buflen,
        const char* sign_method,
        const char *client_id, 
        const char *timestamp_str)
{
    char signature[64];                    
    char hmac_source[256];    

    memset(signature, 0, sizeof(signature));
    memset(hmac_source, 0, sizeof(hmac_source));
    HAL_Snprintf(hmac_source,
                      sizeof(hmac_source),
                      "clientId%s" "deviceName%s" "productKey%s" "timestamp%s",
                      client_id,
                      device_name,
                      product_key,
                      timestamp_str);
    printf("| source: %s (%d)\n", hmac_source, (int)strlen(hmac_source));
    printf("| secret: %s (%d)\n", device_secret, (int)strlen(device_secret));

    if (0 == strncmp(sign_method, "hmacsha1", strlen("hmacsha1"))) {
        utils_hmac_sha1(hmac_source, strlen(hmac_source),
                    signature,
                    device_secret,
                    strlen(device_secret));
    } else if (0 == strncmp(sign_method, "hmacmd5", strlen("hmacmd5"))){
        utils_hmac_md5(hmac_source, strlen(hmac_source),
                   signature,
                   device_secret,
                   strlen(device_secret));
    }

    printf("| signature: %s (%d)\n", signature, (int)strlen(signature));

    memcpy(hmac_sigbuf, signature, hmac_buflen);
    return 0;
}

#if 0
static void _demo_gateway_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    printf("~~~~~~~~ gateway receive MQTT data ~~~~~~~~\n");

    /* print topic name and topic message */
    printf("----\n");
    printf("Topic: '%.*s' (Length: %d)\n",
                  ptopic_info->topic_len,
                  ptopic_info->ptopic,
                  ptopic_info->topic_len);
    printf("Payload: '%.*s' (Length: %d)\n",
                  ptopic_info->payload_len,
                  ptopic_info->payload,
                  ptopic_info->payload_len);
    printf("----\n");
}
#endif

static void _demo_subdevice1_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    printf("~~~~~~~ subdevice1 receive MQTT data ~~~~~~~\n");

    /* print topic name and topic message */
    printf("----\n");
    printf("Topic: '%.*s' (Length: %d)\n",
                  ptopic_info->topic_len,
                  ptopic_info->ptopic,
                  ptopic_info->topic_len);
    printf("Payload: '%.*s' (Length: %d)\n",
                  ptopic_info->payload_len,
                  ptopic_info->payload,
                  ptopic_info->payload_len);
    printf("----\n");
}

       
void rrpc_request_handler(void* gateway,
        const char* product_key,
        const char* device_name,
        const char* message_id,  
        const char* payload)
{
    printf("rrpc_request_callback, product_key [%s], device_name [%s], message_id [%s], payload:%s \n", 
                product_key,
                device_name,
                message_id, 
                payload);

    if (IOT_Gateway_RRPC_Response(gateway, 
            product_key, 
            device_name, 
            message_id, 
            "aa00090000ff00003039001b") >= 0) {
        printf("gateway rrpc repose ok success\n");
    } else {
        printf("gateway rrpc repose ok fail\n");
    }
}

#define TOPO_ADD_PACKET_FMT     "{\"id\":%d,\"version\":\"1.0\",\"params\":[{\"deviceName\":\"%s\",\"productKey\":\"%s\",\"sign\":\"%s\",\"signMethod\":\"%s\",\"timestamp\":\"%s\",\"clientId\":\"%s\"}],\"method\":\"thing.topo.add\"}"
#define DEVICEINFO_PACKET_FMT   "{\"id\":%d,\"version\":\"1.0\",\"params\":%s,\"method\":\"thing.deviceinfo.delete\"}"
int demo_gateway_function(char *msg_buf, char *msg_readbuf)
{
    int rc, cnt = 0;;
    iotx_conn_info_pt puser_info;
    iotx_mqtt_param_t mqtt_t;
    iotx_gateway_param_t gateway_param;
    void* gateway_t = NULL; 
    iotx_mqtt_topic_info_t topic_msg; 
    char msg_pub[1024] = {0}; 
    char sign[41] = {0};
    char timestamp[20] = {0};
    char client_id[32] = {0};
    /*char topo_topic[128] = {0};*/
    char deviceinfo_topic[128] = {0};
    char topic[128] = {0};

    /* Device AUTH */
    rc = IOT_SetupConnInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, (void **)&puser_info);
    
    if (SUCCESS_RETURN != rc) {
        printf("rc = IOT_SetupConnInfo() = %d\n", rc);
        return rc;
    }

    /* Construct a master-slave */
    memset(&mqtt_t, 0, sizeof(iotx_mqtt_param_t));
    memset(&gateway_param, 0, sizeof(iotx_gateway_param_t));

    gateway_param.mqtt = &mqtt_t;
    gateway_param.mqtt->port = puser_info->port;//1883;
    gateway_param.mqtt->host = puser_info->host_name;//"10.101.84.132";
    gateway_param.mqtt->client_id = puser_info->client_id;
    gateway_param.mqtt->username = puser_info->username;
    gateway_param.mqtt->password = puser_info->password;
    gateway_param.mqtt->pub_key = puser_info->pub_key;

    gateway_param.mqtt->request_timeout_ms = 2000;
    gateway_param.mqtt->clean_session = 0;
    gateway_param.mqtt->keepalive_interval_ms = 60000;
    gateway_param.mqtt->pread_buf = msg_readbuf;
    gateway_param.mqtt->read_buf_size = MSG_LEN_MAX;
    gateway_param.mqtt->pwrite_buf = msg_buf;
    gateway_param.mqtt->write_buf_size = MSG_LEN_MAX;

    gateway_param.mqtt->handle_event.h_fp = NULL;
    gateway_param.mqtt->handle_event.pcontext = NULL;
    
    printf(" ~~~~~~~~~~~~~~~~ start test ~~~~~~~~~~~~~~~~~ \n");
    printf(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");
    printf(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");

     
    /* timestamp */
    strncpy(timestamp, "2524608000000", strlen("2524608000000") + 1);
   
    /* client id */
    HAL_Snprintf(client_id, 32, "%s.%s", SUB_1_PRODUCT_KEY, SUB_1_DEVICE_NAME);
    
    /* sign */    
    if (FAIL_RETURN == _calc_sign(SUB_1_PRODUCT_KEY,
                            SUB_1_DEVICE_NAME,
                            SUB_1_DEVICE_SECRET,
                            sign, 
                            41,
                            "hmacsha1",
                            client_id,
                            timestamp)) {
        printf("sign fail \n");
        return FAIL_RETURN;
    }

    /* construct */
    printf(" ~~~~~~~~~~~~~~ start construct ~~~~~~~~~~~~~ \n");
    gateway_t = IOT_Gateway_Construct(&gateway_param);
    if (NULL == gateway_t) {
        printf("construct Gateway failed!\n");
        return rc;
    }
    if (FAIL_RETURN == IOT_Gateway_RRPC_Register(gateway_t, 
            PRODUCT_KEY, 
            DEVICE_NAME, 
            rrpc_request_handler)) {
        printf("rrpc register error\n");
        return FAIL_RETURN;
    }
    printf(" ~~~~~~~~~~~~ construct success ~~~~~~~~~~~~~ \n");
    printf(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");
    

    #if 0
    printf(" ~~~~~~~~~~~~ start subscribe ~~~~~~~~~~~~~~ \n");  
    printf(" ~~~~~~~~~~~ gateway subscribe ~~~~~~~~~~~~~ \n"); 
    HAL_Snprintf(topo_topic,
               128,
               "/sys/%s/%s/thing/topo/add_reply",
               PRODUCT_KEY,
               DEVICE_NAME);
    /* Subscribe the specific topic */
    rc = IOT_Gateway_Subscribe(gateway_t, 
                topo_topic, 
                IOTX_MQTT_QOS1, 
                _demo_gateway_message_arrive, 
                NULL);
    
    if (rc < 0) {
        IOT_Gateway_Destroy((void**)&gateway_t);
        printf("gateway IOT_Gateway_Subscribe() failed, rc = %d", rc);
        return rc;
    }

    printf("gateway subscribe id %d \n", rc);
    IOT_Gateway_Yield(gateway_t, 1000);

    /* Initialize topic information */
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    HAL_Snprintf(msg_pub,
               PAYLOAD_LEN_MAX,
               TOPO_ADD_PACKET_FMT,
               5,
               SUB_1_DEVICE_NAME,
               SUB_1_PRODUCT_KEY,
               sign,
               "hmacsha1",
               timestamp,
               client_id);
    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    /* todo add subdev1 */
    memset(topic, 0x0, 128);
    HAL_Snprintf(topic,
               128,
               "/sys/%s/%s/thing/topo/add",
               PRODUCT_KEY,
               DEVICE_NAME);
    rc = IOT_Gateway_Publish(gateway_t, topic, &topic_msg);
    printf("gateway rc = IOT_Gateway_Publish() = %d\n", rc);

    HAL_SleepMs(1000);
    #else

    printf(" ~~~~~~~~~~~~~ start register ~~~~~~~~~~~~~~~ \n");
    rc = IOT_Subdevice_Register(gateway_t, 
                IOTX_SUBDEV_REGISTER_TYPE_DYNAMIC, //IOTX_SUBDEV_REGISTER_TYPE_STATIC, 
                SUB_1_PRODUCT_KEY, 
                SUB_1_DEVICE_NAME, //SUB_1_DEVICE_NAME, 
                NULL, //timestamp, 
                NULL, //client_id, 
                NULL, //sign, 
                IOTX_SUBDEV_SIGN_METHOD_TYPE_SHA);

    if (SUCCESS_RETURN != rc) {
        printf("1  rc = IOT_Subdevice_Register() = %d\n", rc);
        IOT_Gateway_Destroy((void**)&gateway_t);
        return rc;
    }    

    
    #endif

    printf(" ~~~~~~~~~~~~~~~ start login ~~~~~~~~~~~~~~~~ \n");    
    rc = IOT_Subdevice_Login(gateway_t, 
                SUB_1_PRODUCT_KEY, 
                SUB_1_DEVICE_NAME,
                NULL,//timestamp,
                NULL,//client_id,
                NULL,//sign,
                IOTX_SUBDEV_SIGN_METHOD_TYPE_SHA,
                IOTX_SUBDEV_CLEAN_SESSION_TRUE);

    if (SUCCESS_RETURN != rc) {
        printf("1  rc = IOT_Subdevice_Login() = %d\n", rc);
        IOT_Gateway_Destroy((void**)&gateway_t);
        return rc;
    }  
    
    if (FAIL_RETURN == IOT_Gateway_RRPC_Register(gateway_t, 
            SUB_1_PRODUCT_KEY, 
            SUB_1_DEVICE_NAME, 
            rrpc_request_handler)) {
        printf("rrpc register error\n");
        return FAIL_RETURN;
    }   
    printf(" ~~~~~~~~~~~~~~~ login success ~~~~~~~~~~~~~~~~ \n");    

    printf(" ~~~~~~~~~~~~~~~ start subdev ~~~~~~~~~~~~~~~~~~ \n");   
    HAL_Snprintf(deviceinfo_topic,
               128,
               "/sys/%s/%s/thing/deviceinfo/delete_reply",
               SUB_1_PRODUCT_KEY,
               SUB_1_DEVICE_NAME); 
    /* Subscribe the specific topic */
    rc = IOT_Gateway_Subscribe(gateway_t, 
                deviceinfo_topic, 
                IOTX_MQTT_QOS1, 
                _demo_subdevice1_message_arrive, 
                NULL);
    
    if (rc < 0) {
        IOT_Gateway_Destroy((void**)&gateway_t);
        printf("gateway IOT_Gateway_Subscribe() failed, rc = %d", rc);
        return rc;
    }
    printf(" ~~~~~~~~~~~~~~~ subdev success ~~~~~~~~~~~~~~~~ \n");    
               

    printf(" ~~~~~~~~~~~~~~~ start publish ~~~~~~~~~~~~~~~~~~ \n");      

    /* Initialize topic information */  
    memset(topic, 0x0, 128);
    HAL_Snprintf(topic,
               128,
               "/sys/%s/%s/thing/deviceinfo/delete",
               SUB_1_PRODUCT_KEY,
               SUB_1_DEVICE_NAME); 
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    HAL_Snprintf(msg_pub,
               PAYLOAD_LEN_MAX,
               DEVICEINFO_PACKET_FMT,
               8,
               "[{\"attrKey\":\"Temperature\",\"attrValue\":\"36.8\"}]");
    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    /* todo add subdev1 */
    rc = IOT_Gateway_Publish(gateway_t, topic, &topic_msg);
    printf("gateway rc = IOT_Gateway_Publish() = %d\n", rc);
    printf(" ~~~~~~~~~~~~~~~ publish success ~~~~~~~~~~~~~~~~ \n");    
            
    /* handle the MQTT packet received from TCP or SSL connection */
    while (cnt < 30) {
        IOT_Gateway_Yield(gateway_t, 200);
        cnt++;
    }
    
    memset(topic, 0x0, 128);
    HAL_Snprintf(topic,
               128,
               "/sys/%s/%s/thing/topo/add_reply",
               PRODUCT_KEY,
               DEVICE_NAME);
    IOT_Gateway_Unsubscribe(gateway_t, topic);    
    
    memset(topic, 0x0, 128);
    HAL_Snprintf(topic,
               128,
               "/sys/%s/%s/thing/deviceinfo/delete_reply",
               PRODUCT_KEY,
               DEVICE_NAME);
    IOT_Gateway_Unsubscribe(gateway_t, topic);

    printf(" ~~~~~~~~~~ unsubscribe success ~~~~~~~~~~~~ \n"); 
    printf(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");

    HAL_SleepMs(200);
    
    printf(" ~~~~~~~~~~~~~ start logout ~~~~~~~~~~~~~~~~ \n"); 
    printf(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");
    rc = IOT_Subdevice_Logout(gateway_t, SUB_1_PRODUCT_KEY, SUB_1_DEVICE_NAME);

    if (SUCCESS_RETURN != rc) {
        printf("1  rc = IOT_Subdevice_Logout() = %d\n", rc);
        IOT_Gateway_Destroy((void**)&gateway_t);
        return rc;
    }
    printf(" ~~~~~~~~~~~~~ logout success ~~~~~~~~~~~~~~ \n"); 
    printf(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");

    IOT_Gateway_Destroy((void**)&gateway_t);
    printf(" ~~~~~~~~~~~~ destory success ~~~~~~~~~~~~~~ \n"); 

    return SUCCESS_RETURN;
}


int main()
{
    char msg_buf[MSG_LEN_MAX] ={0};
    char msg_readbuf[MSG_LEN_MAX] ={0};
    
    IOT_OpenLog("masterslave");
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    demo_gateway_function(msg_buf, msg_readbuf);

    printf("out of demo!\n");
    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_CloseLog();

    return 0;
}
