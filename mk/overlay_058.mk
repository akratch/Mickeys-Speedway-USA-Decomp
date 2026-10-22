# Overlay 58 rules, split out of mk/overlays.mk on 2026-10-07 to keep it under
# the clean-room 256 KiB tracked-file limit.
#
# One module TU under the overlay flag set; its .text is the module's full
# 0x5A20 bytes, trailing pad included, so no trim, and resident and
# cross-overlay calls are renamed by the generated surface rebinds. Kept
# until phase 3: the TU's .rodata (the state update's seven-entry switch
# table and 0.02f literal, then the mode update's thirteen-entry table) is
# still owned by the raw data blob at data_rodata +0x3D4, so the section is
# asserted by digest and its three %hi/%lo pairs anchored at the shipped
# rodata-relative 0x104; the tables' own text relocations go with it.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay_058.c.o: CFLAGS += -Wab,-r4300_mul -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay_058.c.o: \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay_058.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:3dbf1ba8a2f53fbf224e3e1c3b43aca87ad4e03ba1a51538cbd7e818f0d158be 0x104 && \
	$(OBJCOPY) --remove-section=.rel.rodata $@
