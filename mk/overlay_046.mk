# Overlay 46 POSTPROCESS rules.
# Included from mk/overlays.mk at the position the first of these rules held.
# Split out on 2026-10-07 to keep mk/overlays.mk under the clean-room
# 256 KiB tracked-file limit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46Submit.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x24
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitializeState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x120
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46ReleaseState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitializeParticles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1D8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitializeBuffers.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitializeBuffers.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/func_overlay_046_F0000874_188EC6C.c.o: CFLAGS += -Wab,-r4300_mul
# The particle update and draw is instruction-exact. Its eleven resident
# callees go through the generated surface entries. Its private pool (two
# captions, then three step factors) duplicates retained overlay rodata: the
# captions sit at +0xC and +0x1C, which the anchor supplies, and the factors at
# +0x4C..+0x54, which the step-pool base supplies for the compiler's own
# +0x28..+0x30 offsets. The pool is dropped by digest. No instruction changes.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/func_overlay_046_F0000874_188EC6C.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/func_overlay_046_F0000874_188EC6C.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/func_overlay_046_F0000874_188EC6C.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym camStandardOrtho=camStandardOrtho_o046Reloc \
		--redefine-sym camDo2DSprite=camDo2DSprite_o046Reloc \
		--redefine-sym func_8002A8C0=func_8002A8C0_o046Reloc \
		--redefine-sym func_8002F618=func_8002F618_o046Reloc \
		--redefine-sym texDPInit=texDPInit_o046Reloc \
		--redefine-sym func_80037658=func_80037658_o046Reloc \
		--redefine-sym fontUseFont=fontUseFont_o046Reloc \
		--redefine-sym func_8004B0B8=func_8004B0B8_o046Reloc \
		--redefine-sym fontBackground=fontBackground_o046Reloc \
		--redefine-sym fontPrintXY=fontPrintXY_o046Reloc \
		--redefine-sym mathRnd=mathRnd_o046Reloc \
		--add-symbol gOverlay46ParticleStepPoolReloc=0x24,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_046_F0000874_188EC6C.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:3f747d583b175b6db0aa98ac4c009a476406195eba33de3c112c6bdba325d2e4 0xC && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x708
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/func_overlay_046_F0001228_188F620.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
# The trail and spark draw is instruction-exact. Its resident callees and the
# resident texture table go through the generated surface entries; the trim
# only pins the size.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/func_overlay_046_F0001228_188F620.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym D_800D31C8=D_800D31C8_o046Reloc \
		--redefine-sym amSndPlay=amSndPlay_o046Reloc \
		--redefine-sym camStandardOrtho=camStandardOrtho_o046Reloc \
		--redefine-sym camDoSprite=camDoSprite_o046Reloc \
		--redefine-sym func_8002A8BC=func_8002A8BC_o046Reloc \
		--redefine-sym func_8002A8C0=func_8002A8C0_o046Reloc \
		--redefine-sym texDPTextureX=texDPTextureX_o046Reloc \
		--redefine-sym mathRnd=mathRnd_o046Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x734
# The sequence update is instruction-exact. Its sixteen resident callees go
# through the generated surface entries, and its switch table is the retained
# overlay table at rodata +0x34: bind the two table references to that owner
# and drop the compiler's private copy by digest. No instruction changes.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46UpdateSequence.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay46UpdateSequence.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46UpdateSequence.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80000F94=func_80000F94_o046Reloc \
		--redefine-sym func_80001608=func_80001608_o046Reloc \
		--redefine-sym func_80028374=func_80028374_o046Reloc \
		--redefine-sym func_80028528=func_80028528_o046Reloc \
		--redefine-sym func_80028D30=func_80028D30_o046Reloc \
		--redefine-sym func_800291B4=func_800291B4_o046Reloc \
		--redefine-sym func_80036F08=func_80036F08_o046Reloc \
		--redefine-sym func_80037414=func_80037414_o046Reloc \
		--redefine-sym func_80037658=func_80037658_o046Reloc \
		--redefine-sym func_80037664=func_80037664_o046Reloc \
		--redefine-sym func_8003A680=func_8003A680_o046Reloc \
		--redefine-sym fontUseFont=fontUseFont_o046Reloc \
		--redefine-sym func_8004B0B8=func_8004B0B8_o046Reloc \
		--redefine-sym fontBackground=fontBackground_o046Reloc \
		--redefine-sym func_80058240=func_80058240_o046Reloc \
		--redefine-sym mathRnd=mathRnd_o046Reloc \
		--add-symbol gOverlay46SequenceJumpTableReloc=0x34,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay46UpdateSequence.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:de2a612733fe559800a48f993a4516fab0d7a6e0dbed588c95a4dd5f2cc3c7e2 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4F4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46UpdateTransition.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46UpdateTransition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0xC:5:gOverlay46DisplayState \
		0x14:6:gOverlay46DisplayState \
		0x18:4:camSetView \
		0x20:5:gOverlay46DisplayState \
		0x24:5:gOverlay46DisplayOutput \
		0x28:6:gOverlay46DisplayOutput \
		0x2C:4:func_80022B94 \
		0x30:6:gOverlay46DisplayState \
		0x84:4:overlay41IsUnitScale \
		0xD0:4:func_80028D30 \
		0x12C:5:gOverlay46FadeOutput \
		0x130:4:frontDrawObj \
		0x134:6:gOverlay46FadeOutput \
		0x13C:5:gOverlay46FadeOutput \
		0x140:6:gOverlay46FadeOutput && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
