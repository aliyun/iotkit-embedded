/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_CONFIG_H__
#define __ALINK_CONFIG_H__

#include "infra_defs.h"

#ifndef CONFIG_ALINK_DEBUG
    #define CONFIG_ALINK_DEBUG              (0)
#endif

#ifndef CONFIG_MQTT_TX_MAXLEN
    #define CONFIG_MQTT_TX_MAXLEN           (1024)
#endif

#ifndef CONFIG_MQTT_RX_MAXLEN
    #define CONFIG_MQTT_RX_MAXLEN           (1024)
#endif

#define CONFIG_CONNECTION_REGION            IOTX_CLOUD_REGION_SHANGHAI

#define CONFIG_SUBDEV_HASH_TABLE_SZIE       (17)    /* 17 19 23 29 31 37 47 59 73 89 97 113 131 149..., must < 2000 */


#endif /* #ifndef __ALINK_CONFIG_H__ */

