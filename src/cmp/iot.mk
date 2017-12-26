LIBA_TARGET := libiot_cmp.a
HDR_REFS    := src

LIBA_TARGET     := libiot_cmp.a
LIB_SRCS        := $(wildcard Link-CMP/*.c)
HDR_REFS        := src
DEPENDS         := src/log src/utils  src/mqtt  src/cloud_conn  src/coap  src/http 

PKG_SOURCE      := Link-CMP.git
PKG_UPSTREAM    := git@gitlab.alibaba-inc.com:iot-middleware/Link-CMP.git

