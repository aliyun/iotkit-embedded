/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */
#include <stdlib.h>
#include "CoAPExport.h"
#include "CoAPServer.h"
#include "bind_msg.h"
#include "bind_utils.h"
#include "bind_token.h"
#include "infra_state.h"
#include "wrappers.h"


#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

extern bind_context_t *g_context;

static void bind_coap_get_token_mcast(void *ctx, const char *path, NetworkAddr *remote, CoAPMessage *request);
static void bind_coap_get_token_ucast(void *ctx, const char *path, NetworkAddr *remote, CoAPMessage *request);
static int bind_coap_get_token_resp(void *ctx, const char *path, NetworkAddr *remote, CoAPMessage *request, int type);
static int bind_coap_send(void *handle, uint8_t *buf, uint32_t len, void *sa, const char *uri, void *cb,
                          uint16_t *msgid);
static int bind_coap_register(void *handle, const char *topic, void *cb);
static int bind_coap_send_resp(void *context, void *remote, unsigned char *buff, uint16_t len, void *req,
                               const char *paths, void *callback, uint16_t *msgid, char qos);
static int bind_parse_notify_response(int result, void *message);
static int bind_get_notify_resp(void *context, int result, void *userdata, void *remote, void *message);

int bind_coap_init(void *handle, void *coap_handle)
{
    int ret;
    char buff[BIND_TOPIC_BUFFER_LEN] = {0};
    bind_context_t *bind_handle = (bind_context_t *)handle;
    if (bind_handle == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }
    if (coap_handle == NULL) {
        bind_handle->coap_handle = (void *)CoAPServer_init();
    }
    if (bind_handle->coap_handle == NULL) {
        iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_COAP_INIT_FAIL, NULL);
        return STATE_BIND_COAP_INIT_FAIL;
    }

    ret = bind_coap_register(bind_handle, BIND_TOPIC_DEVINFO_MCAST, bind_coap_get_token_mcast);
    if (ret < 0) {
        iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_COAP_REGISTER_FAILED, BIND_TOPIC_DEVINFO_MCAST);
    }

    bind_get_topic(BIND_ENUM_TOPIC_DEVINFO_UCAST, buff, BIND_TOPIC_BUFFER_LEN);
    ret = bind_coap_register(bind_handle, buff, bind_coap_get_token_ucast);
    if (ret < 0) {
        iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_COAP_REGISTER_FAILED, (const char *)buff);
    }

    return ret;
}

