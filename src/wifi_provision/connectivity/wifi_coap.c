/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */
#include <stdlib.h>
#include "wifi_provision_internal.h"
#include "CoAPExport.h"
#include "CoAPServer.h"
#include "infra_state.h"
#include "infra_compat.h"
#include "wrappers.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

struct notify_map_t {
    uint8_t notify_type;
    char *notify_method;
    char *notify_topic;
    void *cb;
};

typedef struct {
    char host[16]; /**< host ip(dotted-decimal notation) or host name(string) */
    uint16_t port; /**< udp port or tcp port */
} platform_netaddr_t;

static p_local_handle_t local_handle;
p_local_handle_t *p_local_handle = NULL;

static int wifi_coap_send(void *handle, uint8_t *buf, uint32_t len, void *sa, const char *uri, void *cb,
                          uint16_t *msgid);

static void *wifi_get_dev_info(void *dev_info, int len);
static int process_get_device_info(void *ctx, void *resource, void *remote, void *request, char is_mcast, int type);

static int wifi_process_mcast_get_connectap_info(void *ctx, void *resource, void *remote, void *request);
static int wifi_process_ucast_get_connectap_info(void *ctx, void *resource, void *remote, void *request);
static void wifi_produce_random(uint8_t *random, uint32_t len);
static uint8_t wifi_get_coap_code(void *request);
static int wifi_devinfo_notify_resp(void *context, int result, void *userdata, void *remote, void *message);
static int wifi_suc_notify_resp(void *context, int result, void *userdata, void *remote, void *message);
static int wifi_notify_response(int type, int result, void *message);
static int wifi_get_broadcast_addr(platform_netaddr_t *bcast_addr);
static char *wifi_get_mac_str(char mac_str[HAL_MAC_LEN]);

static const struct notify_map_t notify_map[] = {
    {AWSS_NOTIFY_DEV_RAND_SIGN,  METHOD_AWSS_DEV_INFO_NOTIFY,  TOPIC_AWSS_NOTIFY,           wifi_devinfo_notify_resp},
    {AWSS_NOTIFY_SUCCESS,        METHOD_AWSS_CONNECTAP_NOTIFY, TOPIC_AWSS_CONNECTAP_NOTIFY, wifi_suc_notify_resp}
};

uint8_t *wifi_get_rand()
{
    return local_handle.aes_random;
}
int wifi_got_notify_resp(int type)
{
    return local_handle.notify_resp[type];
}

int wifi_coap_init()
{

    if (p_local_handle != NULL) {
        return 0;
    }
    memset(&local_handle, 0, sizeof(local_handle));
    p_local_handle = &local_handle;

    p_local_handle->coap_handle = (void *)CoAPServer_init();

    if (p_local_handle->coap_handle == NULL) {
        p_local_handle = NULL;
        dump_awss_status(STATE_WIFI_COAP_INIT_FAILED, NULL);
        return STATE_WIFI_COAP_INIT_FAILED;
    }

    wifi_produce_random(p_local_handle->aes_random, RANDOM_MAX_LEN);
    wifi_coap_common_register();
    return 0;
}

int wifi_coap_deinit()
{
    if (p_local_handle == NULL) {
        return 0;
    }
    local_handle.connectap_notify_cnt = 0;
    p_local_handle->coap_handle = NULL;
    p_local_handle = NULL;
    return 0;
}

int wifi_coap_cancel_packet(uint16_t msgid)
{
    if (p_local_handle == NULL) {
        return 0;
    }
    return CoAPMessageId_cancel(p_local_handle->coap_handle, msgid);
}

int wifi_coap_common_register()
{
    char topic[TOPIC_LEN_MAX] = {0};

    wifi_build_topic(TOPIC_AWSS_GET_CONNECTAP_INFO_UCAST, topic, TOPIC_LEN_MAX);
    wifi_coap_register(topic, wifi_process_ucast_get_connectap_info);
    wifi_coap_register(TOPIC_AWSS_GET_CONNECTAP_INFO_MCAST, wifi_process_mcast_get_connectap_info);
    return 0;
}

