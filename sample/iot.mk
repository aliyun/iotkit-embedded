TARGET              := mqtt-example mqtt_rrpc-example
SRCS_mqtt-example   := mqtt/mqtt-example.c
SRCS_mqtt_rrpc-example := mqtt/mqtt_rrpc-example.c

ifeq (y, $(strip $(FEATURE_MQTT_DEVICE_SHADOW)))
TARGET              += shadow-example
SRCS_shadow-example := device-shadow/shadow-example.c
endif

LDFLAGS     := -liot_sdk

ifeq (y,$(strip $(FEATURE_COAP_COMM)))
TARGET              += coap-example
SRCS_coap-example	:= coap/iotx_coap_client.c
DEPENDS             += src/external/recipes/mbedtls
CFLAGS              := $(filter-out -Werror,$(CFLAGS))
endif

ifeq (y,$(strip $(FEATURE_MQTT_ID2_AUTH)))
LDFLAGS     += -ltfs -lcrypto
endif

ifeq (,$(filter -DIOTX_WITHOUT_TLS,$(CFLAGS)))
DEPENDS     += src/external/recipes/mbedtls
endif
