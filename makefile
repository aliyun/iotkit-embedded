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

COMP_LIB            := libiot_sdk.a
COMP_LIB_COMPONENTS := \
    src/auth \
    src/mqtt \
    src/system \
    src/utility \
    src/platform \

SUBDIRS := sample

BUILD_CONFIG    := src/configs/config.x86.desktop
include $(RULE_DIR)/rules.mk
