LIBA_TARGET     := libiot_alink.a

HDR_REFS        += src/infra
HDR_REFS		+= src/mqtt
HDR_REFS		+= src/dev_sign

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_tls

LIB_SRCS_PATTERN     	:= *.c server/*.c client/*.c

TARGET          		:= linkkit-example-solo
LIB_SRCS_EXCLUDE     	:= examples/linkkit_example_solo.c
SRCS_linkkit-example-solo   	+= examples/linkkit_example_solo.c
