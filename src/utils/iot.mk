LIBA_TARGET := libiot-utils.a
LIB_SRCS    := $(wildcard */*.c)
LIB_SRCS    := $(filter-out LITE-utils/lite-utils_prog.c,$(LIB_SRCS))

DEPENDS     += src/log

LIBHDR_DIR  := lite

PKG_SOURCE  := src/packages/LITE-utils
# PKG_UPDATE  := 'git@gitlab.alibaba-inc.com:iot-middleware/LITE-utils.git'

build:
	$(Q)cp utils_config.h LITE-utils/lite-utils_config.h
	$(Q)$(MAKE) -C LITE-utils \
	    $(if $(Q),-s,) \
	    CFLAGS='$(CFLAGS)' \
	    INS_LIBS=$(LIBA_TARGET) \
	    library
