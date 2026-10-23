# Overlay 52 rules.
# Included from mk/overlays.mk at the position the first of these rules held.
# Split out on 2026-10-07 to keep mk/overlays.mk under the clean-room
# 256 KiB tracked-file limit.
# Overlay 52 is one translation unit at the overlay flag set (with it IDO
# emits the HUD easing as the shipped rotated branch-likely loop); its .text
# carries the trailing pad, so nothing is trimmed. --force runs
# asm-processor's static-symbol pass so the two static helpers keep local
# names. The module owns its .data (the atlas's whole initialized range) and
# .bss; the shipped words that address them are section-relative, so every
# site is rebound to a zero-valued base. No instruction is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay_052.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay_052.c.o: ASM_PROCESSOR += --force
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay_052.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	config/normalizations/overlay_052.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay_052.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay52DataBaseReloc=0x0,global \
		--add-symbol gOverlay52BssBaseReloc=0x0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_052.rebind.spec
