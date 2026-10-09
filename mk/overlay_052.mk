# Overlay 52 POSTPROCESS rules.
# Included from mk/overlays.mk at the position the first of these rules held.
# Split out on 2026-10-07 to keep mk/overlays.mk under the clean-room
# 256 KiB tracked-file limit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Initialize.c.o: CFLAGS += \
	-Wab,-r4300_mul
# overlay52Initialize owns overlay 52's .data and .bss.  Its text reaches them
# through the object's own section symbols; the shipped words are
# section-relative, so every site is rebound to a zero-valued base.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Initialize.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	config/normalizations/func_overlay_052_F0000000_189A670.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Initialize.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay52InitDataBaseReloc=0x0,global \
		--add-symbol gOverlay52InitBssBaseReloc=0x0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_052_F0000000_189A670.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4F0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52PatchIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52CopyOffsetEntries.c.o: \
	config/normalizations/overlay52CopyOffsetEntries.sort.py \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52CopyOffsetEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xFC \
		00000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x40:overlay52QuerySecondaryModeReloc:overlay52QueryPrimaryModeReloc && \
	$(OBJCOPY) \
		--redefine-sym overlay52QueryPrimaryModeReloc=func_overlay_052_F0000000_189A670 \
		--redefine-sym gOverlay52Offsets=D_27C $@ && \
	$(HOST_PYTHON) config/normalizations/overlay52CopyOffsetEntries.sort.py $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Cleanup.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x90
# -Wab,-r4300_mul: as for overlay54TailA, IDO then emits the HUD easing as
# the shipped rotated branch-likely loop from a plain for loop. Matched C.
# It defines overlay 52's .data and .bss exactly as overlay52Initialize does
# (the shipped code is only reached with TU-local data); that object owns
# the bytes, so these copies are dropped and their sites rebound to
# zero-valued bases. The resident and overlay 45/56 names it reads and calls
# are renamed to per-module placeholders so the generated relocation
# surface values them from this module's stored addends.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52TailB.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52TailB.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	config/normalizations/func_overlay_052_F000063C_189ACAC.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52TailB.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym amSndPlay=amSndPlay_o052Reloc \
		--redefine-sym camSetNo=camSetNo_o052Reloc \
		--redefine-sym camSetScissor=camSetScissor_o052Reloc \
		--redefine-sym camStandardOrtho=camStandardOrtho_o052Reloc \
		--redefine-sym freeFrontEndItem=freeFrontEndItem_o052Reloc \
		--redefine-sym frontGet2PlayerSplit=frontGet2PlayerSplit_o052Reloc \
		--redefine-sym frontGetScreenMode=frontGetScreenMode_o052Reloc \
		--redefine-sym amTuneSetFade=amTuneSetFade_o052Reloc \
		--redefine-sym func_800016EC=func_800016EC_o052Reloc \
		--redefine-sym func_80005750=func_80005750_o052Reloc \
		--redefine-sym func_80028F54=func_80028F54_o052Reloc \
		--redefine-sym func_800290A0=func_800290A0_o052Reloc \
		--redefine-sym func_8002F618=func_8002F618_o052Reloc \
		--redefine-sym texDPInit=texDPInit_o052Reloc \
		--redefine-sym sprSetTextureFilter=sprSetTextureFilter_o052Reloc \
		--redefine-sym texAnimateSprite=texAnimateSprite_o052Reloc \
		--redefine-sym func_80037414=func_80037414_o052Reloc \
		--redefine-sym frontDrawObj=frontDrawObj_o052Reloc \
		--redefine-sym func_8003A590=func_8003A590_o052Reloc \
		--redefine-sym func_8003A7D0=func_8003A7D0_o052Reloc \
		--redefine-sym joyGetPressed=joyGetPressed_o052Reloc \
		--redefine-sym levelGetLevel=levelGetLevel_o052Reloc \
		--redefine-sym loadFrontEndItem=loadFrontEndItem_o052Reloc \
		--redefine-sym mainChangeCameras=mainChangeCameras_o052Reloc \
		--redefine-sym mainChangeLevel=mainChangeLevel_o052Reloc \
		--redefine-sym mainGetMode=mainGetMode_o052Reloc \
		--redefine-sym viGetCurrentSize=viGetCurrentSize_o052Reloc \
		--redefine-sym overlay45ReleaseDescriptor=overlay45ReleaseDescriptor_o052Reloc \
		--redefine-sym overlay45SetMode=overlay45SetMode_o052Reloc \
		--redefine-sym overlay56SplitTime=overlay56SplitTime_o052Reloc \
		--redefine-sym D_8007C180=D_8007C180_o052Reloc \
		--redefine-sym D_8007C1B0=D_8007C1B0_o052Reloc \
		--redefine-sym D_800C947C=D_800C947C_o052Reloc \
		--redefine-sym D_800D3140=D_800D3140_o052Reloc \
		--redefine-sym D_800D3144=D_800D3144_o052Reloc \
		--redefine-sym D_800D31C8=D_800D31C8_o052Reloc \
		--redefine-sym D_800D3550=D_800D3550_o052Reloc \
		--redefine-sym ext_o1_83e0=ext_o1_83e0_o052Reloc \
		$@ && \
	$(OBJCOPY) --add-symbol gOverlay52TailBDataBaseReloc=0x0,global \
		--add-symbol gOverlay52TailBBssBaseReloc=0x0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_052_F000063C_189ACAC.rebind.spec && \
	$(OBJCOPY) --remove-section=.data --remove-section=.bss \
		--remove-section=.gptab.data --remove-section=.gptab.bss $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A5C
