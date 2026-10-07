# Overlay 70 POSTPROCESS rules.
# Included from mk/overlays.mk at the position the first of these rules held.
# Split out on 2026-10-07 to keep mk/overlays.mk under the clean-room
# 256 KiB tracked-file limit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000000_18C91C8.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000000_18C91C8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x078:5:gOverlay70FloatTableReloc \
		0x094:6:gOverlay70FloatTableReloc \
		0x098:5:gOverlay70VerticalStepReloc \
		0x0ac:6:gOverlay70VerticalStepReloc \
		0x09c:5:gOverlay70AngleReloc \
		0x0bc:6:gOverlay70AngleReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x028:overlay70RandomRange:func_overlay_070_F0000000_18C91C8 \
		0x038:overlay70RandomRange:func_overlay_070_F0000000_18C91C8 \
		0x04c:overlay70RandomRange:func_overlay_070_F0000000_18C91C8 \
		0x05c:overlay70RandomRange:func_overlay_070_F0000000_18C91C8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
# Retail overlay70 routes this carrier's local calls through offset zero and
# stores its two local table references without reloc records.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F00000D8_18C92A0.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000384_18C954C.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000384_18C954C.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000384_18C954C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x264:5:gOverlay70SharedCounterReloc \
		0x274:6:gOverlay70SharedCounterReloc \
		0x278:5:gOverlay70SharedCounterReloc \
		0x280:6:gOverlay70SharedCounterReloc \
		0x2F0:5:gOverlay70SharedCounterReloc \
		0x2F4:6:gOverlay70SharedCounterReloc \
		0x340:5:gOverlay70SharedCounterReloc \
		0x388:6:gOverlay70SharedCounterReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3A4
