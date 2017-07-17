LIBA_TARGET := libiot-utils.a
LIB_SRCS    := $(wildcard */*.c)
LIB_SRCS    := $(filter-out LITE-utils/lite-utils_prog.c,$(LIB_SRCS))
LIBHDR_DIR  := lite
LIB_HEADERS := LITE-utils/{lite-list,lite-utils}.h

DEPENDS     += src/log

PKG_SOURCE  := src/packages/LITE-utils
# PKG_UPDATE  := 'git@gitlab.alibaba-inc.com:iot-middleware/LITE-utils.git'

PREP_BUILD_HOOK := customize_utils
define customize_utils
	$(Q) \
	if ! diff -q utils_config.h LITE-utils/lite-utils_config.h > /dev/null; then \
	    cp utils_config.h LITE-utils/lite-utils_config.h; \
	fi
endef
