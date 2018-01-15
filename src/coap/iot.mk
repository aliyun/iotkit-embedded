LIBA_TARGET := libiot_coap.a
HDR_REFS    := src

LIB_SRCS    := $(wildcard $(TOP_DIR)/$(MODULE_NAME)/*.c)
LIB_SRCS    += $(wildcard $(PACKAGE_DIR)/iot-coap-c/*.c)
