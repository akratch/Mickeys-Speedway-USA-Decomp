# Overlay 2 POSTPROCESS rules.
# Included from mk/overlays.mk at the position the first of these rules held.
# Split out on 2026-10-07 to keep mk/overlays.mk under the clean-room
# 256 KiB tracked-file limit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2Enable.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ValidateRegion.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_8002A910=overlay2AngleReloc \
		--redefine-sym func_8002AA0C=overlay2AngleDifferenceReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1BC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ContainsPoint.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_8002A4C0=overlay2PointAngleReloc \
		--redefine-sym func_8002A5BC=overlay2PointAngleDifferenceReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x128
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2CopyColor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2AppendLine.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x108
# NON_MATCHING/GLOBAL_ASM: retain only friendly-name restoration and
# trailing-section trimming metadata for these extracted functions.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ClassifyBoundary.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_002_F00002C4_18570BC=overlay2ClassifyBoundary $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x13C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2IntersectBoundary.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ClipLines.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x244
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2AdjacentIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001364_185815C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2F4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001364_185815C.c.o: CFLAGS += -Wab,-r4300_mul
# The region-tree build is instruction-exact. Its six resident callees go
# through the generated surface entries; no instruction changes.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0000C90_1857A88.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym _bzero=_bzero_o002Reloc \
		--redefine-sym func_8002B280=func_8002B280_o002Reloc \
		--redefine-sym func_8002B524=func_8002B524_o002Reloc \
		--redefine-sym mmFree=mmFree_o002Reloc \
		--redefine-sym mmGetDelay=mmGetDelay_o002Reloc \
		--redefine-sym mmSetDelay=mmSetDelay_o002Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2QueryNode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3F4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001A94_185888C.c.o: \
	CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001A94_185888C.c.o: \
	POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x364 \
		000000000000000000000000
# Exact C. Its three resident callees go through the generated surface
# entries; no instruction changes.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001DF8_1858BF0.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_8000572C=func_8000572C_o002Reloc \
		--redefine-sym func_8000BCB0=func_8000BCB0_o002Reloc \
		--redefine-sym joyGetButtons=joyGetButtons_o002Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x730
