LIBA_TARGET     := libywss.a

CFLAGS          := $(filter-out -Werror,$(CFLAGS))

HDR_REFS        := src/infra
HDR_REFS        += src/protocol/mqtt
HDR_REFS        += src/protocol/alcs
