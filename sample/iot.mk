ifeq (y, $(strip $(FEATURE_MQTT_DEVICE_SHADOW)))
TARGET              := mqtt-example shadow-example coap-example
SRCS_mqtt-example   := mqtt/mqtt-example.c
SRCS_shadow-example := device-shadow/shadow-example.c
SRCS_coap-example	:= coap/iotx_coap_client.c
else
TARGET      := mqtt-example
SRCS        := mqtt/mqtt-example.c
endif

LDFLAGS     := -liot_sdk

ifeq (y,$(strip $(FEATURE_MQTT_ID2_AUTH)))
LDFLAGS     += -ltfs -lcrypto
endif

CFLAGS += -w

ifeq (,$(filter -DIOTX_WITHOUT_TLS,$(CFLAGS)))
ifeq (mbedtls,$(strip $(PLATFORM_SSL)))
DEPENDS     += src/external/recipes/mbedtls
endif
endif
