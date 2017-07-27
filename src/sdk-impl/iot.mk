LIBA_TARGET := libsdk-impl.a
HDR_REFS    := src

LIBHDR_DIR  := export
LIB_HEADERS := iot_import.h iot_export.h
LIB_HEADERS += $(wildcard sub_export*.h)
