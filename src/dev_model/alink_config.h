/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#ifndef __ALINK_CONFIG_H__
#define __ALINK_CONFIG_H__

#ifndef CONFIG_ALINK_DEBUG
    #define CONFIG_ALINK_DEBUG              (0)
#endif

#ifndef CONFIG_MQTT_TX_MAXLEN
    #define CONFIG_MQTT_TX_MAXLEN           (1024)
#endif

#ifndef CONFIG_MQTT_RX_MAXLEN
    #define CONFIG_MQTT_RX_MAXLEN           (1024)
#endif

#ifndef CONFIG_SDK_THREAD_COST
    #define CONFIG_SDK_THREAD_COST          (0)
#endif

#define UTILS_HASH_TABLE_ITERATOR_ENABLE    (0)






#endif /* #ifndef __ALINK_CONFIG_H__ */