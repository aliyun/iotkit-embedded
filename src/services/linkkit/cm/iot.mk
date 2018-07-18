LIBA_TARGET     := libiot_cmp.a
HDR_REFS        += src/infra/utils
HDR_REFS        += src/infra/log
HDR_REFS        += src/services/linkkit/cm/inc
HDR_REFS        += src/protocol/mqtt/Link-MQTT

CFLAGS          += -DCMP_SUPPORT_TOPIC_DISPATCH \

