LIBA_TARGET := libsdk-impl.a
HDR_REFS    := src

LIBHDR_DIR  := iot-sdk
LIB_HEADERS := iot_import.h iot_export.h
LIB_HEADERS += $(wildcard imports/iot_import*.h)
LIB_HEADERS += $(wildcard exports/iot_export*.h)
