LIBA_TARGET     := libiot_alink.a

HDR_REFS        += src/infra
HDR_REFS		+= src/mqtt
HDR_REFS		+= src/dev_sign

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_tls

LIB_SRCS_PATTERN     	:= *.c */*.c

TARGET          		:= alink-example-solo
LIB_SRCS_EXCLUDE     	:= examples/alink_example_solo.c examples/alink_example_gateway.c
SRCS_alink-example-solo   	+= examples/alink_example_solo.c
SRCS_alink-example-gateway	+= examples/alink_example_gateway.c

$(call Append_Conditional, TARGET, alink-example-gateway, DEVICE_MODEL_GATEWAY)

