define CompLib_Map
$(eval \
    COMP_LIB_COMPONENTS += \
        $(if \
            $(filter y,$(FEATURE_$(strip $(1)))),$(strip $(2)) \
        ) \
)
endef

POST_FINAL_OUT_HOOK := Post_Distro
define Post_Distro
    @find $(FINAL_DIR) -name "*.[ch]" -exec chmod a-x {} \;
    @echo ""
    @echo "========================================================================="
    @echo "o BUILD COMPLETE WITH FOLLOWING SWITCHES:"
    @echo "----"
    @( \
    $(foreach V,$(SETTING_VARS), \
        $(if $(findstring FEATURE_,$(V)), \
            printf "%-32s : %-s\n" "    $(V)" "$($(V))"; \
        ) \
    ) )
    @echo ""
    @echo "o RELEASE PACKAGE LAYOUT:"
    @echo "----"
    @tree $(FINAL_DIR) --noreport -A -I mbedtls \
        |awk '{ printf ("    %s\n", $$0); }'
    @echo ""
    @echo "o BINARY FOOTPRINT CONSIST:"
    @echo "----"
    @STAGED=$(LIBOBJ_TMPDIR) STRIP=$(STRIP) $(SCRIPT_DIR)/stats_static_lib.sh $(FINAL_DIR)/lib/$(COMP_LIB)
    @echo "========================================================================="
    @echo ""
endef

