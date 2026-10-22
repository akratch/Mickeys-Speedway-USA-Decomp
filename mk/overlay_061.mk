# Overlay 61 rules, split out of mk/overlays.mk on 2026-10-07 to keep it under
# the clean-room 256 KiB tracked-file limit.
#
# One module TU under the overlay flag set; its .text is the module's full
# 0x1A90 bytes, trailing pad included, so no trim, and resident calls are
# renamed by the generated surface rebinds. -woff 835: four resident callees
# are deliberately unprototyped (overlay_061.h). Kept until phase 3: the
# TU's .rodata (the pak menu's nine-entry switch table) is still owned by the
# raw data blob, so the section is asserted by digest and its %hi/%lo pair
# anchored at the shipped rodata-relative 0x188; the table's own text
# relocations go with it.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay_061.c.o: CFLAGS += -Wab,-r4300_mul -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay_061.c.o: \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay_061.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:7d32dd8918ee69dd189577037beffda3b905ceda99ea1e8fadd03d362f556145 0x188 && \
	$(OBJCOPY) --remove-section=.rel.rodata $@
