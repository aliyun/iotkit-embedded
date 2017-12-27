LIBA_TARGET     := libiot_mqtt.a
LIB_SRCS        := $(wildcard Link-MQTT/*.c Link-MQTT/*/*.c)
HDR_REFS        := src src/system/iotkit-system
DEPENDS         := src/log src/utils

PKG_SOURCE      := Link-MQTT.git
PKG_UPSTREAM    := git@gitlab.alibaba-inc.com:iot-middleware/Link-MQTT.git
