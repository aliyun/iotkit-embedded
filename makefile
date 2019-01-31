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

$(call CompLib_Map, FEATURE_MQTT_COMM_ENABLED, src/mqtt)
$(call CompLib_Map, FEATURE_SUPPORT_TLS, certs)
$(call CompLib_Map, FEATURE_DYNAMIC_REGISTER, src/dynamic_register)
$(call CompLib_Map, FEATURE_ATM_ENABLED, src/atm)
$(call CompLib_Map, FEATURE_DEV_RESET, src/dev_reset)
$(call CompLib_Map, FEATURE_OTA_ENABLED, src/ota)
$(call CompLib_Map, FEATURE_DEVICE_MODEL_ENABLED, src/dev_model)

SUBDIRS                 += wrappers
SUBDIRS                 += external_libs/mbedtls
SUBDIRS                 += tests

include $(RULE_DIR)/rules.mk

