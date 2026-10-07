# Overlay 84 POSTPROCESS rules.
# Included from mk/overlays.mk at the position the first of these rules held.
# Split out on 2026-10-07 to keep mk/overlays.mk under the clean-room
# 256 KiB tracked-file limit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84CopyPair.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84AdvanceCurrent.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_084_F0000DD0_18D12B0=overlay84AdvanceCurrent $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x148
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84SetBit.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetValues.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ActivateCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x194
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ClearActive.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ClearMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84SetAngle.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x28
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84Mark.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x24
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84SelectCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xAC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitializeAndUpdate.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_084_F0000048_18D0528=overlay84InitializeAndUpdate $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2CC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/func_overlay_084_F0000314_18D07F4.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym Arctanf=Arctanf_o084Reloc \
		--redefine-sym amSndPlay=amSndPlay_o084Reloc \
		--redefine-sym camGetPtr=camGetPtr_o084Reloc \
		--redefine-sym camSetNo=camSetNo_o084Reloc \
		--redefine-sym dAngle=dAngle_o084Reloc \
		--redefine-sym func_8000FAE0=func_8000FAE0_o084Reloc \
		--redefine-sym func_8002A8BC=func_8002A8BC_o084Reloc \
		--redefine-sym func_8002A8C0=func_8002A8C0_o084Reloc \
		--redefine-sym mathDiffAngle=mathDiffAngle_o084Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x740
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/func_overlay_084_F0000314_18D07F4.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetActive.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x28
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84IsUnitScale.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetEnabledCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitializeCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ResetCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x80
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84UpdateResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84RefreshCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF8
