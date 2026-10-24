# Overlay 46 rules.
# Included from mk/overlays.mk at the position the first of these rules held.
# Split out on 2026-10-07 to keep mk/overlays.mk under the clean-room
# 256 KiB tracked-file limit.
#
# Overlay 46 is one translation unit. Its flag set is the overlay set plus
# -Wo,-loopunroll,0: measured at the fold, the whole module matches with
# unrolling off (0 differing words), while IDO's default unroll rewrites the
# buffer initializer's two loops and the trail drawer's loops and changes
# nothing else; the shipped loops are plain counted loops, so no spelling
# reaches them under the default unroll. The .text carries the trailing pad,
# so nothing is trimmed. The common static-symbol step preserves the three
# helpers' local names from compiler metadata. The compiler's .rodata is
# byte-identical to the retained rodata from +0xC (the particle spacing is the
# 0.025f literal it is, not an extern of that pool word): its sites are
# rebound to a base valued 0xC and the copy is dropped after its digest
# agrees, with the switch table's relocations. No instruction is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay_046.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay_046.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay_046.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay_046.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay_046.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay46RodataBaseReloc=0xC,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_046.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:7b53a71d02fb8a3add8e911997f3df5e55943cb228c14fb6df4cfbaf33082709 && \
	$(OBJCOPY) --remove-section .rel.rodata $@
