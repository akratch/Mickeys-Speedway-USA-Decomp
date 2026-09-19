# Overlay compile and object-normalization policy
# ---------------------------------------------------------------------------
#
# These are target-specific settings on objects in the ordinary O_FILES graph,
# not rules for loading or separately linking overlays. Flags affect IDO
# codegen. POSTPROCESS entries below are narrowly reviewed section trims or ELF
# symbol/relocation normalization; tools/postprocess_audit.py classifies them.
# The shipped relocation-table blobs themselves remain generated binary inputs.

# Overlay game code is likewise MIPS II. Every adopted tranche-A object was
# compared instruction-for-instruction at this ISA level before joining this
# rule; MIPS I inserts load-delay nops in several of them.
$(BUILD_DIR)/$(SRC_DIR)/overlays/%.c.o: MIPSISET := -mips2 -32

# overlay10Initialize's three rolled loops match only with the unroller off.
# Default IDO unroll grows the TU from 0x2B0 to 0x350. NON_MATCHING-only until
# the function is promoted; the GLOBAL_ASM path does not compile those loops.
ifeq ($(NON_MATCHING),1)
$(BUILD_DIR)/$(SRC_DIR)/overlays/o010/overlay10Initialize.c.o: CFLAGS += -Wo,-loopunroll,0
endif

# Build architecture note: this section does not implement Mickey's runtime
# overlay loader. Runtime loading and relocation live in src/main/runlink.c and
# are explained in docs/overlays.md sections 5.1-5.4. mickey.us.yaml and
# config/overlays.$(VERSION).json are the layout authorities. The rules below
# only record object-specific compiler flags, symbol/relocation metadata, and
# section-boundary handling needed to reproduce the original ROM from
# function-sized translation units.

# Every standalone, non-idempotent overlay objcopy symbol alias is declared in
# a strict JSON manifest and rendered deterministically. Mixed
# trim/filter/rebind chains remain beside their measured object policy below.
include mk/overlay_aliases.generated.mk

.PHONY: check-overlay-aliases test-overlay-aliases
check-overlay-aliases:
	$(HOST_PYTHON) $(TOOLS_DIR)/render_overlay_aliases.py --check

test-overlay-aliases:
	$(HOST_PYTHON) $(TOOLS_DIR)/test_render_overlay_aliases.py

check-docs: check-overlay-aliases
check-tooling: test-overlay-aliases

# The overlay 66 framebuffer renderer remains NON_MATCHING. It carried a
# `MIPSISET := -mips1 -32` override, adopted because a flag sweep scored that
# group closest -- 12 bytes short against 36 under an -O2 -g3 MIPS II override.
# The override is removed: the target holds two rounding-mode float conversions,
# which IDO cannot emit below -mips2, so -mips1 was never how this was built and
# `tools/check_isa_overrides.py` now says so. The sweep was reading a positional
# differing-word score, which rewards keeping instructions in place; on the
# shape-tolerant split, dropping the override takes byte-exact rows 80 -> 94 and
# structural differences 84 -> 64 while the positional count rises 181 -> 202.
# The two disagree and the target's own instruction set is the tiebreak. The
# -O2 pin below is kept; only the ISA claim was falsified.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o066/func_overlay_066_F00004E0_18C6948.c.o: OPT_FLAGS := -O2

# Rare's audio-bank patcher is an -O3 object in DKR and Mickey. Mickey keeps
# six source boundaries that preserve calls the whole-file DKR build inlines;
# the grouped consolidation probe reversed their emitted order as well.
OVERLAY5_O3_TUS := alSeqFileNew alBnkfNew _bnkfPatchBank _bnkfPatchInst \
                   _bnkfPatchSound _bnkfPatchWaveTable
OVERLAY5_O3_OBJECTS := $(addprefix $(BUILD_DIR)/$(SRC_DIR)/overlays/o005/, \
                       $(addsuffix .c.o,$(OVERLAY5_O3_TUS)))
$(OVERLAY5_O3_OBJECTS): $(BUILD_DIR)/$(SRC_DIR)/overlays/o005/%.c.o: \
                        $(SRC_DIR)/overlays/o005/%.c $(H_FILES) | $(ALL_DIRS) $(SPLAT_STAMP)
	$(CC) -c $(CFLAGS) -O3 -mips2 -32 -o $@ $<
	$(RUN_POSTPROCESS)

# IDO aligns standalone .text sections to 16 bytes, while these reviewed
# overlay functions continue at four-byte boundaries inside a larger module.
# The trimmer only reduces the ELF section header and refuses nonzero tails.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o006/overlay_006.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o076/overlay_076.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x114
$(BUILD_DIR)/$(SRC_DIR)/overlays/o078/overlay_078.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o102/overlay_102.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o103/overlay_103.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o106/overlay_106.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o107/overlay_107.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x28
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/_bnkfPatchBank.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/_bnkfPatchInst.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x98
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/_bnkfPatchSound.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/_bnkfPatchWaveTable.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6C

# rain_init, free_rain_memory and rain_update call the same unresolved resident
# target through different integer and float ABIs.  The source alias preserves IDO's shipped
# integer call sequence; canonicalize only that undefined symbol's name so the
# relocation identity agrees too.  No section contents are changed.
$(BUILD_DIR)/$(SRC_DIR)/main/weather.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym rainInitTrap=TrapDanglingJump $@ && \
	$(OBJCOPY) --redefine-sym rainFreeTrap=TrapDanglingJump $@

# overlay5InitializeAudio reaches the resident segment through the overlay's
# own relocation table; rebind each resident call to the module-local alias
# `gmake overlay-syms` mints for it, exactly as the other overlays do.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/overlay_005.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym alHeapDBAlloc=alHeapDBAlloc_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym alHeapInit=alHeapInit_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym alSurround_OutputType=alSurround_OutputType_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym alSurround_ReverbSetup=alSurround_ReverbSetup_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym func_80000450=func_80000450_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym func_80001740=func_80001740_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym func_80001BA0=func_80001BA0_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym func_800039F0=func_800039F0_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym func_8002B280=func_8002B280_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym func_8002B768=func_8002B768_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym func_8002E148=func_8002E148_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym func_8002E2E0=func_8002E2E0_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym func_8002E35C=func_8002E35C_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym gsSndpNew=gsSndpNew_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym n_alCSPSetMessageQ=n_alCSPSetMessageQ_o005Reloc $@ && \
	$(OBJCOPY) --redefine-sym osCreateMesgQueue=osCreateMesgQueue_o005Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x480
# Two independent operations straddle the same source-line scheduling points
# in the shipped object. Assert IDO's natural order before restoring them.
# The source produces the shipped control flow and every memory operation, but
# IDO assigns two interchangeable integer webs to a1/a3 in the opposite order.
# Assert that bounded natural output before restoring the original coloring.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ChooseFileExtension.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xBC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ChooseFileExtension.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay_007.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_007_F00000A8_185BF30=overlay7AcquireEntry $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x324
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/func_overlay_007_F0000324_185C1AC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x570
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/func_overlay_007_F0000324_185C1AC.c.o: CFLAGS += -Wo,-loopunroll,0
# This pool initializer is naturally instruction-exact. Its ten local-BSS
# records are already owned by overlay 7's shipped runtime relocation table,
# so retain their exact zero-base addends without static-link adjustment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay_007_tail.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay7DispatchModes.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay_007_tail.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_007_F0000894_185C71C=overlay7DispatchModes \
		--redefine-sym func_overlay_007_F0000AA0_185C928=overlay7UpdateOwnerMode \
		--redefine-sym func_overlay_007_F0000CCC_185CB54=overlay7DispatchSelection \
		--redefine-sym func_overlay_007_F0000DBC_185CC44=overlay7CommitSelection \
		--redefine-sym amSndPlay=amSndPlay_o007Reloc \
		--redefine-sym func_800031E8=func_800031E8_o007Reloc \
		--redefine-sym mathRnd=mathRnd_o007Reloc \
		--add-symbol gOverlay7ModeSwitchTableReloc=0x4,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay7DispatchModes.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x618:5:.bss 0x620:6:.bss \
		0x678:5:.bss 0x680:6:.bss \
		0x674:5:.bss 0x67c:6:.bss \
		0x708:5:.bss 0x70c:6:.bss \
		0x710:5:.bss 0x714:6:.bss \
		0x718:5:.bss 0x720:6:.bss && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:1cd7d0d68f99c402f2e0618199e1d010a1e31ab4b0cf0346275aef9c7c86d829 && \
	$(OBJCOPY) --remove-section=.rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x724
# Overlay 1 has three C islands separated by owned assembly.  Mixed
# -Wo,-loopunroll,4 / -Wab,-r4300_mul flag groups require five further
# boundaries; one object cannot span either an asm range or a flag change.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0000050_184C430=overlay1GetEntry \
		--redefine-sym func_overlay_001_F00001AC_184C58C=overlay1FindType47ByAngle \
		--redefine-sym func_overlay_001_F0000378_184C758=overlay1FindType5ByKey \
		--redefine-sym func_overlay_001_F0000414_184C7F4=overlay1FindPreviousUsable \
		--redefine-sym func_overlay_001_F00004B4_184C894=overlay1ActivateObject \
		--redefine-sym func_overlay_001_F0000614_184C9F4=overlay1FindClosestSample $@ && \
	$(OBJCOPY) --redefine-sym func_8000572C=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7B0 && \
	$(OBJCOPY) --add-symbol gOverlay1PhaseScaleLiteral=0xAC,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x20C:.rodata:gOverlay1PhaseScaleLiteral \
		0x214:.rodata:gOverlay1PhaseScaleLiteral && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:8a8d560f346578a2fea1b4b4f87e139d9d8baee3b9187bee3932a0ed72f7eb59
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_build.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_build.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_build.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x424
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_head.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_head.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0000CA8_184D088=overlay1InterpolatePath \
		--redefine-sym func_overlay_001_F0000DF4_184D1D4=overlay1ResolveMotionPoint \
		--redefine-sym func_overlay_001_F0000F84_184D364=overlay1MeasureCurves \
		--redefine-sym func_overlay_001_F00010C8_184D4A8=overlay1LoadBuildRecords \
		--redefine-sym func_overlay_001_F0000614_184C9F4=overlay1ModeResolverReloc \
		--redefine-sym func_overlay_001_F0001A54_184DE34=overlay1BuildObjectMappings \
		--redefine-sym sqrtf=sqrtf_o001Reloc \
		--redefine-sym splinePos=splinePos_o001Reloc \
		--redefine-sym func_8002A8BC=func_8002A8BC_o001Reloc \
		--redefine-sym func_8002A8C0=func_8002A8C0_o001Reloc $@ && \
	$(OBJCOPY) --redefine-sym overlay1SquareRoot=func_overlay_001_F0000000_184C3E0 $@ && \
	$(OBJCOPY) --redefine-sym overlay1AngleFromIndex=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x11A4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0001D78_184E158.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0001D78_184E158.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9CC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0002B4C_184EF2C.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0002B4C_184EF2C.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0002B4C_184EF2C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x70C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0003258_184F638.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0003258_184F638.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0003258_184F638.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x320
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_middle.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0002744_184EB24=overlay1FindNextAngle \
		--redefine-sym func_overlay_001_F000280C_184EBEC=overlay1FindPreviousAngle \
		--redefine-sym func_overlay_001_F000296C_184ED4C=overlay1AdvanceObjectGauges \
		--redefine-sym func_overlay_001_F0002AA4_184EE84=overlay1AdvanceGauge $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x408
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_tail.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_tail.c.o: config/normalizations/overlay1DispatchMode.rebind.spec
# The eight-entry table retains its raw owner at initialized-data +0x274.
# Its runtime LOCAL base is +0x110 into initialized data: bind the unchanged
# compiler pair to stored addend +0x164, without editing instruction fields.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_tail.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0003578_184F958=overlay1InitializeGaugeObjects \
		--redefine-sym func_overlay_001_F00036A0_184FA80=overlay1AssignRecordIndex \
		--redefine-sym func_overlay_001_F0003750_184FB30=overlay1ChoosePath \
		--redefine-sym func_overlay_001_F0003FD8_18503B8=overlay1TransitionState \
		--redefine-sym func_overlay_001_F000438C_185076C=overlay1UpdateObjectPhysics \
		--redefine-sym func_overlay_001_F0005BF4_1851FD4=overlay1StartTimerCallbacks \
		--redefine-sym overlay1GetObjectList=overlay1GetObjectListReloc \
		--redefine-sym sqrtf=overlay1SqrtReloc \
		--redefine-sym overlay1TrigX=overlay1TrigXReloc \
		--redefine-sym overlay1TrigY=overlay1TrigYReloc \
		--redefine-sym func_80005750=func_80005750_o001Reloc \
		--redefine-sym mathRnd=mathRnd_o001Reloc \
		--redefine-sym func_overlay_001_F0005ED4_18522B4=overlay1DispatchMode \
		--redefine-sym func_overlay_008_F0000F1C_185EC74=overlay1ModeAction2 \
		--redefine-sym overlay36SpawnDirectional=overlay1ModeAction3 \
		--redefine-sym overlay36SpawnLinked7F=overlay1ModeAction4 \
		--redefine-sym overlay36SpawnOffsetA9=overlay1ModeAction5 \
		--redefine-sym overlay36CallModeZero=overlay1ModeAction6 \
		--redefine-sym overlay36TickState=overlay1ModeAction7 \
		--redefine-sym overlay36UpdatePeers=overlay1ModeAction8 \
		--redefine-sym overlay36SpawnFinalEffect=overlay1ModeAction9 \
		--redefine-sym overlay1FindPreviousAngle=overlay1ModePreviousAngleReloc \
		--redefine-sym overlay1FindNextAngle=overlay1ModeNextAngleReloc \
		--redefine-sym func_overlay_001_F00061F0_18525D0=overlay1HandleCachedMode \
		--redefine-sym func_overlay_001_F00064F8_18528D8=overlay1SolveAngleCandidates \
		--redefine-sym func_overlay_001_F00067C0_1852BA0=overlay1UpdateRangeFlags \
		--redefine-sym func_overlay_001_F0006A14_1852DF4=overlay1ConsumeNearbyPending \
		--redefine-sym func_overlay_001_F0006D4C_185312C=overlay1UpdateAimedTransient \
		--redefine-sym func_8002A910=func_8002A910_o001Reloc \
		--redefine-sym func_8002A8BC=func_8002A8BC_o001Reloc \
		--redefine-sym func_8002A8C0=func_8002A8C0_o001Reloc \
		--redefine-sym func_overlay_001_F0007130_1853510=overlay1UpdateTransient \
		--redefine-sym func_overlay_001_F00072A4_1853684=overlay1AllocateRecord \
		--redefine-sym func_overlay_001_F0007344_1853724=overlay1CloneRecord \
		--redefine-sym gOverlay1ValueCache=D_8 \
		--redefine-sym func_overlay_001_F0007580_1853960=overlay1AppendPathPoint \
		--redefine-sym func_overlay_001_F0007730_1853B10=overlay1BendPathPoint \
		--redefine-sym func_overlay_001_F00078DC_1853CBC=overlay1AdvancePath \
		--redefine-sym func_overlay_001_F0007B64_1853F44=overlay1FindBestRecord $@ && \
	$(OBJCOPY) --redefine-sym overlay1GetRomlistInfoReloc=func_overlay_001_F0000000_184C3E0 $@ && \
	$(OBJCOPY) --redefine-sym func_8000572C=func_overlay_001_F0000000_184C3E0 $@ && \
	$(OBJCOPY) --redefine-sym func_80005820=func_overlay_001_F0000000_184C3E0 $@ && \
	$(OBJCOPY) --redefine-sym overlay4RemoveObject=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4664 && \
	candidate_rodata_size=$$( $(OBJDUMP) -h $@ | awk '$$2 == ".rodata" { print $$3; exit }' ); \
	if [ -n "$$candidate_rodata_size" ] && [ "$$candidate_rodata_size" != "00000000" ]; then \
		$(OBJCOPY) --add-symbol gOverlay1ModeTable=0x164,global $@ && \
		$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
			@config/normalizations/overlay1DispatchMode.rebind.spec && \
		$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
			sha256:553c0965d61758fe6e8486169ca6ae3a1c4def4245d49c53bdb058142fa7fead && \
		$(OBJCOPY) --remove-section=.rel.rodata $@; \
	fi
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_create.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_create.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym sqrtf=overlay1SqrtReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x190
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_end.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0007D6C_185414C=overlay1ResolvePathPoint $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x350
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_scaled.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_scaled.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58

