LIBA_TARGET := libiot_tls.a
CFLAGS      := $(filter-out -ansi,$(CFLAGS))

LIB_HEADERS := $(wildcard $(TOP_DIR)/$(MODULE_NAME)/mbedtls-in-iotkit/include/mbedtls/*.h)

PKG_SOURCE  := mbedtls-in-iotkit.git
