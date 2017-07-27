LIBA_TARGET := libcoap.a
HDR_REFS    := src

ifeq (mbedtls,$(strip $(PLATFORM_SSL)))
DEPENDS     += src/external/recipes/mbedtls
endif

# TODO: fix warnings
CFLAGS := $(filter-out -Werror,$(CFLAGS))

PKG_SOURCE  := src/packages/iot-coap-c
# PKG_UPDATE  := 'git@gitlab.alibaba-inc.com:iot-middleware/iot-coap-c.git'
