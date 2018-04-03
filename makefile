include project.mk
include make.settings
include src/configs/default_settings.mk
include src/scripts/parse_make_settings.mk

# CFLAGS  += -DTEST_MQTT_DAILY
# CFLAGS  += -DTEST_HTTP_DAILY
# CFLAGS  += -DTEST_COAP_DAILY
# CFLAGS  += -DTEST_OTA_PRE
# CFLAGS  += -DINSPECT_MQTT_FLOW

include src/scripts/mock_build_options.mk

COMP_LIB            := libiot_sdk.a
COMP_LIB_COMPONENTS := \
    src/utils \
    src/log \
    src/system \

$(call CompLib_Map, MQTT_COMM_ENABLED, \
    src/mqtt \
)
$(call CompLib_Map, OTA_ENABLED, src/ota)
$(call CompLib_Map, MQTT_SHADOW, src/shadow)
$(call CompLib_Map, COAP_COMM_ENABLED, src/coap)
$(call CompLib_Map, MQTT_ID2_AUTH, src/tfs)
$(call CompLib_Map, HTTP_COMM_ENABLED, src/http)
$(call CompLib_Map, SUBDEVICE_ENABLED, src/subdev)
$(call CompLib_Map, CLOUD_CONN_ENABLED, src/cloud_conn)
$(call CompLib_Map, CMP_ENABLED, src/cmp)
$(call CompLib_Map, DM_ENABLED, src/dm)
$(call CompLib_Map, SERVICE_OTA_ENABLED, src/fota)
$(call CompLib_Map, SERVICE_OTA_ENABLED, src/cota)
include $(RULE_DIR)/rules.mk
