LIBA_TARGET := libiot_tls.a
CFLAGS      := $(filter-out -ansi,$(CFLAGS))

LIB_SRCS    := $(wildcard mbedtls-in-iotkit/library/*.c)
LIB_HEADERS := $(wildcard mbedtls-in-iotkit/include/mbedtls/*.h)

PKG_SOURCE  := src/packages/mbedtls-in-iotkit.git
