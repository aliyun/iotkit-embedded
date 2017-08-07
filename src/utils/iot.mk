LIBA_TARGET := libiot-utils.a
LIB_SRCS    := $(wildcard */*.c)
LIB_SRCS    := $(filter-out LITE-utils/lite-utils_prog.c,$(LIB_SRCS))

HDR_REFS    := src

PKG_SOURCE  := src/packages/LITE-utils
