TARGET              := mqtt-example mqtt_rrpc-example
SRCS_mqtt-example   := mqtt/mqtt-example.c
SRCS_mqtt_rrpc-example := mqtt/mqtt_rrpc-example.c

ifeq (y, $(strip $(FEATURE_MQTT_DEVICE_SHADOW)))
TARGET              += shadow-example
SRCS_shadow-example := device-shadow/shadow-example.c
endif

# if FEATURE_COAP
TARGET              += coap-example
SRCS_coap-example	:= coap/iotx_coap_client.c
# endif

LDFLAGS     := -liot_sdk

ifeq (y,$(strip $(FEATURE_MQTT_ID2_AUTH)))
LDFLAGS     += -ltfs -lcrypto
endif

# For CoAP
CFLAGS += -w

ifeq (,$(filter -DIOTX_WITHOUT_TLS,$(CFLAGS)))
ifeq (mbedtls,$(strip $(PLATFORM_SSL)))
DEPENDS     += src/external/recipes/mbedtls
endif
endif
