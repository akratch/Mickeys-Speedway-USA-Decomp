# Overlay 60 POSTPROCESS rules.
# Included from mk/overlays.mk at the position the first of these rules held.
# Split out on 2026-10-07 to keep mk/overlays.mk under the clean-room
# 256 KiB tracked-file limit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60DrawBorder.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x10C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60DrawLine.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/func_overlay_060_F0002F54_18BCD2C.c.o: \
	CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/func_overlay_060_F0002F54_18BCD2C.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80034554=func_80034554_o060Reloc \
		--redefine-sym func_80036600=func_80036600_o060Reloc \
		--redefine-sym func_80036660=func_80036660_o060Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x378
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60ReassignChoiceSlots.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_060_F0003488_18BD260=overlay60ReassignChoiceSlots $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x334
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60ReleaseResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
# Matched C. The resident globals it reads are spelled as per-module
# placeholders in the source; the eight cross-overlay callees and the 51
# resident callees are renamed here so the generated relocation surface
# values them from the module's own stored addends without moving any
# other module's definition. A resident jal cannot reach the module's own
# 0xF0000000 region, so every one of those call sites needs its per-module
# placeholder or the link fails with R_MIPS_26 overflow. The
# compiler's private
# ten-entry switch table duplicates the one already in the extracted
# data/rodata asset at rodata +0x50; rebind the text pair to that proved
# runtime-local addend and discard the duplicate.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60Prefix.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60Prefix.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_041_F000124C_1888584=overlay41IsUnitScale_o060Reloc \
		--redefine-sym func_overlay_048_F0000000_1895408=overlay48Initialize_o060Reloc \
		--redefine-sym func_overlay_056_F00000B8_18A2E30=overlay56SplitTime_o060Reloc \
		--redefine-sym func_overlay_068_F000146C_18C85CC=overlay68CheckKind_o060Reloc \
		--redefine-sym func_overlay_082_F0000498_18CF618=overlay82GetSelection_o060Reloc \
		--redefine-sym func_overlay_082_F00004A4_18CF624=overlay82IsActive_o060Reloc \
		--redefine-sym func_overlay_082_F00004B0_18CF630=overlay82Disable_o060Reloc \
		--redefine-sym func_overlay_082_F00004C0_18CF640=overlay82Enable_o060Reloc \
		--redefine-sym amSndPlay=amSndPlay_o060Reloc \
		--redefine-sym animseqStartPath=animseqStartPath_o060Reloc \
		--redefine-sym animseqStopPath=animseqStopPath_o060Reloc \
		--redefine-sym camStandardOrtho=camStandardOrtho_o060Reloc \
		--redefine-sym camStandardPersp=camStandardPersp_o060Reloc \
		--redefine-sym fontColour=fontColour_o060Reloc \
		--redefine-sym frontGet2PlayerSplit=frontGet2PlayerSplit_o060Reloc \
		--redefine-sym frontGetBgmVolume=frontGetBgmVolume_o060Reloc \
		--redefine-sym frontGetScreenMode=frontGetScreenMode_o060Reloc \
		--redefine-sym frontGetSfxVolume=frontGetSfxVolume_o060Reloc \
		--redefine-sym frontGetStereoMode=frontGetStereoMode_o060Reloc \
		--redefine-sym frontGetWideAdjust=frontGetWideAdjust_o060Reloc \
		--redefine-sym frontSetBgmVolume=frontSetBgmVolume_o060Reloc \
		--redefine-sym frontSetLanguage=frontSetLanguage_o060Reloc \
		--redefine-sym frontSetSfxVolume=frontSetSfxVolume_o060Reloc \
		--redefine-sym frontSetStereoMode=frontSetStereoMode_o060Reloc \
		--redefine-sym frontSetWideAdjust=frontSetWideAdjust_o060Reloc \
		--redefine-sym func_80000510=func_80000510_o060Reloc \
		--redefine-sym func_800005CC=func_800005CC_o060Reloc \
		--redefine-sym func_8000572C=func_8000572C_o060Reloc \
		--redefine-sym func_80009E78=func_80009E78_o060Reloc \
		--redefine-sym func_80020D8C=func_80020D8C_o060Reloc \
		--redefine-sym func_80021504=func_80021504_o060Reloc \
		--redefine-sym func_80023F84=func_80023F84_o060Reloc \
		--redefine-sym func_8002468C=func_8002468C_o060Reloc \
		--redefine-sym func_80029198=func_80029198_o060Reloc \
		--redefine-sym func_800291C4=func_800291C4_o060Reloc \
		--redefine-sym func_8002F618=func_8002F618_o060Reloc \
		--redefine-sym func_8002FB34=func_8002FB34_o060Reloc \
		--redefine-sym func_800336A8=func_800336A8_o060Reloc \
		--redefine-sym func_80033FE0=func_80033FE0_o060Reloc \
		--redefine-sym func_80034920=func_80034920_o060Reloc \
		--redefine-sym func_800349A4=func_800349A4_o060Reloc \
		--redefine-sym func_800355A0=func_800355A0_o060Reloc \
		--redefine-sym func_800359D4=func_800359D4_o060Reloc \
		--redefine-sym func_800367A4=func_800367A4_o060Reloc \
		--redefine-sym func_8003A2C8=func_8003A2C8_o060Reloc \
		--redefine-sym func_8003A520=func_8003A520_o060Reloc \
		--redefine-sym func_8003A700=func_8003A700_o060Reloc \
		--redefine-sym func_8004B0A4=func_8004B0A4_o060Reloc \
		--redefine-sym func_8004B0DC=func_8004B0DC_o060Reloc \
		--redefine-sym func_8004B0F8=func_8004B0F8_o060Reloc \
		--redefine-sym func_800508B4=func_800508B4_o060Reloc \
		--redefine-sym func_8005ABA8=func_8005ABA8_o060Reloc \
		--redefine-sym func_8005AD64=func_8005AD64_o060Reloc \
		--redefine-sym levelGetBlurEffect=levelGetBlurEffect_o060Reloc \
		--redefine-sym mainChangeLevel=mainChangeLevel_o060Reloc \
		--redefine-sym mathRnd=mathRnd_o060Reloc \
		--redefine-sym rcpClearZBuffer=rcpClearZBuffer_o060Reloc \
		--redefine-sym sprintf=sprintf_o060Reloc \
		--redefine-sym viDisplayingScreen0=viDisplayingScreen0_o060Reloc \
		--add-symbol gOverlay60PrefixSwitchTableReloc=0x50,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x324:.rodata:gOverlay60PrefixSwitchTableReloc \
		0x32C:.rodata:gOverlay60PrefixSwitchTableReloc && \
	$(OBJCOPY) --remove-section=.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2B94
