TARGET      := mqtt-example shadow-example
HDR_REFS    := src

LDFLAGS     := -liot_sdk

ifeq (y,$(strip $(FEATURE_EQUIP_ID2)))
LDFLAGS     += -ltfs -lcrypto
endif

ifeq (,$(filter -DIOTX_MQTT_TCP,$(CFLAGS)))
ifeq (mbedtls,$(strip $(PLATFORM_SSL)))
DEPENDS     += src/external/recipes/mbedtls
endif
endif

SRCS_mqtt-example   := mqtt/mqtt.c
SRCS_shadow-example := deviceshadow/deviceshadow.c
