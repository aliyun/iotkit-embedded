LIBA_TARGET := libcoap.a
HDR_REFS    := src

DEPENDS     += src/external/mbedtls

# TODO: fix warnings
CFLAGS := $(filter-out -Werror,$(CFLAGS))

PKG_SOURCE  := src/packages/iot-coap-c
# PKG_UPDATE  := 'git@gitlab.alibaba-inc.com:iot-middleware/iot-coap-c.git'
