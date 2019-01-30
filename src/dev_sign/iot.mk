LIBA_TARGET     := libiot_auth.a

HDR_REFS        := src/infra

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_tls

TARGET          := dev-sign-example

LIB_SRCS_EXCLUDE        := examples/dev_sign_example.c
SRCS_dev-sign-example   += examples/dev_sign_example.c
