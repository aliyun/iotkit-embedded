LIBA_TARGET := libcoap.a
HDR_REFS    := src
CFLAGS      := $(filter-out -ansi,$(CFLAGS))

DEPENDS     += src/external/mbedtls

PKG_SOURCE  := src/packages/iot-coap-c
# PKG_UPDATE  := 'git@gitlab.alibaba-inc.com:iot-middleware/iot-coap-c.git'