int wifi_coap_register(const char *topic, void *cb)
{
    if (p_local_handle == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    return CoAPServer_register(p_local_handle->coap_handle, (const char *)topic, (CoAPRecvMsgHandler)cb);

}

int wifi_notify_dev_info(int type)
{
    char *buf = NULL;
    char *dev_info = NULL;
    int ret = -1;
    int i;
    platform_netaddr_t notify_sa;

    if (p_local_handle == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    memset(&notify_sa, 0, sizeof(notify_sa));

    do {
        void *cb = NULL;
        char *method = NULL, *topic = NULL;
        for (i = 0; i < sizeof(notify_map) / sizeof(notify_map[0]); i ++) {
            if (notify_map[i].notify_type != type) {
                continue;
            }

            method = notify_map[i].notify_method;
            topic = notify_map[i].notify_topic;
            cb = notify_map[i].cb;
            break;
        }
        if (method == NULL || topic == NULL) {
            awss_err("parametes invalid");
            break;
        }

        buf = awss_zalloc(DEV_INFO_LEN_MAX);
        dev_info = awss_zalloc(DEV_INFO_LEN_MAX);
        if (buf == NULL || dev_info == NULL) {
            awss_err("alloc mem fail");
            break;
        }

        memset(&notify_sa, 0, sizeof(notify_sa));
        wifi_get_broadcast_addr(&notify_sa);
        awss_info("bcast ip = %s\n", notify_sa.host);

        wifi_build_dev_info(type, dev_info, DEV_INFO_LEN_MAX);

        HAL_Snprintf(buf, DEV_INFO_LEN_MAX - 1, WIFI_DEV_NOTIFY_FMT, ++ p_local_handle->notify_id, method, dev_info);

        awss_info("topic:%s\n", topic);
        awss_debug("payload:%s\n", buf);

        ret = wifi_coap_send(p_local_handle->coap_handle, (uint8_t *)buf, strlen(buf),  &notify_sa, topic, cb,
                             &p_local_handle->notify_msg_id[type]);
        if (type == AWSS_NOTIFY_DEV_RAND_SIGN) {
            dump_awss_status(STATE_WIFI_SENT_DEVINFO_NOTIFY, NULL);
        } else if (type == AWSS_NOTIFY_SUCCESS) {
            dump_awss_status(STATE_WIFI_SENT_CONNECTAP_NOTIFY, NULL);
        }
        awss_info("send notify %s", ret == 0 ? "success" : "fail");

    } while (0);

    if (buf) {
        HAL_Free(buf);
    }
    if (dev_info) {
        HAL_Free(dev_info);
    }

    return ret;
}

static int wifi_get_broadcast_addr(platform_netaddr_t *bcast_addr)
{
    static uint8_t mask_level = 3;
    char ip[20] = {0};
    uint8_t level = 0;

    if (bcast_addr == NULL) {
        return STATE_USER_INPUT_INVALID;
    }

    /* update mask_level */
    if (++mask_level >= 4) {
        mask_level = 0;
    }

    /* setup port */
    bcast_addr->port = WIFI_NOTIFY_PORT;

    /* setup ip */
    HAL_Wifi_Get_IP(ip, NULL);

    if (ip[0] != '\0' && mask_level != 0) {
        uint8_t i = 0;
        for (i = 0; i < strlen(ip); i++) {
            bcast_addr->host[i] = ip[i];
            if (ip[i] == '.') {
                if (++level == mask_level) {
                    break;
                }
            }
        }

        if (mask_level == 1) {
            if (i + strlen("255.255.255") < 16) {
                memcpy(bcast_addr->host + strlen(bcast_addr->host), "255.255.255", strlen("255.255.255"));
                return 0;
            }
        } else if (mask_level == 2) {
            if (i + strlen("255.255") < 16) {
                memcpy(bcast_addr->host + strlen(bcast_addr->host), "255.255", strlen("255.255"));
                return 0;
            }
        } else if (mask_level == 3) {
            if (i + strlen("255") < 16) {
                memcpy(bcast_addr->host + strlen(bcast_addr->host), "255", strlen("255"));
                return 0;
            }
        }
    }

    memcpy(bcast_addr->host, WIFI_NOTIFY_HOST, strlen(WIFI_NOTIFY_HOST));
    return 0;
}

int wifi_build_topic(const char *topic_fmt, char *topic, uint32_t tlen)
{
    char pk[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char dev_name[IOTX_DEVICE_NAME_LEN + 1] = {0};
    if (topic == NULL || topic_fmt == NULL || tlen == 0) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, pk);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_NAME, dev_name);

    HAL_Snprintf(topic, tlen - 1, topic_fmt, pk, dev_name);

    return 0;
}

int wifi_coap_send_resp(void *buf, uint32_t len, void *sa, const char *uri, void *req, void *cb, uint16_t *msgid,
                        char qos)
{
    if (p_local_handle == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    return CoAPServerResp_send(p_local_handle->coap_handle, (NetworkAddr *)sa, (uint8_t *)buf, (uint16_t)len, req, uri,
                               (CoAPSendMsgHandler)cb, msgid, qos);
}

char *wifi_get_coap_payload(void *request, int *payload_len)
{
    struct CoAPMessage *msg = (struct CoAPMessage *)request;
    if (request == NULL) {
        return NULL;
    }

    msg = (struct CoAPMessage *)request;
    if (payload_len) {
        *payload_len = msg->payloadlen;
    }
    awss_debug("payload:%s\r\n", msg->payload);
    return (char *)msg->payload;
}

static void *wifi_get_dev_info(void *dev_info, int len)
{
    char dev_name[IOTX_DEVICE_NAME_LEN + 1] = {0};
    char mac_str[HAL_MAC_LEN + 1] = {0};
    char pk[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char ip_str[OS_IP_LEN + 1] = {0};

    if (dev_info == NULL || len <= 0) {
        return NULL;
    }

    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, pk);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_NAME, dev_name);
    wifi_get_mac_str(mac_str);
    HAL_Wifi_Get_IP(ip_str, NULL);

    HAL_Snprintf(dev_info, len - 1, WIFI_DEV_INFO_FMT, AWSS_VER, pk, dev_name, mac_str, ip_str,
                 awss_get_encrypt_type());

    return dev_info;
}

void *wifi_build_dev_info(int type, void *dev_info, int info_len)
{
    int len = 0;
    char *buf = NULL;

    if (dev_info == NULL || info_len <= 0 || p_local_handle == NULL) {
        return NULL;
    }

    buf = awss_zalloc(DEV_INFO_LEN_MAX);
    if (buf == NULL) {
        return NULL;
    }

    len += HAL_Snprintf((char *)dev_info + len, info_len - len - 1, "%s", (char *)wifi_get_dev_info(buf, DEV_INFO_LEN_MAX));
    HAL_Free(buf);
    buf = NULL;

    switch (type) {
        case AWSS_NOTIFY_DEV_BIND_TOKEN: {
#ifdef DEV_BIND
            extern int IOT_Bind_SetToken(uint8_t *);
            IOT_Bind_SetToken(wifi_get_rand());
#endif
            char rand_str[(RANDOM_MAX_LEN << 1) + 1] = {0};
            utils_hex_to_str(wifi_get_rand(), RANDOM_MAX_LEN, rand_str, sizeof(rand_str));
            len += HAL_Snprintf((char *)dev_info + len, info_len - len - 1, WIFI_DEV_BIND_TOKEN_FMT, rand_str,
                                90 * 1000, 0);
            break;
        }
        case AWSS_NOTIFY_SUCCESS: {
            len += HAL_Snprintf((char *)dev_info + len, info_len - len - 1, WIFI_SUCCESS_FMT, 0);
            break;
        }
        case AWSS_NOTIFY_DEV_RAND_SIGN: {
            char sign_str[DEV_SIGN_SIZE * 2 + 1] = {0};
            char rand_str[(RANDOM_MAX_LEN << 1) + 1] = {0};
            {
                int txt_len = 80;
                char txt[80] = {0};
                char key[IOTX_DEVICE_SECRET_LEN + 1] = {0};
                uint8_t sign[DEV_SIGN_SIZE + 1] = {0};

                if (awss_get_encrypt_type() == 3) { /* aes-key per product */
                    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_SECRET, key);
                } else if (awss_get_encrypt_type() == 4) { /* aes-key per device */
                    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_SECRET, key);
                }
                wifi_build_sign_src(txt, &txt_len);
                produce_signature(sign, (uint8_t *)txt, txt_len, key);
                utils_hex_to_str(sign, DEV_SIGN_SIZE, sign_str, sizeof(sign_str));
            }
            utils_hex_to_str(p_local_handle->aes_random, RANDOM_MAX_LEN, rand_str, sizeof(rand_str));
            len += HAL_Snprintf((char *)dev_info + len, info_len - len - 1, WIFI_DEV_RAND_SIGN_FMT, rand_str, 0, sign_str);
            break;
        }
        default:
            break;
    }

    return dev_info;
}

