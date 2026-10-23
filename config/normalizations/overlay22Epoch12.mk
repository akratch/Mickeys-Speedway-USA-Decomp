# Overlay 22 is one TU under the overlay flag set: no trim, and its resident
# calls take the generated rebinds. What remains is phase-3 scaffolding for
# rodata the TU does not own yet, none of it touching an instruction word:
# the update's compiler pool -- 14.4f, 0.8f and 0.03f -- duplicates the
# module literals at rodata-relative +0x4 that the shipped %hi/%lo pairs
# encode. Rebind its six references to a pool symbol and discard the
# digest-checked duplicate (overlay 86's metadata-only form).
O22_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay_022.c.o
$(O22_OBJ): \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay_022.rebind.spec
$(O22_OBJ): CFLAGS += -Wab,-r4300_mul
$(O22_OBJ): POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay22UpdatePoolReloc=0x4,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_022.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:60271c5644e3e162f2075f0f7b3aa442f11bbcde8d236ae430cf2dd1dd6ab887
