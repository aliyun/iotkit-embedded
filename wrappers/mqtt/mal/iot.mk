LIBA_TARGET     := libiot_mal.a

LIB_SRCS_PATTERN := mal.c

ifneq (,$(filter -DMAL_ICA_ENABLED, $(CFLAGS)))
    LIB_SRCS_PATTERN += ica/*.c  \
                        ica/test/*.c
endif

HDR_REFS        := src/infra