char *wifi_build_sign_src(char *sign_src, int *sign_src_len)
{
    char *pk = NULL, *dev_name = NULL;
    int dev_name_len, pk_len, text_len;

    if (sign_src == NULL || sign_src_len == NULL) {
        goto build_sign_src_err;
    }

    pk = awss_zalloc(IOTX_PRODUCT_KEY_LEN + 1);
    dev_name = awss_zalloc(IOTX_DEVICE_NAME_LEN + 1);
    if (pk == NULL || dev_name == NULL) {
        goto build_sign_src_err;
    }

    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, pk);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_NAME, dev_name);

    pk_len = strlen(pk);
    dev_name_len = strlen(dev_name);

    text_len = RANDOM_MAX_LEN + dev_name_len + pk_len;
    if (*sign_src_len < text_len) {
        goto build_sign_src_err;
    }

    *sign_src_len = text_len;
    if(p_local_handle == NULL) {
        wifi_produce_random(local_handle.aes_random, RANDOM_MAX_LEN);
    }
    memcpy(sign_src, local_handle.aes_random, RANDOM_MAX_LEN);
    memcpy(sign_src + RANDOM_MAX_LEN, dev_name, dev_name_len);
    memcpy(sign_src + RANDOM_MAX_LEN + dev_name_len, pk, pk_len);

    HAL_Free(pk);
    HAL_Free(dev_name);

    return sign_src;

