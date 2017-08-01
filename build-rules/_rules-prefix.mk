MAKE_FN_VARS := \
$(foreach v, \
    $(shell grep -o 'CONFIG_FN_[_A-Z]*' $(CONFIG_TPL) 2>/dev/null), \
        $(subst CONFIG_FN_,,$(v)) \
)
MAKE_FN_VARS := $(sort $(MAKE_FN_VARS))

$(foreach V, \
    $(MAKE_FN_VARS), \
        $(eval \
            CFLAGS += -D$(V)=\\\"$(CONFIG_FN_$(V))\\\" \
         ) \
)

CFLAGS  := $(sort $(CFLAGS) $(CONFIG_ENV_CFLAGS))
LDFLAGS := $(sort $(LDFLAGS) $(CONFIG_ENV_LDFLAGS))

MAKE_ENV_VARS := \
$(foreach v, \
    $(shell grep -o 'CONFIG_ENV_[_A-Z]*' $(CONFIG_TPL) 2>/dev/null), \
        $(subst CONFIG_ENV_,,$(v)) \
)

# $(eval ...) causes '$' in CFLAGS lost
MAKE_ENV_VARS := $(sort $(filter-out CFLAGS LDFLAGS,$(MAKE_ENV_VARS)))

$(foreach V, \
    $(MAKE_ENV_VARS), \
        $(eval export $(V) := $(sort $(CONFIG_ENV_$(V)))) \
)
