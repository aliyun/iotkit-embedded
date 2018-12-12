LIBA_TARGET     := libiot_auth.a

HDR_REFS        := src/infra

DEPENDS += wrappers external_libs/references src/infra certs
LDFLAGS += -liot_sdk -liot_hal -liot_refs

TARGET := dev-sign-example
SRC_dev-sign-example += dev_sign_example.c
