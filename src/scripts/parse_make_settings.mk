include $(CURDIR)/src/scripts/internal_make_funcs.mk

SETTING_VARS := \
    BUILD_TYPE \
    PLATFORM_CC \
    PLATFORM_AR \
    PLATFORM_OS \
    FEATURE_MQTT_COMM_ENABLED \
    FEATURE_MQTT_DEVICE_SHADOW \
    FEATURE_MQTT_DIRECT \
    FEATURE_MQTT_DIRECT_NOTLS \
    FEATURE_COAP_COMM_ENABLED \
    FEATURE_COAP_DTLS_SUPPORT \
    FEATURE_OTA_ENABLED \
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

ifneq (y,$(strip $(FEATURE_MQTT_COMM_ENABLED)))

    ifneq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))
    $(error Either CoAP or MQTT required to be y!)
    endif

$(foreach V,DEVICE_SHADOW DIRECT DIRECT_NOTLS, \
    $(if $(filter y,$(strip $(FEATURE_MQTT_$(V)))), \
        $(error FEATURE_MQTT_$(V) = y requires FEATURE_MQTT_COMM_ENABLED = y!) \
    ) \
)
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
