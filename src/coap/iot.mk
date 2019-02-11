LIBA_TARGET         := libiot_coap.a
HDR_REFS            := src/infra
LIB_SRCS_PATTERN    := *.c

# SRCS_mqtt-example       := examples/mqtt_example.c
# SRCS_mqtt-example-at    := examples/mqtt_example_at.c

$(call Append_Conditional, LIB_SRCS_PATTERN, client/*.c, COAP_COMM_ENABLED)
$(call Append_Conditional, LIB_SRCS_PATTERN, CoAPPacket/*.c, COAP_COMM_ENABLED)
$(call Append_Conditional, LIB_SRCS_PATTERN, CoAPPacket/*.c, ALCS_ENABLED)
$(call Append_Conditional, LIB_SRCS_PATTERN, server/*.c, ALCS_ENABLED)
# $(call Append_Conditional, TARGET, mqtt-example, MQTT_COMM_ENABLED, ATM_ENABLED BUILD_AOS)
# $(call Append_Conditional, TARGET, mqtt-example-at, ATM_ENABLED BUILD_AOS)

LDFLAGS         += -liot_sdk -liot_hal -liot_tls
