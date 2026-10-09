# Overlay 58 POSTPROCESS rules.
# Included from mk/overlays.mk at the position the first of these rules held.
# Split out on 2026-10-07 to keep mk/overlays.mk under the clean-room
# 256 KiB tracked-file limit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58FinalizePackedStatus.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80028F54=func_80028F54_o058Reloc \
		--redefine-sym func_800291B4=func_800291B4_o058Reloc \
		--redefine-sym func_8003A680=func_8003A680_o058Reloc $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58EnsureResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
# The race-order update is instruction-exact. Its four resident callees and
# overlay 56's time splitter go through the generated surface entries.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/func_overlay_058_F0000000_18AF1E8.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80028F54=func_80028F54_o058Reloc \
		--redefine-sym initColourCycle=initColourCycle_o058Reloc \
		--redefine-sym joyResetMap=joyResetMap_o058Reloc \
		--redefine-sym loadFrontEndList=loadFrontEndList_o058Reloc \
		--redefine-sym overlay56SplitTime=overlay58Overlay56SplitTimeReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C0
# Exact C. The compiler's private pool for this unit is the seven-entry state
# switch table followed by the 0.02f literal; the retained overlay data
# segment already owns those bytes at data_rodata +0x3D4 (rodata-relative
# +0x104, which the shipped %hi/%lo pairs encode). Rebind the four references
# to a pool symbol and discard the digest-checked duplicate, the whale's
# metadata-only form; no instruction or compiler addend is edited. The thirteen
# resident callees and overlay 41's scale test go through surface entries.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/func_overlay_058_F00005FC_18AF7E4.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/func_overlay_058_F00005FC_18AF7E4.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/func_overlay_058_F00005FC_18AF7E4.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym amSndPlay=amSndPlay_o058Reloc \
		--redefine-sym amSndStop=amSndStop_o058Reloc \
		--redefine-sym animseqStartPath=animseqStartPath_o058Reloc \
		--redefine-sym animseqStopPath=animseqStopPath_o058Reloc \
		--redefine-sym camSetView=camSetView_o058Reloc \
		--redefine-sym func_80028F54=func_80028F54_o058Reloc \
		--redefine-sym func_800291B4=func_800291B4_o058Reloc \
		--redefine-sym func_8003A680=func_8003A680_o058Reloc \
		--redefine-sym func_8005055C=func_8005055C_o058Reloc \
		--redefine-sym func_800508B4=func_800508B4_o058Reloc \
		--redefine-sym joyCreateMap=joyCreateMap_o058Reloc \
		--redefine-sym mainChangeCameras=mainChangeCameras_o058Reloc \
		--redefine-sym mainChangeLevel=mainChangeLevel_o058Reloc \
		--redefine-sym overlay41IsUnitScale=overlay41IsUnitScale_o058Reloc \
		--add-symbol gOverlay58StatePoolReloc=0x104,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_058_F00005FC_18AF7E4.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:ae888b2b74f48833a0133b07724e89710d4efed14898ebecea7763d646d347a8 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xCF4
# The compiler's private pool for this unit is the thirteen-entry mode-switch
# table; the retained overlay data segment already owns those bytes at
# data_rodata +0x3F4 (rodata-relative +0x124, which the shipped %hi/%lo pair
# encodes).  Rebind only metadata and discard the checked duplicate table;
# no instruction or compiler addend is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/func_overlay_058_F000138C_18B0574.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/func_overlay_058_F000138C_18B0574.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/func_overlay_058_F000138C_18B0574.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym overlay56SplitTime=overlay58Overlay56SplitTimeReloc \
		--add-symbol gOverlay58ModeJumpTableReloc=0x124,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_058_F000138C_18B0574.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:77de523043975c80a3e242600f52cf9cbb6ad5ed735965e74ed1e11bbf345b03 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3878
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawSegmentStrip.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_058_F0004C04_18B3DEC=overlay58DrawSegmentStrip $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x324
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58ReleaseResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58SetNodeValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C
