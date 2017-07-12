LIBA_TARGET := libplatform.a
LIB_SRCS    := $(wildcard *.c)

LIB_SRCS    += $(wildcard network/$(PLATFORM_NETWORK)/*.c)
LIB_SRCS    += $(wildcard os/$(PLATFORM_OS)/*.c)
LIB_SRCS    += $(wildcard ssl/$(PLATFORM_SSL)/*.c)

ifeq (mbedtls,$(strip $(PLATFORM_SSL)))
DEPENDS     += src/external/recipes/mbedtls
else
endif
