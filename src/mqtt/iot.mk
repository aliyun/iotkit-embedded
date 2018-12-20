LIBA_TARGET := libiot_mqtt.a

HDR_REFS        := src/infra

DEPENDS         += external_libs/refs hal
LIB_SRCS_PATTERN := *.c

ifneq (,$(filter -DMQTT_DEFAULT_IMPL,$(CFLAGS)))
LIB_SRCS_PATTERN += mqtt_impl/*.c mqtt_impl/*/*.c
endif

LDFLAGS         += -liot_sdk -liot_hal -liot_tls

TARGET          := mqtt-example

LIB_SRCS_EXCLUDE        := mqtt_example.c mqtt_example_at.c

ifneq (,$(filter -DSAL_ENABLED -DMAL_ENABLED, $(CFLAGS)))
SRCS_mqtt-example       += mqtt_example_at.c
else
SRCS_mqtt-example       += mqtt_example.c
endif