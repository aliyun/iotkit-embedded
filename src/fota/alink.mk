LIBA_TARGET         := libilop-fota.a
LIB_SRCS_PATTERN    := src/*.c

HDR_REFS            += base sdk-encap layers/cmp service/fota layers/dm

#CFLAGS              := $(filter-out -Werror,$(CFLAGS))

ifeq (,$(filter -DSERVICE_OTA_ENABLED,$(CFLAGS)))
LIB_SRCS_PATTERN    :=
LIB_SRCS :=
endif
