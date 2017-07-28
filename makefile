include make.settings
include src/scripts/parse_make_settings.mk

CFLAGS  += -Os -Wall -Werror
CFLAGS  += -DTEST_ID2_DAILY
CFLAGS  += -DINSPECT_MQTT_FLOW

COMP_LIB            := libiot_sdk.a
COMP_LIB_COMPONENTS := \
    src/log \
    src/utils \
    src/security \
    src/sdk-impl \
    src/guider \
    src/mqtt \
    src/coap \
    src/system \
    src/platform \

ifeq (y,$(strip $(FEATURE_MQTT_DEVICE_SHADOW)))
COMP_LIB_COMPONENTS += src/shadow
endif

SUBDIRS := sample
SUBDIRS += src/sdk-tests

COVERAGE_CMD    := $(SCRIPT_DIR)/walk_through_examples.sh
BUILD_CONFIG    := src/configs/config.desktop.x86

include $(RULE_DIR)/rules.mk
