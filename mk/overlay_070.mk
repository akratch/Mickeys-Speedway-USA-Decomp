# Overlay 70: one module TU under the overlay flag set (-Wab,-r4300_mul). Its
# .text is the module's full 0x730 bytes, trailing pad included, so no trim,
# and resident calls are renamed by the generated surface rebinds.
#
# Kept: the shipped tables carry no relocation record for fourteen HI16/LO16
# sites (the spawner's three table reads, +0x078..+0x0BC, and the manager's
# shared-counter accesses, +0x5E8..+0x70C), so their records are removed,
# asserted one by one; no instruction byte changes. The tables themselves are
# still owned by the raw data blob until phase 3.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/overlay_070.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/overlay_070.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/overlay_070.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x078:5:gOverlay70FloatTableReloc \
		0x094:6:gOverlay70FloatTableReloc \
		0x098:5:gOverlay70VerticalStepReloc \
		0x0ac:6:gOverlay70VerticalStepReloc \
		0x09c:5:gOverlay70AngleReloc \
		0x0bc:6:gOverlay70AngleReloc \
		0x5E8:5:gOverlay70SharedCounterReloc \
		0x5F8:6:gOverlay70SharedCounterReloc \
		0x5FC:5:gOverlay70SharedCounterReloc \
		0x604:6:gOverlay70SharedCounterReloc \
		0x674:5:gOverlay70SharedCounterReloc \
		0x678:6:gOverlay70SharedCounterReloc \
		0x6C4:5:gOverlay70SharedCounterReloc \
		0x70C:6:gOverlay70SharedCounterReloc