$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3UpdateTimedEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3SelectTarget.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x150
# The typed body naturally reproduces the complete call/CFG/FP inventory.
# Select its one relocation-aware carrier cycle and complete private owner webs.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3RunCachedModeAction.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_003_F00000B8_1859DE8=overlay3RunCachedModeAction $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C4
# The natural source has the exact 77-word operation/CFG topology. Select the
# one complete four-use temporary allocation web with field-only guards.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3FindClosestObject.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_003_F000027C_1859FAC=overlay3FindClosestObject $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x134
# The measured R4300 multiply-hazard flag supplies the target FP spacing nop.
# Then select the complete carrier/schedule and two stack-owner webs.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3SelectScoredObject.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3SelectScoredObject.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_003_F00003B0_185A0E0=overlay3SelectScoredObject $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1D8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3TouchObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19Dispatch.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xAC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildOutput.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x134
# NON_MATCHING/GLOBAL_ASM: retain only friendly-name restoration and
# trailing-section trimming metadata for these extracted functions.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildPlanes.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildAdjacency.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1EC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19FindAdjacent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildSpatialMasks.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_019_F0000F58_18761B0=overlay19BuildSpatialMasks $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay_004.c.o: CFLAGS += -Wab,-r4300_mul
# The update function's resident calls use module-local stored addends. Keep
# the target-authenticated generator rebinds in the reproducible mixed
# alias/trim chain; symbol names change, never compiled section contents.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay_004.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80004590=func_80004590_o004Reloc \
		--redefine-sym func_8000590C=func_8000590C_o004Reloc \
		--redefine-sym func_80029274=func_80029274_o004Reloc \
		--redefine-sym func_8002997C=func_8002997C_o004Reloc \
		--redefine-sym func_8002A8C0=func_8002A8C0_o004Reloc \
		--redefine-sym func_8002AA0C=func_8002AA0C_o004Reloc \
		--redefine-sym func_8005ABA8=func_8005ABA8_o004Reloc \
		--redefine-sym sqrtf=func_overlay_004_F0000000_185A678 \
		--redefine-sym func_overlay_004_F0000138_185A7B0=overlay4UpdateObjectMotion $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xCAC
O8_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay_008.c.o
$(O8_OBJ): config/normalizations/overlay8UpdateChannels.rebind.spec
# -Wo,-loopunroll,0: the shipped +0x34A0 body walks its four-entry angle
# array as a single rolled do-while, while the default -O2 unroller emitted a
# four-wide body plus a runtime remainder prologue from the two-variable
# counter form this file used to carry.  The whole ROM still rebuilds
# byte-identically with the flag, so no already-matched function in this
# translation unit depends on unrolling.
#
# Re-measured against the current source: the flag is now byte-inert.  Every
# function in this unit compiles to identical .text with and without it, in
# both the canonical and -DNON_MATCHING builds, because the single-counter
# do-while spelling already denies the rotator its peel.  Keep it as the
# recorded constraint; it is not a lever for this unit's other unmatched
# functions.  See docs/ido-learnings.md.
$(O8_OBJ): CFLAGS += -Wab,-r4300_mul -Wo,-loopunroll,0
$(O8_OBJ): POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		gO8P34A0ScaleReloc=D_0 $@ && \
	$(OBJCOPY) --redefine-sym \
		gO8P34A0ModeReloc=D_0 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P34A0RandomReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P34A0TerrainReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P34A0EffectReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P34A0SetModeReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P34A0AnimateReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P34A0EventReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P34A0StateEffectReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P34A0ApproachReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P34A0TrigAReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P34A0TrigBReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P34A0DecayReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P34A0BlendReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		gO8P0058MirrorGateReloc=D_0 $@ && \
	$(OBJCOPY) --redefine-sym \
		gO8P0058PresentReloc=D_0 $@ && \
	$(OBJCOPY) --redefine-sym \
		gO8P0058ResultReloc=D_0 $@ && \
	$(OBJCOPY) --redefine-sym \
		gO8P0058ActiveReloc=D_0 $@ && \
	$(OBJCOPY) --redefine-sym \
		gO8P0058SpawnGateReloc=D_0 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058ResetReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058ModeReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058AcquireReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058SpawnReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058OrientReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058RotateReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058SurfaceReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058CollisionReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058EffectReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058SampleReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058UpdateReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058ReleaseReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058CreateReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P0058BounceReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8Call0894Reloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8StartMotionResourceReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8Approach291CReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8ApplyColorsReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P42A8SampleReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P42A8RandomReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P42A8ApproachReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P42A8TrigAReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8P42A8TrigBReloc=func_overlay_008_F0000000_185DD58 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8Call0894EmitReloc=func_overlay_008_F0002640_1860398 $@ && \
	$(OBJCOPY) --redefine-sym \
		o8Surface291CReloc=func_overlay_008_F0004CF0_1862A48 $@ && \
	$(OBJCOPY) --redefine-sym \
		gOverlay8SelectorScales=D_520 $@ && \
	$(OBJCOPY) --redefine-sym \
		overlay8SampleChannel=func_8002A878_o008Reloc $@ && \
	$(OBJCOPY) --redefine-sym \
		overlay8EmitChannel=func_8001F320_o008Reloc $@ && \
	$(OBJCOPY) --redefine-sym \
		func_overlay_008_F0003018_1860D70=overlay8UpdateChannels $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5128 && \
	$(OBJCOPY) --add-symbol \
		gOverlay8UpdateChannelConstants=0x1BC,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay8UpdateChannels.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:47d844f716456f64428bd9db585865758b727df87be434cc7be7f0d28f28a9a8

$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay_009.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay_009.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	config/normalizations/overlay9Output.filter.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay_009.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		ext_o0_1353c=func_overlay_009_F0000000_1866678 $@ && \
	$(OBJCOPY) --redefine-sym \
		ext_o0_7cd8=func_overlay_009_F0000000_1866678 $@ && \
	$(OBJCOPY) --redefine-sym \
		ext_o0_2b90=func_overlay_009_F0000000_1866678 $@ && \
	$(OBJCOPY) --redefine-sym \
		ext_o0_2952c=func_overlay_009_F0000000_1866678 $@ && \
	$(OBJCOPY) --redefine-sym \
		ext_o0_2d70=func_overlay_009_F0000000_1866678 $@ && \
	$(OBJCOPY) --redefine-sym \
		ext_o0_2c64=func_overlay_009_F0000000_1866678 $@ && \
	$(OBJCOPY) --redefine-sym \
		o9P540MathDiffAngleReloc=func_overlay_009_F0000000_1866678 $@ && \
	$(OBJCOPY) --redefine-sym \
		o9P540CosReloc=func_overlay_009_F0000000_1866678 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay9Output.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1520 && \
	candidate_rodata_size=$$( $(OBJDUMP) -h $@ | awk '$$2 == ".rodata" { print $$3; exit }' ); \
	if [ -n "$$candidate_rodata_size" ] && [ "$$candidate_rodata_size" != "00000000" ]; then \
		rodata_payload=$@.rodata.tmp; \
		$(OBJCOPY) --dump-section .rodata=$$rodata_payload $@ && \
		rodata_digest=$$(shasum -a 256 $$rodata_payload | cut -d' ' -f1); \
		case "$$rodata_digest" in \
			003a4315d6e2fefde7e81a4ff6174a90d55b37fa9f92f19c9d3c229ab00cca32) \
				$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
					sha256:003a4315d6e2fefde7e81a4ff6174a90d55b37fa9f92f19c9d3c229ab00cca32 ;; \
			b5a06959b28bb8c05743f313985499350feb8c5de91f76af25815341f0cb1ea5) \
				$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
					sha256:b5a06959b28bb8c05743f313985499350feb8c5de91f76af25815341f0cb1ea5 0x390 ;; \
			79fc408bc15884cfa8328f9fa40521762b8928b0ab0d3315414676fdff93f10f) \
				: "F0000CE4 promoted: the C pool now carries the module's own" \
				: "0x20..0x50 layout, so the addends are already module-relative" \
				: "and no base is added"; \
				$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
					sha256:79fc408bc15884cfa8328f9fa40521762b8928b0ab0d3315414676fdff93f10f ;; \
			*) echo "overlay 9: unreviewed .rodata digest $$rodata_digest"; exit 1 ;; \
		esac; \
		rodata_status=$$?; rm -f $$rodata_payload; exit $$rodata_status; \
	fi
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31InitializeParticleAssets.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_031_F00002E8_187F808=func_overlay_031_F00002E8_187F808 $@ && \
	$(OBJCOPY) --remove-section=.data --remove-section=.rel.data \
		--remove-section=.gptab.data $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay_012.c.o: CFLAGS += -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay_012.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/func_overlay_012_F00000C4_186D344.c.o: CFLAGS += -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/func_overlay_012_F00000C4_186D344.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym gOverlay12Resource0=D_8 \
		--redefine-sym gOverlay12Resource1=D_C \
		--redefine-sym gOverlay12Resource2=D_10 \
		--redefine-sym gOverlay12Resource3=D_14 \
		--redefine-sym gOverlay12Resource4=D_18 \
		--redefine-sym gOverlay12Resource5=D_1C $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF0
