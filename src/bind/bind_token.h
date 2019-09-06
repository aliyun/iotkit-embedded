/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __BIND_TOKEN_H__
#define __BIND_TOKEN_H__

int bind_refresh_token(void *handle);
int bind_set_token(void *handle, uint8_t *token, uint8_t token_len, char *passwd, uint8_t *bssid, uint8_t bssid_len);
#endif
