LIBA_TARGET     := libiot_cmp.a

CFLAGS          += -DCMP_SUPPORT_TOPIC_DISPATCH

HDR_REFS        += src/infra
HDR_REFS        += src/protocol/mqtt/Link-MQTT
