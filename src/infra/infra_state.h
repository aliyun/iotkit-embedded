/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __INFRA_STATE_H__
#define __INFRA_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define STATE_BASE                                  (0x0000)

/* General: 0x0000 ~ 0x00FF */
#define STATE_SUCCESS                               (STATE_BASE - 0x0000)

/* General: 0x0000 ~ 0x00FF */

/* User Input: 0x0100 ~ 0x01FF */
#define STATE_USER_INPUT_BASE                       (-0x0100)

/* User Input: 0x0100 ~ 0x01FF */

/* System: 0x0200 ~ 0x02FF */
#define STATE_SYS_DEPEND_BASE                       (-0x0200)
#define STATE_SYS_DEPEND_MALLOC                     (STATE_SYS_DEPEND_BASE - 0x0001)
#define STATE_SYS_DEPEND_LOCK_CREATE                (STATE_SYS_DEPEND_BASE - 0x0002)
#define STATE_SYS_DEPEND_LOCK_RELEASE               (STATE_SYS_DEPEND_BASE - 0x0003)
#define STATE_SYS_DEPEND_KV_GET                     (STATE_SYS_DEPEND_BASE - 0x0004)
#define STATE_SYS_DEPEND_KV_SET                     (STATE_SYS_DEPEND_BASE - 0x0005)
#define STATE_SYS_DEPEND_KV_DELETE                  (STATE_SYS_DEPEND_BASE - 0x0006)
#define STATE_SYS_DEPEND_TIMER_CREATE               (STATE_SYS_DEPEND_BASE - 0x0007)
#define STATE_SYS_DEPEND_TIMER_START                (STATE_SYS_DEPEND_BASE - 0x0008)
#define STATE_SYS_DEPEND_TIMER_STOP                 (STATE_SYS_DEPEND_BASE - 0x0009)
#define STATE_SYS_DEPEND_TIMER_DELETE               (STATE_SYS_DEPEND_BASE - 0x000A)

/* System: 0x0200 ~ 0x02FF */

/* MQTT: 0x0300 ~ 0x03FF */
#define STATE_MQTT_BASE                             (-0x0300)

/* Deserialized CONNACK from MQTT server says protocol version is unacceptable */
#define STATE_MQTT_CONNACK_VERSION_UNACCEPT         (STATE_MQTT_BASE - 0x0001)
/* Deserialized CONNACK from MQTT server says identifier is rejected */
#define STATE_MQTT_CONNACK_IDENT_REJECT             (STATE_MQTT_BASE - 0x0002)
/* Deserialized CONNACK from MQTT server says service is not available */
#define STATE_MQTT_CONNACK_SERVICE_NA               (STATE_MQTT_BASE - 0x0003)
/* Deserialized CONNACK from MQTT server says username/password is invalid */
#define STATE_MQTT_CONNACK_BAD_USERDATA             (STATE_MQTT_BASE - 0x0005)

/* MQTT: 0x0300 ~ 0x03FF */

/* WiFi Provision: 0x0400 ~ 0x04FF */
#define STATE_WIFI_BASE                             (-0x0400)

/* WiFi Provision: 0x0400 ~ 0x04FF */

/* COAP: 0x0500 ~ 0x05FF */
#define STATE_COAP_BASE                             (-0x0500)

/* COAP: 0x0500 ~ 0x05FF */

/* HTTP: 0x0600 ~ 0x06FF */
#define STATE_HTTP_BASE                             (-0x0600)

/* HTTP: 0x0600 ~ 0x06FF */

/* OTA: 0x0700 ~ 0x07FF */
#define STATE_OTA_BASE                              (-0x0700)

/* OTA: 0x0700 ~ 0x07FF */

/* Bind: 0x0800 ~ 0x08FF */
#define STATE_BIND_BASE                             (-0x0800)

/* Bind: 0x0800 ~ 0x08FF */

/* Device Model: 0x0900 ~ 0x09FF */
#define STATE_DEV_MODEL_BASE                        (-0x0900)

/* Device Model: 0x0900 ~ 0x09FF */

/* SubDevice Mgmt: 0x0A00 ~ 0x0AFF */
#define STATE_SUB_DEVICE_BASE                       (-0x0A00)

/* SubDevice Mgmt: 0x0A00 ~ 0x0AFF */

#ifdef __cplusplus
}
#endif
#endif  /* __INFRA_STATE_H__ */

