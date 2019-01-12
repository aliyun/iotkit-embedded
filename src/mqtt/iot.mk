LIBA_TARGET := libiot_mqtt.a

HDR_REFS        := src/infra

LIB_SRCS_PATTERN := *.c

ifneq (,$(filter -DMQTT_DEFAULT_IMPL,$(CFLAGS)))
LIB_SRCS_PATTERN += mqtt_impl/*.c mqtt_impl/*/*.c
endif


ifneq (,$(filter -DATPARSER_ENABLED,$(CFLAGS)))
LDFLAGS         += -liot_at -liot_sdk -liot_hal -liot_tls
else
LDFLAGS         += -liot_sdk -liot_hal -liot_tls
endif

TARGET          := mqtt-example mqtt-example-report

LIB_SRCS_EXCLUDE        := mqtt_example.c mqtt_example_at.c mqtt_example_report

ifneq (,$(filter -DSAL_ENABLED -DMAL_ENABLED, $(CFLAGS)))
SRCS_mqtt-example       += mqtt_example_at.c
else
SRCS_mqtt-example       += mqtt_example.c
endif

SRCS_mqtt-example-report	+= mqtt_example_report.c
