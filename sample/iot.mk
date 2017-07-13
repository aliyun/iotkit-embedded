TARGET      := mqtt-example
SRCS        := mqtt/mqtt.c
HDR_REFS    := src

LDFLAGS     := -liot_sdk

ifeq (mbedtls,$(strip $(PLATFORM_SSL)))
DEPENDS     += src/external/recipes/mbedtls
endif
