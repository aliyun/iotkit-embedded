LIBA_TARGET := libshadow.a
HDR_REFS    := src
CFLAGS      := $(filter-out -Werror,$(CFLAGS))
