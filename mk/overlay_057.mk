# Overlay 57 POSTPROCESS rules.
# Included from mk/overlays.mk at the position these rules always held, so
# evaluation order is unchanged. Split out on 2026-10-02 when mk/overlays.mk
# crossed the clean-room 256 KiB tracked-file limit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ApplyValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x68
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateInterface.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6CC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57InitializeMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57BeginMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x90
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57StartMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x98
# The module initializer is instruction-exact. Its mode switch table is the
# retained overlay table at rodata +0x6C: bind the two table references to
# that owner and drop the compiler's private copy by digest. Its resident
# callees go through the generated surface entries.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0000000_18A3BF8.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/func_overlay_057_F0000000_18A3BF8.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0000000_18A3BF8.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym animseqStartPath=animseqStartPath_o057Reloc \
		--redefine-sym fontColour=fontColour_o057Reloc \
		--redefine-sym func_8000590C=func_8000590C_o057Reloc \
		--redefine-sym func_80028F54=func_80028F54_o057Reloc \
		--redefine-sym func_8003A754=func_8003A754_o057Reloc \
		--redefine-sym func_8004B0A4=func_8004B0A4_o057Reloc \
		--redefine-sym func_800508B4=func_800508B4_o057Reloc \
		--redefine-sym func_8005AD64=func_8005AD64_o057Reloc \
		--redefine-sym initColourCycle=initColourCycle_o057Reloc \
		--redefine-sym joyResetMap=joyResetMap_o057Reloc \
		--add-symbol gOverlay57InitModeJumpTableReloc=0x6C,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_057_F0000000_18A3BF8.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:8331339c4a36b067bfd51dbb9354792b63fed3c4aa69893b7386269e8345fd0a && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x954
# The grid quad draw is instruction-exact. Its resident callees and data go
# through the generated surface entries; the trim only pins the size.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0001020_18A4C18.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym D_800D3140=D_800D3140_o057Reloc \
		--redefine-sym D_800D3144=D_800D3144_o057Reloc \
		--redefine-sym D_800D3148=D_800D3148_o057Reloc \
		--redefine-sym D_800D31C8=D_800D31C8_o057Reloc \
		--redefine-sym func_8002109C=func_8002109C_o057Reloc \
		--redefine-sym func_800221E8=func_800221E8_o057Reloc \
		--redefine-sym func_800349A4=func_800349A4_o057Reloc \
		--redefine-sym func_800367A4=func_800367A4_o057Reloc \
		--redefine-sym func_800508B4=func_800508B4_o057Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x958
# The mode dispatch is instruction-exact. Its switch table is the retained
# overlay table at rodata +0xB0: bind the two table references to that owner
# and drop the compiler's private copy by digest. No instruction changes.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0001AE8_18A56E0.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/func_overlay_057_F0001AE8_18A56E0.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0001AE8_18A56E0.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay57DispatchJumpTableReloc=0xB0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_057_F0001AE8_18A56E0.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:97d2a20db5917a686e817319bc36f7a4771d8e9799e1d0e8c0b6104c1f2dbdb6 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDCC
# The one-player menu step and start is instruction-exact. Its resident
# callees go through the generated surface entries; the trim only pins the size.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0004460_18A8058.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym amSndPlay=amSndPlay_o057Reloc \
		--redefine-sym animseqStartPath=animseqStartPath_o057Reloc \
		--redefine-sym animseqStopPath=animseqStopPath_o057Reloc \
		--redefine-sym func_80005548=func_80005548_o057Reloc \
		--redefine-sym func_800291B4=func_800291B4_o057Reloc \
		--redefine-sym func_8003A680=func_8003A680_o057Reloc \
		--redefine-sym joyCreateMap=joyCreateMap_o057Reloc \
		--redefine-sym mainChangeCameras=mainChangeCameras_o057Reloc \
		--redefine-sym mainChangeLevel=mainChangeLevel_o057Reloc \
		--redefine-sym mainSetAnimGroup=mainSetAnimGroup_o057Reloc \
		--redefine-sym mainSetMode=mainSetMode_o057Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7B8
# The menu step and start is instruction-exact. Its resident callees go
# through the generated surface entries; the trim only pins the size.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F00060F8_18A9CF0.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym amSndPlay=amSndPlay_o057Reloc \
		--redefine-sym animseqStartPath=animseqStartPath_o057Reloc \
		--redefine-sym animseqStopPath=animseqStopPath_o057Reloc \
		--redefine-sym joyCreateMap=joyCreateMap_o057Reloc \
		--redefine-sym mainChangeCameras=mainChangeCameras_o057Reloc \
		--redefine-sym mainChangeLevel=mainChangeLevel_o057Reloc \
		--redefine-sym mainSetMode=mainSetMode_o057Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6E4
# The middle-panel update is instruction-exact. Its resident callees go
# through the generated surface entries; the text is already 0x12E0 so the
# trim only pins the size. No instruction changes.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0004E18_18A8A10.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80000F94=func_80000F94_o057Reloc \
		--redefine-sym func_80005548=func_80005548_o057Reloc \
		--redefine-sym func_80022A50=func_80022A50_o057Reloc \
		--redefine-sym func_80025444=func_80025444_o057Reloc \
		--redefine-sym func_80025D60=func_80025D60_o057Reloc \
		--redefine-sym func_80028374=func_80028374_o057Reloc \
		--redefine-sym func_80028528=func_80028528_o057Reloc \
		--redefine-sym func_80028540=func_80028540_o057Reloc \
		--redefine-sym func_80028D24=func_80028D24_o057Reloc \
		--redefine-sym func_800291B4=func_800291B4_o057Reloc \
		--redefine-sym func_800291C4=func_800291C4_o057Reloc \
		--redefine-sym func_8002F618=func_8002F618_o057Reloc \
		--redefine-sym func_8002FB34=func_8002FB34_o057Reloc \
		--redefine-sym func_80039E34=func_80039E34_o057Reloc \
		--redefine-sym func_8003A680=func_8003A680_o057Reloc \
		--redefine-sym func_8003A700=func_8003A700_o057Reloc \
		--redefine-sym func_800429A4=func_800429A4_o057Reloc \
		--redefine-sym func_8004B0A4=func_8004B0A4_o057Reloc \
		--redefine-sym func_8004B0B8=func_8004B0B8_o057Reloc \
		--redefine-sym func_8004B0F8=func_8004B0F8_o057Reloc \
		--redefine-sym func_80050688=func_80050688_o057Reloc \
		--redefine-sym func_80050704=func_80050704_o057Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x12E0
# The packed-status finalizer is instruction-exact. Its three resident
# callees go through the generated surface entries; no instruction changes.
