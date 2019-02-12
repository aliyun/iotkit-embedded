LIBA_TARGET     := libiot_reset.a

HDR_REFS        := src/infra src/mqtt

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_tls

TARGET          := dev-reset-example

LIB_SRCS_EXCLUDE         += dev_reset_example.c
SRCS_dev-reset-example   := dev_reset_example.c