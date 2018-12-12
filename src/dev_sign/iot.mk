LIBA_TARGET     := libiot_auth.a

HDR_REFS        := src/infra

DEPENDS += wrappers external_libs/mbedtls src/infra certs
LDFLAGS += -liot_hal -liot_tls -liot_infra -liot_cert

TARGET := dev-sign-example
SRC_dev-sign-example += dev_sign_example.c