# The typed source naturally owns all 76 instruction words. The overlay-local
# globals and three runtime roles use the split target's established carriers;
# normalize only those symbol identities and trim compiler section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay_012_tail.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay_012_tail.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym gOverlay12Effects=D_20 \
		--redefine-sym gOverlay12EffectCount=D_0 \
		--redefine-sym overlay12Initialize=func_overlay_012_F0000000_186D280 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xD0:overlay12Lookup:func_overlay_012_F0000000_186D280 \
		0xFC:overlay12Lookup:func_overlay_012_F0000000_186D280 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x130
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/func_overlay_012_F00002E4_186D564.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym gOverlay12ParticleCount=D_4 \
		--redefine-sym gOverlay12Particles=D_1520 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/func_overlay_012_F00003A8_186D628.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/func_overlay_012_F00003A8_186D628.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x568
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/func_overlay_012_F0000910_186DB90.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/func_overlay_012_F0000910_186DB90.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x990
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
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/func_overlay_014_F0001830_1871108.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym gOverlay14ValueC0=D_C0 $@ && \
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
	$(OBJCOPY) --redefine-sym func_overlay_014_F00006FC_186FFD4=overlay14CreateValue $@ && \
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
		--redefine-sym func_8002B280=overlay14AllocateReloc \
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
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36CallGlobal.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36InitObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay_027.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_027_F0000A1C_187C3F4=overlay27UpdateCoordinates $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xBC0
O41_SAMPLE_CURVE_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41SampleCurve.c.o
$(O41_SAMPLE_CURVE_OBJ): config/normalizations/overlay41SampleCurve.rebind.spec
# The compiler reproduces the retained jump table and literal pool exactly.
# Keep their canonical raw owner and bind the C text to its module-local base;
# these steps change relocation metadata and section ownership, not instructions.
$(O41_SAMPLE_CURVE_OBJ): POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay41SampleCurveData=0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay41SampleCurve.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:0fc98a51a677ca2117008120dcc4e3985a3fe23612117a351d398c3f94f3bd00 && \
	$(OBJCOPY) --remove-section .rel.rodata $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41InterpolateAngle.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
# NON_MATCHING/GLOBAL_ASM: retain only friendly-name restoration where needed
# and trailing-section trimming metadata for these extracted functions.
# overlay41UpdateCurveObject's six float constants are the module's own
# +0x3C..+0x54 rodata, already shipped in overlay_041_data_rodata.bin. The C
# reproduces them exactly (guarded by the digest below), so anchor the
# references at +0x3C and contribute no bytes: emitting the section as well
# would append a second copy and grow the module by 0x20.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateCurveObject.c.o: CFLAGS += \
	-Wab,-r4300_mul
# The three-component sampler is reached through Overlay 41's own runtime
# export table, so every shipped call to it stores immediate zero. Give the
# site a module placeholder -- resolving the real same-overlay symbol would
# link the module-local target into the word instead.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateCurveObject.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_041_F00002AC_18875E4=overlay41SampleCurveReloc \
		$@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9F8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:0a69e9d2d2e87ba3149239321e9ffda3704af5b6c5c406af7af8907809282651 \
		0x3C && \
	$(OBJCOPY) --remove-section .rel.rodata $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41IsUnitScale.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41AdvanceStepRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x124
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateColorRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x188
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41UpdateProgress.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1CC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41ProcessEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1EC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41AddSlot.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC
O41_SPAWN_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41SpawnItems.c.o
$(O41_SPAWN_OBJ): config/normalizations/overlay41SpawnItems.rebind.spec
$(O41_SPAWN_OBJ): POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym mathRnd=overlay41RandomRangeReloc \
		--redefine-sym mathOneFloatPY=overlay41ConvertPairReloc \
		--redefine-sym func_overlay_012_F00001B4_186D434=overlay41SpawnEffectReloc \
		--redefine-sym func_overlay_041_F0001740_1888A78=overlay41SpawnItems $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x21C && \
	$(OBJCOPY) --add-symbol gOverlay41SpawnConstants=0x58,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay41SpawnItems.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:d7c6fadb83cccf1b66eb029d9abf6c67653a548a0a77831c47f6e70cd1af6ad4 && \
	$(OBJCOPY) --remove-section .rel.rodata $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41EnqueueTransition.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41EnqueueTransition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41TickTransitions.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x184
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41DrawItem.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41DrawItem.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_8004B0A4=overlay41SetModeReloc \
		--redefine-sym func_8004B0DC=overlay41SetColorReloc \
		--redefine-sym func_8004BA8C=overlay41MeasureReloc \
		--redefine-sym func_8004B0B8=overlay41SetColorIntensityReloc \
		--redefine-sym func_8004B0F8=overlay41DrawResourceReloc \
		--redefine-sym gOverlay41Resources=gOverlay41ResourcesReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53ReleaseResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x78
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54ReleaseResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29BuildChain.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x78
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29UpdateRatio.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29UpdateRatio.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29Sample.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x128
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29InitializeObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x198
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/func_overlay_029_F00005C4_187D874.c.o: CFLAGS += -Wab,-r4300_mul
ifneq ($(NON_MATCHING),1)
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/func_overlay_029_F00005C4_187D874.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x91C
endif
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29HandleEffects.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29HandleEffects.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x404
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29DrawGroups.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_029_F00014C8_187E778=overlay29DrawGroups $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x204
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay26InitializeObject.c.o: \
	config/normalizations/overlay26InitializeObject.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay26InitializeObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay26InitializeObject.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o051/overlay_051.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8AC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ResetFlags.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14GetFlagC4.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14GetFlagC8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14ReleaseCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3C
# overlay15DrawScreenStars reads its fade scale as the float literal
# 255.0f / 292.0f, so the compiler emits a one-word private pool that duplicates
# the retained data word at data_rodata +0x40 (the last word of
# gOverlay15InitializedData; the shipped LOCAL %hi/%lo pair encodes 0x0000 and
# the loader supplies the base).  Rebind the two references to a pool symbol
# and discard the digest-checked duplicate (overlay 86's metadata-only form).
O15_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay_015.c.o
$(O15_OBJ): \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay15DrawScreenStars.rebind.spec
$(O15_OBJ): CFLAGS += -Wab,-r4300_mul
$(O15_OBJ): POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_015_F000004C_18723E4=overlay15InitStarsAndPalette \
		--redefine-sym func_overlay_015_F0000428_18727C0=overlay15MoveStars \
		--redefine-sym func_overlay_015_F0000500_1872898=overlay15DrawScreenStars \
		--redefine-sym func_overlay_015_F00006E8_1872A80=overlay15InitStars \
		--redefine-sym func_overlay_015_F00009E0_1872D78=overlay15UpdateMovingStars \
		--redefine-sym func_overlay_015_F0000B94_1872F2C=overlay15DrawRain \
		--add-symbol gOverlay15FadePoolReloc=0x0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay15DrawScreenStars.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:eca26b0fe4ae2523bfa3733b2543e689b87d531790ea5c2b879f17144cade2a9 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC6C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34SetValue10.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
# The resident call cannot be valued under its own global name in
# overlay_undefined_syms.us.txt without moving the resident function for
# every resident caller; tools/reloc_surface.py derives a per-module
# placeholder for the R_MIPS_26 site and the rebind is declared here.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34InitStorage.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_034_F0000000_18811A8=overlay34InitStorage \
		--redefine-sym func_8002B280=func_8002B280_o034Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34InterpolateColor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34CreateRecord.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_034_F00000D4_188127C=overlay34CreateRecord $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1F4
include config/normalizations/overlay34Records.mk
include config/normalizations/overlay22Epoch12.mk
include config/normalizations/overlay46Epoch12.mk
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay41Ignore.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14
$(BUILD_DIR)/$(SRC_DIR)/overlays/o066/overlay66GetCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o066/overlay66Select.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61RecordSize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x18
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79SetLink.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x10
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79InitState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79UpdateTimers.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x44
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79FindNearby.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay79FindNearby.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0000FA0_18CDF40.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym sqrtf=overlay79SqrtReloc \
		--redefine-sym Arctanf=ext_o0_2a4c0 \
		--redefine-sym func_8002A8BC=ext_o0_2a46c $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2E0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0000FA0_18CDF40.c.o: CFLAGS += -Wab,-r4300_mul
# The exact C body retains overlay 79's shipped local-data relocations and
# resident call identities; only relocation metadata and alignment change.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0000000_18CCFA0.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0000000_18CCFA0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_079_F0000000_18CCFA0.filter.spec && \
	$(OBJCOPY) \
		--redefine-sym mathRnd=ext_o0_2952c \
		--redefine-sym func_8002A8C0=ext_o0_2a470 \
		--redefine-sym func_8002A8BC=ext_o0_2a46c \
		--redefine-sym func_8005AD64=ext_o0_5a914 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x134
ifeq ($(NON_MATCHING),0)
# This Phase-B body retains its assembly fallback until the source is exact.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0000134_18CD0D4.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC8
endif
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0000134_18CD0D4.c.o: CFLAGS += -Wab,-r4300_mul
# The second assembly fallback likewise needs only boundary trimming.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0001290_18CE230.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1EC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o080/overlay80InitializeContact.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o080/overlay80InitializeContact.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_080_F0000000_18CE8C8=overlay80InitializeContact $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x11C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o080/overlay80UpdateContact.c.o: CFLAGS += -Wab,-r4300_mul
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
# The natural O86 tail owns the exact 662-word semantic program and all 32
# call anchors. Four closed one-to-one schedule basins and the complete private
# register/frame web select retail codegen. The compiler literal/jump pool is
# asserted against the retained module data, while the loader remains sole
# owner of its six LOCAL HILO roles. All 32 static calls are folded to the raw
# overlay carrier only after their runtime identities have been independently
# censused.
# The compiler's private pool for this unit -- two float constants and the
# five-entry state-switch table -- duplicates the retained overlay data segment
# at data_rodata +0x80 (rodata-relative +0x0, which the shipped %hi/%lo pairs
# encode; the loader's six LOCAL HILO roles supply the base).  Rebind the six
# references to a pool symbol and discard the digest-checked duplicate; no
# instruction or compiler addend is edited (overlay 58's metadata-only form).
O86_0474_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/func_overlay_086_F0000474_18D22AC.c.o
$(O86_0474_OBJ): \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/func_overlay_086_F0000474_18D22AC.rebind.spec
$(O86_0474_OBJ): POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay86StatePoolReloc=0x0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_086_F0000474_18D22AC.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:08f3e1a4c518600a11dd66005593764a3a35e9d74386fd2bb4c863c379dbee52 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA58
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86ProcessCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7C
# The vector table address is loader-owned in the shipped overlay relocation
# table; retain its zero-base addend and bind the sole static call to the raw
# overlay carrier used by the extracted object.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86ScaledVectorPosition.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86ScaledVectorPosition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x18:5:gOverlay86Vectors 0x24:6:gOverlay86Vectors && \
	$(OBJCOPY) --redefine-sym \
		overlay86TransformVectorReloc=func_overlay_086_F0000000_18D1E38 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay86BuildTransform.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_086_F0000158_18D1F90=overlay86BuildTransform $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x18C
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o094/overlay94UpdateController.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_094_F0000110_18D6CB0=overlay94UpdateController \
		--redefine-sym func_80019AB8=func_80019AB8_o094Reloc \
		--redefine-sym func_800254FC=func_800254FC_o094Reloc \
		--redefine-sym func_8002565C=func_8002565C_o094Reloc \
		--redefine-sym func_8002A878=func_8002A878_o094Reloc \
		--redefine-sym func_8002A910=func_8002A910_o094Reloc \
		--redefine-sym func_8002B040=func_8002B040_o094Reloc \
		--redefine-sym func_8005ABA8=func_8005ABA8_o094Reloc \
		--redefine-sym func_8005AF14=func_8005AF14_o094Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x44C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o094/overlay94SetValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101AllocateEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F0002510_18DDD30.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x494
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F000512C_18E094C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5F0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F000571C_18E0F3C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6EC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F0005E08_18E1628.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5F0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F00063F8_18E1C18.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5F0
# func_overlay_101_F00069E8_18E2208 reaches the resident segment through the
# overlay's own relocation table; rebind each resident call to the module-local
# alias `gmake overlay-syms` mints for it, exactly as the other overlays do.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F00069E8_18E2208.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_800355A0=func_800355A0_o101Reloc $@ && \
	$(OBJCOPY) --redefine-sym func_80036DD0=func_80036DD0_o101Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF0C
