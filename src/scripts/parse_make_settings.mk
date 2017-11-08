include $(CURDIR)/src/scripts/internal_make_funcs.mk

SWITCH_VARS := \
    FEATURE_MQTT_COMM_ENABLED \
    FEATURE_MQTT_SHADOW \
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

include build-rules/settings.mk
sinclude $(CONFIG_TPL)

ifeq (,$(filter reconfig distclean,$(MAKECMDGOALS)))
ifneq (HTTP,$(strip $(FEATURE_OTA_FETCH_CHANNEL)))
$(error FEATURE_OTA_FETCH_CHANNEL must be HTTP!)
endif

ifneq (y,$(strip $(FEATURE_MQTT_COMM_ENABLED)))

    ifneq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))
    $(error Either CoAP or MQTT required to be y!)
    endif

$(foreach V,SHADOW DIRECT DIRECT_NOTLS, \
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

    ifneq (gcc,$(strip $(CC)))
    $(error FEATURE_MQTT_ID2_AUTH requires $(CC) equal gcc!)
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
endif

SUBDIRS += src/tls
SUBDIRS += src/platform
SUBDIRS += sample
SUBDIRS += src/sdk-tests
