CONFIG_ENV_CFLAGS   += \
    -Os -Wall \
    --coverage \

ifneq (Darwin,$(strip $(shell uname)))
CONFIG_ENV_CFLAGS   += -rdynamic
CONFIG_ENV_CFLAGS   += -Werror
else
CONFIG_src/tools/linkkit_tsl_convert :=
endif

LDFLAGS             += -lpthread -lrt

OVERRIDE_STRIP      := strip

