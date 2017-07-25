LIBA_TARGET := libmqtt.a
HDR_REFS    := src
DEPENDS     += src/log
CFLAGS      := $(filter-out -Werror,$(CFLAGS))
