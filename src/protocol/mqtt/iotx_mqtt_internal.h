#ifndef __IOTX_MQTT_INTERNAL_H__
#define __IOTX_MQTT_INTERNAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iot_import.h"
#include "iot_export_mqtt.h"
#include "iotx_mqtt.h"

#define MQTT_DYNBUF_MARGIN                      (64)
#define MQTT_CONNECT_REQUIRED_BUFLEN            (256)

#if WITH_MQTT_DYNBUF
#define RESET_SERIALIZE_BUF(cli, b, s)          do { \
        LITE_free(cli->b); \
        cli->b = NULL; \
        cli->s = 0; \
        mqtt_debug("FREED: curr buf_send = %p, curr buf_size_send = %d", cli->b, cli->s); \
    } while (0)

#define ALLOC_SERIALIZE_BUF(cli, b, s, l, n)    do { \
        int     tmpbuf_len = 0; \
        \
        tmpbuf_len = l + MQTT_DYNBUF_MARGIN; \
        mqtt_debug("START: orig buf_send = %p, orig buf_size_send = %d, required payload_len = %d", cli->b, cli->s, l); \
        cli->b = LITE_malloc(tmpbuf_len); \
        if (NULL == cli->b) { \
            mqtt_err("Unable to allocate %d bytes for '%s', abort!", tmpbuf_len, (n)?(n):""); \
            return ERROR_NO_MEM; \
        } \
        cli->buf_size_send = tmpbuf_len; \
        mqtt_debug("ALLOC: curr buf_send = %p, curr buf_size_send = %d", cli->buf_send, cli->buf_size_send); \
    } while (0)

#else
#define ALLOC_SERIALIZE_BUF(cli, b, s, l, n)
#define RESET_SERIALIZE_BUF(cli, b, s)
#endif

/* MQTT send publish packet */

#endif  /* __IOTX_MQTT_INTERNAL_H__ */
