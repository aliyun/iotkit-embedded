LIBA_TARGET := libiot_mqtt.a

HDR_REFS        := src/infra

DEPENDS         += external_libs/refs wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_tls

TARGET          := mqtt-example

LIB_SRCS_EXCLUDE        := mqtt_example.c
SRCS_mqtt-example       += mqtt_example.c