DEPENDS             := src/platform
LDFLAGS             := -liot_sdk
LDFLAGS             += -liot_platform

ifeq (y,$(strip $(FEATURE_MQTT_COMM_ENABLED)))
TARGET              += mqtt-example mqtt_rrpc-example
SRCS_mqtt-example   := mqtt/mqtt-example.c
SRCS_mqtt_rrpc-example := mqtt/mqtt_rrpc-example.c

    ifeq (y, $(strip $(FEATURE_OTA_ENABLED)))
    TARGET                += ota_mqtt-example
    SRCS_ota_mqtt-example := ota/ota_mqtt-example.c
    endif

    ifeq (y, $(strip $(FEATURE_MQTT_DEVICE_SHADOW)))
    TARGET              += shadow-example
    SRCS_shadow-example := device-shadow/shadow-example.c
    endif

    ifeq (y,$(strip $(FEATURE_MQTT_ID2_AUTH)))
    LDFLAGS     += -ltfs -lmbedcrypto
    endif

endif

ifeq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))
TARGET              += coap-example

    ifeq (y,$(strip $(FEATURE_MQTT_COMM_ENABLED)))
    SRCS_coap-example	:= coap/iotx_coap_client.c
    else
    SRCS                := coap/iotx_coap_client.c
    endif

DEPENDS             += src/external/mbedtls
endif

ifeq (,$(filter -DIOTX_WITHOUT_TLS,$(CFLAGS)))
DEPENDS             += src/external/mbedtls
endif
