LIBA_TARGET     := libiot_cmp.a
HDR_REFS        := src

CFLAGS          += -DCMP_SUPPORT_TOPIC_DISPATCH \

PKG_SOURCE      := Link-CMP.git
PKG_UPSTREAM    := git@gitlab.alibaba-inc.com:iot-middleware/Link-CMP.git
