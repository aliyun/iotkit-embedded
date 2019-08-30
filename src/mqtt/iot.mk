LIBA_TARGET         := libiot_mqtt.a
HDR_REFS            := src/infra
LIB_SRCS_PATTERN    := *.c

SRCS_mqtt-example       := examples/mqtt_example.c $(TOP_DIR)/extension/state_desc.c
SRCS_mqtt-example-auto  := examples/mqtt_example_auto.c examples/cJSON.c
SRCS_mqtt-task-example  := examples/mqtt_task_example.c examples/cJSON.c
SRCS_mqtt-example-at    := examples/mqtt_example_at.c

$(call Append_Conditional, LIB_SRCS_PATTERN, impl/*.c, MQTT_DEFAULT_IMPL)
$(call Append_Conditional, TARGET, mqtt-example, MQTT_COMM_ENABLED, ATM_ENABLED BUILD_AOS NO_EXECUTABLES)
$(call Append_Conditional, TARGET, mqtt-example-auto, MQTT_COMM_ENABLED, ATM_ENABLED BUILD_AOS NO_EXECUTABLES)
$(call Append_Conditional, TARGET, mqtt-task-example, MQTT_COMM_ENABLED, ATM_ENABLED BUILD_AOS NO_EXECUTABLES)
$(call Append_Conditional, TARGET, mqtt-example-at, ATM_ENABLED, BUILD_AOS NO_EXECUTABLES)

DEPENDS         += wrappers external_libs/mbedtls
LDFLAGS         += -liot_sdk -liot_hal -liot_tls
