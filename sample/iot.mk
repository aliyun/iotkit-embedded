ifeq (y, $(strip $(FEATURE_MQTT_DEVICE_SHADOW)))
TARGET              := mqtt-example shadow-example
SRCS_mqtt-example   := mqtt/mqtt-example.c
SRCS_shadow-example := device-shadow/shadow-example.c
else
TARGET      := mqtt-example
SRCS        := mqtt/mqtt-example.c
endif

LDFLAGS     := -liot_sdk

ifeq (y,$(strip $(FEATURE_MQTT_ID2_AUTH)))
LDFLAGS     += -ltfs -lcrypto
endif

ifeq (,$(filter -DIOTX_WITHOUT_TLS,$(CFLAGS)))
ifeq (mbedtls,$(strip $(PLATFORM_SSL)))
DEPENDS     += src/external/recipes/mbedtls
endif
endif
