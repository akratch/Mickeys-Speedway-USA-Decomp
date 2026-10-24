# Overlay 61's real readonly literal context precedes its nine-case table.
# The untouched compiler emits the full runtime RO image. The actual section
# retains its zero-based runtime role; no synthetic offset or site rebind.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay_061.c.o: CFLAGS += -Wab,-r4300_mul -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay_061.c.o: $(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay_061.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:f7ab29af1991287fb508602d2159cb760cb5cae96f81770821eaae8e0d42d0eb 0 keep && \
	$(OBJCOPY) --remove-section=.rel.rodata $@

O61_INITIAL_STATE := $(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61InitialState.c.o
$(O61_INITIAL_STATE): $(TOOLS_DIR)/trim_elf_section.py
$(O61_INITIAL_STATE): POSTPROCESS = $(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0xB4 --compiler-end-padding
$(O61_INITIAL_STATE): private PLAIN_DATA_PROOF = 1
$(O61_INITIAL_STATE): private PLAIN_DATA_RUNTIME_ROLE = data
