LIBA_TARGET := libiot_sdkimpl.a
HDR_REFS    := src
DEPENDS     := src/utils

LIBHDR_DIR  := iot-sdk
LIB_HEADERS := iot_import.h iot_export.h
LIB_HEADERS += $(wildcard imports/iot_import*.h)
LIB_HEADERS += $(wildcard exports/iot_export*.h)
