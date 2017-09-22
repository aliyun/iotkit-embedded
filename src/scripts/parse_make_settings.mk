include $(CURDIR)/src/scripts/internal_make_funcs.mk

SETTING_VARS := \
    BUILD_TYPE \
    PLATFORM_CC \
    PLATFORM_AR \
    PLATFORM_OS \

SWITCH_VARS := \
    FEATURE_MQTT_COMM_ENABLED \
    FEATURE_MQTT_DEVICE_SHADOW \
    FEATURE_MQTT_DIRECT \
    FEATURE_MQTT_DIRECT_NOTLS \
    FEATURE_COAP_COMM_ENABLED \
    FEATURE_COAP_DTLS_SUPPORT \
    FEATURE_OTA_ENABLED \
    FEATURE_OTA_SIGNAL_CHANNEL \
    FEATURE_OTA_FETCH_CHANNEL \
    FEATURE_MQTT_ID2_AUTH \
    FEATURE_MQTT_ID2_CRYPTO \
    FEATURE_MQTT_ID2_ENV \
    FEATURE_HTTP_COMM_ENABLED \

$(foreach v, \
    $(SETTING_VARS) $(SWITCH_VARS), \
    $(eval export $(v)=$($(v))) \
)

$(foreach v, \
    $(SWITCH_VARS), \
    $(if $(filter y,$($(v))), \
        $(eval CFLAGS += -D$(subst FEATURE_,,$(v)))) \
)

ifeq (y,$(strip $(FEATURE_OTA_ENABLED)))

ifeq (MQTT,$(strip $(FEATURE_OTA_SIGNAL_CHANNEL)))
ifneq (y,$(strip $(FEATURE_MQTT_COMM_ENABLED)))
$(error FEATURE_OTA_SIGNAL_CHANNEL = MQTT requires FEATURE_MQTT_COMM_ENABLED = y!)
endif
CFLAGS += -DOTA_SIGNAL_CHANNEL=1
else
ifeq (COAP,$(strip $(FEATURE_OTA_SIGNAL_CHANNEL)))
ifneq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))
$(error FEATURE_OTA_SIGNAL_CHANNEL = COAP requires FEATURE_COAP_COMM_ENABLED = y!)
endif
CFLAGS += -DOTA_SIGNAL_CHANNEL=2
else
ifeq (HTTP,$(strip $(FEATURE_OTA_SIGNAL_CHANNEL)))
CFLAGS += -DOTA_SIGNAL_CHANNEL=4
else
$(error FEATURE_OTA_SIGNAL_CHANNEL must be MQTT or COAP or HTTP!)
endif # HTTP
endif # COAP
endif # MQTT

endif # OTA Enabled

ifneq (HTTP,$(strip $(FEATURE_OTA_FETCH_CHANNEL)))
$(error FEATURE_OTA_FETCH_CHANNEL must be HTTP!)
endif

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

    ifeq (y,$(strip $(FEATURE_MQTT_DIRECT_NOTLS)))
    CFLAGS  += -DIOTX_WITHOUT_TLS
    endif

    ifeq (y,$(strip $(FEATURE_MQTT_ID2_CRYPTO)))
    $(error FEATURE_MQTT_ID2_CRYPTO + FEATURE_MQTT_DIRECT is not supported!)
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

ifeq (pre,$(strip $(FEATURE_MQTT_ID2_ENV)))
CFLAGS  += -DTEST_ID2_PRE
else
ifeq (daily,$(strip $(FEATURE_MQTT_ID2_ENV)))
CFLAGS  += -DTEST_ID2_DAILY
endif
endif

OVERRIDE_CC := $(strip $(PLATFORM_CC))
OVERRIDE_AR := $(strip $(PLATFORM_AR))
OVERRIDE_STRIP := $(strip $(PLATFORM_STRIP))
