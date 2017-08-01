ifdef COMP_LIB
final-out: sub-mods
	$(TOP_Q) \
	if [ -f $(STAMP_PRJ_CFG) ]; then true; else \
	    rm -rf $(DIST_DIR); \
	    mkdir -p $(DIST_DIR) $(FINAL_DIR); \
	    if  [ ! -f $(SYSROOT_LIB)/lib$(COMP_LIB_NAME).a ] && \
	        [ ! -f $(SYSROOT_LIB)/lib$(COMP_LIB_NAME).so ]; then \
	        $(call Build_CompLib, FORCE) \
	    fi; \
	    for i in bin lib include; do \
	        [ -d $(OUTPUT_DIR)/usr/$${i} ] && cp -rf $(OUTPUT_DIR)/usr/$${i} $(FINAL_DIR) || true; \
	    done; \
	fi
else
final-out: sub-mods
	$(TOP_Q) \
	if [ -f $(STAMP_PRJ_CFG) ]; then true; else \
	    rm -rf $(DIST_DIR); \
	    mkdir -p $(DIST_DIR) $(FINAL_DIR); \
	    for i in bin lib include; do \
	        [ -d $(OUTPUT_DIR)/usr/$${i} ] && cp -rf $(OUTPUT_DIR)/usr/$${i} $(FINAL_DIR) || true; \
	    done; \
	fi
endif

	$(TOP_Q)$(STRIP) $(FINAL_DIR)/bin/* $(FINAL_DIR)/lib/*
