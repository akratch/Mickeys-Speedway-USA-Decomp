# Overlay 61 POSTPROCESS rules.
# Included from mk/overlays.mk at the position the first of these rules held.
# Split out on 2026-10-07 to keep mk/overlays.mk under the clean-room
# 256 KiB tracked-file limit.
# Two independent operations straddle the same source-line scheduling points
# in the shipped object. Assert IDO's natural order before restoring them.
# The source produces the shipped control flow and every memory operation, but
# IDO assigns two interchangeable integer webs to a1/a3 in the opposite order.
# Assert that bounded natural output before restoring the original coloring.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ChooseFileExtension.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xBC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ChooseFileExtension.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61RecordSize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x18
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61InitResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x21C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61UpdateInput.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ResetCounters.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61AddEntry.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_061_F00001DC_18BF5A4=overlay61AddEntry $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1E4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61DrawEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x404
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61DrawList.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_061_F00007C4_18BFB8C=overlay61DrawList $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61WriteCharacter.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_061_F00017B8_18C0B80=overlay61WriteCharacter $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ReadCharacter.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_061_F00018A0_18C0C68=overlay61ReadCharacter $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x110
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ReadCharacter.c.o: MIPSISET := -mips2 -32
# Exact C owns all 92 words and 11 relocation records; the measured function
# extent is 0x170 bytes with no target padding.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/func_overlay_061_F0001648_18C0A10.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x170
# The controller pak menu is instruction-exact. Its switch table is the
# retained overlay table at rodata +0x188: bind the table pair to that owner
# and drop the compiler's private copy by digest. No instruction changes. The
# seven resident callees are SYMBOL records and go through the _o061Reloc
# surface.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/func_overlay_061_F0000B84_18BFF4C.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/func_overlay_061_F0000B84_18BFF4C.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/func_overlay_061_F0000B84_18BFF4C.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym amSndPlay=amSndPlay_o061Reloc \
		--redefine-sym func_8002F618=func_8002F618_o061Reloc \
		--redefine-sym texAnimateTexSprite=texAnimateTexSprite_o061Reloc \
		--redefine-sym mainTitlePageInit=mainTitlePageInit_o061Reloc \
		--redefine-sym packDeleteFile=packDeleteFile_o061Reloc \
		--redefine-sym packDirectory=packDirectory_o061Reloc \
		--redefine-sym packFreeSpace=packFreeSpace_o061Reloc \
		--add-symbol gOverlay61MenuJumpTableReloc=0x188,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_061_F0000B84_18BFF4C.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:1c3aa751ee6883212c591b71174c4f646495df4b9b57e7233ebe9482fb199f92 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9F4
