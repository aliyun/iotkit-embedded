LIBA_TARGET := libiot_utils.a
LIB_SRCS    := $(wildcard $(TOP_DIR)/$(MODULE_NAME)/*/*.c)
LIB_SRCS    := $(filter-out $(TOP_DIR)/$(MODULE_NAME)/LITE-utils/lite-utils_prog.c,$(LIB_SRCS))

HDR_REFS    := src

DEPENDS     := src/log
PKG_SOURCE  := LITE-utils.git
