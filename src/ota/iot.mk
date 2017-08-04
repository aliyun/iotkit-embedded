LIBA_TARGET := libota.a
HDR_REFS    := src
LIB_SRCS    := ota.c 

# TODO: fix warnings
CFLAGS := $(filter-out -Werror,$(CFLAGS))

# PKG_UPDATE  := 'git@gitlab.alibaba-inc.com:iot-middleware/iot-coap-c.git'
