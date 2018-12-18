LIBA_TARGET     := libiot_reset.a

HDR_REFS        := src/infra src/mqtt

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal

TARGET          := dev-reset-example

LIB_SRCS_EXCLUDE        := dev_reset_example.c
SRCS_dev-sign-example   += dev_reset_example.c