build_sign_src_err:
    if (pk) {
        HAL_Free(pk);
    }
    if (dev_name) {
        HAL_Free(dev_name);
    }
    return NULL;
}

static int process_get_device_info(void *ctx, void *resource, void *remote, void *request, char is_mcast, int type)
{
    char *buf = NULL;
    char *dev_info = NULL;
    int len = 0, id_len = 0;
    char *msg = NULL, *id = NULL;
    const char *topic_fmt = NULL;
    int ret = 0;
    char topic[TOPIC_LEN_MAX] = {0};
    char req_msg_id[MSG_REQ_ID_LEN] = {0};

    buf = awss_zalloc(DEV_INFO_LEN_MAX);
    if (!buf) {
        goto DEV_INFO_ERR;
    }

    dev_info = awss_zalloc(DEV_INFO_LEN_MAX);
    if (!dev_info) {
        goto DEV_INFO_ERR;
    }

    msg = wifi_get_coap_payload(request, &len);
    if (!msg || len == 0) {
        goto DEV_INFO_ERR;
    }

    id = json_get_value_by_name(msg, len, "id", &id_len, 0);
    if (id && id_len < MSG_REQ_ID_LEN) {
        memcpy(req_msg_id, id, id_len);
    }

    if (type == AWSS_NOTIFY_DEV_RAND_SIGN) {
        dump_awss_status(STATE_WIFI_GOT_DEVINFO_QUERY, NULL);
        topic_fmt = is_mcast ? TOPIC_AWSS_GETDEVICEINFO_MCAST : TOPIC_AWSS_GETDEVICEINFO_UCAST;
    } else if (type == AWSS_NOTIFY_SUCCESS) {
        dump_awss_status(STATE_WIFI_GOT_CONNECTAP_QUERY, NULL);
        topic_fmt = is_mcast ? TOPIC_AWSS_GET_CONNECTAP_INFO_MCAST : TOPIC_AWSS_GET_CONNECTAP_INFO_UCAST;
    } else {
        goto DEV_INFO_ERR;
    }
    wifi_build_dev_info(type, buf, DEV_INFO_LEN_MAX);
    HAL_Snprintf(dev_info, DEV_INFO_LEN_MAX - 1, "{%s}", buf);

    memset(buf, 0x00, DEV_INFO_LEN_MAX);
    HAL_Snprintf(buf, DEV_INFO_LEN_MAX - 1, WIFI_ACK_FMT, req_msg_id, 200, dev_info);

    HAL_Free(dev_info);

    awss_info("tx msg to app: %s", buf);

    wifi_build_topic(topic_fmt, topic, TOPIC_LEN_MAX);

    ret = wifi_coap_send_resp((uint8_t *)buf, strlen(buf), request, topic, remote, NULL, NULL, 0);

    if (ret != 0) {
        awss_err("tx dev info rsp fail.");
    } else {
        if (type == AWSS_NOTIFY_DEV_RAND_SIGN) {
            dump_awss_status(STATE_WIFI_SENT_DEVINFO_RESP, NULL);

        } else if (type == AWSS_NOTIFY_SUCCESS) {
            dump_awss_status(STATE_WIFI_SENT_CONNECTAP_RESP, NULL);
        }
    }

    HAL_Free(buf);
    return 0;

DEV_INFO_ERR:
    if (buf) {
        HAL_Free(buf);
    }
    if (dev_info) {
        HAL_Free(dev_info);
    }

    return STATE_WIFI_COAP_RSP_INVALID;
}

