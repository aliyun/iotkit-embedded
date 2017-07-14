TARGET      := mqtt-example shadow-example
HDR_REFS    := src

LDFLAGS     := -liot_sdk

ifeq (mbedtls,$(strip $(PLATFORM_SSL)))
DEPENDS     += src/external/recipes/mbedtls
endif
DEPENDS     += src/log

SRCS_mqtt-example   := mqtt/mqtt.c
SRCS_shadow-example := deviceshadow/deviceshadow.c
