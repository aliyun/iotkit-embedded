/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_UPSTREAM__
#define __ALINK_UPSTREAM__

#include "iotx_alink_internal.h"


int alink_upstream_thing_property_post_req(char *pk, char *dn, uint8_t *payload, uint16_t len);


int alink_upstream_thing_property_set_rsp(char *pk, char *dn, uint8_t *payload, uint16_t len);



#endif /* #ifndef __ALINK_UPSTREAM__ */