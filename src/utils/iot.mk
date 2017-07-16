LIBA_TARGET := libutils.a
LIB_SRCS    := $(wildcard */*.c)
LIB_SRCS    := $(filter-out LITE-utils/lite-utils_prog.c,$(LIB_SRCS))

DEPENDS     += src/log

LIBHDR_DIR  := lite
LIB_HEADERS := \
    LITE-utils/lite-utils.h \
    LITE-utils/lite-list.h \

PKG_SOURCE  := src/packages/LITE-utils
# PKG_UPDATE  := 'git@gitlab.alibaba-inc.com:iot-middleware/LITE-utils.git'

build:
	$(Q)cp utils_config.h LITE-utils/lite-utils_config.h
	$(Q)$(MAKE) -s -C LITE-utils library
