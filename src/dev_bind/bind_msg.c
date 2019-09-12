/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */
#include <memory.h>
#include "mqtt_api.h"
#include "bind_utils.h"
#include "bind_msg.h"
#include "infra_json_parser.h"
#include "infra_state.h"
#include "infra_string.h"
#include "infra_compat.h"
#include "wrappers.h"

void bind_set_step(void *context, bind_steps_type step)
{
    bind_context_t *ct = (bind_context_t *)context;
    if (context == NULL) {
        return;
    }
    ct->step = step;
    /*schedule immediately*/
    ct->sched_time = HAL_UptimeMs();
}


int bind_get_topic(bind_topic_type type, char *buff, int buff_len)
{
    char dn[IOTX_DEVICE_NAME_LEN + 1] = {0};
    char pk[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    int len = -1;
    if (buff == NULL || buff_len <= 0) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, pk);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_NAME, dn);

    switch (type) {
        case BIND_ENUM_TOPIC_MATCH:
            len = HAL_Snprintf(buff, buff_len - 1, BIND_TOPIC_TOKEN_FMT, pk, dn);
            break;
        case BIND_ENUM_TOPIC_MATCH_REPLY:
            len = HAL_Snprintf(buff, buff_len - 1, BIND_TOPIC_TOKEN_REPY_FMT, pk, dn);
            break;
        case BIND_ENUM_TOPIC_RESET:
            len = HAL_Snprintf(buff, buff_len - 1, BIND_TOPIC_RESET_FMT, pk, dn);
            break;
        case BIND_ENUM_TOPIC_RESET_REPLY:
            len = HAL_Snprintf(buff, buff_len - 1, BIND_TOPIC_RESET_REPY_FMT, pk, dn);
            break;
        case BIND_ENUM_TOPIC_EVENT:
            len = HAL_Snprintf(buff, buff_len - 1, BIND_TOPIC_EVENT_FMT, pk, dn);
            break;
        case BIND_ENUM_TOPIC_EVENT_REPLY:
            len = HAL_Snprintf(buff, buff_len - 1, BIND_TOPIC_EVENT_REPLY_FMT, pk, dn);
            break;
        case BIND_ENUM_TOPIC_DEVINFO_MCAST:
            strncpy(buff, BIND_TOPIC_DEVINFO_MCAST, buff_len - 1);
            len = strlen(buff);
            break;
        case BIND_ENUM_TOPIC_DEVINFO_UCAST:
            len = HAL_Snprintf(buff, buff_len - 1, BIND_TOPIC_DEVINFO_FMT, pk, dn);
            break;
        default:
            break;
    }
    return len;
}


int bind_req_payload_parser(const char *payload, int len, char **id, int *id_len, char **data, int *datalen)
{
    if (!payload || !len || !id || !id_len || !datalen || !data) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    *id = json_get_value_by_name((char *)payload, len, "id", id_len, NULL);
    if (*id == NULL || *id_len == 0 || *id_len > BIND_UINT32_STRLEN) {
        return STATE_BIND_COAP_REQ_INVALID;
    }
    *data = json_get_value_by_name((char *)payload, len, "params", datalen, NULL);
    if (*data == NULL || datalen == 0) {
        return STATE_BIND_COAP_REQ_INVALID;
    }
    return 0;
}

/* format mac string uppercase */
char *bind_get_mac_str(char mac_str[HAL_MAC_LEN])
{
    char *str;
    int colon_num = 0, i;

    str = HAL_Wifi_Get_Mac(mac_str);
    bind_info("mac = %s ", str);
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


int bind_build_dev_info(bind_context_t *handle, void *dev_info, int info_len)
{
    int len = 0;
    char dev_name[IOTX_DEVICE_NAME_LEN + 1] = {0};
    char mac_str[HAL_MAC_LEN + 1] = {0};
    char pk[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char ip_str[BIND_IP_ADDR_LEN + 1] = {0};
    char rand_str[(BIND_TOKEN_LEN << 1) + 1] = {0};
    uint32_t remain_time;
    bind_context_t *bind_context = (bind_context_t *)handle;
    if (dev_info == NULL || info_len <= 0 || handle == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    IOT_Ioctl(IOTX_IOCTL_GET_PRODUCT_KEY, pk);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_NAME, dev_name);
    bind_get_mac_str(mac_str);
    HAL_Wifi_Get_IP(ip_str, NULL);
    if (bind_time_is_expired(bind_context->token_exp_time)) {
        return STATE_BIND_TOKEN_EXPIRED;
    }
    remain_time =  bind_context->token_exp_time - (uint32_t)HAL_UptimeMs();

    LITE_hexbuf_convert(bind_context->token,  rand_str, BIND_TOKEN_LEN,  1);

    len = HAL_Snprintf(dev_info, info_len - 1, BIND_DEVINFO_DATA_FMT, BIND_AWSS_VER, pk, dev_name, mac_str, ip_str,
                       BIND_CRYPTO_LEVEL, rand_str, remain_time, 0);

    return len;
}

/**
 * @brief alternate bcast addr
 *
 * @param bcast_addr[out] bcast addr output
 * @param mask_level 0 ~ 4(255), 1 ~ 3(255), 2 ~ 2(255), 3 ~ 1(255), others ~ invalid,
 */
int bind_get_broadcast_addr(bind_netaddr_t *bcast_addr)
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
    bcast_addr->port = BIND_NOTIFY_PORT;

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

    memcpy(bcast_addr->host, BIND_NOTIFY_HOST, strlen(BIND_NOTIFY_HOST));
    return 0;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif
