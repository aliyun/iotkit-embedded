LIBA_TARGET         := libiot_mqtt.a
HDR_REFS            := src/infra
LIB_SRCS_PATTERN    := *.c

SRCS_mqtt-example       := examples/mqtt_example.c
SRCS_mqtt-example-at    := examples/mqtt_example_at.c

$(call Append_Conditional, LIB_SRCS_PATTERN, mqtt_impl/*.c, MQTT_DEFAULT_IMPL)
$(call Append_Conditional, TARGET, mqtt-example, MQTT_COMM_ENABLED, ATM_ENABLED)
$(call Append_Conditional, TARGET, mqtt-example-at, ATM_ENABLED)

LDFLAGS         += -liot_sdk -liot_hal -liot_tls
