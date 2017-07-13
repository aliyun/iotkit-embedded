include make.settings

SETTING_VARS := \
    BUILD_TYPE \
    PLATFORM_CC \
    PLATFORM_AR \
    PLATFORM_OS \
    PLATFORM_NETWORK \
    PLATFORM_SSL \
    FEATURE_DIRECT_MQTT

$(foreach v, \
    $(SETTING_VARS), \
    $(eval export $(v)=$($(v))) \
)

include build-rules/settings.mk
sinclude $(CONFIG_TPL)

CFLAGS  := -Os -Wall # -Werror

ifeq (debug,$(strip $(BUILD_TYPE)))
CFLAGS  += -DALIOT_DEBUG
endif
ifeq (y,$(strip $(FEATURE_DIRECT_MQTT)))
CFLAGS  += -DDIRECT_MQTT
endif

CROSS_PREFIX        := $(shell echo "$(strip $(PLATFORM_CC))"|sed 's:gcc::1')

COMP_LIB            := libiot_sdk.a
COMP_LIB_COMPONENTS := \
    src/auth \
    src/mqtt \
    src/system \
    src/utility \
    src/platform \

SUBDIRS := sample

BUILD_CONFIG    := src/configs/config.desktop.x86
include $(RULE_DIR)/rules.mk
