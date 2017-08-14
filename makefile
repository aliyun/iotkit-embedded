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
    src/sdk-impl \

$(call CompLib_Map, MQTT_COMM_ENABLED, \
    src/guider \
    src/mqtt \
    src/system \
)
$(call CompLib_Map, OTA_ENABLED, src/ota)
$(call CompLib_Map, MQTT_DEVICE_SHADOW, src/shadow)
$(call CompLib_Map, COAP_COMM_ENABLED, src/coap)
$(call CompLib_Map, MQTT_ID2_AUTH, src/security)

COVERAGE_CMD    := $(SCRIPT_DIR)/walk_through_examples.sh
BUILD_CONFIG    := src/configs/config.desktop.x86

POST_FINAL_OUT_HOOK := Post_Distro

ifneq (gcc,$(strip $(PLATFORM_CC)))
BUILD_CONFIG    := src/configs/config.generic-linux.embedded
endif

include $(RULE_DIR)/rules.mk
