/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef __INFRA_STATE_H__
#define __INFRA_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define STATE_BASE                                (0x0000)

/* General: 0x0000 ~ 0x00FF */
#define STATE_SUCCESS                             (STATE_BASE - 0x0000)

/* ProductKey from SDK user is NULL or too long */
#define STATE_INVALID_PK                          (STATE_BASE - 0x0001)
/* ProductSecret from SDK user is NULL or too long */
#define STATE_INVALID_PS                          (STATE_BASE - 0x0002)
/* DeviceName from SDK user is NULL or too long */
#define STATE_INVALID_DN                          (STATE_BASE - 0x0003)
/* DeviceSecret from SDK user is NULL or too long */

#define STATE_INVALID_DS                          (STATE_BASE - 0x0004)
/* General: 0x0000 ~ 0x00FF */

/* Utils: 0x0100 ~ 0x01FF */
#define STATE_UTILS_BASE                          (-0x0100)

/* Utils: 0x0100 ~ 0x01FF */

/* System: 0x0200 ~ 0x02FF */
#define STATE_SYS_BASE                            (-0x0200)

/* System: 0x0200 ~ 0x02FF */

/* MQTT: 0x0300 ~ 0x03FF */
#define STATE_MQTT_BASE                           (-0x0300)

/* Deserialized CONNACK from MQTT server says protocol version is unacceptable */
#define STATE_MQTT_CONNACK_VERSION_UNACCEPT       (STATE_MQTT_BASE - 0x0001)
/* Deserialized CONNACK from MQTT server says identifier is rejected */
#define STATE_MQTT_CONNACK_IDENT_REJECT           (STATE_MQTT_BASE - 0x0002)
/* Deserialized CONNACK from MQTT server says service is not available */
#define STATE_MQTT_CONNACK_SERVICE_NA             (STATE_MQTT_BASE - 0x0003)
/* Deserialized CONNACK from MQTT server says username/password is invalid */
#define STATE_MQTT_CONNACK_BAD_USERDATA           (STATE_MQTT_BASE - 0x0005)

/* MQTT: 0x0300 ~ 0x03FF */

/* WiFi Provision: 0x0400 ~ 0x04FF */
#define STATE_WIFI_BASE                           (-0x0400)

/* WiFi Provision: 0x0400 ~ 0x04FF */

/* COAP: 0x0500 ~ 0x05FF */
#define STATE_COAP_BASE                           (-0x0500)

/* COAP: 0x0500 ~ 0x05FF */

/* HTTP: 0x0600 ~ 0x06FF */
#define STATE_HTTP_BASE                           (-0x0600)

/* HTTP: 0x0600 ~ 0x06FF */

/* OTA: 0x0700 ~ 0x07FF */
#define STATE_OTA_BASE                            (-0x0700)

/* OTA: 0x0700 ~ 0x07FF */

/* Bind: 0x0800 ~ 0x08FF */
#define STATE_BIND_BASE                           (-0x0800)

/* Bind: 0x0800 ~ 0x08FF */

/* Device Model: 0x0900 ~ 0x09FF */
#define STATE_DEV_MODEL_BASE                      (-0x0900)

/* Device Model: 0x0900 ~ 0x09FF */

/* SubDevice Mgmt: 0x0A00 ~ 0x0AFF */
#define STATE_SUBDEV_MGMT_BASE                    (-0x0A00)

/* SubDevice Mgmt: 0x0A00 ~ 0x0AFF */

#ifdef __cplusplus
}
#endif
#endif

