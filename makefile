include make.settings
include src/configs/default_settings.mk
include src/scripts/parse_make_settings.mk

SUBDIRS := sample
SUBDIRS += src/sdk-tests

CFLAGS  += -Os
CFLAGS  += -DTEST_ID2_DAILY
# CFLAGS  += -DTEST_OTA_PRE
# CFLAGS  += -DINSPECT_MQTT_FLOW

COMP_LIB            := libiot_sdk.a
COMP_LIB_COMPONENTS := \
    src/log \
    src/utils \
    src/security \
    src/sdk-impl \
    src/guider \
    src/mqtt \
    src/ota \
    src/system \
    src/platform \

ifeq (y,$(strip $(FEATURE_MQTT_DEVICE_SHADOW)))
COMP_LIB_COMPONENTS += src/shadow
endif

ifeq (y,$(strip $(FEATURE_COAP_COMM_ENABLED)))
COMP_LIB_COMPONENTS += src/coap
endif

COVERAGE_CMD    := $(SCRIPT_DIR)/walk_through_examples.sh
BUILD_CONFIG    := src/configs/config.desktop.x86

ifneq (gcc,$(strip $(PLATFORM_CC)))
BUILD_CONFIG    := src/configs/config.generic-linux.embedded
endif

include $(RULE_DIR)/rules.mk
