LIBA_TARGET := libplatform.a
LIB_SRCS    := $(wildcard *.c)

LIB_SRCS    += $(wildcard network/$(PLATFORM_NETWORK)/*.c)
LIB_SRCS    += $(wildcard os/$(PLATFORM_OS)/*.c)

ifeq (,$(filter -DIOTX_WITHOUT_TLS,$(CFLAGS)))
LIB_SRCS    += $(wildcard ssl/$(PLATFORM_SSL)/*.c)
endif

ifeq (,$(filter -DIOTX_WITHOUT_TLS,$(CFLAGS)))
ifeq (mbedtls,$(strip $(PLATFORM_SSL)))
DEPENDS     += src/external/recipes/mbedtls
else
endif
endif
DEPENDS     += src/log

HDR_REFS    += src/sdk-impl
