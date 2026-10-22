# Overlay 60 rules, split out of mk/overlays.mk on 2026-10-07 to keep it under
# the clean-room 256 KiB tracked-file limit.
#
# One module TU under the overlay flag set; its .text is the module's full
# 0x3560 bytes, trailing pad included, so no trim, and resident and
# cross-overlay calls are renamed by the generated surface rebinds. Kept
# until phase 3: the TU's .rodata (the menu's ten-entry switch table and the
# 0.67f literal) is still owned by the raw data blob, so the section is
# asserted by digest and its two %hi/%lo pairs anchored at the shipped
# rodata-relative 0x50; the table's own text relocations go with it.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay_060.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay_060.c.o: \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay_060.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:25e7eb532a0f47e402a7998bc5f3f404eb09f41912d87d8712eea3d83ec30b75 0x50 && \
	$(OBJCOPY) --remove-section=.rel.rodata $@
