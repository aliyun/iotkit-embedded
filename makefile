include tools/project.mk
include make.settings
include tools/default_settings.mk
include tools/parse_make_settings.mk
include $(RULE_DIR)/funcs.mk

# CFLAGS  += -DINSPECT_MQTT_FLOW
# CFLAGS  += -DINSPECT_MQTT_LIST

COMP_LIB            := libiot_sdk.a
COMP_LIB_COMPONENTS := \
    src/infra \
    src/dev_sign \

SUBDIRS                 += wrappers
SUBDIRS                 += external_libs/mbedtls
SUBDIRS                 += external_libs/references

CFLAGS += $(foreach V,$(shell find external_libs -type d),-I$(V))

include $(RULE_DIR)/rules.mk
