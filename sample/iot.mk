TARGET      := mqtt-example
SRCS        := mqtt/mqtt.c
HDR_REFS    := src

LDFLAGS     := -liot_sdk

ifeq (mbedtls,$(strip $(PLATFORM_SSL)))
LDFLAGS     += -lmbedtls -lmbedcrypto -lmbedx509
endif
