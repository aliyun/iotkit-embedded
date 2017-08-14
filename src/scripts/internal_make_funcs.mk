define CompLib_Map
$(eval \
    COMP_LIB_COMPONENTS += \
        $(if \
            $(filter y,$(FEATURE_$(strip $(1)))),$(strip $(2)) \
        ) \
)
endef

define Post_Distro
    @find $(FINAL_DIR) -name "*.[ch]" -exec chmod a-x {} \;
    @mkdir -p $(FINAL_DIR)/src
    @cat doc/export.sdk.demo/head.mk >  $(FINAL_DIR)/src/Makefile
    $(if $(filter y,$(FEATURE_MQTT_COMM_ENABLED)),
        @cp -f sample/mqtt/mqtt-example.c $(FINAL_DIR)/src/mqtt-example.c
        @cat doc/export.sdk.demo/mqtt.mk >> $(FINAL_DIR)/src/Makefile)
    $(if $(filter y,$(FEATURE_COAP_COMM_ENABLED)),
        @cp -f sample/coap/iotx_coap_client.c $(FINAL_DIR)/src/coap-example.c
        @cat doc/export.sdk.demo/coap.mk >> $(FINAL_DIR)/src/Makefile)
    @chmod a-x $(FINAL_DIR)/src/*

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
    @tree -A $(FINAL_DIR) --noreport -A -I mbedtls \
        |awk '{ printf ("    %s\n", $$0); }'
    @echo ""
    @echo "o BINARY FOOTPRINT CONSIST:"
    @echo "----"
    @STAGED=$(LIBOBJ_TMPDIR) STRIP=$(STRIP) $(SCRIPT_DIR)/stats_static_lib.sh $(FINAL_DIR)/lib/$(COMP_LIB)
    @echo "========================================================================="
    @echo ""
endef

