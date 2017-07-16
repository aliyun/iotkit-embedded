.PHONY: coverage lcov

ifeq (,$(COVERAGE_CMD))
coverage lcov:
	@echo "COVERAGE_CMD not defined, skip"
else
ifeq (,$(filter --coverage,$(CFLAGS)))
coverage lcov:
	@echo "CFLAGS not containing '--coverage', skip"
	@$(call Dump_Var,CFLAGS)
else
coverage lcov: all
	$(Q)rm -rf $(OUTPUT_DIR)/$(LCOV_DIR) $(DIST_DIR)/$(LCOV_DIR)
	$(Q)rm -f $(OUTPUT_DIR)/{files,tests,all,final}.info
	$(Q)find $(OUTPUT_DIR) -name "*.gcno" -o -name "*.gcda" -exec rm -f {} \;
	OUTPUT_DIR=$(OUTPUT_DIR) $(COVERAGE_CMD) || true
	$(Q)lcov --quiet \
	    --capture --initial --directory $(OUTPUT_DIR)/ \
	    -o $(OUTPUT_DIR)/files.info
	$(Q)lcov --quiet \
	    --capture --directory $(OUTPUT_DIR) \
	    -o $(OUTPUT_DIR)/tests.info
	$(Q)lcov --quiet \
	    --add-tracefile $(OUTPUT_DIR)/files.info \
	    --add-tracefile $(OUTPUT_DIR)/tests.info \
	    -o $(OUTPUT_DIR)/all.info
	$(Q)lcov --quiet \
	    --remove $(OUTPUT_DIR)/all.info \
	    -o $(OUTPUT_DIR)/final.info '*.h'
	$(Q)genhtml --quiet \
	    --legend --no-branch-coverage -o $(OUTPUT_DIR)/$(LCOV_DIR) $(OUTPUT_DIR)/final.info
	$(Q)cp -rf $(OUTPUT_DIR)/$(LCOV_DIR) $(DIST_DIR)/$(LCOV_DIR)
	$(Q)cd $(DIST_DIR)/$(LCOV_DIR) && \
	    sed -i 's:\(coverFile.*\)>$(OUTPUT_DIR)/:\1>:g' index.html
	$(Q)bash $(SCRIPT_DIR)/generate_lcov_report.sh $(DIST_DIR)/$(LCOV_DIR)
endif
endif
