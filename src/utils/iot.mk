LIBA_TARGET := libiot_utils.a
HDR_REFS    := src

LIB_SRCS    := $(wildcard $(TOP_DIR)/$(MODULE_NAME)/*/*.c)
LIB_SRCS    += $(wildcard $(PACKAGE_DIR)/LITE-utils/*.c)