static int wifi_process_mcast_get_connectap_info(void *ctx, void *resource, void *remote, void *request)
{
    if (p_local_handle == NULL) {
        return STATE_WIFI_COAP_INIT_FAILED;
    }
    return process_get_device_info(ctx, resource, remote, request, 1, AWSS_NOTIFY_SUCCESS);
}

static int wifi_process_ucast_get_connectap_info(void *ctx, void *resource, void *remote, void *request)
{
    if (p_local_handle == NULL) {
        return STATE_WIFI_COAP_INIT_FAILED;
    }
    return process_get_device_info(ctx, resource, remote, request, 0, AWSS_NOTIFY_SUCCESS);
}



static int wifi_coap_send(void *handle, uint8_t *buf, uint32_t len, void *sa, const char *uri, void *cb,
                          uint16_t *msgid)
{
    if (handle == NULL || buf == NULL || uri == NULL || msgid == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    CoAPMessageId_cancel(handle, *msgid);

    return CoAPServerMultiCast_send(handle, (NetworkAddr *)sa, uri, (uint8_t *)buf,
                                    (uint16_t)len, (CoAPSendMsgHandler)cb, msgid);
}


static int wifi_devinfo_notify_resp(void *context, int result,
                                    void *userdata, void *remote,
                                    void *message)
{
    return wifi_notify_response(AWSS_NOTIFY_DEV_RAND_SIGN, result, message);
}

static int wifi_suc_notify_resp(void *context, int result,
                                void *userdata, void *remote,
                                void *message)
{
    return wifi_notify_response(AWSS_NOTIFY_SUCCESS, result, message);
}

static int wifi_notify_response(int type, int result, void *message)
{

    int val = 0;
    uint8_t code, i;
    int len = 0, mlen = 0;
    char *payload = NULL, *elem = NULL;

    if (p_local_handle == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    if (message == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    if (result != 0) {
        awss_err("result = %d", result);
        return STATE_WIFI_COAP_RSP_INVALID;
    }
    code = wifi_get_coap_code(message);
    if (code >= 0x60) {
        awss_err("code:%02x", code);
        return STATE_WIFI_COAP_RSP_INVALID;
    }

    if ((payload = wifi_get_coap_payload(message, &len)) == NULL ||
        len > 0x40 || len == 0) {
        awss_err("payload invalid , len =%d", len);
        return STATE_WIFI_COAP_RSP_INVALID;
    }

    elem = json_get_value_by_name(payload, len, WIFI_JSON_CODE, &mlen, 0);
    if (elem == NULL) {
        awss_err("no code in payload");
        return STATE_WIFI_COAP_RSP_INVALID;
    }
    if (strstr(elem, "200") == NULL) {
        awss_err("code = %s", elem);
        return STATE_WIFI_COAP_RSP_INVALID;
    }


    for (i = 0; i < sizeof(notify_map) / sizeof(notify_map[0]); i ++) {
        if (notify_map[i].notify_type != type) {
            continue;
        }

        p_local_handle->notify_resp[type] = 1;
        break;
    }

    return p_local_handle->notify_resp[type];
}

static uint8_t wifi_get_coap_code(void *request)
{
    struct CoAPMessage *msg = NULL;
    if (request == NULL) {
        return 0x60;
    }
    msg = (struct CoAPMessage *)request;
    return msg->header.code;
}

/* format mac string uppercase */
static char *wifi_get_mac_str(char mac_str[HAL_MAC_LEN])
{
    char *str;
    int colon_num = 0, i;

    str = HAL_Wifi_Get_Mac(mac_str);
    awss_info("mac = %s ", str);
    /* sanity check */
    while (str) {
        str = strchr(str, ':');
        if (str) {
            colon_num ++;
            str ++; /* eating char ':' */
        }
    }

    /* convert to capital letter */
    for (i = 0; i < HAL_MAC_LEN && mac_str[i]; i ++) {
        if ('a' <= mac_str[i] && mac_str[i] <= 'z') {
            mac_str[i] -= 'a' - 'A';
        }
    }

    return mac_str;
}

static void wifi_produce_random(uint8_t *random, uint32_t len)
{
    int i = 0;
    int time = HAL_UptimeMs();
    HAL_Srandom(time);
    for (i = 0; i < len; i ++) {
        random[i] = HAL_Random(0xFF);
    }
}

int wifimgr_process_mcast_get_device_info(void *ctx, void *resource, void *remote, void *request)
{
    return process_get_device_info(ctx, resource, remote, request, 1, AWSS_NOTIFY_DEV_RAND_SIGN);
}

int wifimgr_process_ucast_get_device_info(void *ctx, void *resource, void *remote, void *request)
{
    return process_get_device_info(ctx, resource, remote, request, 0, AWSS_NOTIFY_DEV_RAND_SIGN);
}

int wifi_connectap_notify(void)
{

    if (local_handle.connectap_notify_cnt > 0) {
        uint32_t cur_time;
        if (wifi_got_notify_resp(AWSS_NOTIFY_SUCCESS)) {
            local_handle.connectap_notify_cnt = 0;
            return 0;
        }
        cur_time = HAL_UptimeMs();
        if ((cur_time - local_handle.notify_start_time) > (UINT32_MAX / 2)) {
            return 0;
        }

        wifi_notify_dev_info(AWSS_NOTIFY_SUCCESS);
        local_handle.connectap_notify_cnt--;
        local_handle.notify_start_time = cur_time + WIFI_CONNECTAP_NOTIFY_DURATION;
    }
    return 0;
}

int wifi_start_connectap_notify(void)
{
    local_handle.connectap_notify_cnt = WIFI_CONNECTAP_NOTIFY_CNT_MAX;
    local_handle.notify_start_time = HAL_UptimeMs();
    return 0;
}

#ifndef AWSS_DISABLE_REGISTRAR
extern int registar_yield();
#endif

int wifi_coap_yield(void)
{
    wifi_connectap_notify();
#ifndef AWSS_DISABLE_REGISTRAR
    registar_yield();
#endif
    return 0;
}
#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif
