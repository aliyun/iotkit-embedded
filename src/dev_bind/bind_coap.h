/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __BIND_COAP_H__
#define __BIND_COAP_H__

int bind_coap_init(void *bind_handle, void *coap_handle);
int bind_coap_notify(void *bind_handle);
int bind_coap_deinit(void *bind_handle);
#endif
