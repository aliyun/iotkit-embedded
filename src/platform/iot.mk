LIBA_TARGET := libiot_platform.a
LIB_SRCS    := $(wildcard *.c)

LIB_SRCS    += $(wildcard os/$(CONFIG_VENDOR)/*.c)
LIB_SRCS    += $(wildcard ssl/mbedtls/*.c)

HDR_REFS    += src/sdk-impl

CFLAGS      := $(filter-out -ansi,$(CFLAGS))
