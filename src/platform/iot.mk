LIBA_TARGET := libiot_platform.a
HDR_REFS    += src/sdk-impl src/tls
CFLAGS      := $(filter-out -ansi,$(CFLAGS))

LIB_SRCS_PATTERN    += os/$(CONFIG_VENDOR)/*.c
LIB_SRCS_PATTERN    += ssl/mbedtls/*.c
LIB_SRCS_PATTERN    += ssl/itls/*.c
