include $(CURDIR)/tools/internal_make_funcs.mk

SWITCH_VARS := \
$(shell grep '''config [_A-Z]*''' \
    $$(find -L $(TOP_DIR)/tools -name 'Config.in') \
        | cut -d: -f2 \
        | grep -v menuconfig \
        | grep -v SRCPATH \
        | awk '{ print $$NF }' \
)
SWITCH_VARS := $(foreach V,$(sort $(SWITCH_VARS)),FEATURE_$(V))

$(foreach v, \
    $(SWITCH_VARS), \
    $(if $(filter y,$($(v))), \
        $(eval CFLAGS += -D$(subst FEATURE_,,$(v)))) \
)

ifneq (,$(shell find src -maxdepth 1 -name "dev_model_classic"))
    # CFLAGS += -DDEVICE_MODEL_CLASSIC -DINFRA_CLASSIC
endif