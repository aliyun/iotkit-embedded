LIBA_TARGET     := libiot_ota.a

HDR_REFS        := src/infra

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_tls

TARGET          := ota-example

LIB_SRCS_EXCLUDE        := ota_example.c
SRCS_dev-sign-example   += ota_example.c