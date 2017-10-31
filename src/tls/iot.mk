LIBA_TARGET := libiot-tls.a
LIB_SRCS    := $(wildcard mbedtls-in-iotkit/library/*.c)
CFLAGS      := $(filter-out -ansi,$(CFLAGS))

PKG_SOURCE  := src/packages/mbedtls-in-iotkit.git
