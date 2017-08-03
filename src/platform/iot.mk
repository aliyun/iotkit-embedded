LIBA_TARGET := libplatform.a
LIB_SRCS    := $(wildcard *.c)

LIB_SRCS    += $(wildcard network/$(PLATFORM_NETWORK)/*.c)
LIB_SRCS    += $(wildcard os/$(PLATFORM_OS)/*.c)
LIB_SRCS    += $(wildcard ssl/$(PLATFORM_SSL)/*.c)
LIB_SRCS    += $(wildcard product/$(PLATFORM_OS)/*.c)

DEPENDS     += src/external/recipes/mbedtls

# TODO: fix coap warnings
CFLAGS := $(filter-out -Werror,$(CFLAGS))

HDR_REFS    += src/sdk-impl
