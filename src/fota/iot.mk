LIBA_TARGET         := libilop-fota.a
LIB_SRCS_PATTERN    := src/*.c

ifeq (,$(filter -DSERVICE_OTA_ENABLED,$(CFLAGS)))
LIB_SRCS_PATTERN    :=
LIB_SRCS :=
endif

