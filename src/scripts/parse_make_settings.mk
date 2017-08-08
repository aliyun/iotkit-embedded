PLATFORM_CC            ?= gcc
PLATFORM_AR            ?= ar
PLATFORM_OS             = linux
PLATFORM_NETWORK        = linuxsock
PLATFORM_SSL            = mbedtls

FEATURE_MQTT_DEVICE_SHADOW  ?= y
FEATURE_COAP_DTLS_SUPPORT   ?= $(FEATURE_COAP_COMM_ENABLED)
FEATURE_MQTT_ID2_AUTH       ?= n
FEATURE_MQTT_ID2_CRYPTO     ?= n

SETTING_VARS := \
    BUILD_TYPE \
    PLATFORM_CC \
    PLATFORM_AR \
    PLATFORM_OS \
    PLATFORM_NETWORK \
    PLATFORM_SSL \
    FEATURE_MQTT_DEVICE_SHADOW \
    FEATURE_MQTT_DIRECT \
    FEATURE_MQTT_DIRECT_NOTLS \
    FEATURE_COAP_COMM_ENABLED \
    FEATURE_COAP_DTLS_SUPPORT \
    FEATURE_MQTT_ID2_AUTH \
    FEATURE_MQTT_ID2_CRYPTO \

$(foreach v, \
    $(SETTING_VARS), \
    $(eval export $(v)=$($(v))) \
)

$(foreach v, \
    $(SETTING_VARS), \
    $(if $(filter y,$($(v))), \
        $(eval CFLAGS += -D$(subst FEATURE_,,$(v)))) \
)

include build-rules/settings.mk
sinclude $(CONFIG_TPL)

ifeq (debug,$(strip $(BUILD_TYPE)))
CFLAGS  += -DIOTX_DEBUG
endif

ifeq (y,$(strip $(FEATURE_MQTT_DIRECT)))

    ifeq (y,$(strip $(FEATURE_MQTT_ID2_AUTH)))
    $(error FEATURE_MQTT_ID2_AUTH + FEATURE_MQTT_DIRECT not implemented!)
    endif

    ifeq (y,$(strip $(FEATURE_MQTT_DIRECT_NOTLS)))
    CFLAGS  += -DIOTX_WITHOUT_TLS
    endif

else    # ifeq (y,$(strip $(FEATURE_MQTT_DIRECT)))

    ifeq (y,$(strip $(FEATURE_MQTT_DIRECT_NOTLS)))
    $(error FEATURE_MQTT_DIRECT_NOTLS = y requires FEATURE_MQTT_DIRECT = y!)
    endif

endif   # ifeq (y,$(strip $(FEATURE_MQTT_DIRECT)))

ifeq (y,$(strip $(FEATURE_MQTT_ID2_AUTH)))

    ifneq (gcc,$(strip $(PLATFORM_CC)))
    $(error FEATURE_MQTT_ID2_AUTH requires PLATFORM_CC equal gcc!)
    endif

else    # ifeq (y,$(strip $(FEATURE_MQTT_ID2_AUTH)))

    ifeq (y,$(strip $(FEATURE_MQTT_ID2_CRYPTO)))
    $(error FEATURE_MQTT_ID2_CRYPTO = y requires FEATURE_MQTT_ID2_AUTH = y!)
    endif

endif   # ifeq (y,$(strip $(FEATURE_MQTT_ID2_AUTH)))

ifeq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))
else    # ifeq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))

    ifeq (y,$(strip $(FEATURE_COAP_DTLS_SUPPORT)))
    $(error FEATURE_COAP_DTLS_SUPPORT = y requires FEATURE_COAP_COMM_ENABLED = y!)
    endif

endif   # ifeq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))

PREFIX_CC := $(shell echo "$(strip $(PLATFORM_CC))"|sed 's:gcc$$::1')
PREFIX_AR := $(shell echo "$(strip $(PLATFORM_AR))"|sed 's:ar$$::1')

ifneq ($(strip $(PREFIX_AR)),$(strip $(PREFIX_CC)))
$(error $(strip $(PREFIX_AR)) != $(strip $(PREFIX_CC)), AR/CC requires same prefix!)
endif

CROSS_PREFIX        := $(shell echo "$(strip $(PLATFORM_CC))"|sed 's:gcc$$::1')
