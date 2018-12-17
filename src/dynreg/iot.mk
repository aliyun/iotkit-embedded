LIBA_TARGET     := libiot_dynreg.a

HDR_REFS        := src/infra

DEPENDS         += wrappers
LDFLAGS         += -liot_hal

TARGET          := dynreg-example

LIB_SRCS_EXCLUDE        := dynreg_example.c
SRCS_dev-sign-example   += dynreg_example.c