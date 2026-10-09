# Overlay 14 POSTPROCESS rules.
# Included from mk/overlays.mk at the position these rules always held, so
# evaluation order is unchanged. Split out on 2026-10-02 when mk/overlays.mk
# crossed the clean-room 256 KiB tracked-file limit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14Reset.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ReturnOne.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ReturnOneCallbacks.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x18
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ReleaseOwner.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x28
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14FinalizeActiveHandle.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_800053D0=overlay14LookupObjectReloc \
		--redefine-sym func_8001EF1C=overlay14ApplyObjectPositionReloc \
		--redefine-sym func_800280FC=overlay14AcquireFirstReloc \
		--redefine-sym func_800389C0=overlay14AcquireSecondReloc \
		--redefine-sym func_80027F24=overlay14SubmitHandleReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14CallUpdate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14PrepareInputState.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_014_F0000B5C_1870434=overlay14PrepareInputState $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14AdvanceCommand.c.o: \
	config/normalizations/overlay14AdvanceCommand.filter.spec \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14AdvanceCommand.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay14AdvanceCommand.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay14InitializeMode=func_overlay_014_F0000000_186F8D8 \
		--redefine-sym gOverlay14Transition=D_D8 \
		--redefine-sym gOverlay14Cursor=D_DC \
		--redefine-sym overlay14ResetMode=func_overlay_014_F0000498_186FD70 \
		--redefine-sym overlay14ApplyValues=func_overlay_014_F0000328_186FC00 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1FC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14StepCommand.c.o: \
	config/normalizations/overlay14StepCommand.filter.spec \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14StepCommand.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay14StepCommand.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay14ResetMode=func_overlay_014_F0000498_186FD70 \
		--redefine-sym overlay14DispatchCommand=func_overlay_014_F0001040_1870918 \
		--redefine-sym overlay14MoveCommandCursor=func_overlay_014_F0000578_186FE50 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F00013F4_1870CCC.c.o: \
	config/normalizations/func_overlay_014_F00013F4_1870CCC.filter.spec \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F00013F4_1870CCC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_014_F00013F4_1870CCC.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym overlay14BuildPanel=func_overlay_014_F00012D8_1870BB0 \
		--redefine-sym overlay14CreateHandle=func_overlay_014_F0001830_1871108 \
		--redefine-sym overlay14DrawPrimitive=func_overlay_014_F0000000_186F8D8 \
		--redefine-sym gOverlay14Args2C=D_2C \
		--redefine-sym gOverlay14Args30=D_30 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14C
# The compiler's private pool for this unit is the seven-entry command-switch
# table; the retained overlay data segment already owns those bytes at
# data_rodata +0x174 (rodata-relative +0x54, which the shipped %hi/%lo pair
# encodes).  Rebind only metadata and discard the checked duplicate table;
# no instruction or compiler addend is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0001830_1871108.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/func_overlay_014_F0001830_1871108.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0001830_1871108.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym gOverlay14ValueC0=D_C0 \
		--add-symbol gOverlay14DrawJumpTableReloc=0x54,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_014_F0001830_1871108.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:33daf9d5887b336364b83ed63a3b51498c7b8b4c6adcbbf9605fe7e5436b8086 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x324
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0001540_1870E18.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2F0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F00009F4_18702CC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
# The dedicated source alias denotes runtime ORT 1652 / overlay 14 +0x1B54.
# The zero-field placeholder below is link scaffolding; promotion must retain
# the pre-postprocess alias and prove the runtime identity independently.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ResetMode.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_014_F0000498_186FD70=overlay14ResetMode \
		--redefine-sym overlay14ResetReleaseOwnerReloc=func_overlay_014_F0000000_186F8D8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0000000_186F8D8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x13C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F000013C_186FA14.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1E0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ApplyValues.c.o: \
	config/normalizations/overlay14ApplyValues.filter.spec \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ApplyValues.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay14ApplyValues.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym gOverlay14StateC8=D_C8 \
		--redefine-sym gOverlay14CommandCountEC=D_EC \
		--redefine-sym overlay14CreateValue=func_overlay_014_F00006FC_186FFD4 \
		--redefine-sym overlay14MoveCommandCursor=func_overlay_014_F0000578_186FE50 \
		--redefine-sym gOverlay14ResultF8=D_F8 \
		--redefine-sym gOverlay14QueuedCommands128=D_128 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x170
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14MoveCommandCursor.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_014_F0000578_186FE50=overlay14MoveCommandCursor $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x184
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14CreateValue.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		frontGetLanguage=frontGetLanguage_o014Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x180
# The seven compiler jump labels agree with the retained table at initialized
# data +0x158. Runtime LOCAL relocations use base +0x1D60 and addend +0x38.
# Rebind only metadata and discard the checked duplicate table; no instruction
# or compiler addend is edited. The source names authenticate both resident calls.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14LoadRelocatedValue.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay14LoadRelocatedValue.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14LoadRelocatedValue.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym mmAlloc=overlay14AllocateReloc \
		--redefine-sym piRomLoadSection=overlay14LoadReloc \
		--add-symbol gOverlay14LoadJumpTableReloc=0x38,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay14LoadRelocatedValue.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:27fa18303ce99b4b7e8fc171c369fbb9c388032a82113540a43005cbc3c96e36 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x178
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14UpdateTransition.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	config/normalizations/overlay14UpdateTransition.filter.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14UpdateTransition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay14UpdateTransition.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym func_overlay_014_F0001184_1870A5C=overlay14UpdateTransition \
		--redefine-sym gOverlay14TransitionValue=D_C0 \
		--redefine-sym overlay14PrepareReloc=func_overlay_014_F0000B5C_1870434 \
		--redefine-sym overlay14AdvanceReloc=func_overlay_014_F0000D68_1870640 \
		--redefine-sym overlay14RetreatReloc=func_overlay_014_F0000F64_187083C \
		--redefine-sym overlay14InitializeReloc=func_overlay_014_F0000000_186F8D8 \
		--redefine-sym overlay14DrawPrimaryReloc=func_overlay_014_F00013F4_1870CCC \
		--redefine-sym overlay14DrawAlternateReloc=func_overlay_014_F0001540_1870E18 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xF4:overlay14SetActiveReloc:func_overlay_014_F0000000_186F8D8 \
		0x13C:overlay14SetActiveReloc:func_overlay_014_F0000000_186F8D8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x154
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14DispatchCommand.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_014_F0001040_1870918=overlay14DispatchCommand $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x124
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14BuildRects.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		overlay14SubmitRectsReloc=func_overlay_014_F0000000_186F8D8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x11C
