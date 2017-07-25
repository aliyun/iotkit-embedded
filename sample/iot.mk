TARGET      := mqtt-example shadow-example
HDR_REFS    := src

LDFLAGS     := -liot_sdk

ifeq (y,$(strip $(FEATURE_ID2_AUTH)))
LDFLAGS     += -ltfs -lcrypto
endif

ifeq (,$(filter -DIOTX_WITHOUT_TLS,$(CFLAGS)))
ifeq (mbedtls,$(strip $(PLATFORM_SSL)))
DEPENDS     += src/external/recipes/mbedtls
endif
endif

SRCS_mqtt-example   := mqtt/mqtt.c
SRCS_shadow-example := deviceshadow/deviceshadow.c
