LIBA_TARGET := libiot-crypto.a
LIBHDR_DIR  := lite
LIB_HEADERS := LITE-crypto/lite-crypto.h

LIB_SRCS        := $(wildcard LITE-crypto/*.c)
LIB_SRCS        += $(wildcard LITE-crypto/mbedtls/library/*.c)

DEPENDS     += src/log
DEPENDS     += src/utils

PKG_SOURCE  := src/packages/LITE-crypto
# PKG_UPDATE  := 'git@gitlab.alibaba-inc.com:iot-middleware/LITE-crypto.git'

PREP_BUILD_HOOK := customize_myself

define customize_myself
    @if ! diff -q iot-crypto_config.h LITE-crypto/lite-crypto_config.h; then \
        cp -f iot-crypto_config.h LITE-crypto/lite-crypto_config.h; \
    fi
    @cp -f iot-mbedtls_config.h LITE-crypto/mbedtls/config.h
endef
