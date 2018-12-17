LIBA_TARGET     := libiot_alink.a

HDR_REFS        := src/infra 
HDR_REFS		+= src/mqtt
HDR_REFS		+= src/dev_sign

DEPENDS         += external_libs/refs wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_refs -liot_tls -liot_mqtt

TARGET          := alink-example

LIB_SRCS_PATTERN     := *.c
LIB_SRCS_EXCLUDE     := alink_example.c
SRCS_alink-example   += alink_example.c