/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */
#include <string.h>
#include "infra_sha256.h"
#include "coap_api.h"
#include "infra_state.h"
#include "bind_msg.h"
#include "wrappers.h"
#include "infra_string.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

static uint8_t app_token[BIND_TOKEN_LEN] = {0};

static int bind_complete_token(uint8_t *token_in, uint8_t token_len, char passwd[HAL_MAX_PASSWD_LEN], uint8_t *bssid,
                               uint8_t bssid_len,
                               uint8_t token_out[BIND_TOKEN_LEN])
{
    /*need to complete the token*/
    int ret = STATE_SUCCESS;
    if (token_len != 0 && token_len < 16 && token_in != NULL) {
        int org_token_len = 0;
        unsigned char buff[128] = {0};
        unsigned char gen_token[32] = {0};

        uint8_t pwd_len = 0;
        if (passwd != NULL) {
            pwd_len = strlen(passwd);
        }

        if (bssid != NULL) {
            memcpy(buff + org_token_len, bssid, bssid_len);
            org_token_len += bssid_len;
        }

        memcpy(buff + org_token_len, token_in, token_len);
        org_token_len += token_len;

        if (pwd_len != 0 && 128 >= pwd_len + org_token_len) {
            memcpy(buff + org_token_len, passwd, pwd_len);
            org_token_len += pwd_len;
        }

        utils_sha256(buff, org_token_len, gen_token);
        memcpy(token_out, gen_token, BIND_TOKEN_LEN);

    } else if (token_len == BIND_TOKEN_LEN && token_in != NULL) {
        memcpy(token_out, token_in, BIND_TOKEN_LEN);
    } else {
        ret = STATE_BIND_ASSEMBLE_APP_TOKEN_FAILED;
    }

    return ret;
}

int bind_refresh_token(void *handle)
{
    int i = 0;
    uint32_t time;
    bind_context_t *bind_context = (bind_context_t *)handle;

    if (handle == NULL) {
        return STATE_USER_INPUT_NULL_POINTER;
    }
    for (i = 0; i < BIND_TOKEN_LEN; i++) {
        if (app_token[i] != 0) {
            break;
        }
    }

    /*using pre_init token, eg:app token*/
    if (i < BIND_TOKEN_LEN) {
        memcpy(bind_context->token, app_token, BIND_TOKEN_LEN);
        memset(app_token, 0, BIND_TOKEN_LEN);
    } else {
        time = HAL_UptimeMs();
        HAL_Srandom(time);
        for (i = 0; i < BIND_TOKEN_LEN; i ++) {
            bind_context->token[i] = HAL_Random(0xFF);
        }
    }
    bind_time_countdown_ms(&bind_context->token_exp_time, BIND_TOKEN_LIFE);
    return 0;
}

int bind_set_token(void *handle, uint8_t *token, uint8_t token_len, char *passwd, uint8_t *bssid, uint8_t bssid_len)
{
    int ret;
    uint8_t *p_token;
    bind_context_t *bind_context = (bind_context_t *)handle;

    if (token == NULL || token_len == 0) {
        return STATE_USER_INPUT_NULL_POINTER;
    }

    if (handle == NULL) {
        p_token = app_token;
    } else {
        p_token = bind_context->token;
    }

    ret = bind_complete_token(token, token_len, passwd, bssid, bssid_len, p_token);

    if (ret < 0) {
        iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_ASSEMBLE_APP_TOKEN_FAILED, NULL);
    } else {
        char rand_str[(BIND_TOKEN_LEN << 1) + 1] = {0};
        LITE_hexbuf_convert(p_token,  rand_str, BIND_TOKEN_LEN,  1);
        iotx_state_event(ITE_STATE_DEV_BIND, STATE_BIND_SET_APP_TOKEN, rand_str);
    }
    return ret;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif
