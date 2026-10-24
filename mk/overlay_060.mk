# O60 remains one exact text TU. Its genuine const labels/formats and the
# compiler-generated switch/literal pool form one complete 128-byte RO input.
# The zero section anchor preserves loader-relative operands; R32 records
# remain raw compiler authority, while host linking keeps stored text offsets.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay_060.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay_060.c.o: $(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay_060.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:fc11a59828e28fdcf650f700b8fb28a10fe8599ca2f3b1b2e06fe2c1a45f31bf 0 keep && \
	$(OBJCOPY) --remove-section=.rel.rodata $@

# Complete ordinary C leaves: genuine resource/preview objects, no imports.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60MenuResources.c.o: private PLAIN_DATA_PROOF = 1
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60MenuResources.c.o: private PLAIN_DATA_RUNTIME_ROLE = data
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60PreviewTables.c.o: private PLAIN_DATA_PROOF = 1
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60PreviewTables.c.o: private PLAIN_DATA_RUNTIME_ROLE = data