# Tier A: the R4300 multiply scheduler resolves this TU's text junction.
# This flag applies only to this single-function translation unit.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F0003A58_18DF278.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F0003A58_18DF278.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_800355A0=func_800355A0_o101Reloc $@ && \
	$(OBJCOPY) --redefine-sym func_80036DD0=func_80036DD0_o101Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x16D4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F00078F4_18E3114.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x834
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F0008128_18E3948.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x834
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F000895C_18E417C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x834
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F0009190_18E49B0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x834
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101Reset.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101FindEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry12.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ActivateSlot.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101AdvanceSlot.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateByte17.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateByte16.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry8B.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateEntry8C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateFloat12.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x78
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateDelta16.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateByte18.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x118
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateGlobalPair.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x108
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateColor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x198
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildIntensityColors.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildBorder.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x13C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawPanel.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailAB4C.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailAB4C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9F8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailB544.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailBA34.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailC144.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailC144.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5A4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailC6E8.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101TailC6E8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4F4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawClock.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawClock.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_101_F000332C_18DEB4C=overlay101DrawClock $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3B8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildFrame.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101SetScissor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x198
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101Cleanup.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x158
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdatePresentation.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x174
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawChain.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x130
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateChains.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x184
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawSlots.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101SchedulePair.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101SchedulePair12.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleByte17.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleByte16.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o021/overlay21RegisterPlane.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x10C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o021/overlay21RegisterPlane.c.o: CFLAGS += -Wab,-r4300_mul
# Exact C owns all 114 words and nine relocation records. Trim only compiler
# section alignment beyond the function's measured 0x1C8-byte extent.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o021/overlay21ApplyPriorities.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o021/overlay21ApplyPriorities.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o030/overlay30Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2B4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o030/overlay30TransposePixels.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_030_F00002B4_187F1AC=overlay30TransposePixels $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x184
$(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23SpawnAttachments.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x208
$(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x148
$(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23Init.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x118
$(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay23Update.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o024/overlay_024.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x414 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		3e99999a000000000000000000000000
# The exact initializer needs the measured R4300 multiply-hazard schedule.
# Its resident call names carry the stable runtime identities until objcopy
# restores the overlay's stored-zero link aliases. The updater remains asm.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay_025.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay_025.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_8002A8C0=overlay25SinReloc \
		--redefine-sym func_8002A8BC=overlay25CosReloc \
		--redefine-sym func_800299E8=overlay25RandomReloc \
		--redefine-sym func_overlay_025_F000017C_1879E04=overlay25UpdateEffect $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x608
$(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay_056.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xAF4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o039/overlay_039.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x168
$(BUILD_DIR)/$(SRC_DIR)/overlays/o039/overlay_039.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x114
# The typed reconstruction naturally owns 852 bytes plus one proved zero
# alignment word. Extend that word into the symbol, select the complete guarded
# frame/register/FP/schedule bijection, and bind resident calls to the overlay's
# stored-zero runtime proxy without collapsing the relocation sites.
# Target emits the R4300 FP-mul hazard nop after the else-arm blend muls;
# without this pass the object is one word short (measured 213 vs 214).
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37Render.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37Render.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_037_F000019C_18857BC=overlay37RenderEffect $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x358
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37RecordMinimum.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay37RecordActive.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40AddEntry.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_040_F0000000_18868B0=overlay40AddEntry $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x84
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40DrawEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x164
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40RemoveEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40UpdateEntries.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_040_F00000E8_1886998=overlay40UpdateEntries $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40BuildFrame.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_040_F00001A0_1886A50=overlay40BuildFrame $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x144
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40SetValues.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40Interpolate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40DrawTintRectangle.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_040_F0000534_1886DE4=overlay40DrawTintRectangle $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay40FadeRecords.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_040_F0000690_1886F40=overlay40FadeRecords $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x194
$(BUILD_DIR)/$(SRC_DIR)/overlays/o042/overlay_042.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x700
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43InitializeState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x194
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43FlushPending.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43FlushPending.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x004:5:D_C8 \
		0x008:6:D_C8 \
		0x038:5:ext_4d258 \
		0x03c:6:ext_4d258 \
		0x09c:5:D_C8 \
		0x0a0:6:D_C8 \
		0x0c4:5:D_C8 \
		0x0e0:6:D_C8 && \
	$(OBJCOPY) --redefine-sym \
		func_8002E800=func_overlay_043_F0000000_1889FD0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x094:osRecvMesg:func_overlay_043_F0000000_1889FD0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
# Six resident release roles are stored through one overlay carrier. The source
# is otherwise instruction-natural; rebind the exceptional release site and
# trim only compiler section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43ReleaseResources.c.o: \
	config/normalizations/overlay43ReleaseResources.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43ReleaseResources.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_8002B768=func_overlay_043_F0000000_1889FD0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay43ReleaseResources.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/func_overlay_043_F0000324_188A2F4.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/func_overlay_043_F0000BE4_188ABB4.c.o: \
	MIPSISET := -mips1 -32
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/func_overlay_043_F0000BE4_188ABB4.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C4
ifneq ($(NON_MATCHING),0)
# Retail overlay43 stores this local D_0 reference without reloc records.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43ComputeMotion.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43ComputeMotion.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x090:5:D_0 0x094:6:D_0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC
else
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43ComputeMotion.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC
endif
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43AllocateResources.c.o: \
	config/normalizations/overlay43AllocateResources.calls.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43AllocateResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay43AllocateResources.calls.spec && \
	$(OBJCOPY) --redefine-sym \
		func_8002B280=func_overlay_043_F0000000_1889FD0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43SubmitChildren.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x114
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay43FilterImage.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_043_F0001378_188B348=overlay43FilterImage $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xAC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o044/overlay44CreateAnimationState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x224
$(BUILD_DIR)/$(SRC_DIR)/overlays/o044/overlay44ReleaseHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x70
# NON_MATCHING/GLOBAL_ASM: restore the friendly symbol and retain the
# trailing-section trim for the extracted function.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o044/overlay44UpdateFrameCache.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_044_F0000294_188BAF4=overlay44UpdateFrameCache $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2EC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o044/func_overlay_044_F0000580_188BDE0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x574
$(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay69Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C
# The source naturally owns the exact frame, schedule, and 73 instruction
# words. The shipped overlay table retains four distinct runtime callees; the
# split target normalizes those sites to its offset-zero proxy, so fold the
# role names through one carrier and preserve that established proxy binding.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay69UpdateAnchor.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay69UpdateAnchor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x80:overlay69AngleReloc:overlay69RotateVectorReloc \
		0x94:overlay69SinReloc:overlay69RotateVectorReloc \
		0xc0:overlay69CosReloc:overlay69RotateVectorReloc && \
	$(OBJCOPY) --redefine-sym \
		overlay69RotateVectorReloc=func_overlay_069_F0000000_18C8A68 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x124
# Overlay 69 and overlay 88 ship the same reviewed renderer bytes, but each
# remains an independent object verdict. Splat emits overlay 69's synthetic
# symbol; restore its friendly name and discard only section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay69DrawSortedGeometry.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_069_F0000170_18C8BD8=overlay69DrawSortedGeometry $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x59C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o067/overlay_067.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x14C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/overlay71UpdateCoordinates.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
# NON_MATCHING/GLOBAL_ASM: retain only trailing-section trims where these
# extracted functions are not naturally aligned.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000000_18C9B20.c.o: \
	config/normalizations/overlay71InitializeObject.calls.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000000_18C9B20.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay71InitializeObject.calls.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x278
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000278_18C9D98.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000278_18C9D98.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80032BF0=func_80032BF0_o071Reloc \
		--redefine-sym sqrtf=sqrtf_o071Reloc $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000870_18CA390.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/func_overlay_071_F0000870_18CA390.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2D8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o072/overlay_072.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x168
$(BUILD_DIR)/$(SRC_DIR)/overlays/o073/overlay73Draw.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138
$(BUILD_DIR)/$(SRC_DIR)/overlays/o074/overlay74Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o074/overlay74Init.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75MarkSlot.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x24
$(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x214
$(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75Init.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75UpdateMovingObject.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay75UpdateMovingObject.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_075_F0000214_18CC17C=overlay75UpdateMovingObject $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o077/overlay_077.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3B8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o077/overlay_077.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o077/overlay_077_tail.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x78
$(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay_081.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay_081.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o082/overlay_082.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
$(BUILD_DIR)/$(SRC_DIR)/overlays/o082/overlay_082.c.o: CFLAGS += -Wo,-loopunroll,2
$(BUILD_DIR)/$(SRC_DIR)/overlays/o082/overlay_082_tail.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay88Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C
# Source naturally reproduces every instruction word and the seven retail
# runtime relocation sites. The split target aliases those sites to its
# offset-zero static proxy, so fold only the role names and trim alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay88UpdateAnchor.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay88UpdateAnchor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x60:overlay88UpdateNodeReloc:overlay88PrepareNodeReloc \
		0x80:overlay88RefreshNodeReloc:overlay88PrepareNodeReloc \
		0xc4:overlay88ForwardVectorReloc:overlay88PrepareNodeReloc \
		0xd0:overlay88AngleReloc:overlay88PrepareNodeReloc \
		0xec:overlay88SinReloc:overlay88PrepareNodeReloc \
		0x118:overlay88CosReloc:overlay88PrepareNodeReloc && \
	$(OBJCOPY) --redefine-sym \
		overlay88PrepareNodeReloc=func_overlay_088_F0000000_18D3A88 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x158
$(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay88DrawSortedGeometry.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_088_F00001A4_18D3C2C=overlay88DrawSortedGeometry $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x59C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89UpdateEffect.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89UpdateEffect.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89Evaluate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x70
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89InitializeEffect.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89InitializeEffect.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_089_F0000270_18D44A0=overlay89InitializeEffect $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x334
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay89UpdateStateAndParticles.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o092/overlay92Init.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x68
$(BUILD_DIR)/$(SRC_DIR)/overlays/o092/overlay92FindNearestCourse.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o092/func_overlay_092_F0000308_18D6228.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o092/func_overlay_092_F0000308_18D6228.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x728
$(BUILD_DIR)/$(SRC_DIR)/overlays/o093/overlay_093.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o093/overlay_093.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o095/overlay_095.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1D8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitRadius.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x184
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitBounds.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1FC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96Unregister.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_096_F0000070_18D76A8=overlay96Unregister $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96BuildVolume.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96BuildVolume.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3C4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96FindVolume.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96TestBit.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay96DrawObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x10C
# Restore NON_MATCHING fallback symbols and trim the matched updater's merged
# trailing tail. These renames change symbol metadata only.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98CollectUniqueY.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_098_F0000000_18D89C0=overlay98CollectUniqueY $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x144
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98RenderReflections.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_098_F0000234_18D8BF4=overlay98RenderReflections $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x614
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98CheckObject.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_098_F0000848_18D9208=overlay98CheckObject $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1BC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay98CheckObject.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitRadius.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97CopyAngles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitTransform.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitSelection.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitPlane.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x110
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitPlane.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97CreateDescriptor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97AssignState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitDirection.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x130
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitDirection.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay97InitScale.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100RemoveEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
# IDO emits the three independent loop initializers in the opposite legal
# order. Assert that exact output before restoring the shipped schedule.
# The source is exact except for one complete stack-home lifetime. Preserve the
# independently decoded three-call runtime identities while using the common
# pre-loader relocation carrier required by the configured overlay link.
# Natural source supplies the exact pool traversal and relocation-bearing
# local addends. Select retail's equivalent private suffix register web.
# Preserve the complete initializer instruction/relocation permutation and
# bounded temporary web, then fold its three runtime calls to the pre-loader
# carrier while retaining their shipped table identities.
# The natural source supplies the exact cache traversal, integer/FP conversion
# paths, and local relocation pair. Select retail's equivalent two-register web.
# The natural body owns the exact boundary, frame, CFG, FP schedule, calls,
# delay slots, memory effects, and all eight runtime relocations. Select only
# the two interchangeable private GPR-color webs and fold the resident sqrtf
# call onto its shipped pre-loader relocation carrier.
# R4300 multiply hazards are target-proven for this exact TU. Natural source
# supplies the complete instruction stream and all ten runtime relocations;
# fold only the independently decoded resident sqrtf call to its shipped
# pre-loader carrier.
# Natural source owns the exact 162-op schedule, frame, CFG, memory effects,
# stack layout, and all 22 relocation sites. Select only the complete private
# temporary-color web, then fold the two independently decoded external routes
# onto their shipped pre-loader carriers.
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87InitializeObject.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87InitializeObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xe0:mathRnd:overlay87InitializeObject \
		0x10c:func_8005AD64:overlay87InitializeObject && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x128
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/func_overlay_087_F0000128_18D3090.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/func_overlay_087_F0000128_18D3090.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x768
# All four random-range calls are instruction-natural and share retail's
# offset-zero stored overlay carrier; retain distinct runtime identities in
# the authoritative relocation ledger and trim only section alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay_045.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		overlay45RandomRangeStoredReloc=func_overlay_045_F0000000_188C458 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x764
O45_RENDER_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/func_overlay_045_F0000764_188CBBC.c.o
$(O45_RENDER_OBJ): \
	config/normalizations/func_overlay_045_F0000764_188CBBC.rebind.spec
$(O45_RENDER_OBJ): CFLAGS += -Wab,-r4300_mul
# Preserve the compiler's exact instruction fields while binding its private
# literal-pool references to the three identical constants already retained
# in Overlay 45's initialized-data owner. The duplicate pool is discarded only
# after its full payload digest agrees. Call/scissor renames change symbols,
# not instructions; the runtime relocation table remains the identity oracle.
$(O45_RENDER_OBJ): POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80034920=overlay45DisplayCallReloc \
		--redefine-sym func_8004B0A4=overlay45FontCallReloc \
		--redefine-sym func_800221E8=overlay45MatrixPushReloc \
		--redefine-sym func_80022A50=overlay45MatrixLoadReloc \
		--redefine-sym func_8004B0B8=overlay45ColorCallReloc \
		--redefine-sym func_8002997C=overlay45RandomRangeReloc \
		--redefine-sym func_8002A8C0=overlay45FloatCallReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9F4 && \
	$(OBJCOPY) \
		--add-symbol gOverlay45RetainedMotionConstants=0x24,global \
		--add-symbol overlay45ScissorReloc=0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_045_F0000764_188CBBC.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:c52cb05fd2fed4ffaf80d0f50519a1b99d267a85960aeefe5e036df19162ad00
ifeq ($(NON_MATCHING),0)
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/func_overlay_045_F0001158_188D5B0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA88
endif
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay_045_tail.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3C
# The selector table is the overlay-local +0x510 address already encoded in
# retail, so retain that addend without a static-link relocation. The three
# runtime calls use the extracted range's offset-zero carrier; their distinct
# identities remain authoritative in overlay 47's shipped relocation tables.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/overlay47SpawnObject.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/overlay47SpawnObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x10:5:D_0 0x20:6:D_0 && \
	$(OBJCOPY) --redefine-sym \
		func_8000590C=func_overlay_047_F0000000_1890E18 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x94:func_80005768:func_overlay_047_F0000000_1890E18 \
		0xB0:func_8005AD64:func_overlay_047_F0000000_1890E18 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/overlay47ReleaseResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x160
$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/func_overlay_047_F0000B30_1891948.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
# The compiler's six-entry table is the same table retained in overlay 47's
# initialized aggregate at runtime-local +0x56C. Rebind the text pair to that
# owner and discard only the duplicate private section. The canonical
# GLOBAL_ASM fallback has neither candidate relocation, so leave that object
# untouched; promotion_trial.py splices the C body into the normal build and
# exposes both asserted .rodata relocations before this normalization runs.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/func_overlay_047_F0000B30_1891948.c.o: POSTPROCESS = \
	if $(OBJDUMP) -r $@ | awk \
		'$$1 == "00000f04" && $$3 == ".rodata" { hi = 1 } \
		 $$1 == "00000f0c" && $$3 == ".rodata" { lo = 1 } \
		 END { exit !(hi && lo) }'; then \
		$(OBJCOPY) --add-symbol overlay47DispatchSwitchTable=0x56C,global $@ && \
		$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
			0xF04:.rodata:overlay47DispatchSwitchTable \
			0xF0C:.rodata:overlay47DispatchSwitchTable && \
		$(OBJCOPY) --remove-section=.rodata $@; \
	fi
$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/func_overlay_047_F0000000_1890E18.c.o: CFLAGS += \
	-Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/func_overlay_047_F0000000_1890E18.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9D0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68PayloadLimit.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8
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
$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/func_overlay_061_F0000B84_18BFF4C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9F4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o085/overlay_085.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x29C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o085/overlay_085.c.o: CFLAGS += -Wab,-r4300_mul
# NON_MATCHING/GLOBAL_ASM per docs/acceleration-survey.md sec.13.2: this
# object's instructions used to be reached by rewriting three fields after
# compilation (normalize_elf_instructions.py), which no gold-standard N64
# decomp does. The .c now GLOBAL_ASMs the extracted retail bytes instead;
# only the symbol rename below (metadata, not instructions) survives.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14GetFlagCC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ReleaseHandle.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ReleaseTree.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7C
# The typed resource initializer is exact; discard only compiler alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ConfigureResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateObjectResource.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_020_F0000204_18767DC=overlay20UpdateObjectResource $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x188
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/func_overlay_020_F000038C_1876964.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x438
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/func_overlay_020_F000038C_1876964.c.o: OPT_FLAGS := -O2 -g3
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20BuildTileCommands.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x218
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20RemoveEntry.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_020_F0001018_18775F0=overlay20RemoveEntry $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
# The typed entry allocator is exact. Retail encodes three zero-base data
# references directly and retains relocations only for the active mask/pool.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ConfigureEntry.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ConfigureEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x18:5:gOverlay20EntryCount 0x1C:6:gOverlay20EntryCount \
		0x78:5:gOverlay20Entries 0x80:6:gOverlay20Entries \
		0xC4:5:D_0 0x108:6:D_0 && \
	$(OBJCOPY) \
		--redefine-sym gOverlay20ActiveBits=D_4 \
		--redefine-sym gOverlay20Pool=D_80 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x150
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ReleaseEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20MarkNested.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20AdvanceEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20CreateEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20DrawResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateGrid.c.o: CFLAGS += \
	-Wab,-r4300_mul -DEXPLICIT_BOUNDS -DSCAN_TOP_LOAD
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateGrid.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_020_F0000A68_1877040=overlay20UpdateGrid $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x35C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/func_overlay_020_F0001148_1877720.c.o: $(TOOLS_DIR)/set_elf_flags.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/func_overlay_020_F0001148_1877720.c.o: MIPSISET := -mips3 -32
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/func_overlay_020_F0001148_1877720.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/func_overlay_020_F0001148_1877720.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym sqrtf=overlay20TailSqrtReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/set_elf_flags.py $@ 0x10000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x348
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31CreateRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB8
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31BuildLookupTables.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_031_F0000000_187F520=func_overlay_031_F0000000_187F520 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2E8
# Exact C owns all 245 words and 54 relocation records. Trim only compiler
# section alignment beyond the function's measured 0x3D4-byte extent.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31InitializeBuffers.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3D4
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31CreatePool.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_031_F0000E7C_188039C=overlay31CreatePool $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31BuildPalettes.c.o: \
	config/normalizations/overlay31BuildPalettes.filter.spec \
	config/normalizations/overlay31BuildPalettes.calls.spec \
	$(TOOLS_DIR)/trim_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31BuildPalettes.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1B8 \
		0000000000000000 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay31BuildPalettes.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay31BuildPalettes.calls.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33CallA.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
# NON_MATCHING/GLOBAL_ASM: retain only friendly-name restoration and
# trailing-section trimming metadata for these extracted functions.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33InitializeBuffers.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_033_F0000000_18807E8=overlay33InitializeBuffers $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x144
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33ReleaseGlobal.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33CallB.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
# The resident rcpFast3d call cannot be valued under its own global name in
# overlay_undefined_syms.us.txt; tools/reloc_surface.py derives a per-module
# placeholder and the rebind is declared here so a fresh worktree still links.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33PresentAndSwap.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_033_F000066C_1880E54=overlay33PresentAndSwap \
		--redefine-sym rcpFast3d=rcpFast3d_o033Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36CallModes.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
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
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/func_overlay_046_F0000874_188EC6C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x708
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/func_overlay_046_F0001228_188F620.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,0
ifneq ($(NON_MATCHING),1)
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/func_overlay_046_F0001228_188F620.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x738
endif
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46UpdateTransition.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46UpdateTransition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0xC:5:gOverlay46DisplayState \
		0x14:6:gOverlay46DisplayState \
		0x18:4:func_800221E8 \
		0x20:5:gOverlay46DisplayState \
		0x24:5:gOverlay46DisplayOutput \
		0x28:6:gOverlay46DisplayOutput \
		0x2C:4:func_80022B94 \
		0x30:6:gOverlay46DisplayState \
		0x84:4:overlay41IsUnitScale \
		0xD0:4:func_80028D30 \
		0x12C:5:gOverlay46FadeOutput \
		0x130:4:func_80039E34 \
		0x134:6:gOverlay46FadeOutput \
		0x13C:5:gOverlay46FadeOutput \
		0x140:6:gOverlay46FadeOutput && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
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

# The typed source owns O64's complete procedural texture generator. IDO's
# natural stream contains four redundant representations; the target-local
# digest-guarded preparation removes exactly those words before a complete
# decoded schedule/register selection. Restore all 20 shipped runtime carrier
# records, then expose only the two configured R26 call records; the retained
# relocation tail owns the 18 loader-local HILO records.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o064/overlay64GenerateTexture.c.o: CFLAGS += -woff 835

$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/func_overlay_038_F0000000_1885D10.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x154

$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/overlay38UpdateParticles.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/overlay38UpdateParticles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x328

$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/func_overlay_038_F000047C_188618C.c.o: CFLAGS += \
	-Wab,-r4300_mul -DO38_TRANSFORM_TAIL -DO38_TAIL_SIZE=8 \
	-DO38_VOLATILE_TEST -DO38_POOL_CURSOR -DO38_FINISH_ONE \
	-DO38_VOLATILE_FINAL
$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/func_overlay_038_F000047C_188618C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x36C


$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000000_18C91C8.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000000_18C91C8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x078:5:gOverlay70FloatTableReloc \
		0x094:6:gOverlay70FloatTableReloc \
		0x098:5:gOverlay70VerticalStepReloc \
		0x0ac:6:gOverlay70VerticalStepReloc \
		0x09c:5:gOverlay70AngleReloc \
		0x0bc:6:gOverlay70AngleReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x028:overlay70RandomRange:func_overlay_070_F0000000_18C91C8 \
		0x038:overlay70RandomRange:func_overlay_070_F0000000_18C91C8 \
		0x04c:overlay70RandomRange:func_overlay_070_F0000000_18C91C8 \
		0x05c:overlay70RandomRange:func_overlay_070_F0000000_18C91C8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8

# Retail overlay70 routes this carrier's local calls through offset zero and
# stores its two local table references without reloc records.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F00000D8_18C92A0.c.o: CFLAGS += -Wab,-r4300_mul
ifneq ($(NON_MATCHING),0)
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F00000D8_18C92A0.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F00000D8_18C92A0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x168:5:gOverlay70PairTableReloc \
		0x16C:6:gOverlay70PairTableReloc \
		0x194:5:gOverlay70HeightTableReloc \
		0x1CC:6:gOverlay70HeightTableReloc && \
	$(OBJCOPY) --redefine-sym \
		overlay70Reset=func_overlay_070_F0000000_18C91C8 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x0F0:overlay70RandomRange:func_overlay_070_F0000000_18C91C8 \
		0x100:overlay70RandomRange:func_overlay_070_F0000000_18C91C8 \
		0x124:overlay70RandomRange:func_overlay_070_F0000000_18C91C8 \
		0x134:overlay70RandomRange:func_overlay_070_F0000000_18C91C8 \
		0x14C:overlay70Sin:func_overlay_070_F0000000_18C91C8 \
		0x15C:overlay70Cos:func_overlay_070_F0000000_18C91C8 \
		0x28C:overlay70Apply:func_overlay_070_F0000000_18C91C8 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2AC
else
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F00000D8_18C92A0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2AC
endif

$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000384_18C954C.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000384_18C954C.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000384_18C954C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x264:5:gOverlay70SharedCounterReloc \
		0x274:6:gOverlay70SharedCounterReloc \
		0x278:5:gOverlay70SharedCounterReloc \
		0x280:6:gOverlay70SharedCounterReloc \
		0x2F0:5:gOverlay70SharedCounterReloc \
		0x2F4:6:gOverlay70SharedCounterReloc \
		0x340:5:gOverlay70SharedCounterReloc \
		0x388:6:gOverlay70SharedCounterReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3A4
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
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ByteLength.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101PromoteSlot.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x88
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedPair.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedPair2.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedFloat.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedByte.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedPair3.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xEC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedScaled.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x100
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleLinkedColor.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x120
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101UpdateFrames.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleFrames.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x140
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ScheduleGlobalPair.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DispatchEvents.c.o: CFLAGS += -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DispatchEvents.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DispatchEvents.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--add-symbol overlay101DispatchEventsJumpTable=0xE4C,global \
		--redefine-sym \
			overlay101SchedulePair=func_overlay_101_F0000000_18DB820 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x0B8:.rodata:overlay101DispatchEventsJumpTable \
		0x0C0:.rodata:overlay101DispatchEventsJumpTable \
		0x104:overlay101SchedulePair12:func_overlay_101_F0000000_18DB820 \
		0x11C:overlay101ActivateSlot:func_overlay_101_F0000000_18DB820 \
		0x134:overlay101AdvanceSlot:func_overlay_101_F0000000_18DB820 \
		0x14C:overlay101PromoteSlot:func_overlay_101_F0000000_18DB820 \
		0x170:overlay101ScheduleByte17:func_overlay_101_F0000000_18DB820 \
		0x194:overlay101ScheduleByte16:func_overlay_101_F0000000_18DB820 \
		0x1C4:overlay101ScheduleLinkedPair:func_overlay_101_F0000000_18DB820 \
		0x1F4:overlay101ScheduleLinkedPair2:func_overlay_101_F0000000_18DB820 \
		0x21C:overlay101ScheduleLinkedFloat:func_overlay_101_F0000000_18DB820 \
		0x244:overlay101ScheduleLinkedScaled:func_overlay_101_F0000000_18DB820 \
		0x26C:overlay101ScheduleLinkedByte:func_overlay_101_F0000000_18DB820 \
		0x29C:overlay101ScheduleLinkedPair3:func_overlay_101_F0000000_18DB820 \
		0x2E4:overlay101ScheduleLinkedColor:func_overlay_101_F0000000_18DB820 \
		0x314:overlay101ScheduleFrames:func_overlay_101_F0000000_18DB820 \
		0x338:overlay101ScheduleGlobalPair:func_overlay_101_F0000000_18DB820 \
		0x350:overlay101ScheduleGlobalPair:func_overlay_101_F0000000_18DB820 && \
	$(OBJCOPY) --remove-section=.rodata $@
# The compiler emits the 16-entry switch table already owned by the overlay's
# data/rodata asset at runtime-local +0xE0C. Rebind the text pair to that table,
# leave call-site relocation ownership with the extracted overlay table, and
# discard only the duplicate private table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DispatchActive.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DispatchActive.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol overlay101DispatchActiveJumpTable=0xE0C,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x44:.rodata:overlay101DispatchActiveJumpTable \
		0x4C:.rodata:overlay101DispatchActiveJumpTable && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x05C:4:overlay101UpdateEntry \
		0x070:4:overlay101UpdateEntry12 \
		0x084:4:overlay101UpdateByte17 \
		0x098:4:overlay101UpdateByte16 \
		0x0AC:4:overlay101UpdateEntry8 \
		0x0C0:4:overlay101UpdateEntry8B \
		0x0D4:4:overlay101UpdateFloat12 \
		0x0E8:4:overlay101UpdateDelta16 \
		0x0FC:4:overlay101UpdateByte18 \
		0x110:4:overlay101UpdateEntry8C \
		0x124:4:overlay101UpdateColor \
		0x138:4:overlay101UpdateFrames \
		0x14C:4:overlay101UpdateGlobalPair && \
	$(OBJCOPY) --remove-section=.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x17C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawElement.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101GetBounds.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101DrawTransformed.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_101_F00029A4_18DE1C4=overlay101DrawTransformed $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x298
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101BuildPresentationD.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_101_F000A384_18E5BA4=overlay101BuildPresentationD $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x338
# NON_MATCHING/GLOBAL_ASM: retain only friendly-name restoration and
# trailing-section trimming metadata for the extracted function.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o016/overlay_016.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_016_F00001E0_1873678=overlay16ApplyGradient $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x424
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitializeAndUpdate.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_084_F0000048_18D0528=overlay84InitializeAndUpdate $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2CC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o084/func_overlay_084_F0000314_18D07F4.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x740
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
$(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63Release.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
# NON_MATCHING/GLOBAL_ASM: restore friendly symbols and retain only the
# trailing-section trim metadata for these extracted functions.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63Initialize.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_063_F0000000_18C2B88=overlay63Initialize \
		--redefine-sym gO63ChainReload=gO63Chain $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1D4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63UpdateEffects.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_063_F00001D4_18C2D5C=overlay63UpdateEffects $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x578
$(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63UpdateSequence.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_063_F000077C_18C3304=overlay63UpdateSequence $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1AC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68CreateEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ReleasePrimary.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ReleaseSecondary.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68CreatePayload.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68AttachObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68UpdateTrail.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ClearNestedFlag.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68FinishEntry.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68StartTimer.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68PromoteSecondary.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_068_F000051C_18C767C=overlay68PromoteSecondary $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x134
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68Interpolate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x290
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68Interpolate.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68InitializeObject.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68UpdateAnimation.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_068_F000096C_18C7ACC=overlay68UpdateAnimation $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x590
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68DrawSortedEntries.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_068_F0000EFC_18C805C=overlay68DrawSortedEntries $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x354
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68RebuildSecondaryEntry.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_068_F0001250_18C83B0=overlay68RebuildSecondaryEntry $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1E8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ReleaseTertiary.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x34
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68CheckKind.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_068_F000146C_18C85CC=overlay68CheckKind \
		--redefine-sym func_8002B280=overlay68AllocProbeReloc \
		--redefine-sym piRomLoadSection=overlay68FillProbeReloc \
		--redefine-sym func_800291C4=overlay68GetResidentEntriesReloc \
		--redefine-sym levelGetBlurEffect=overlay68MapResidentIndexReloc \
		--redefine-sym mmFree=overlay68FreeProbeReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x140
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
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0000C90_1857A88.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2QueryNode.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_002_F00016A0_1858498=overlay2QueryNode $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3F4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001A94_185888C.c.o: \
	CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001A94_185888C.c.o: \
	POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x364 \
		000000000000000000000000
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001DF8_1858BF0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x730
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
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13Call.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x124
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13CreateRecord.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xFC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13Release.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
# Matched C (lane s2-b, 2026-09-17): the TU's one resident call goes through
# the generated surface alias; retain the exact text extent.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13ProcessRecord.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym SetLinkSlot=SetLinkSlot_o013Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x284
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13DrawRecord.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_013_F0000580_186F098=overlay13DrawRecord $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2F4
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13DrawActive.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_013_F0000874_186F38C=overlay13DrawActive $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x298
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11EnableHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F0000150_1868998.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11DisableHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateSelection.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateSelection.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80000F94=overlay11PlaySoundReloc \
		--redefine-sym func_8002554C=overlay11ReadInputReloc \
		--redefine-sym func_overlay_045_F0001BF4_188E04C=overlay11SetValue $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C8
# These one-to-one proxies describe the thirteen runtime-authenticated call
# roles required by a future C promotion. The current NON_MATCHING fallback
# does not emit the friendly names, so its ordinary object leaves these
# mappings inert until the candidate body becomes canonical.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateMenu.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_011_F0001398_1869BE0=overlay11UpdateMenu \
		--redefine-sym func_80028F54=overlay11GetStatusReloc \
		--redefine-sym func_80000F94=overlay11PlaySoundReloc \
		--redefine-sym func_overlay_045_F0001BF4_188E04C=overlay11SetValue \
		--redefine-sym func_8002554C=overlay11ReadInputReloc \
		--redefine-sym func_overlay_066_F0000000=overlay11Overlay66SelectReloc \
		--redefine-sym func_800290AC=overlay11ResidentModeReloc \
		--redefine-sym func_800291D8=overlay11Func800291D8Reloc \
		--redefine-sym func_800006BC=overlay11Func800006BCReloc \
		--redefine-sym func_overlay_011_F0002BF4_186B43C=overlay11Func2BF4Reloc \
		--redefine-sym func_80005820=overlay11Func80005820Reloc \
		--redefine-sym func_80028374=overlay11Func80028374Reloc \
		--redefine-sym func_80028528=overlay11Func80028528Reloc \
		--redefine-sym func_8003A754=overlay11Func8003A754Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4B4
# Overlay-local data addends are encoded in retail, while its runtime calls
# all use the extracted range's offset-zero carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateTwoOptionMenu.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateTwoOptionMenu.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x0:5:D_INPUT 0x4:6:D_INPUT 0x8:5:D_0 0x14:6:D_0 \
		0x5C:5:D_INPUT 0x60:6:D_INPUT \
		0x64:5:D_0_reload_success 0x70:6:D_0_reload_success \
		0x7C:5:D_INPUT 0x80:6:D_INPUT \
		0x84:5:D_0_reload_failure 0x8C:6:D_0_reload_failure \
		0xF0:5:D_menuBase 0xF8:6:D_menuBase \
		0x120:5:D_INPUT 0x128:6:D_INPUT \
		0x134:5:D_INPUT 0x138:6:D_INPUT \
		0x198:5:D_cfgA 0x1A0:6:D_cfgA \
		0x1A4:5:D_INPUT 0x1A8:6:D_INPUT \
		0x214:5:D_cfgA 0x218:6:D_cfgA && \
	$(OBJCOPY) --redefine-sym \
		func_80000F94=func_overlay_011_F0000000_1868848 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x100:func_overlay_045_F0001BF4_188E04C:func_overlay_011_F0000000_1868848 \
		0x124:func_8002554C:func_overlay_011_F0000000_1868848 \
		0x168:func_overlay_066_F0000000:func_overlay_011_F0000000_1868848 \
		0x170:func_800290AC:func_overlay_011_F0000000_1868848 \
		0x178:func_800291D8:func_overlay_011_F0000000_1868848 \
		0x188:func_800006BC:func_overlay_011_F0000000_1868848 \
		0x190:func_overlay_011_F0002BF4_186B43C:func_overlay_011_F0000000_1868848 \
		0x1E8:func_80028528:func_overlay_011_F0000000_1868848 \
		0x20C:func_80028374:func_overlay_011_F0000000_1868848
# The compiler emits the exact five-entry switch table already present at
# overlay-local +0x40. Rebind the text pair there, discard only the duplicate
# private table, and preserve the retail offset-zero runtime call carriers.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateFiveOptionMenu.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateFiveOptionMenu.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80000F94=func_overlay_011_F0000000_1868848 \
		--add-symbol gOverlay11FiveOptionSwitchTableReloc=0x40,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x100:func_overlay_045_F0001BF4_188E04C:func_overlay_011_F0000000_1868848 \
		0x124:func_8002554C:func_overlay_011_F0000000_1868848 \
		0x164:.rodata:gOverlay11FiveOptionSwitchTableReloc \
		0x16C:.rodata:gOverlay11FiveOptionSwitchTableReloc \
		0x178:func_overlay_066_F0000000:func_overlay_011_F0000000_1868848 \
		0x180:func_800290AC:func_overlay_011_F0000000_1868848 \
		0x188:func_800291D8:func_overlay_011_F0000000_1868848 \
		0x198:func_800006BC:func_overlay_011_F0000000_1868848 \
		0x1A0:func_overlay_011_F0002BF4_186B43C:func_overlay_011_F0000000_1868848 \
		0x218:func_80005820:func_overlay_011_F0000000_1868848 \
		0x220:func_8002675C:func_overlay_011_F0000000_1868848 \
		0x240:func_80028374:func_overlay_011_F0000000_1868848 \
		0x2B0:func_80028374:func_overlay_011_F0000000_1868848 \
		0x320:func_80028374:func_overlay_011_F0000000_1868848 \
		0x3A8:func_80028374:func_overlay_011_F0000000_1868848 && \
	$(OBJCOPY) --remove-section=.rodata $@
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F0001E4C_186A694.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_011_F0001E4C_186A694=func_overlay_011_F0001E4C_186A694 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x49C
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F00022E8_186AB30.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_011_F00022E8_186AB30=func_overlay_011_F00022E8_186AB30 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x42C
# Overlay-local data addends are encoded in retail, while its runtime calls
# use the extracted range's offset-zero carrier.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateModeSix.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateModeSix.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x0:5:D_INPUT 0x4:6:D_INPUT 0x8:5:D_0 0x14:6:D_0 \
		0x5C:5:D_INPUT 0x60:6:D_INPUT \
		0x64:5:D_0_reload_success 0x70:6:D_0_reload_success \
		0x7C:5:D_INPUT 0x80:6:D_INPUT \
		0x84:5:D_0_reload_failure 0x8C:6:D_0_reload_failure \
		0xF0:5:D_menuBase 0xF8:6:D_menuBase \
		0x120:5:D_INPUT 0x128:6:D_INPUT \
		0x134:5:D_INPUT 0x138:6:D_INPUT \
		0x198:5:D_cfgA 0x1A0:6:D_cfgA \
		0x1A4:5:D_INPUT 0x1A8:6:D_INPUT \
		0x1F4:5:D_lastMode 0x1F8:6:D_lastMode \
		0x218:5:D_cfgA 0x21C:6:D_cfgA && \
	$(OBJCOPY) --redefine-sym \
		func_80000F94=func_overlay_011_F0000000_1868848 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x100:func_overlay_045_F0001BF4_188E04C:func_overlay_011_F0000000_1868848 \
		0x124:func_8002554C:func_overlay_011_F0000000_1868848 \
		0x168:func_overlay_066_F0000000:func_overlay_011_F0000000_1868848 \
		0x170:func_800290AC:func_overlay_011_F0000000_1868848 \
		0x178:func_800291D8:func_overlay_011_F0000000_1868848 \
		0x188:func_800006BC:func_overlay_011_F0000000_1868848 \
		0x190:func_overlay_011_F0002BF4_186B43C:func_overlay_011_F0000000_1868848 \
		0x210:func_80028374:func_overlay_011_F0000000_1868848 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x234
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11CreateHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDC
# The compiler emits the exact six-entry switch table already present in the
# overlay's extracted data/rodata asset. Rebind the text pair to its proved
# runtime-local `+8` addend, then discard only the duplicate private table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11Initialize.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11Initialize.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_800290AC=overlay11ResidentModeReloc \
		--redefine-sym func_800005CC=overlay11ResidentFloatReloc \
		--redefine-sym overlay66Select=overlay11Overlay66SelectReloc \
		--redefine-sym func_80028F54=overlay11GetStatusReloc \
		--redefine-sym func_8004B0A4=overlay11DrawModeReloc \
		--redefine-sym func_8004B0B8=overlay11DrawColorReloc $@ && \
	$(OBJCOPY) --add-symbol gOverlay11SwitchTableReloc=0x8,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xA8:.rodata:gOverlay11SwitchTableReloc \
		0xB0:.rodata:gOverlay11SwitchTableReloc && \
	$(OBJCOPY) --remove-section=.rodata $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeFour.c.o: \
	$(TOOLS_DIR)/trim_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeFour.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80028F54=overlay11GetStatusReloc \
		--redefine-sym sprintf=overlay11FormatReloc \
		--redefine-sym func_overlay_045_F000000C_188B438=overlay11CreateReloc \
		--redefine-sym D_800D31BC=gOverlay11ResidentFlagsReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x194
# The compiler emits the exact six-entry switch table already present in the
# overlay's extracted data/rodata asset. Rebind the text pair to its proved
# runtime-local +0x7C addend, then discard only the duplicate private table.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseCurrentGroup.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseCurrentGroup.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay11ReleaseSwitchTableReloc=0x7C,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x30:.rodata:gOverlay11ReleaseSwitchTableReloc \
		0x38:.rodata:gOverlay11ReleaseSwitchTableReloc && \
	$(OBJCOPY) --remove-section=.rodata $@
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13ProcessActive.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x78
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Submit.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x28
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83BuildLine.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1AC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawLines.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF4
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Update.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_083_F00002A0_18CFA60=overlay83Update $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x274
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83SubmitAll.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_083_F0000A18_18D01D8=overlay83SubmitAll $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x148
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x74
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawStrip.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_083_F0000850_18D0010=overlay83DrawStrip $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x134
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Dispatch.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x94
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99GetEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ReleaseEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99InitializeEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1B8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ProjectVector.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x84
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ApplySegment.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
ifeq ($(NON_MATCHING),1)
# The compiler's private constants duplicate the retained overlay table at +0xB0.
# Rebind only those text relocations; the linked default path remains GLOBAL_ASM.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ApplySegment.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol overlay99ApplySegmentPrivateTable=0xB0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xBC:.rodata:overlay99ApplySegmentPrivateTable \
		0xE4:.rodata:overlay99ApplySegmentPrivateTable && \
	$(OBJCOPY) --remove-section=.rodata $@ && \
	$(OBJCOPY) --redefine-sym \
		func_overlay_099_F00002A0_18D9850=overlay99ApplySegment $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x398
else
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ApplySegment.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_099_F00002A0_18D9850=overlay99ApplySegment $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x398
endif
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99BuildHeightGrid.c.o: CFLAGS += -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99BuildHeightGrid.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_099_F0000638_18D9BE8=overlay99BuildHeightGrid $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99RenderSortedEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3A4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99RenderSegments.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99RenderSegments.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_099_F0000BA4_18DA154=overlay99RenderSegments $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x238
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/func_overlay_099_F0000DDC_18DA38C.c.o: CFLAGS += -Wo,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/func_overlay_099_F0000DDC_18DA38C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x584
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x334
$(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60ReleaseResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
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
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0000000_18A3BF8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x954
ifeq ($(NON_MATCHING),0)
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0001020_18A4C18.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x958
endif
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0004460_18A8058.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7B8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F00060F8_18A9CF0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6E4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58EnsureResource.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/func_overlay_058_F0000000_18AF1E8.c.o: CFLAGS += -Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/func_overlay_058_F0000000_18AF1E8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/func_overlay_058_F00005FC_18AF7E4.c.o: OPT_FLAGS := -O2 -g3
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/func_overlay_058_F00005FC_18AF7E4.c.o: POSTPROCESS = \
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
	$(OBJCOPY) --add-symbol gOverlay58ModeJumpTableReloc=0x124,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_058_F000138C_18B0574.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:77de523043975c80a3e242600f52cf9cbb6ad5ed735965e74ed1e11bbf345b03 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3878
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawSegmentStrip.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_058_F0004C04_18B3DEC=overlay58DrawSegmentStrip $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x324
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17CalculateEndpoints.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_017_F0000000_18739B8=overlay17CalculateEndpoints $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x318
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17ReleaseChain.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x40
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17AdvanceChain.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_017_F0000668_1874020=overlay17AdvanceChain $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x24C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17DrawStrip.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_017_F00008B4_187426C=overlay17DrawStrip $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1DC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58
$(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Load.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1F4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Reconfigure.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_018_F000024C_1874804=overlay18Reconfigure $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2A8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18InitializeBuffers.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_018_F00004F4_1874AAC=overlay18InitializeBuffers $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x15C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x13C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55ReleaseAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x38
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/func_overlay_055_F000031C_18A1E34.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x914
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/func_overlay_055_F000031C_18A1E34.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58ReleaseResources.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay62Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay62Update.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_062_F00000D4_18C22F4=overlay62Update $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x498
$(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay62ReleaseAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x44
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87ReleaseCurrent.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x30
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87HasNearby.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87HasNearby.c.o: CFLAGS += -Wab,-r4300_mul
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50Initialize.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_050_F0000000_1896970=func_overlay_050_F0000000_1896970 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2E4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50PatchIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/func_overlay_050_F0000334_1896CA4.c.o: OPT_FLAGS := \
	-O2 -g3
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/func_overlay_050_F0000334_1896CA4.c.o: CFLAGS += \
	-Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/func_overlay_050_F0000334_1896CA4.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x189C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50Cleanup.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x84
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50SubmitTimeGlyphs.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x214
# NON_MATCHING/GLOBAL_ASM: the extracted function already has its canonical
# auto-generated symbol and requires no postprocess metadata.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Initialize.c.o: CFLAGS += \
	-Wab,-r4300_mul
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
$(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53PatchIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53Initialize.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym gOverlay53Value280=D_280 \
		--redefine-sym gOverlay53Height290=D_290 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x11C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53CopyOffsetEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3CC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54PatchIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55PatchIndices.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay_091.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay_091_mul.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x04c:5:overlay91GlobalA 0x05c:6:overlay91GlobalA \
		0x078:5:.rodata 0x080:6:.rodata \
		0x134:4:overlay91CallProxy 0x1a4:4:overlay91CallProxy \
		0x1ac:4:overlay91CallProxy 0x1d0:4:overlay91CallProxy \
		0x228:4:overlay91CallProxy 0x270:4:overlay91CallProxy \
		0x278:4:overlay91CallProxy 0x2f0:4:overlay91CallProxy \
		0x2f8:4:overlay91CallProxy 0x300:5:overlay91GlobalB \
		0x308:6:overlay91GlobalB 0x360:4:overlay91CallProxy \
		0x3a4:4:overlay91CallProxy && \
	$(OBJCOPY) --remove-section=.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x528
$(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay_091_mul.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseHandles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixA.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixB.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixC.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeThreeA.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeThreeB.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x8C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57SetNodeValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58SetNodeValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x54
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54CopyOffsetRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54GetOffsets.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54TailA.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x18E8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52TailB.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A5C

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
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55CopyOffsetRecords.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55GetOffsets.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup4.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup3A.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6A.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6B.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup3B.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29RotateForward.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29RotateBackward.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29Select.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x84
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36TickState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x5C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36PrepareAndTick.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x84
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SelectState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x74
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36ChooseWeightedState.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36ChooseWeightedState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2A8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36UpdateInteractiveEntity.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_036_F00001D0_1883688=overlay36UpdateInteractiveEntity $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnTransient.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x11C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36InitVectorState.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x68
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36CheckNearbyHeight.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xFC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36FlushQueue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x90
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36QueueAction.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnFinalEffect.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnAtPosition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnAndUpdate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnLinked7F.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x194
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnDirectional.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x164
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnOffsetA9.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x164
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36UpdatePeers.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_036_F000150C_18849C4=overlay36UpdatePeers $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x17C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100InitializeMotion.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100InitializeMotion.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_100_F0000000_18DAD28=overlay100InitializeMotion $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x214
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100ReleaseAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x64
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100ApplyValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x74
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100UpdateMotion.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100ApplyToValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x74
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100DrawMotion.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100DrawMotion.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_100_F0000580_18DB2A8=overlay100DrawMotion $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3CC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o090/overlay_090.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB1C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3ResetObjects.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x68
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3ContainsValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x50
$(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48Initialize.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x60
# NON_MATCHING/GLOBAL_ASM: restore friendly symbols and retain only the
# trailing-section trim metadata for these extracted functions.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o049/overlay_049.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o049/overlay_049.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_049_F0000000_1896410=overlay49Initialize \
		--redefine-sym func_80028F54=func_80028F54_o049Reloc \
		--redefine-sym func_800508B4=func_800508B4_o049Reloc \
		--redefine-sym func_8002917C=func_8002917C_o049Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x218:func_800254FC:overlay65UpdateReloc \
		0x224:func_8002554C:overlay65UpdateReloc \
		0x2c0:func_800016EC:overlay65UpdateReloc \
		0x2c8:D_8007BF08:gOverlay49Timer \
		0x2cc:D_8007BF08:gOverlay49Timer \
		0x2d8:func_8003A754:overlay65UpdateReloc \
		0x2e0:D_8007BF04:gOverlay49Timer \
		0x2e4:D_8007BF04:gOverlay49Timer \
		0x2f8:overlay48InitializeReloc:overlay65UpdateReloc \
		0x314:func_80028374:overlay65UpdateReloc \
		0x324:D_800D0000:gOverlay49Timer \
		0x328:D_800D0004:gOverlay49Timer \
		0x32c:D_800D0004:gOverlay49Timer \
		0x330:D_800D0000:gOverlay49Timer && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x374
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48InitializeState.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_048_F0000060_1895468=overlay48InitializeState $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xE4
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48UpdateState.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_048_F0000144_189554C=overlay48UpdateState $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2C8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48ReleaseAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x60
O28_MERGED_OBJ := \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o028/overlay_028.c.o
$(O28_MERGED_OBJ): \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/trim_elf_section.py
$(O28_MERGED_OBJ): CFLAGS += -Wab,-r4300_mul
# The loader owns the reset callback HILO and the update-vertices call carrier.
# The render tail's imports likewise retain their overlay-root and resident
# relocation carriers after the function becomes local to the merged TU.
$(O28_MERGED_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x1D8:5:overlay28ResetBuffer 0x1F4:6:overlay28ResetBuffer && \
	$(OBJCOPY) \
		--add-symbol func_overlay_028_F0000000_187C8D0=.text:0,global \
		--add-symbol D_80000028=0x80000028,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x41C:overlay28UpdateVertices:ext_o0_29e00 \
		0x4F0:ext_o0_241dc:func_overlay_028_F0000000_187C8D0 \
		0x51C:ext_o0_29e00:func_overlay_028_F0000000_187C8D0 \
		0x5AC:ext_o0_6ec00:func_overlay_028_F0000000_187C8D0 \
		0x5F8:ext_o0_9ab8:func_overlay_028_F0000000_187C8D0 \
		0x674:ext_o0_2409c:func_overlay_028_F0000000_187C8D0 \
		0x688:ext_o0_34554:func_overlay_028_F0000000_187C8D0 \
		0x744:gOverlay28DisplayList28:D_80000028 \
		0x754:gOverlay28DisplayList28:D_80000028 \
		0x7D0:ext_o0_241bc:func_overlay_028_F0000000_187C8D0 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7EC
$(BUILD_DIR)/$(SRC_DIR)/overlays/o035/overlay35SelectHeight.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x68
$(BUILD_DIR)/$(SRC_DIR)/overlays/o035/func_overlay_035_F00001E0_1881EC0.c.o: CFLAGS += \
	-Wo,-loopunroll,0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o035/func_overlay_035_F00001E0_1881EC0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x590
$(BUILD_DIR)/$(SRC_DIR)/overlays/o035/func_overlay_035_F0000B40_1882820.c.o: CFLAGS += \
	-Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o035/func_overlay_035_F0000B40_1882820.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x840

$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay26HandleEffects.c.o: CFLAGS += -Wab,-r4300_mul
# Stored overlay call carriers are zero; keep each resident identity in its
# own declared alias without changing compiler-emitted instruction words.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay26HandleEffects.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_80006EA0=func_80006EA0_o026Reloc \
		--redefine-sym func_8002A910=func_8002A910_o026Reloc \
		--redefine-sym sqrtf=sqrtf_o026Reloc \
		--redefine-sym func_80029FE4=func_80029FE4_o026Reloc \
		--redefine-sym func_8002997C=func_8002997C_o026Reloc \
		--redefine-sym func_8003EDEC=func_8003EDEC_o026Reloc \
		--redefine-sym func_80002FE0=func_80002FE0_o026Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x434

$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/func_overlay_026_F00001A0_187A598.c.o: CFLAGS += -Wab,-r4300_mul
ifneq ($(NON_MATCHING),1)
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/func_overlay_026_F00001A0_187A598.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x978
endif

$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/func_overlay_026_F0000B18_187AF10.c.o: CFLAGS += -Wab,-r4300_mul
# The resident call cannot be valued under its own global name in
# overlay_undefined_syms.us.txt without moving the resident function for
# every resident caller; tools/reloc_surface.py derives a per-module
# placeholder for the R_MIPS_26 site and the rebind is declared here.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/func_overlay_026_F0000B18_187AF10.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym sqrtf=sqrtf_o026Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x20C

$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay26DrawGroups.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x218

$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29ProjectPoint.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29ProjectPoint.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1E4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Release.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x70
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59PrepareEntry.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym \
		func_overlay_059_F0000070_18B87C0=overlay59PrepareEntry $@ && \
	$(OBJCOPY) --redefine-sym \
		overlay59PrepareReleaseReloc=func_overlay_059_F0000000_18B8750 $@ && \
	$(OBJCOPY) --redefine-sym \
		func_80034448=func_80034448_o059Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xF8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59ResetEntries.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x6C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59ReleaseAll.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x48
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Update.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9C
# The compiler's private pool for this unit is the 0.15f approach factor
# followed by the six-entry switch table; the retained overlay data segment
# already owns those bytes at +0x768 and +0x76C.  Anchor the two %hi/%lo pairs
# on the module-relative 0x18 the shipped text encodes and contribute no bytes.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Advance.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:15fc13fbb3e01c022332cdcc79623566f105d3a5922afe1ff8228f1f525f3f34 0x18 && \
	$(OBJCOPY) --remove-section=.rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x418
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59BuildList.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xA0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59AppendValue.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xB4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Interpolate.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xC8
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59DrawFrame.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x130
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57EaseAndLatch.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_057_F00028B4_18A64AC=overlay57EaseAndLatch $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x374
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateSelection.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_057_F00035E0_18A71D8=overlay57UpdateSelection $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x46C
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateModeState.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_057_F0003A4C_18A7644=overlay57UpdateModeState $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x588
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57HandleModeInput.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_057_F0004064_18A7C5C=overlay57HandleModeInput $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x364
ifeq ($(NON_MATCHING),0)
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0000000_18A3BF8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x954
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0001020_18A4C18.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x958
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0004460_18A8058.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7B8
endif
# overlay57UpdateModeTrigger owns overlay 57's local BSS block. IDO only shares
# one `lui $at` between the two constant-index setup stores when the array is
# defined in this translation unit, and that shared materialisation is in the
# shipped bytes -- so the block is defined here as `static`, which also makes
# every local access a section-relative record whose addend is already the
# module-relative offset the shipped word carries. Overlay 57's own runtime
# relocation table owns those 31 HI16/LO16 records, so the static link must not
# adjust them a second time: drop them exactly as overlay 7's tail does. No
# instruction word is edited -- the compiled immediates are already the shipped
# ones, which is why the object scores 0 masked and 3 raw (the three jal
# targets the link still resolves).
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateModeTrigger.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateModeTrigger.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym func_overlay_057_F0004C18_18A8810=overlay57UpdateModeTrigger $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x0:5:.bss 0xc:6:.bss 0x8:5:.bss 0x10:6:.bss \
		0x3c:5:.bss 0x44:6:.bss 0x5c:5:.bss 0x7c:6:.bss \
		0x74:6:.bss 0x60:5:.bss 0x64:6:.bss 0xb4:5:.bss \
		0xb8:6:.bss 0xbc:5:.bss 0xc4:6:.bss 0xc0:5:.bss \
		0xc8:6:.bss 0xd8:5:.bss 0xe0:6:.bss 0xe4:5:.bss \
		0xec:6:.bss 0xf4:5:.bss 0x104:6:.bss 0x118:5:.bss \
		0x120:6:.bss 0x10c:5:.bss 0x124:6:.bss 0x12c:5:.bss \
		0x138:6:.bss 0x158:5:.bss 0x15c:6:.bss && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x178
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57CheckDistance.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x100
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateTransition.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1F0
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateNode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x100
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ApplyTable.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x138

$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Interpolate.c.o: CFLAGS += -Wab,-r4300_mul

OVERLAY_TRIMMED_OBJECTS := \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o006/overlay_006.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o076/overlay_076.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o078/overlay_078.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o102/overlay_102.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o103/overlay_103.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o106/overlay_106.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o107/overlay_107.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_build.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_head.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0001D78_184E158.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0002B4C_184EF2C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0003258_184F638.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_middle.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_tail.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_create.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_end.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_scaled.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ChooseFileExtension.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay_007.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/func_overlay_007_F0000324_185C1AC.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay_007_tail.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3SelectTarget.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3RunCachedModeAction.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3FindClosestObject.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3SelectScoredObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3TouchObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3UpdateTimedEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19Dispatch.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildOutput.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildPlanes.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildAdjacency.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19FindAdjacent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19ClassifyEdge.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay19BuildSpatialMasks.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay_004.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay_008.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay_009.c.o \
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34RemoveRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay34CreateRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87InitializeObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o087/func_overlay_087_F0000128_18D3090.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o079/func_overlay_079_F0000134_18CD0D4.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay_045.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/func_overlay_045_F0000764_188CBBC.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/func_overlay_045_F0001158_188D5B0.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay_045_tail.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/func_overlay_047_F0000000_1890E18.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/overlay47ReleaseResources.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/overlay47SpawnObject.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61UpdateInput.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61InitResources.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61AddEntry.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61DrawEntry.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61DrawList.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ReleaseResources.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/func_overlay_061_F0001648_18C0A10.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61WriteCharacter.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ReadCharacter.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/overlay61ResetCounters.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o061/func_overlay_061_F0000B84_18BFF4C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68PayloadLimit.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o085/overlay_085.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14GetFlagCC.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ReleaseTree.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ReleaseHandle.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ConfigureResource.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateObjectResource.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/func_overlay_020_F000038C_1876964.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20BuildTileCommands.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20RemoveEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ConfigureEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20ReleaseEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20MarkNested.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20AdvanceEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20CreateEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20DrawResource.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay20UpdateGrid.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/func_overlay_020_F0001148_1877720.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31BuildLookupTables.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31InitializeParticleAssets.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31BuildPalettes.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31InitializeBuffers.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31CreateConfig.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31CreateRecords.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay31CreatePool.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33InitializeBuffers.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33ReleaseGlobal.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33CallA.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33BuildDisplayList.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33PresentAndSwap.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay33CallB.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36CallModes.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitializeState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46ReleaseState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46Submit.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitializeBuffers.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o046/func_overlay_046_F0000874_188EC6C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o046/func_overlay_046_F0001228_188F620.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Initialize.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65UpdateParticles.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Release.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65ResetSlots.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o065/func_overlay_065_F0000C38_18C4EA0.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o064/overlay64GenerateTexture.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/func_overlay_038_F0000000_1885D10.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/overlay38UpdateParticles.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/func_overlay_038_F000047C_188618C.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000000_18C91C8.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F00000D8_18C92A0.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o070/func_overlay_070_F0000384_18C954C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay101ByteLength.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F0002510_18DDD30.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F0003A58_18DF278.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F000512C_18E094C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F000571C_18E0F3C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F0005E08_18E1628.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F00063F8_18E1C18.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F00069E8_18E2208.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F00078F4_18E3114.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F0008128_18E3948.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F000895C_18E417C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o101/func_overlay_101_F0009190_18E49B0.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitializeAndUpdate.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/func_overlay_084_F0000314_18D07F4.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetActive.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84IsUnitScale.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84GetEnabledCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84InitializeCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84ResetCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84UpdateResource.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84RefreshCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o084/overlay84SelectCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63UpdateEffects.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63Release.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay63UpdateSequence.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68CreateEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ReleasePrimary.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ReleaseSecondary.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68CreatePayload.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68AttachObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68UpdateTrail.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ClearNestedFlag.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68FinishEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68StartTimer.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68PromoteSecondary.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68InitializeObject.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68UpdateAnimation.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68DrawSortedEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68ReleaseTertiary.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay68CheckKind.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ValidateRegion.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ContainsPoint.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2Enable.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2CopyColor.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2AppendLine.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ClassifyBoundary.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2IntersectBoundary.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2ClipLines.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0000C90_1857A88.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001364_185815C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2AdjacentIndices.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay2QueryNode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001A94_185888C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o002/func_overlay_002_F0001DF8_1858BF0.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60DrawBorder.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60DrawLine.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o060/func_overlay_060_F0002F54_18BCD2C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60ReassignChoiceSlots.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14DispatchCommand.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13CreateRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13Release.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13ProcessRecord.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F0000150_1868998.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11EnableHandles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11DisableHandles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateSelection.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateMenu.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateTwoOptionMenu.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateFiveOptionMenu.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F0001E4C_186A694.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o011/func_overlay_011_F00022E8_186AB30.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11UpdateModeSix.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11CreateHandles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeFour.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseCurrentGroup.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13ProcessActive.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13DrawRecord.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13DrawActive.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay13Call.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Submit.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83BuildLine.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawLines.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83BuildBatch.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83SubmitAll.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawEntries.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83DrawStrip.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay83Dispatch.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99GetEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ReleaseEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99InitializeEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ProjectVector.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99ApplySegment.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99BuildHeightGrid.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99RenderSortedEntries.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay99RenderSegments.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/func_overlay_099_F0000DDC_18DA38C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o060/overlay60ReleaseResources.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ApplyValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateInterface.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57BeginMode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57StartMode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57InitializeMode.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ReleaseAll.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0000000_18A3BF8.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0001020_18A4C18.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0004460_18A8058.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F00060F8_18A9CF0.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57EaseAndLatch.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57SmoothAndCheckDistance.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57Draw32A0.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateModeState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58EnsureResource.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58RefreshRankSet.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawSegmentStrip.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawPointQuad.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58DrawLargePointQuad.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58FinalizePackedStatus.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/func_overlay_058_F0000000_18AF1E8.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/func_overlay_058_F00005FC_18AF7E4.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/func_overlay_058_F000138C_18B0574.c.o
OVERLAY_TRIMMED_OBJECTS += \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17CalculateEndpoints.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17ReleaseChain.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17AdvanceChain.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay17DrawStrip.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Load.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18Reconfigure.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay18InitializeBuffers.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o055/func_overlay_055_F000031C_18A1E34.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55ReleaseAll.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58ReleaseResources.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay62Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay62Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay62ReleaseAll.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87ReleaseCurrent.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay87HasNearby.c.o
OVERLAY_TRIMMED_OBJECTS += \
	$(O22_UPDATE_OBJECT_OBJ)
OVERLAY_TRIMMED_OBJECTS += \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50Initialize.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50PatchIndices.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50Cleanup.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay50SubmitTimeGlyphs.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52PatchIndices.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52CopyOffsetEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o052/overlay52Cleanup.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53PatchIndices.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay53CopyOffsetEntries.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54PatchIndices.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55PatchIndices.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay_091.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay_091_mul.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseHandles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixA.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixB.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeSixC.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeThreeA.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11InitializeThreeB.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57SetNodeValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o058/overlay58SetNodeValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46InitState.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54CopyOffsetRecords.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay54GetOffsets.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55CopyOffsetRecords.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay55GetOffsets.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup4.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup3A.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6A.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6B.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup6C.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay11ReleaseGroup3B.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29RotateForward.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29RotateBackward.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay29Select.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36TickState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36PrepareAndTick.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SelectState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36ChooseWeightedState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36UpdateInteractiveEntity.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnTransient.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36InitVectorState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36CheckNearbyHeight.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36FlushQueue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36QueueAction.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnFinalEffect.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnAtPosition.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnAndUpdate.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnLinked7F.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnDirectional.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36SpawnOffsetA9.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay36UpdatePeers.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100ReleaseAll.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100ApplyValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100UpdateMotion.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100ApplyToValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay100DrawMotion.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o090/overlay_090.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3ResetObjects.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay3ContainsValue.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48InitializeState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48UpdateState.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay48ReleaseAll.c.o
OVERLAY_TRIMMED_OBJECTS += \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o049/overlay_049.c.o
OVERLAY_TRIMMED_OBJECTS += \
	$(O28_MERGED_OBJ)
OVERLAY_TRIMMED_OBJECTS += \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/func_overlay_026_F00001A0_187A598.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/func_overlay_026_F0000B18_187AF10.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o029/func_overlay_029_F00005C4_187D874.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o035/overlay35SelectHeight.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o035/func_overlay_035_F00001E0_1881EC0.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o035/func_overlay_035_F0000B40_1882820.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o035/overlay35BuildGridMasks.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Release.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59PrepareEntry.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59ResetEntries.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59ReleaseAll.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Update.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Advance.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59AppendValue.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59Interpolate.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59BuildList.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay59DrawFrame.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0000000_18A3BF8.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0001020_18A4C18.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57CheckDistance.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateTransition.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateNode.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57ApplyTable.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0004460_18A8058.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o046/overlay46UpdateTransition.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay14UpdateTransition.c.o \
	$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/overlay57UpdateModeTrigger.c.o

$(OVERLAY_TRIMMED_OBJECTS): $(TOOLS_DIR)/trim_elf_section.py

# Overlay 57's prefix owner ends at the measured 0xDCC-byte function boundary.
# The assembly-backed C scaffold makes IDO/asm-processor round the standalone
# .text section to 0xDD0, so discard only its trailing alignment word before
# the following overlay57EaseAndLatch subsegment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o057/func_overlay_057_F0001AE8_18A56E0.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xDCC
