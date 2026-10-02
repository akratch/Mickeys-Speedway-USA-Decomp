O22_REMOVE_OBJECT_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay22RemoveObject.c.o
O22_RESOLVE_PLANE_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay22ResolvePlane.c.o
O22_INITIALIZE_OBJECT_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/overlay22InitializeObject.c.o
O22_UPDATE_OBJECT_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o022/func_overlay_022_F00002B0_18783B8.c.o

# Exact C. Its compiler pool -- 14.4f, 0.8f and 0.03f -- duplicates the
# module data overlay22RemoveObject emits at rodata-relative +0x4, which the
# shipped %hi/%lo pairs encode. Rebind the six references to a pool symbol and
# discard the digest-checked duplicate; no instruction or compiler addend is
# edited (overlay 86's metadata-only form). The resident callees go through
# the generated surface entries.
$(O22_UPDATE_OBJECT_OBJ): \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/func_overlay_022_F00002B0_18783B8.rebind.spec
$(O22_UPDATE_OBJECT_OBJ): CFLAGS += -Wab,-r4300_mul
$(O22_UPDATE_OBJECT_OBJ): POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym Arctanf=Arctanf_o022Reloc \
		--redefine-sym sqrtf=sqrtf_o022Reloc \
		--redefine-sym func_8002A8C0=func_8002A8C0_o022Reloc \
		--redefine-sym func_8002A8BC=func_8002A8BC_o022Reloc \
		--redefine-sym trackMakePolylist=trackMakePolylist_o022Reloc \
		--redefine-sym func_80010900=func_80010900_o022Reloc \
		--redefine-sym func_80008128=func_80008128_o022Reloc \
		--redefine-sym partUpdateTriggers=partUpdateTriggers_o022Reloc \
		--redefine-sym func_80001620=func_80001620_o022Reloc \
		--redefine-sym func_800031E8=func_800031E8_o022Reloc \
		--redefine-sym func_80002FE0=func_80002FE0_o022Reloc \
		--redefine-sym func_8000309C=func_8000309C_o022Reloc \
		--redefine-sym func_80036544=func_80036544_o022Reloc \
		--add-symbol gOverlay22UpdatePoolReloc=0x4,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_022_F00002B0_18783B8.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:60271c5644e3e162f2075f0f7b3aa442f11bbcde8d236ae430cf2dd1dd6ab887 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7CC

# The compiler reproduces the retail body; the flag is load-bearing. The
# self-rename is a historical no-op metadata marker and changes no instruction.
$(O22_INITIALIZE_OBJECT_OBJ): CFLAGS += -Wab,-r4300_mul
$(O22_INITIALIZE_OBJECT_OBJ): POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_022_F0000000_1878108=func_overlay_022_F0000000_1878108 $@


# The compiler reproduces the retail body; retain the exact owned text
# extent. The self-rename is a historical no-op metadata marker.
$(O22_REMOVE_OBJECT_OBJ): POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_022_F0000D30_1878E38=func_overlay_022_F0000D30_1878E38 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x16C

# The compiler reproduces the retail body. Overlay 22's runtime relocation
# binds the resident sqrtf call through the module base entry; restore that
# identity, then retain the exact owned text extent. No instruction changes.
$(O22_RESOLVE_PLANE_OBJ): CFLAGS += -Wab,-r4300_mul
$(O22_RESOLVE_PLANE_OBJ): POSTPROCESS = \
	$(OBJCOPY) --redefine-sym sqrtf=func_overlay_022_F0000000_1878108 $@ && \
	$(OBJCOPY) --redefine-sym func_overlay_022_F0000A7C_1878B84=func_overlay_022_F0000A7C_1878B84 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2B4
