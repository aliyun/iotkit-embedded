LIBA_TARGET := libiot_platform.a
HDR_REFS    += src/sdk-impl src/external/tls
CFLAGS      := $(filter-out -ansi,$(CFLAGS))
DEPENDS		+= src/external/tls

LIB_SRCS_PATTERN    += os/$(CONFIG_VENDOR)/*.c
LIB_SRCS_PATTERN    += ssl/mbedtls/*.c