int bind_coap_deinit(void *handle)
{

    bind_context_t *bind_handle = (bind_context_t *)handle;
    if (bind_handle == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    if (bind_handle->coap_handle) {
        CoAPServer_deinit(bind_handle->coap_handle);
        bind_handle->coap_handle = NULL;
    }
    return 0;
}

int bind_coap_notify(void *handle)
{
    int ret = 0;

    char *buf = NULL;
    char *dev_info = NULL;
    bind_context_t *bind_handle = (bind_context_t *)handle;
    bind_netaddr_t notify_sa;
    if (bind_handle == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    buf = bind_zalloc(BIND_COAP_MSG_LEN);
    if (buf == NULL) {
        bind_err("alloc mem fail");
        return STATE_SYS_DEPEND_MALLOC;
    }
    dev_info = bind_zalloc(BIND_MSG_BUFFER_LEN);
    if (dev_info == NULL) {
        bind_err("alloc mem fail");
        bind_free(buf);
        return STATE_SYS_DEPEND_MALLOC;
    }
    memset(&notify_sa, 0, sizeof(notify_sa));
    /*多个地址循环notify*/
    bind_get_broadcast_addr(&notify_sa);
    bind_info("bcast ip = %s\n", notify_sa.host);
    bind_build_dev_info(bind_handle, dev_info, BIND_MSG_BUFFER_LEN);

    HAL_Snprintf(buf, BIND_COAP_MSG_LEN - 1, BIND_DEV_NOTIFY_FMT, bind_handle->coap_msg_id, dev_info);
    bind_debug("topic:%s\n", BIND_TOPIC_NOTIFY);
    bind_debug("payload:%s\n", buf);
    /*cb now is null,may need add*/
    ret = bind_coap_send(bind_handle->coap_handle, (uint8_t *)buf, strlen(buf), &notify_sa, BIND_TOPIC_NOTIFY,
                         bind_get_notify_resp,
                         &bind_handle->coap_msg_id);
    if(ret >= 0) {
        char rand_str[(BIND_TOKEN_LEN << 1) + 1] = {0};
        LITE_hexbuf_convert(g_context->token,  rand_str, BIND_TOKEN_LEN,  1);
        iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_NOTIFY_TOKEN_SENT, rand_str);
    }
    if (buf) {
        bind_free(buf);
    }
    if (dev_info) {
        bind_free(dev_info);
    }

    return 0;
}

int bind_coap_register(void *handle, const char *topic, void *cb)
{
    bind_context_t *bind_handle = (bind_context_t *)handle;
    if (handle == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }
    CoAPServer_register(bind_handle->coap_handle, (const char *)topic, (CoAPRecvMsgHandler)cb);
    return 0;
}
static void bind_coap_get_token_mcast(void *ctx, const char *path, NetworkAddr *remote, CoAPMessage *request)
{
    int ret;
    ret = bind_coap_get_token_resp(ctx, path, remote, request, 1);
    if (ret < 0) {
        iotx_state_event(ITE_STATE_DEV_BIND, ret, NULL);
    }
}

static void bind_coap_get_token_ucast(void *ctx, const char *path, NetworkAddr *remote, CoAPMessage *request)
{
    int ret;
    ret = bind_coap_get_token_resp(ctx, path, remote, request, 0);
    if (ret < 0) {
        iotx_state_event(ITE_STATE_DEV_BIND, ret, NULL);
    }

}


static int bind_coap_get_token_resp(void *ctx, const char *path, NetworkAddr *remote, CoAPMessage *request, int type)
{
    int ret = 0;
    int id_len, data_len;
    char *id, *data;
    char req_msg_id[BIND_ID_LEN] = {0};
    char topic[BIND_TOPIC_BUFFER_LEN] = {0};
    if (g_context == NULL || request == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_RECV_TOKEN_QUERY, (const char *)request->payload);

    ret = bind_req_payload_parser((const char *)request->payload, request->payloadlen, &id, &id_len, &data, &data_len);

    if (ret < 0) {
        return ret;
    }

    if (id_len > BIND_ID_LEN) {
        return STATE_BIND_COAP_REQ_INVALID;
    }

    memcpy(req_msg_id, id, id_len);

    if (type) {
        bind_get_topic(BIND_ENUM_TOPIC_DEVINFO_MCAST, topic, BIND_TOPIC_BUFFER_LEN);
    } else {
        bind_get_topic(BIND_ENUM_TOPIC_DEVINFO_UCAST, topic, BIND_TOPIC_BUFFER_LEN);
    }

    ret = bind_time_is_expired(g_context->token_exp_time);
    if (ret) {
        HAL_MutexLock(g_context->lock);
        g_context->report_cnt = 0;
        bind_set_step(g_context, BIND_ENUM_STEP_REPORTING_TOKEN);
        HAL_MutexUnlock(g_context->lock);
    }
    bind_info("req msg = %s", request->payload);
    if (g_context->step == BIND_ENUM_STEP_REPORTED_TOKEN) { /*resp token*/
        char buf[BIND_COAP_MSG_LEN];
        int len2;
        ret = HAL_Snprintf(buf, BIND_COAP_MSG_LEN - 1, BIND_COAP_MSG_RESP_FMT, req_msg_id, 200);

        len2 = bind_build_dev_info(g_context, buf + ret, BIND_COAP_MSG_LEN - ret - 1);
        if (ret <= 0) {
            iotx_state_event(ITE_STATE_DEV_BIND, ret, NULL);
            return ret;
        }
        strcpy(buf + len2 + ret, "}");
        bind_info("send msg = %s", buf);

        ret = bind_coap_send_resp(g_context->coap_handle, remote, (uint8_t *)buf, strlen(buf), request, path, NULL, NULL, 0);
        HAL_MutexLock(g_context->lock);
        g_context->report_cnt = 0;
        bind_set_step(g_context, BIND_ENUM_STEP_REPORTING_TOKEN);
        HAL_MutexUnlock(g_context->lock);

    } else { /*resp err */
        char buf[BIND_COAP_MSG_LEN / 2];
        ret = HAL_Snprintf(buf, BIND_COAP_MSG_LEN - 1, BIND_COAP_MSG_RESP_FMT, req_msg_id, 400);

        strcpy(buf + ret, "{}}");
        bind_info("send msg = %s", buf);

        ret = bind_coap_send_resp(g_context->coap_handle, remote, (uint8_t *)buf, strlen(buf), request, path, NULL, NULL, 0);

    }
    return ret;
}

static int bind_coap_send(void *handle, uint8_t *buf, uint32_t len, void *sa, const char *uri, void *cb,
                          uint16_t *msgid)
{
    if (handle == NULL || buf == NULL || uri == NULL || msgid == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    CoAPMessageId_cancel(handle, *msgid);

    return CoAPServerMultiCast_send(handle, (NetworkAddr *)sa, uri, (uint8_t *)buf,
                                    (uint16_t)len, (CoAPSendMsgHandler)cb, msgid);
}

static uint8_t bind_get_coap_code(void *request)
{
    struct CoAPMessage *msg = NULL;
    if (request == NULL) {
        return 0x60;
    }
    msg = (struct CoAPMessage *)request;
    return msg->header.code;
}

static char *bind_get_coap_payload(void *request, int *payload_len)
{
    struct CoAPMessage *msg = (struct CoAPMessage *)request;
    if (request == NULL) {
        return NULL;
    }

    msg = (struct CoAPMessage *)request;
    if (payload_len) {
        *payload_len = msg->payloadlen;
    }
    bind_debug("payload:%s\r\n", msg->payload);
    return (char *)msg->payload;
}

static int bind_coap_send_resp(void *context, void *remote, unsigned char *buff, uint16_t len, void *req,
                               const char *paths, void *callback, uint16_t *msgid, char qos)
{
    if (context == NULL) {
        return STATE_USER_INPUT_NULL_POINTER; /*must be err code  */
    }

    iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_SENT_TOKEN_RESP, (const char *)buff);

    return CoAPServerResp_send(context, (NetworkAddr *)remote, (uint8_t *)buff, (uint16_t)len, req, paths,
                               (CoAPSendMsgHandler)callback, msgid, qos);
}

static int bind_parse_notify_response(int result, void *message)
{

    int val = 0;
    uint8_t code;
    int len = 0, mlen = 0;
    char *payload = NULL, *elem = NULL;

    bind_info("bind_parse_notify_response");
    if (message == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    if (result != 0) {
        bind_err("result = %d", result);
        return STATE_BIND_COAP_RSP_INVALID;
    }
    code = bind_get_coap_code(message);
    if (code >= 0x60) {
        bind_err("code:%02x", code);
        return STATE_BIND_COAP_RSP_INVALID;
    }

    if ((payload = bind_get_coap_payload(message, &len)) == NULL ||
        len > 0x40 || len == 0) {
        bind_err("payload invalid , len =%d", len);
        return STATE_BIND_COAP_RSP_INVALID;
    }

    elem = json_get_value_by_name(payload, len, BIND_JSON_CODE, &mlen, 0);
    if (elem == NULL) {
        bind_err("no code in payload");
        return STATE_BIND_COAP_RSP_INVALID;
    }
    if (strstr(elem, "200") == NULL) {
        bind_err("code = %s", elem);
        return STATE_BIND_COAP_RSP_INVALID;
    }

    return 0;
}

static int bind_get_notify_resp(void *context, int result, void *userdata, void *remote, void *message)
{
    int res = bind_parse_notify_response(result, message);
    bind_debug("res :%04x\r\n", res);
    if (res == 0) {
        HAL_MutexLock(g_context->lock);
        g_context->report_cnt = 0;
        bind_time_countdown_ms(&g_context->token_exp_time, BIND_TOKEN_LIFE);
        bind_set_step(g_context, BIND_ENUM_STEP_REPORTING_TOKEN);
        bind_refresh_token(g_context);
        HAL_MutexUnlock(g_context->lock);
    }
    return res;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif
