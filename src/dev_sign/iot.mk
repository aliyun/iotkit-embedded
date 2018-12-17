LIBA_TARGET     := libiot_auth.a

HDR_REFS        := src/infra

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal

TARGET          := dev-sign-example

LIB_SRCS_EXCLUDE        := dev_sign_example.c
SRCS_dev-sign-example   += dev_sign_example.c
