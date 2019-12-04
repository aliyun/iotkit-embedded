LIBA_TARGET         := libiot_shadow.a
HDR_REFS            += src/infra
LIB_SRCS_PATTERN    := *.c

DEPENDS         += wrappers 

SRCS_shadow-example       := examples/shadow_example.c

$(call Append_Conditional, TARGET, shadow-example, MQTT_SHADOW, ATM_ENABLED BUILD_AOS NO_EXECUTABLES)

DEPENDS         += wrappers external_libs/mbedtls
LDFLAGS         += -liot_sdk -liot_hal -liot_tls