include project.mk
include make.settings
include src/board/default_settings.mk
include src/tools/parse_make_settings.mk

# CFLAGS  += -DTEST_MQTT_DAILY
# CFLAGS  += -DTEST_HTTP_DAILY
# CFLAGS  += -DTEST_COAP_DAILY
# CFLAGS  += -DTEST_OTA_PRE
# CFLAGS  += -DINSPECT_MQTT_FLOW
# CFLAGS    += -DENABLE_THING_DEBUG
include src/tools/mock_build_options.mk

COMP_LIB            := libiot_sdk.a
COMP_LIB_COMPONENTS := \
    src/utils \
    src/system \
    src/framework/services/linkkit \

$(call CompLib_Map, MQTT_COMM_ENABLED, \
    src/framework/connectivity/mqtt \
)
$(call CompLib_Map, OTA_ENABLED, src/framework/services/ota)
$(call CompLib_Map, MQTT_SHADOW, src/framework/services/shadow)
$(call CompLib_Map, COAP_COMM_ENABLED, src/framework/services/coap)
$(call CompLib_Map, MQTT_ID2_AUTH, src/system/tfs)
$(call CompLib_Map, HTTP_COMM_ENABLED, src/framework/connectivity/http)
$(call CompLib_Map, SUBDEVICE_ENABLED, src/framework/services/subdev)
$(call CompLib_Map, CLOUD_CONN_ENABLED, src/framework/connectivity/cloud_conn)
$(call CompLib_Map, CMP_ENABLED, src/framework/cm)
$(call CompLib_Map, DM_ENABLED, src/framework/dm)
$(call CompLib_Map, SERVICE_OTA_ENABLED, src/framework/services/fota)
$(call CompLib_Map, SERVICE_OTA_ENABLED, src/framework/services/cota)

include $(RULE_DIR)/rules.mk
