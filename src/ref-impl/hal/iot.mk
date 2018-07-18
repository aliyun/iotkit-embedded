LIBA_TARGET := libiot_hal.a
CFLAGS      := $(filter-out -ansi,$(CFLAGS))

HDR_REFS    += src/ref-impl/tls
HDR_REFS    += src/infra
HDR_REFS    += src/security/pro

DEPENDS     += src/ref-impl/tls

LIB_SRCS_PATTERN    += os/$(CONFIG_VENDOR)/*.c
LIB_SRCS_PATTERN    += ssl/mbedtls/*.c
LIB_SRCS_PATTERN    += ssl/itls/*.c

define Extra_CMake_Head
    echo 'IF (WIN32)' $(1)
    echo '    SET (OS_DIR win7)' $(1)
    echo 'ELSE (WIN32)' $(1)
    echo '    SET (OS_DIR ubuntu)' $(1)
    echo 'ENDIF (WIN32)' $(1)
    echo 'FILE (GLOB EXTRA_C_SOURCES $${PROJECT_SOURCE_DIR}/$(MODULE_NAME)/os/$${OS_DIR}/*.c)' $(1)
    echo '' $(1)
endef
