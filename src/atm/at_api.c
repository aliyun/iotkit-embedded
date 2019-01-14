/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include "infra_config.h"

int at_conn_init(void);
int at_parser_init(void);

int IOT_ATM_Init(void)
{
#ifdef AT_PARSER_ENABLED
    if (at_parser_init() < 0) {
    	return -1;
    }
#endif

#ifdef AT_TCP_ENABLED
    if (at_conn_init() < 0) {
    	return -1;
    }
#endif

    return 0;
}
