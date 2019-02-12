LIBA_TARGET     := libiot_alink.a

HDR_REFS        += src/infra
HDR_REFS		+= src/mqtt
HDR_REFS		+= src/dev_sign

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_tls

LIB_SRCS_PATTERN     	:= *.c server/*.c client/*.c

LIB_SRCS_EXCLUDE     	  := examples/linkkit_example_solo.c cJSON.c examples/app_entry.c
SRCS_linkkit-example-solo := examples/linkkit_example_solo.c cJSON.c examples/app_entry.c

LIB_SRCS_EXCLUDE             += examples/linkkit_example_gateway.c examples/app_entry.c
SRCS_linkkit-example-gateway := examples/linkkit_example_gateway.c examples/app_entry.c

$(call Append_Conditional, LIB_SRCS_PATTERN, alcs/*.c, ALCS_ENABLED)

$(call Append_Conditional, TARGET, linkkit-example-solo, DEVICE_MODEL_ENABLED, BUILD_AOS)
$(call Append_Conditional, TARGET, linkkit-example-gateway, DEVICE_MODEL_GATEWAY, BUILD_AOS)

