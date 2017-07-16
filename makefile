include make.settings
include src/scripts/parse_make_settings.mk

COMP_LIB            := libiot_sdk.a
COMP_LIB_COMPONENTS := \
    src/log \
    src/utils \
    src/auth \
    src/shadow \
    src/mqtt \
    src/system \
    src/utility \
    src/platform \

SUBDIRS := sample

COVERAGE_CMD    := $(SCRIPT_DIR)/walk_through_examples.sh
BUILD_CONFIG    := src/configs/config.desktop.x86
include $(RULE_DIR)/rules.mk
