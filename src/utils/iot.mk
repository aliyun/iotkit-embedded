LIBA_TARGET := libiot-utils.a
LIB_SRCS    := $(wildcard */*.c)
LIB_SRCS    := $(filter-out LITE-utils/lite-utils_prog.c,$(LIB_SRCS))

HDR_REFS    := src

PKG_SOURCE  := src/packages/LITE-utils

PREP_BUILD_HOOK := customize_utils
define customize_utils
    $(call Customize_With, utils_config.h, LITE-utils/lite-utils_config.h)
endef
