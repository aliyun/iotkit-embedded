LIBA_TARGET := libiot_tls.a
CFLAGS      := $(filter-out -ansi,$(CFLAGS))

PKG_SOURCE  := mbedtls-in-iotkit.git
