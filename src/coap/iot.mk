LIBA_TARGET := libiot_coap.a
HDR_REFS    := src
EXTRA_SRCS  := $(wildcard $(PACKAGE_DIR)/iot-coap-c/*.c)

LIB_SRCS    := $(wildcard $(TOP_DIR)/$(MODULE_NAME)/*.c)
LIB_SRCS    += $(foreach S,$(EXTRA_SRCS),$(subst $(PACKAGE_DIR)/iot-coap-c/,,$(S)))
