LIBA_TARGET := libiot_utils.a
LIB_SRCS    := $(wildcard */*.c)
LIB_SRCS    := $(filter-out LITE-utils/lite-utils_prog.c,$(LIB_SRCS))
LIB_HEADERS := $(wildcard LITE-utils/*.h)

HDR_REFS    := src

DEPENDS     := src/log
PKG_SOURCE  := src/packages/LITE-utils.git
