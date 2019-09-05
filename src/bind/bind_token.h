/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __BIND_TOKEN_H__
#define __BIND_TOKEN_H__

int bind_refresh_token(void *handle);

int bind_complete_token(uint8_t *token_in, uint8_t token_len, char passwd[HAL_MAX_PASSWD_LEN], uint8_t *bssid,
                        uint8_t bssid_len,
                        uint8_t *token_out);
#endif
