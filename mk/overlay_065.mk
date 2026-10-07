# Overlay 65 POSTPROCESS rules.
# Included from mk/overlays.mk at the position the first of these rules held.
# Split out on 2026-10-07 to keep mk/overlays.mk under the clean-room
# 256 KiB tracked-file limit.
# Every call in the particle updater is an overlay-local call through the
# offset-zero carrier (jal 0 plus a runtime record): the first callee name is
# redefined onto the carrier and the other sites are rebound onto it.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65UpdateParticles.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65UpdateParticles.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym o65BeginDraw=func_overlay_065_F0000000_18C4268 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xB4:o65GetCamera:func_overlay_065_F0000000_18C4268 \
		0xC0:o65PrepareCamera:func_overlay_065_F0000000_18C4268 \
		0xCC:o65LoadCursor:func_overlay_065_F0000000_18C4268 \
		0x270:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x298:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x2AC:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x2BC:o65Sin:func_overlay_065_F0000000_18C4268 \
		0x2C8:o65Cos:func_overlay_065_F0000000_18C4268 \
		0x318:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x328:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x338:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x348:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x358:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x368:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x378:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x388:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x3B0:o65FindGround:func_overlay_065_F0000000_18C4268 \
		0x5F4:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x648:o65Cos:func_overlay_065_F0000000_18C4268 \
		0x680:o65Transform:func_overlay_065_F0000000_18C4268 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB40
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Release.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x80
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Initialize.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
# The zero-base spawn pool is already encoded in retail. Its camera/random
# calls use the overlay's offset-zero carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65SpawnRecord.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65SpawnRecord.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65SpawnRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x4C:5:D_0 0x64:6:D_0 && \
	$(OBJCOPY) --redefine-sym \
		o65GetCamera=func_overlay_065_F0000000_18C4268 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xCC:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0xE8:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x104:o65RandomRange:func_overlay_065_F0000000_18C4268 \
		0x11C:o65RandomRange:func_overlay_065_F0000000_18C4268
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65ResetSlots.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65ResetSlots.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
# The trail updater is compiled C. Its seven resident callees are renamed to
# this module's relocation-surface placeholders here, because an overlay linked
# at 0xF0000000 cannot `jal` a 0x8000xxxx entry point directly; without the
# renames the link fails with R_MIPS_26 overflow in a fresh worktree, where
# tools/reloc_surface.py has not yet rewritten the object.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/func_overlay_065_F0000C38_18C4EA0.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym camSetNo=camSetNo_o065Reloc \
		--redefine-sym func_80021970=func_80021970_o065Reloc \
		--redefine-sym func_800221E8=func_800221E8_o065Reloc \
		--redefine-sym func_8002A8BC=func_8002A8BC_o065Reloc \
		--redefine-sym func_8002A8C0=func_8002A8C0_o065Reloc \
		--redefine-sym func_800349A4=func_800349A4_o065Reloc \
		--redefine-sym mathRnd=mathRnd_o065Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDDC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/func_overlay_065_F0000C38_18C4EA0.c.o: \
	MIPSISET := -mips2 -32
