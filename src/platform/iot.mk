LIBA_TARGET := libiot_platform.a
DEPENDS     += src/tls

LIB_SRCS    += $(wildcard $(TOP_DIR)/$(MODULE_NAME)/os/$(CONFIG_VENDOR)/*.c)
LIB_SRCS    += $(wildcard $(TOP_DIR)/$(MODULE_NAME)/ssl/mbedtls/*.c)

HDR_REFS    += src/sdk-impl src/tls

CFLAGS      := $(filter-out -ansi,$(CFLAGS))
