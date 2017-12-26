LIBA_TARGET     := libiot_ota.a
LIB_SRCS        := $(wildcard Link-OTA/ota.c)
HDR_REFS        := src
DEPENDS         := src/log src/utils src/coap src/mqtt

PKG_SOURCE      := Link-OTA.git
PKG_UPSTREAM    := git@gitlab.alibaba-inc.com:iot-middleware/Link-OTA.git

