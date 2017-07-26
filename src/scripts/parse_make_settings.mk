PLATFORM_CC             = gcc
PLATFORM_AR             = ar
PLATFORM_OS             = linux
PLATFORM_NETWORK        = linuxsock
PLATFORM_SSL            = mbedtls

SETTING_VARS := \
    BUILD_TYPE \
    PLATFORM_CC \
    PLATFORM_AR \
    PLATFORM_OS \
    PLATFORM_NETWORK \
    PLATFORM_SSL \
    FEATURE_DIRECT_MQTT \
    FEATURE_DIRECT_MQTT_NOTLS \
    FEATURE_ID2_AUTH \
    FEATURE_ID2_CRYPTO \

$(foreach v, \
    $(SETTING_VARS), \
    $(eval export $(v)=$($(v))) \
)

include build-rules/settings.mk
sinclude $(CONFIG_TPL)

ifeq (debug,$(strip $(BUILD_TYPE)))
CFLAGS  += -DIOTX_DEBUG
endif

ifeq (y,$(strip $(FEATURE_DIRECT_MQTT)))
ifeq (y,$(strip $(FEATURE_ID2_AUTH)))
$(error FEATURE_ID2_AUTH + FEATURE_DIRECT_MQTT not implemented!)
endif

CFLAGS  += -DDIRECT_MQTT
ifeq (y,$(strip $(FEATURE_DIRECT_MQTT_NOTLS)))
CFLAGS  += -DIOTX_WITHOUT_TLS
endif

else    # ifeq (y,$(strip $(FEATURE_DIRECT_MQTT)))

ifeq (y,$(strip $(FEATURE_DIRECT_MQTT_NOTLS)))
$(error FEATURE_DIRECT_MQTT_NOTLS = y requires FEATURE_DIRECT_MQTT = y!)
endif

endif   # ifeq (y,$(strip $(FEATURE_DIRECT_MQTT)))

ifeq (y,$(strip $(FEATURE_ID2_AUTH)))

CFLAGS  += -DID2_AUTH
ifeq (y,$(strip $(FEATURE_ID2_CRYPTO)))
CFLAGS  += -DID2_CRYPTO
endif

else    # ifeq (y,$(strip $(FEATURE_ID2_AUTH)))

ifeq (y,$(strip $(FEATURE_ID2_CRYPTO)))
$(error FEATURE_ID2_CRYPTO = y requires FEATURE_ID2_AUTH = y!)
endif

endif   # ifeq (y,$(strip $(FEATURE_ID2_AUTH)))

ifneq ($(subst gcc,,$(PLATFORM_CC)),$(subst ar,,$(PLATFORM_AR)))
$(error PLATFORM_CC and PLATFORM_AR requires same prefix!)
endif

CROSS_PREFIX        := $(shell echo "$(strip $(PLATFORM_CC))"|sed 's:gcc::1')
