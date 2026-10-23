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

# IDO writes no ELF definition for a static function, so a module folded into
# one TU would link with no boundary at its statics. tools/static_symbols.py
# adds a sized local STT_FUNC per static procedure from the object's .mdebug,
# straight after the compile while those offsets are still IDO's. Symbol table
# only; an object without statics is left byte-identical.
$(BUILD_DIR)/$(SRC_DIR)/overlays/%.c.o: STATIC_SYMBOLS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/static_symbols.py $@

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
# Resident-call placeholder renames derived by `gmake overlay-syms`, applied by
# the compile recipe after POSTPROCESS (tools/resident_rebinds.py). Every
# overlay object depends on the file, not only the listed ones: an entry that
# disappears must rebuild its object without the rename, or the stale object
# would keep it and `check-overlay-syms` would agree with the edited file.
include mk/overlay_resident_rebinds.generated.mk
$(filter $(BUILD_DIR)/$(SRC_DIR)/overlays/%.c.o,$(O_FILES)): mk/overlay_resident_rebinds.generated.mk

.PHONY: check-overlay-aliases test-overlay-aliases
check-overlay-aliases:
	$(HOST_PYTHON) $(TOOLS_DIR)/render_overlay_aliases.py --check

test-overlay-aliases:
	$(HOST_PYTHON) $(TOOLS_DIR)/test_render_overlay_aliases.py

check-docs: check-overlay-aliases
check-tooling: test-overlay-aliases

include mk/overlay_061.mk
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
# Overlay 56: one module TU under the overlay flag set (the minimap
# renderer's rotated-coordinate products carry the R4300 FP-mul hazard nops).
# IDO's 16-byte .text alignment emits the trailing pad, so no trim; resident
# calls are renamed by the generated surface rebinds.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o056/overlay_056.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 67: one module TU under the overlay flag set; IDO's 16-byte .text
# alignment emits the module's trailing pad itself, so no trim.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o067/overlay_067.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 72: one module TU under the overlay flag set; IDO's 16-byte .text
# alignment emits the module's trailing pad itself, so no trim.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o072/overlay_072.c.o: CFLAGS += -Wab,-r4300_mul
include mk/overlay_065.mk
# The typed source owns O64's complete procedural texture generator.
# Overlay 64: one module TU under the overlay flag set; its .text is the
# module's whole 0x690 bytes and resident references are renamed by the
# generated surface rebinds. Kept, for a mechanical reason: the TU defines the
# module's 16-byte .data (the two function statics), whose eighteen HI16/LO16
# sites the runtime tables relocate from the data base. The section symbol
# becomes an absolute zero, payload kept, so each site stores only its field
# displacement; the payload is asserted by digest. No instruction is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o064/overlay_064.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .data \
		sha256:374708fff7719dd5979ec875d56cd2286f6d3cf7ec317a3b25632aab28ec37bb keep
$(BUILD_DIR)/$(SRC_DIR)/overlays/o064/overlay_064.c.o: CFLAGS += -Wab,-r4300_mul -woff 835
include mk/overlay_070.mk
include mk/overlay_060.mk
include mk/overlay_057.mk
include mk/overlay_058.mk
# Overlay 55: one module TU under the overlay flag set; its .text is the
# module's full 0xC70 bytes, trailing pad included, so no trim. Kept until
# phase 3 (data into C): the TU's .rodata is the 0.66f icon-scale pool, whose
# bytes the raw data_rodata blob still owns at +0x110, so the section is
# asserted by digest and its one hi/lo pair anchored at the rodata base.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay_055.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o055/overlay_055.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:d131bb7de7509c98fc3533790f960348a16e2e461e7d1f5ffff1cd58ae2243e0
# Overlay 66: one module TU under the overlay flag set; its .text is the
# module's whole text, so no trim, and resident calls are renamed by the
# generated surface rebinds.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o066/overlay_066.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 69: one module TU under the overlay flag set. Its renderer body is
# overlay69DrawSortedGeometry.inc.c, which overlay 88's TU also includes. The
# .text is the module's full 0x710 bytes, so no trim; resident calls are
# renamed by the generated surface rebinds.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay_069.c.o: \
	$(SRC_DIR)/overlays/o069/overlay69DrawSortedGeometry.inc.c
$(BUILD_DIR)/$(SRC_DIR)/overlays/o069/overlay_069.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 71: one module TU under the overlay flag set; its .text is the
# module's full 0xB50 bytes, trailing pad included, so no trim, and resident
# calls are renamed by the generated surface rebinds. Kept, for a mechanical
# reason: the initializer's call to the exported refresh (+0x25C) is a SYMBOL
# record in the shipped table, so its stored word is the 0xF0000000 base, not
# the callee's offset; the declared rebind binds that one site to the
# offset-zero export.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/overlay_071.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/overlay_071.c.o: \
	config/normalizations/overlay71InitializeObject.calls.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o071/overlay_071.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay71InitializeObject.calls.spec
# Overlay 62: one module TU under the overlay flag set; its .text is the
# module's whole 0x5B0 bytes, so no trim, and resident calls are renamed by
# the generated surface rebinds.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o062/overlay_062.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 63: one module TU under the overlay flag set; its .text is the
# module's full 0x930 bytes, trailing pad included, so no trim, and resident
# calls are renamed by the generated surface rebinds. The initializer's
# second read of the chain head goes through the weak alias gO63ChainReload,
# which the generated surface values with the chain's own stored address.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o063/overlay_063.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 59: one module TU under the overlay flag set; its .text is the
# module's full 0xA20 bytes, trailing pad included, so no trim, and resident
# calls are renamed by the generated surface rebinds. Kept until phase 3: the
# TU's .rodata (the advance step's 0.15f factor and six-entry switch table)
# is still owned by the raw data blob at +0x768, so the section is asserted
# by digest and its two %hi/%lo pairs bound to a pool symbol at the module-relative 0x18
# the shipped text encodes; the table's own text relocations go with it.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay_059.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay_059.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o059/overlay_059.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay59RodataReloc=0x18,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x39C:.rodata:gOverlay59RodataReloc 0x3C0:.rodata:gOverlay59RodataReloc 0x3F0:.rodata:gOverlay59RodataReloc 0x3F8:.rodata:gOverlay59RodataReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:614e19769956f461b87337c69ff451b22c7a557d16267e7b597407dfa11aa0a0 && \
	$(OBJCOPY) --remove-section=.rel.rodata $@
# Overlay 68: one module TU under the overlay flag set; its .text is the
# module's full 0x15B0 bytes, trailing pad included, so no trim, and resident
# calls are renamed by the generated surface rebinds. -woff 835: two calls
# are deliberately unprototyped (overlay_068.h).
$(BUILD_DIR)/$(SRC_DIR)/overlays/o068/overlay_068.c.o: CFLAGS += -Wab,-r4300_mul -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o102/overlay_102.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o103/overlay_103.c.o: CFLAGS += -Wab,-r4300_mul
# Overlays 104 and 105 are single-file modules whose text needs no trim.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o104/overlay_104.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o105/overlay_105.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o106/overlay_106.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o107/overlay_107.c.o: CFLAGS += -Wab,-r4300_mul

# Overlay 7 is one translation unit at the overlay flag set and owns its pad;
# its resident calls go through the generated surface rebinds. What remains is
# data ownership, kept until phase 3 (docs/OVERLAY_STRUCTURE_SPRINT.md):
# - The TU's .rodata (the runtime update's decay literal, then the mode
#   switch table and its pool) is the retained overlay data at data_rodata
#   +0x930, whose shipped LOCAL records store rodata-relative addends. Assert
#   it by digest and anchor it at zero; the table's .rel.rodata goes with it.
# - The pool's BSS is defined here as statics (the final 0x10 stays
#   atlas-owned), so every access is a section-relative record whose addend is
#   already the module-relative offset the shipped word carries, and overlay
#   7's own runtime table owns them: the static link must not adjust them, so
#   the 36 .bss records are dropped. The shared static-symbol step preserves
#   the local procedure names used by promotion proofs.
O7_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o007/overlay_007.c.o
$(O7_OBJ): CFLAGS += -Wab,-r4300_mul
$(O7_OBJ): $(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py
$(O7_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x0:5:.bss 0x4:6:.bss 0x18:5:.bss 0x1c:6:.bss \
		0x58:5:.bss 0x68:6:.bss 0x8c:5:.bss 0x90:6:.bss \
		0xac:5:.bss 0xb0:6:.bss 0x1ac:5:.bss 0x1b0:6:.bss \
		0x1b8:5:.bss 0x1bc:6:.bss 0x1e8:5:.bss 0x1ec:6:.bss \
		0x398:5:.bss 0x3a8:6:.bss 0x64c:5:.bss 0x650:6:.bss \
		0x69c:5:.bss 0x6a0:6:.bss 0x858:5:.bss 0x85c:6:.bss \
		0xeac:5:.bss 0xeb4:6:.bss 0xf0c:5:.bss 0xf14:6:.bss \
		0xf08:5:.bss 0xf10:6:.bss 0xf9c:5:.bss 0xfa0:6:.bss \
		0xfa4:5:.bss 0xfa8:6:.bss 0xfac:5:.bss 0xfb4:6:.bss && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:f5087dde1cdcf2caa664eb528a658f0e0eafbaa8fdee0750b326b52a8aff5c53 && \
	$(OBJCOPY) --remove-section=.rel.rodata $@
# Overlay 1 has three C islands separated by owned assembly.  Mixed
# -Wo,-loopunroll,4 / -Wab,-r4300_mul flag groups require five further
# boundaries; one object cannot span either an asm range or a flag change.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym splinePos=splinePos_o001Reloc $@ && \
	$(OBJCOPY) --redefine-sym func_8000572C=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x7B0 && \
	$(OBJCOPY) --add-symbol gOverlay1PhaseScaleLiteral=0xAC,global \
		--add-symbol gOverlay1ClosestDistanceLiteral=0xAC,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x20C:.rodata:gOverlay1PhaseScaleLiteral \
		0x214:.rodata:gOverlay1PhaseScaleLiteral \
		0x638:.rodata:gOverlay1ClosestDistanceLiteral \
		0x65C:.rodata:gOverlay1ClosestDistanceLiteral && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:1d7b89af4d351d9ddbd145e42d03699fe64438f929f54453cd84005fa12e645e
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_build.c.o: OPT_FLAGS := -O2 -Wo,-loopunroll,4
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_build.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_build.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x424
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_head.c.o: CFLAGS += -Wab,-r4300_mul
# overlay1LoadBuildRecords is instruction-exact. Its seven float literals
# (0.8, 0.98, 1.15, 0.65, 0.9, 1.1 and FLT_MAX) are a pool that duplicates the
# retained overlay data at rodata-relative +0xBC (the shipped hi/lo pairs are
# LOCAL records): bind the pairs to an absolute 0xBC symbol, the form the
# overlay's rank-weight update uses, and assert the pool by digest. Resident
# callees go through the generated surface entries; no instruction is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_head.c.o: config/normalizations/overlay1LoadBuildRecords.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_head.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0000F84_184D364=overlay1MeasureCurves \
		--redefine-sym func_overlay_001_F0000614_184C9F4=overlay1ModeResolverReloc \
		--redefine-sym sqrtf=sqrtf_o001Reloc \
		--redefine-sym splinePos=splinePos_o001Reloc \
		--redefine-sym func_8002A8BC=func_8002A8BC_o001Reloc \
		--redefine-sym func_8002A8C0=func_8002A8C0_o001Reloc $@ && \
	$(OBJCOPY) --redefine-sym overlay1PreviousIndex=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x11A4 && \
	$(OBJCOPY) --add-symbol gOverlay1BuildRecordsPool=0xBC,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay1LoadBuildRecords.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:a9e8bb2ddde2c0045fe459a7a498fe7611a867b6388de34508f2f49849204ffa
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0001D78_184E158.c.o: CFLAGS += -Wab,-r4300_mul
# Every call but overlay1InitMotionScale's is a SYMBOL record (the shipped
# word carries the 0xF0000000 addend), the overlay 1 callees included, so all
# of them go through generated surface names. The -666.0f sentinel is a
# one-constant pool duplicating the retained overlay data at rodata-relative
# +0xD8 (a LOCAL hi/lo record with stored addend 0xD8): the pair is bound to
# an absolute 0xD8 symbol and the pool asserted by digest, as in the sibling
# func_overlay_001_F0003258_184F638. No instruction is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0001D78_184E158.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80028F54=func_80028F54_o001Reloc \
		--redefine-sym func_80005750=func_80005750_o001Reloc \
		--redefine-sym amSndPlay=amSndPlay_o001Reloc \
		--redefine-sym func_800291C4=func_800291C4_o001Reloc \
		--redefine-sym levelGetNumber=levelGetNumber_o001Reloc \
		--redefine-sym levelGetBlurEffect=levelGetBlurEffect_o001Reloc \
		--redefine-sym func_8003A55C=func_8003A55C_o001Reloc \
		--redefine-sym overlay7UpdateOwnerMode=overlay1Overlay7UpdateOwnerModeReloc \
		--redefine-sym overlay7CommitSelection=overlay1Overlay7CommitSelectionReloc \
		--redefine-sym overlay68StartTimer=overlay1Overlay68StartTimerReloc \
		--redefine-sym overlay1FindClosestSample=overlay1FindClosestSampleReloc \
		--redefine-sym overlay1TestDirection=overlay1TestDirectionReloc \
		--redefine-sym overlay1NextPointer=overlay1NextPointerReloc \
		--redefine-sym overlay1PreviousPointer=overlay1PreviousPointerReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x9CC && \
	$(OBJCOPY) --add-symbol gOverlay1PositionSentinelLiteral=0xD8,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x8FC:.rodata:gOverlay1PositionSentinelLiteral \
		0x900:.rodata:gOverlay1PositionSentinelLiteral && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:e0209c986e6e6bc1c834c0bc7b6616de0b4512a3cbadf1f68ab10c158c66bc53
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0002B4C_184EF2C.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0002B4C_184EF2C.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym sqrtf=sqrtf_o001Reloc \
		--redefine-sym levelGetLevel=levelGetLevel_o001Reloc \
		--redefine-sym func_80005750=func_80005750_o001Reloc \
		--redefine-sym func_80005820=func_80005820_o001Reloc \
		--redefine-sym func_800291D8=func_800291D8_o001Reloc \
		--redefine-sym func_8005830C=func_8005830C_o001Reloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x70C
# The rank-weight update is instruction-exact. Its -666.0f sentinel is a
# one-constant pool that duplicates the retained overlay data at
# rodata-relative +0xE4 (the shipped hi/lo pair is a LOCAL record with stored
# addend 0xE4): bind the pair to an absolute 0xE4 symbol (overlay_001.c's
# literal form) and assert the pool by digest. The three resident callees and
# overlay 7's selection dispatch go through the generated surface entries; no
# instruction is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/func_overlay_001_F0003258_184F638.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_80005750=func_80005750_o001Reloc \
		--redefine-sym func_80028F54=func_80028F54_o001Reloc \
		--redefine-sym levelGetLevel=levelGetLevel_o001Reloc \
		--redefine-sym overlay7DispatchSelection=overlay1Overlay7DispatchSelectionReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x320 && \
	$(OBJCOPY) --add-symbol gOverlay1RankSentinelLiteral=0xE4,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x84:.rodata:gOverlay1RankSentinelLiteral \
		0xA0:.rodata:gOverlay1RankSentinelLiteral && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:e0209c986e6e6bc1c834c0bc7b6616de0b4512a3cbadf1f68ab10c158c66bc53
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_middle.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x408
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_tail.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_tail.c.o: config/normalizations/overlay1DispatchMode.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_tail.c.o: config/normalizations/overlay1UpdateObjectPhysics.rebind.spec
# The eight-entry table retains its raw owner at initialized-data +0x274.
# Its runtime LOCAL base is +0x110 into initialized data: bind the unchanged
# compiler pair to stored addend +0x164, without editing instruction fields.
# The object's .rodata is ChoosePath's four float literals (-1.2f, 400.5f,
# 0.1f, 0.1f; shipped LOCAL addends +0xE8..+0xF4), then UpdateObjectPhysics's
# 26 (+0xF8..+0x15C), then that table at object +0x78, so the table symbol
# is 0x164 - 0x78 and every literal pair binds to a base of 0xE8. Assert the
# whole pool by digest.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_tail.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym func_overlay_001_F0003578_184F958=overlay1InitializeGaugeObjects \
		--redefine-sym func_overlay_001_F00036A0_184FA80=overlay1AssignRecordIndex \
		--redefine-sym func_overlay_001_F0003750_184FB30=overlay1ChoosePath \
		--redefine-sym func_overlay_001_F0003FD8_18503B8=overlay1TransitionState \
		--redefine-sym func_overlay_001_F000438C_185076C=overlay1UpdateObjectPhysics \
		--redefine-sym func_overlay_001_F0005BF4_1851FD4=overlay1StartTimerCallbacks \
		--redefine-sym overlay1GetObjectList=overlay1GetObjectListReloc \
		--redefine-sym overlay27CanUse=overlay1Overlay27CanUseReloc \
		--redefine-sym overlay3RunCachedModeAction=overlay1Overlay3RunCachedModeActionReloc \
		--redefine-sym sqrtf=overlay1SqrtReloc \
		--redefine-sym func_80005750=func_80005750_o001Reloc \
		--redefine-sym Arctanf=Arctanf_o001Reloc \
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
		--redefine-sym amSndPlayXYZ=amSndPlayXYZ_o001Reloc \
		--redefine-sym amSndSetXYZ=amSndSetXYZ_o001Reloc \
		--redefine-sym amSndStopXYZ=amSndStopXYZ_o001Reloc \
		--redefine-sym func_80008128=func_80008128_o001Reloc \
		--redefine-sym func_8001357C=func_8001357C_o001Reloc \
		--redefine-sym func_8001D41C=func_8001D41C_o001Reloc \
		--redefine-sym func_8001D910=func_8001D910_o001Reloc \
		--redefine-sym func_8001D960=func_8001D960_o001Reloc \
		--redefine-sym func_8001DD70=func_8001DD70_o001Reloc \
		--redefine-sym func_8001E5C4=func_8001E5C4_o001Reloc \
		--redefine-sym func_8001F14C=func_8001F14C_o001Reloc \
		--redefine-sym func_8001F25C=func_8001F25C_o001Reloc \
		--redefine-sym func_8002675C=func_8002675C_o001Reloc \
		--redefine-sym func_800299E8=func_800299E8_o001Reloc \
		--redefine-sym func_80029F2C=func_80029F2C_o001Reloc \
		--redefine-sym func_8002A878=func_8002A878_o001Reloc \
		--redefine-sym func_8002AA0C=func_8002AA0C_o001Reloc \
		--redefine-sym func_8003EDEC=func_8003EDEC_o001Reloc \
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
	$(OBJCOPY) --redefine-sym GetRomlistInfo=func_overlay_001_F0000000_184C3E0 $@ && \
	$(OBJCOPY) --redefine-sym func_8000572C=func_overlay_001_F0000000_184C3E0 $@ && \
	$(OBJCOPY) --redefine-sym func_80005820=func_overlay_001_F0000000_184C3E0 $@ && \
	$(OBJCOPY) --redefine-sym overlay4RemoveObject=func_overlay_001_F0000000_184C3E0 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4664 && \
	candidate_rodata_size=$$( $(OBJDUMP) -h $@ | awk '$$2 == ".rodata" { print $$3; exit }' ); \
	if [ -n "$$candidate_rodata_size" ] && [ "$$candidate_rodata_size" != "00000000" ]; then \
		$(OBJCOPY) --add-symbol gOverlay1ModeTable=0xEC,global \
			--add-symbol gOverlay1ChoosePathLiterals=0xE8,global $@ && \
		$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
			@config/normalizations/overlay1DispatchMode.rebind.spec && \
		$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
			@config/normalizations/overlay1UpdateObjectPhysics.rebind.spec && \
		$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
			0x4B8:.rodata:gOverlay1ChoosePathLiterals \
			0x4FC:.rodata:gOverlay1ChoosePathLiterals \
			0x5EC:.rodata:gOverlay1ChoosePathLiterals \
			0x5F0:.rodata:gOverlay1ChoosePathLiterals \
			0x718:.rodata:gOverlay1ChoosePathLiterals \
			0x71C:.rodata:gOverlay1ChoosePathLiterals \
			0x734:.rodata:gOverlay1ChoosePathLiterals \
			0x738:.rodata:gOverlay1ChoosePathLiterals && \
		$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
			sha256:19bf9f9139510fe5a4bb8f6ac08f0857e9d257e0fb70474f046534d12c6e46da && \
		$(OBJCOPY) --remove-section=.rel.rodata $@; \
	fi
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_create.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_create.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym sqrtf=overlay1SqrtReloc $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x190
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_end.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x350
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_scaled.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_scaled.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x58

# Overlay 19 is one TU under the overlay flag set; no post-processing.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o019/overlay_019.c.o: CFLAGS += -Wab,-r4300_mul

# Overlay 4 is one translation unit at the overlay flag set and owns its pad.
# Its resident calls go through the generated surface rebinds.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o004/overlay_004.c.o: CFLAGS += -Wab,-r4300_mul
O8_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o008/overlay_008.c.o
$(O8_OBJ): config/normalizations/overlay8P0058.rebind.spec
$(O8_OBJ): config/normalizations/overlay8P1294.rebind.spec
$(O8_OBJ): config/normalizations/overlay8UpdateChannels.rebind.spec
$(O8_OBJ): config/normalizations/overlay8P2640.rebind.spec
$(O8_OBJ): config/normalizations/overlay8P34A0.rebind.spec
$(O8_OBJ): config/normalizations/overlay8P42A8.rebind.spec
$(O8_OBJ): config/normalizations/overlay8P4CF0.rebind.spec
# The object's .rodata is seven literal pools in function order: the +0x0058
# update's seven floats (retail 0xB0 on), the +0x1294 update's forty (one
# entry per use, retail 0xF8 through 0x194), the +0x2640 emitter's two
# (retail 0x198, 0x19C), the +0x3018 channel update's (retail 0x1BC on), the
# +0x34A0 update's thirty-four (retail 0x1D8 through 0x25C), the +0x42A8
# update's eleven (retail 0x260 through 0x288; retail's unreferenced 0.04 at
# 0x28C has no entry here), then the +0x4CF0 surface update's (retail 0x290
# on).  Each pool is rebound to an absolute anchor and keeps its
# section-relative addend, so each anchor is its retail offset less the bytes
# that precede it in the object: 0xF8 less 0x1C, 0x198 less 0xBC, 0x1BC less
# 0xC4, 0x1D8 less 0xD8, 0x260 less 0x160, 0x290 less 0x18C.
# gOverlay8CommandBuffer is a second header name for gOverlay8Buffer (the
# +0x34A0 update's second address build; one name would share the address).
# No -Wo,-loopunroll,0 (removed 2026-10-02).  It was added for the +0x34A0
# body's angle loop and was byte-inert on every function of this unit by then;
# the +0x0058 body's four selector reads are a plain for loop that the
# default -O2 unroller expands exactly as shipped, which the flag forbade.
# Overlay 8 is one translation unit at the overlay flag set and owns its pad;
# Its placeholder calls and data
# references are valued by the generated surface and its resident calls go
# through the generated rebinds, so no symbol is renamed here. What remains is
# rodata ownership, kept until phase 3 (docs/OVERLAY_STRUCTURE_SPRINT.md):
# the seven literal pools are anchored to the retained image as described
# above and the duplicate .rodata is dropped by digest.
$(O8_OBJ): CFLAGS += -Wab,-r4300_mul
$(O8_OBJ): POSTPROCESS = \
	$(OBJCOPY) --add-symbol \
		gOverlay8P0058Constants=0xB0,global $@ && \
	$(OBJCOPY) --add-symbol \
		gOverlay8P1294Constants=0xDC,global $@ && \
	$(OBJCOPY) --add-symbol \
		gOverlay8P2640Constants=0xDC,global $@ && \
	$(OBJCOPY) --add-symbol \
		gOverlay8UpdateChannelConstants=0xF8,global $@ && \
	$(OBJCOPY) --add-symbol \
		gOverlay8P34A0Constants=0x100,global $@ && \
	$(OBJCOPY) --add-symbol \
		gOverlay8P42A8Constants=0x100,global $@ && \
	$(OBJCOPY) --add-symbol \
		gOverlay8P4CF0Constants=0x104,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay8P0058.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay8P1294.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay8P2640.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay8UpdateChannels.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay8P34A0.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay8P42A8.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay8P4CF0.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:cb542eb96174af8e212c2049a1ab797e4bdd93850668aeb935a4771504af5aca

# Overlay 9 is one translation unit at the overlay flag set and owns its pad.
# Data externalization remains until the module's initialized data is C-owned
# (docs/OVERLAY_STRUCTURE_SPRINT.md phase 3): func_overlay_009_F0000000_1866678
# defines the parameter block in this object's .data at its recorded offset
# (+0x2D0) only so as1 shares one high half between two stores; the retained
# overlay image owns the bytes, so the copy is dropped and its 35 sites are
# rebound to a zero-valued base. The TU's literal pool duplicates the retained
# overlay rodata and is asserted by digest.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay_009.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay_009.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	config/normalizations/func_overlay_009_F0000000_1866678.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o009/overlay_009.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay9DataBaseReloc=0x0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_009_F0000000_1866678.rebind.spec && \
	$(OBJCOPY) --remove-section=.data --remove-section=.gptab.data $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:2c21a6f383952f9246cda00fcc1f0456af3e324811b7ca17f018fd4c4f0658d9

# Overlay 36 is one TU under the overlay flag set; no post-processing.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o036/overlay_036.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 27 is one TU under the overlay flag set; its .text is the module's
# whole text and its resident calls take the generated rebinds. The one
# remaining step is phase-3 scaffolding: func_overlay_027_F0000064_187BA3C's
# four float literals are the first sixteen bytes of the retained overlay
# rodata, so bind them to that zero-base owner until C owns the module's
# data; the digest fails closed if the compiler ever emits a different pool.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay_027.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay_027.c.o: \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o027/overlay_027.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:dd6fc9f17b7ca59ee611df3687276c9a16893e66f1c08e0b6d5ef3800683dd6f
# Overlay 41 is one translation unit at the overlay flag set, filling the
# module's text exactly. The whole module's .rodata (the sampler's
# jump table and literals, the path updater's literals, the fade scale, the
# spawn switch table and its constant) is byte-identical to the module's
# initialized range, which stays a retained blob until phase 3: assert it by
# digest, anchor its section symbol at the loader's zero base so every
# section-relative %hi/%lo keeps its stored offset, and drop the jump-table
# relocations whose words the blob already carries. No instruction is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay_041.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay_041.c.o: \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o041/overlay_041.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:11717def9b3693b05fbb74a032a7280f9579104310696d672798e29ae4898a3f && \
	$(OBJCOPY) --remove-section .rel.rodata $@
# Overlay 53 is one translation unit at the overlay flag set (the HUD easing
# needs -Wab,-r4300_mul); its .text carries the trailing pad, so nothing is
# trimmed. Resident and cross-overlay references use the generated surface.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o053/overlay_053.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 54 is one translation unit at the overlay flag set (with it IDO
# emits the HUD height easing as the shipped rotated branch-likely loop); its
# .text carries the trailing pad, so nothing is trimmed. The module owns its
# .data (the atlas's data_rodata +0x0..+0x2D0) and .bss. Their shipped address
# words are section-relative, so every
# site is rebound to a zero-valued base. Its 0.66f pool duplicates the
# retained rodata at +0x2D0 and is asserted by digest until phase 3 gives the
# module its rodata too. No instruction is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay_054.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay_054.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay_054.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o054/overlay_054.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay54DataBaseReloc=0x0,global \
		--add-symbol gOverlay54BssBaseReloc=0x0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_054.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:d131bb7de7509c98fc3533790f960348a16e2e461e7d1f5ffff1cd58ae2243e0

# Overlay 29 is one TU under the overlay flag set: no trim, and its resident
# calls take the generated rebinds. What remains is phase-3 scaffolding for
# rodata the TU does not own yet, none of it touching an instruction word:
# the update's compiler pool -- -0.1f, -0.2f, FLT_MAX, 750.0f and 0.4f --
# duplicates the retained overlay data at data_rodata +0x0 (rodata-relative
# +0x0, which the shipped %hi/%lo pairs encode). Rebind its ten references to
# a pool symbol and discard the digest-checked duplicate (overlay 86's
# metadata-only form).
O29_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o029/overlay_029.c.o
$(O29_OBJ): \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay_029.rebind.spec
$(O29_OBJ): CFLAGS += -Wab,-r4300_mul
$(O29_OBJ): POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay29UpdatePoolReloc=0x0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_029.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:b59302a556a40d537bcc94b8e036c741672de52d8d671fb8bf20d087fa0b8930
# Overlay 51 is one translation unit at the overlay flag set (the HUD easing
# loop and clock-hand multiply need -Wab,-r4300_mul); its .text carries the
# trailing pad, so nothing is trimmed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o051/overlay_051.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 15 is one translation unit at the overlay flag set; its .text now
# carries the module's trailing pad (no trim), and its resident calls go
# through the generated surface rebinds. What remains is data and BSS
# ownership, kept until phase 3 gives them C owners
# (docs/OVERLAY_STRUCTURE_SPRINT.md):
# overlay15DrawScreenStars reads its fade scale as the float literal
# 255.0f / 292.0f, so the compiler emits a one-word private pool that duplicates
# the retained data word at data_rodata +0x40 (the last word of
# gOverlay15InitializedData; the shipped LOCAL %hi/%lo pair encodes 0x0000 and
# the loader supplies the base).  Rebind the two references to a pool symbol
# and discard the digest-checked duplicate (overlay 86's metadata-only form).
# The unit defines its two particle fields as `static` BSS (the shipped code
# shares one high half per adjacent pair of bound loads, which IDO only does
# for a locally-defined symbol). Every access is then a section-relative record
# whose addend is already the module-relative offset the shipped word carries,
# and overlay 15's own runtime table owns those records, so the static link
# must not adjust them a second time: drop them, as overlay 57 does. No
# instruction word is edited.
O15_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o015/overlay_015.c.o
# Retain the existing local BSS names after the last GLOBAL_ASM is removed.
# Promotion proofs use these physical section definitions; this metadata pass
# leaves the stock compiler flags, allocated bytes and relocations unchanged.
$(O15_OBJ): \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	config/normalizations/overlay15DrawScreenStars.rebind.spec
$(O15_OBJ): CFLAGS += -Wab,-r4300_mul
$(O15_OBJ): POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay15FadePoolReloc=0x0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay15DrawScreenStars.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:eca26b0fe4ae2523bfa3733b2543e689b87d531790ea5c2b879f17144cade2a9 && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x430:5:.bss 0x438:6:.bss 0x468:5:.bss 0x470:6:.bss \
		0x474:6:.bss 0x478:5:.bss 0x47c:6:.bss 0x480:6:.bss \
		0x488:5:.bss 0x4a0:6:.bss 0x4c0:6:.bss 0x4c4:5:.bss \
		0x4c8:6:.bss 0x4cc:6:.bss 0x4d0:5:.bss 0x4d4:6:.bss \
		0xa00:5:.bss 0xa04:6:.bss 0xa30:5:.bss 0xa34:6:.bss \
		0xa3c:5:.bss 0xa48:6:.bss 0xa5c:6:.bss 0xa88:5:.bss \
		0xa8c:6:.bss 0xa94:5:.bss 0xa9c:6:.bss 0xaac:6:.bss \
		0xab0:5:.bss 0xab8:6:.bss 0xabc:5:.bss 0xacc:6:.bss \
		0xad0:6:.bss 0xad4:5:.bss 0xadc:6:.bss 0xae4:5:.bss \
		0xae8:6:.bss 0xaf0:6:.bss 0xaf4:5:.bss 0xaf8:6:.bss \
		0xafc:6:.bss 0xb00:5:.bss 0xb14:6:.bss 0xb18:6:.bss \
		0xb20:5:.bss 0xb2c:6:.bss 0xb30:6:.bss 0xb34:5:.bss \
		0xb3c:6:.bss 0xbfc:5:.bss 0xc00:6:.bss 0xc24:5:.bss \
		0xc28:6:.bss 0xc2c:6:.bss 0xc30:5:.bss 0xc34:6:.bss \
		0x738:5:.bss 0x73c:6:.bss
# Overlay 34 is one TU under the overlay flag set; no post-processing.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o034/overlay_034.c.o: CFLAGS += -Wab,-r4300_mul
include config/normalizations/overlay22Epoch12.mk
# Overlay 94 is one translation unit under the overlay flag set. Its .text
# includes the trailing pad; resident callees use the generated surface renames.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o094/overlay_094.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 101 is one translation unit under the overlay flag set (-woff 835
# silences the unused-label warnings the switch bodies raise; it is not a
# codegen flag). POSTPROCESS remains until the packer owns the overlay link
# and phase 3 gives the module's rodata a C owner:
# - 132 calls to exported functions of this module are SYMBOL records whose
#   shipped jal field is zero; the static link would resolve them, so their
#   relocations are filtered (overlay_101.filter.spec, checked site by site
#   against the shipped records). Calls to unexported callees are JUMP records
#   and link as compiled.
# - The object's .rodata is the shipped pool tail at rodata +0xE0C..+0xF10
#   (both switch tables, the clock and builder literals, the selector table),
#   apart from the DispatchActive/DispatchEvents table residuals; the bytes
#   stay in the retained data blob. Its 16 text HI/LO pairs are rebound to an
#   absolute +0xE0C anchor (no instruction word changes), the payload is
#   asserted by digest and dropped, with its now-orphaned table relocations.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay_101.c.o: CFLAGS += -Wab,-r4300_mul -woff 835
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay_101.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay_101.rodata.rebind.spec \
	config/normalizations/overlay_101.filter.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o101/overlay_101.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol overlay101RodataE0C=0xE0C,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_101.rodata.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay_101.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:8a8fd1a2dc784d558ac66b908e977fc3581d6aeabdc409d536cd287716dd511f && \
	$(OBJCOPY) --remove-section .rel.rodata $@
# Overlay 21 is one TU under the overlay flag set; no post-processing.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o021/overlay_021.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 30 is one TU under the overlay flag set; no post-processing.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o030/overlay_030.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 23 is one TU under the overlay flag set; no post-processing.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o023/overlay_023.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 24 owns its compiler-emitted pool; the whole module is linked
# section-relative before its initialized sections enter the ROM link.
MODULE_LINKED_OVERLAYS += 024
$(BUILD_DIR)/$(SRC_DIR)/overlays/o024/overlay_024.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 25 is one TU under the overlay flag set. Its .text needs no trim and
# its resident calls take the generated rebinds. The one remaining step is
# phase-3 scaffolding: the TU's literal pool is the retained overlay rodata at
# +0x20 byte for byte (the updater's six literals, then
# overlay25SetVectorFlags' 0.707f), so assert it by digest and keep the
# section-relative addends on a zero base until C owns the module's data.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay_025.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay_025.c.o: \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o025/overlay_025.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:258adccbc890f89cf0c2393e490673ba6b5f4d39ca12eeeb9f7d74881dd69d02
# Overlay 39 at the overlay flag set. The two 16-byte copy loops are written
# per byte: IDO's default unroll (factor 4) emits the shipped four-copy loop
# body, which the former hand-unrolled spelling only matched with unrolling
# disabled. Its .text carries the trailing pad, so nothing is trimmed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o039/overlay_039.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 37 is one translation unit at the overlay flag set (the effect
# renderer's blend multiplies carry the R4300 FP-mul hazard nops). Its .text
# is the module's text, trailing pad included, so nothing is trimmed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o037/overlay_037.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 40 is one translation unit at the overlay flag set; its .text carries
# the trailing pad, so nothing is trimmed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o040/overlay_040.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 42 is one translation unit at the overlay flag set; its .text is the
# module's whole text, so nothing is trimmed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o042/overlay_042.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 43 ("clone.c") is one translation unit at the overlay flag set (the
# projection builder keeps the radius multiply out of the call's delay slot,
# which is -Wab,-r4300_mul); its .text carries the trailing pad, so nothing is
# trimmed. The pending-queue flush reaches the queue count
# and the message block through D_C8 and ext_4d258, whose shipped words carry
# the stored offset with no relocation record; the filter keeps those six
# addends in place until the module's data is C-owned (phase 3). The
# compiler's two-float pool duplicates the retained rodata at +0x20: its
# pairs are bound to that base and the copy dropped after its digest agrees.
# No instruction is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay_043.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay_043.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay_043.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o043/overlay_043.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x198:5:D_C8 \
		0x19c:6:D_C8 \
		0x1cc:5:ext_4d258 \
		0x1d0:6:ext_4d258 \
		0x230:5:D_C8 \
		0x234:6:D_C8 \
		0x258:5:D_C8 \
		0x274:6:D_C8 && \
	$(OBJCOPY) --add-symbol gOverlay43RodataBaseReloc=0x20,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_043.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:7b3d5ee74adddfd3455dcdc8268465d8eef652d3915984a4b06ba99fd0692c37
# Overlay 44 is one translation unit at the overlay flag set; its .text carries
# the trailing pad, so nothing is trimmed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o044/overlay_044.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 92 is one translation unit under the overlay flag set; its text
# ends exactly at the module's text size, so no trim is needed, and its
# resident and cross-module callees go through the generated surface renames.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o092/overlay_092.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o093/overlay_093.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o095/overlay_095.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 97 directly owns its transform context and scale literal pool.
# The whole-module link preserves their separate section-relative bases.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o097/overlay_097.c.o: CFLAGS += -Wab,-r4300_mul
MODULE_LINKED_OVERLAYS += 097
# Overlay 96 is one translation unit under the overlay flag set. Its call to
# the exported overlay96Register is a SYMBOL record whose shipped word holds
# a zero addend, so that relocation is filtered until the packer owns the
# overlay link. Its .text includes the trailing pad.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay_096.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o096/overlay_096.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x474:4:overlay96Register
# Overlay 98 is one translation unit under the overlay flag set; its .text
# includes the trailing pad.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o098/overlay_098.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 100 is one translation unit under the overlay flag set. Its four
# calls to exported functions of the same module are SYMBOL records, whose
# shipped words hold the addend (zero) rather than the callee's address; the
# static link would resolve them, so the relocations are filtered until the
# packer owns the overlay link. Its .text includes the trailing pad.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay_100.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o100/overlay_100.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x24c:4:overlay100RemoveEntry 0x35c:4:overlay100UpdateMotion \
		0x3c8:4:overlay100RemoveEntry 0x550:4:overlay100DrawMotion
# Overlay 45 is one translation unit at the overlay flag set; its .text
# carries the trailing pad, so nothing is trimmed. The module owns its .data
# (data_rodata +0x0..+0x80); the
# shipped words that address it are section-relative, so the setup-list
# sites are rebound to a zero-valued base, and the path updater's float pool,
# a duplicate of the three motion constants at data +0x24, is bound to a base
# valued 0x24 and dropped after its digest agrees. No instruction is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay_045.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay_045.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay_045.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o045/overlay_045.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay45DataBaseReloc=0x0,global \
		--add-symbol gOverlay45RetainedMotionConstants=0x24,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_045.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:c52cb05fd2fed4ffaf80d0f50519a1b99d267a85960aeefe5e036df19162ad00
# NON_MATCHING/GLOBAL_ASM per docs/acceleration-survey.md sec.13.2: this
# object's instructions used to be reached by rewriting three fields after
# compilation (normalize_elf_instructions.py), which no gold-standard N64
# decomp does. The .c now GLOBAL_ASMs the extracted retail bytes instead;
# only the symbol rename below (metadata, not instructions) survives.
# Overlay 20 is one TU under the overlay flag set: no trim, and its resident
# calls take the generated rebinds. One relocation-metadata step on the entry
# configurator (+0xE28) stays, touching no instruction word: retail encodes
# three zero-base data references directly (the entry count, the entry table
# and the lifetime block), so the shipped tables carry no record for those
# HI/LO pairs. Offsets are within overlay_020.c's .text.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay_020.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay_020.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o020/overlay_020.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0xE40:5:gOverlay20EntryCount 0xE44:6:gOverlay20EntryCount \
		0xEA0:5:gOverlay20Entries 0xEA8:6:gOverlay20Entries \
		0xEEC:5:D_0 0xF30:6:D_0
# Overlay 31 is one TU under the overlay flag set: no trim, and its resident
# calls take the generated rebinds. Two relocation-metadata steps on the
# palette builder (+0x4F8) stay, neither touching an instruction word: the
# item table's HI/LO pairs have no shipped record (filtered), and its two
# colour-cycle calls are SYMBOL records whose shipped target is mmAlloc
# (rebound). Offsets in both specs are within overlay_031.c's .text.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay_031.c.o: \
	config/normalizations/overlay_031.filter.spec \
	config/normalizations/overlay_031.calls.spec \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay_031.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o031/overlay_031.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay_031.filter.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_031.calls.spec
# Overlay 33 is one TU under the overlay flag set; no post-processing.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o033/overlay_033.c.o: CFLAGS += -Wab,-r4300_mul
include mk/overlay_046.mk

# Overlay 38 is one translation unit at the overlay flag set; its .text carries
# the module's trailing pad, so nothing is trimmed. The update's three rates
# are float literals, so the compiler emits a 0x10-byte pool that duplicates
# the retained overlay data at rodata-relative +0x0 (the shipped hi/lo pairs
# are LOCAL records with stored addends 0, 4 and 8). Until the module's data
# is C-owned (sprint phase 3), assert the pool by digest and bind the pairs to
# the loader-owned base; no instruction is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/overlay_038.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o038/overlay_038.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:bdcefd8e01ed3fbf749d94f1d2eca6ebf09ccfef55954c89ecb731ed6d8a3752

ifneq ($(NON_MATCHING),0)
else
endif

# Overlay 6 is one translation unit at the overlay flag set; its .text carries
# the module's trailing pad, so nothing is trimmed. All three functions are
# exported.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o006/overlay_006.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 10 is one translation unit at the overlay flag set.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o010/overlay_010.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 18 is one translation unit at the overlay flag set.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o018/overlay_018.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 17 is one translation unit at the overlay flag set.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o017/overlay_017.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 3 is one translation unit at the overlay flag set.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o003/overlay_003.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 13 is one translation unit at the overlay flag set.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o013/overlay_013.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 5 is one translation unit at the overlay flag set. The libaudio bank
# patcher, six -O3 objects before the fold (DKR builds it at -O3), compiles to
# the same bytes at -O2 as static functions: -O2 does not inline them.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o005/overlay_005.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 12 is one translation unit at the overlay flag set and owns its pad;
# The resident calls go through
# the generated surface rebinds. What remains is rodata ownership, kept until
# phase 3: the TU's five float literals are the retained data at data_rodata
# +0x130, in the order the shipped rodata-relative LOCAL records expect, so the
# pool is asserted by digest and anchored at zero.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay_012.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay_012.c.o: \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o012/overlay_012.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:0d02c190457cd94e2b33b9a8fe45a73687e90716f4b71bf32203ccd7723733d6
# Overlay 2 is one translation unit at the overlay flag set.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o002/overlay_002.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 11 is one translation unit at the overlay flag set and owns its pad.
# Resident and cross-overlay calls use the generated surface rebinds. Rodata
# ownership remains until phase 3: the TU's merged .rodata (six switch
# tables and the background's two 0.05f literals) is the retained data at
# data_rodata +0x218, and the shipped LOCAL records' rodata addends run from a
# base eight bytes before it, so its 16 references are rebound to a base
# symbol at +0x8 and the section is dropped by digest. (Its
# ReleaseCurrentGroup table disagrees with the shipped one in four entries;
# the retained bytes are what links.)
O11_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o011/overlay_011.c.o
$(O11_OBJ): CFLAGS += -Wab,-r4300_mul
$(O11_OBJ): $(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py
$(O11_OBJ): POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay11RodataBaseReloc=0x8,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0xa8:.rodata:gOverlay11RodataBaseReloc 0xb0:.rodata:gOverlay11RodataBaseReloc \
		0x224:.rodata:gOverlay11RodataBaseReloc 0x228:.rodata:gOverlay11RodataBaseReloc \
		0x258:.rodata:gOverlay11RodataBaseReloc 0x25c:.rodata:gOverlay11RodataBaseReloc \
		0x91c:.rodata:gOverlay11RodataBaseReloc 0x924:.rodata:gOverlay11RodataBaseReloc \
		0x1be0:.rodata:gOverlay11RodataBaseReloc 0x1be8:.rodata:gOverlay11RodataBaseReloc \
		0x1fbc:.rodata:gOverlay11RodataBaseReloc 0x1fc4:.rodata:gOverlay11RodataBaseReloc \
		0x2458:.rodata:gOverlay11RodataBaseReloc 0x2460:.rodata:gOverlay11RodataBaseReloc \
		0x2c24:.rodata:gOverlay11RodataBaseReloc 0x2c2c:.rodata:gOverlay11RodataBaseReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:808256f231ed1e6ec17b5941d4dc25adfe13414394067ec3f26266c7358c1d49 && \
	$(OBJCOPY) --remove-section .rel.rodata $@
# Overlay 14 is one translation unit at the overlay flag set; its placeholder,
# resident and data
# references are valued and rebound by the generated surface. What remains is
# rodata ownership, kept until phase 3: the TU's .rodata (the loader's and the
# draw's seven-entry switch tables) is the retained data at data_rodata
# +0x158, and the shipped LOCAL records' rodata addends run from a base 0x38
# bytes before it, so its four references are rebound to a base symbol at
# +0x38 and the section is dropped by digest.
O14_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o014/overlay_014.c.o
$(O14_OBJ): CFLAGS += -Wab,-r4300_mul
$(O14_OBJ): $(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py
$(O14_OBJ): POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay14RodataBaseReloc=0x38,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x914:.rodata:gOverlay14RodataBaseReloc 0x91c:.rodata:gOverlay14RodataBaseReloc \
		0x1944:.rodata:gOverlay14RodataBaseReloc 0x194c:.rodata:gOverlay14RodataBaseReloc && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:a1d78d3f7fb36b44d65aae7a5aed14af5a74f46d965b1cb4408485477bbdb033 && \
	$(OBJCOPY) --remove-section .rel.rodata $@
# Overlay 16 is one translation unit at the overlay flag set; its .text
# carries the module's trailing pad, so nothing is trimmed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o016/overlay_016.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 99 is one translation unit under the overlay flag set (the former
# -Wo,-loopunroll,0 on overlay99BuildHeightGrid was inert). Its one pool
# literal (182.0444f) duplicates the retained overlay rodata at +0x0, which
# the shipped hi/lo pair encodes, so until phase 3 gives the module's rodata
# a C owner the pool is asserted by digest and externalized; no instruction
# is edited. The call to the exported overlay99RenderSegments is a SYMBOL
# record whose shipped word holds a zero addend, so that relocation is
# filtered. Its .text includes the trailing pad.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay_099.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay_099.c.o: \
	$(TOOLS_DIR)/externalize_elf_section.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o099/overlay_099.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x1324:4:overlay99RenderSegments && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:15fbd0fd7f255861ebff752dc2b8e418edd9110dfe29426ed775e3dc124c3f17
# Overlay 50 is one translation unit at the overlay flag set; its .text carries
# the trailing pad, so nothing is trimmed. The module
# defines its .data and .bss at their recorded offsets, but the retained
# overlay image owns the bytes until phase 3: the copies are dropped and every
# site is rebound to a base placeholder (data and bss at 0, the 0.7f pool at
# rodata +0x4), and the duplicate pool is asserted by digest. Only relocation
# symbol indices and section ownership change; no instruction is edited.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay_050.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay_050.c.o: \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay_050.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o050/overlay_050.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay50DataBaseReloc=0x0,global \
		--add-symbol gOverlay50BssBaseReloc=0x0,global \
		--add-symbol gOverlay50ScaleReloc=0x4,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_050.rebind.spec && \
	$(OBJCOPY) --remove-section=.data --remove-section=.bss \
		--remove-section=.gptab.data --remove-section=.gptab.bss $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:54719d33292c6bb1573315eb3140be35c277ea530a17f384ba3a0027e82bf244
include mk/overlay_052.mk
# Overlay 91 is one translation unit under the overlay flag set. Until phase 3
# gives the module's initialized data and rodata a C owner, the object's own
# .rodata is dropped (the shipped blob supplies it) and the relocations that
# name blob data, rodata, or a callee reached through the runtime table are
# filtered so the linker leaves the compiler's stored words alone. Its .text
# includes the trailing pad.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay_091.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o091/overlay_091.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x098:5:overlay91GlobalA 0x0a8:6:overlay91GlobalA \
		0x0c4:5:.rodata 0x0cc:6:.rodata \
		0x180:4:func_8002A8C0 0x1f0:4:func_80000F94 \
		0x1f8:4:func_overlay_007_F0000DBC_185CC44 0x21c:4:func_8002A8C0 \
		0x274:4:func_80000F94 0x2bc:4:func_80000F94 \
		0x2c4:4:func_overlay_007_F0000DBC_185CC44 0x33c:4:func_80000F94 \
		0x344:4:func_overlay_007_F0000DBC_185CC44 0x34c:5:overlay91GlobalB \
		0x354:6:overlay91GlobalB 0x3ac:4:func_80006EA0 \
		0x3f0:4:func_8002A8BC && \
	$(OBJCOPY) --remove-section=.rodata $@
# Overlay 48 is one translation unit at the overlay flag set; its .text carries
# the trailing pad, so nothing is trimmed.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o048/overlay_048.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 49 is one translation unit at the overlay flag set; its .text carries
# the trailing pad, so nothing is trimmed. Its resident and cross-overlay
# references go through the generated relocation surface.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o049/overlay_049.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 28 is one TU under the overlay flag set: no trim, and its resident
# calls take the generated rebinds. What remains is relocation metadata the
# one-TU build does not yet express (phase 1/3 work), none of it touching an
# instruction word:
# - the loader owns the data-base HI/LO stored in the reset field, so the
#   shipped tables carry no record for those two sites (filtered);
# - two calls are SYMBOL records whose shipped target differs from the name
#   the C uses: the update-vertices call carries the external at 0x29E00,
#   and the render tail's external call carries the module's offset-0 entry;
# - the render tail's display-list words are absolute 0x80000028 references.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o028/overlay_028.c.o: \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/overlays/o028/overlay_028.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o028/overlay_028.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x1D8:5:gOverlay28DataBase 0x1F4:6:gOverlay28DataBase && \
	$(OBJCOPY) \
		--add-symbol D_80000028=0x80000028,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		0x41C:overlay28UpdateVertices:ext_o0_29e00 \
		0x51C:ext_o0_29e00:overlay28ResetBuffer \
		0x744:gOverlay28DisplayList28:D_80000028 \
		0x754:gOverlay28DisplayList28:D_80000028
# Overlay 35 is one TU under the overlay flag set; no post-processing.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o035/overlay_035.c.o: CFLAGS += -Wab,-r4300_mul

# Overlay 26 is one TU under the overlay flag set: no trim, and its resident
# calls take the generated rebinds. What remains is phase-3 scaffolding for
# rodata the TU does not own yet, none of it touching an instruction word:
# the update's compiler pool (-0.1f, -0.2f, 6.283185f and two 0.9f)
# duplicates the retained overlay data at rodata-relative +0x0, which the
# shipped hi/lo pairs encode. Rebind its ten references to a pool symbol and
# discard the digest-checked duplicate (overlay 29's metadata-only form).
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay_026.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay_026.c.o: \
	config/normalizations/overlay_026.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/overlays/o026/overlay_026.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay26UpdatePoolReloc=0x0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_026.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:65b37d85616c58b2a7b1db845df735b73a82241585f505fda670a9739bdeea4b
OVERLAY_TRIMMED_OBJECTS := \
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
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o001/overlay_001_scaled.c.o
OVERLAY_TRIMMED_OBJECTS += \
    $(O22_UPDATE_OBJECT_OBJ)
OVERLAY_TRIMMED_OBJECTS += \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Initialize.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65UpdateParticles.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65Release.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o065/overlay65ResetSlots.c.o \
    $(BUILD_DIR)/$(SRC_DIR)/overlays/o065/func_overlay_065_F0000C38_18C4EA0.c.o

include mk/overlay_084.mk

# Overlays 73-90 (one translation unit per module; OVERLAY_STRUCTURE_SPRINT.md phase 2).
# Overlay 76 is one translation unit under the overlay flag set; its .text is
# the module's 0x120 bytes, trailing pad included, so it needs no trim.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o076/overlay_076.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 78 is one translation unit under the overlay flag set; its .text
# is the module's whole text, trailing pad included, so it needs no trim.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o078/overlay_078.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 79 is one translation unit under the overlay flag set; its .text is
# the module's 0x14E0 bytes, so it needs no trim. overlay79FindNearby and
# func_overlay_079_F0000FA0_18CDF40 have no ORT row and are static (JUMP
# records). Its .data is C-owned. Until its remaining data moves into C
# (docs/OVERLAY_STRUCTURE_SPRINT.md phase 3), two metadata steps stay:
# - func_overlay_079_F0000000_18CCFA0's init-scale %hi/%lo pair is a LOCAL
#   record into retained data with a zero base addend, so its two
#   relocations are dropped (the loader owns them);
# - the float literals of the +0x134 and +0xFA0 functions, one compiler pool
#   here, bind to the retained constant pool at +0x4 (the shipped records'
#   addends +0x4.. and +0x30..), and the digest-checked duplicate is
#   discarded. Only relocation symbol indices change.
# Resident callees go through the generated _o079Reloc renames.
O79_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o079/overlay_079.c.o
$(O79_OBJ): CFLAGS += -Wab,-r4300_mul
$(O79_OBJ): \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay_079.filter.spec \
	config/normalizations/overlay_079.rebind.spec
$(O79_OBJ): POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		@config/normalizations/overlay_079.filter.spec && \
	$(OBJCOPY) --add-symbol gOverlay79LiteralPoolReloc=0x4,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_079.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:4f4b5cad0bbe4b59b64cdaf4faf9b845a7b61724d8c86ccc7faf7b2b40e01960
# Overlay 80 is one translation unit under the overlay flag set; its .text is
# the module's 0x3F0 bytes, trailing pad included, so it needs no trim.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o080/overlay_080.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 86 is one translation unit under the overlay flag set; its .text is
# the module's 0xED0 bytes, trailing pad included, so it needs no trim. Two
# steps remain until the module's data moves into C
# (docs/OVERLAY_STRUCTURE_SPRINT.md phase 3):
# - The compiler's pool for func_overlay_086_F0000474_18D22AC -- two float
#   constants and the five-entry state-switch table -- duplicates the
#   retained overlay data at data_rodata +0x80 (rodata-relative +0x0, which
#   the shipped %hi/%lo pairs encode). Rebind the six references to a pool
#   symbol and discard the digest-checked duplicate; no instruction or
#   compiler addend is edited.
# - overlay86ScaledVectorPosition's vector-table address is loader-owned in
#   the shipped table (a LOCAL pair with zero base addend), so its two
#   relocations against the blob-owned gOverlay86Vectors are dropped.
# Same-module calls to exported functions name the _o086Reloc placeholders.
O86_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o086/overlay_086.c.o
$(O86_OBJ): CFLAGS += -Wab,-r4300_mul
$(O86_OBJ): \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	$(TOOLS_DIR)/filter_elf_relocations.py \
	config/normalizations/overlay_086.rebind.spec
$(O86_OBJ): POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay86StatePoolReloc=0x0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_086.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:eaecfbbed7d41ea06a4083ce0ec8d40fe5d624fab5574d44903561c845808cec && \
	$(OBJCOPY) --remove-section .rel.rodata $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/filter_elf_relocations.py $@ .text \
		0x94:5:gOverlay86Vectors 0xa0:6:gOverlay86Vectors
# Overlay 73 is one translation unit under the overlay flag set (the updater's
# floating multiplies need the R4300 hazard schedule); its .text is the
# module's 0xEB0 bytes, trailing pad included, so it needs no trim. Until the
# module's data moves into C (docs/OVERLAY_STRUCTURE_SPRINT.md phase 3), the
# compiler pool -- the updater's five-entry mode-switch table and float
# literals, byte-identical to the retained overlay data at data_rodata +0xDC
# (rodata-relative +0xC, which the shipped %hi/%lo pairs encode) -- is bound
# to a pool symbol and the digest-checked duplicate discarded; no instruction
# or compiler addend is edited. Resident callees go through the generated
# surface entries.
O73_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o073/overlay_073.c.o
$(O73_OBJ): CFLAGS += -Wab,-r4300_mul
$(O73_OBJ): \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/overlay_073.rebind.spec
$(O73_OBJ): POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay73UpdatePoolReloc=0xC,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/overlay_073.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:de0b301581ff894b38ba7352b7227b858eb5ef8b43ad3b7147ad849732053c8d && \
	$(OBJCOPY) --remove-section .rel.rodata $@
# Overlay 74 is one translation unit under the overlay flag set; its .text is
# the module's 0x250 bytes, trailing pad included, so it needs no trim.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o074/overlay_074.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 75 is one translation unit under the overlay flag set; its .text is
# the module's 0x700 bytes, trailing pad included, so it needs no trim.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o075/overlay_075.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 77 directly owns its initialized state and compiler literal pool.
# The whole-TU module link resolves their separate section-relative bases.
MODULE_LINKED_OVERLAYS += 077
$(BUILD_DIR)/$(SRC_DIR)/overlays/o077/overlay_077.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 81 is one translation unit under the overlay flag set; its .text
# is the module's whole text, trailing pad included, so it needs no trim.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o081/overlay_081.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 82 is one translation unit under the overlay flag set. Its
# six-element initializer clear matches at the default unroll setting, as do
# the update loops; the module's .text includes its trailing alignment.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o082/overlay_082.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 88 is one translation unit under the overlay flag set; its .text is
# the module's 0x740 bytes, so it needs no trim. Its renderer is overlay 69's
# source, included and renamed (overlay_088.c), so the C spells overlay 69's
# callees; this rename gives the module its own runtime-relocation
# placeholders for them, one of which (overlay17DrawStrip) is cross-overlay
# and so has no generated rename. The anchor update's resident calls go
# through the generated _o088Reloc renames.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay_088.c.o: \
	$(SRC_DIR)/overlays/o069/overlay69DrawSortedGeometry.inc.c
$(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay_088.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/overlays/o088/overlay_088.c.o: POSTPROCESS = \
	$(OBJCOPY) \
		--redefine-sym overlay17DrawStrip=overlay88DrawFixedResourceReloc \
		--redefine-sym camGetProjZ=overlay88MetricReloc \
		--redefine-sym func_80022E80=overlay88PrepareTransformReloc \
		--redefine-sym func_80022FD4=overlay88SubmitDynamicReloc \
		--redefine-sym fxDrawCone=overlay88DrawConeReloc $@
# Overlay 89 is one translation unit under the overlay flag set; its .text is
# the module's 0x7D0 bytes, trailing pad included, so it needs no trim. Its
# three unexported functions are static (JUMP records).
$(BUILD_DIR)/$(SRC_DIR)/overlays/o089/overlay_089.c.o: CFLAGS += -Wab,-r4300_mul
# NON_MATCHING fallback assembly supplies the retail body; restore the
# friendly source symbol and retain the exact text extent when needed.
# Overlay 87 is one translation unit under the overlay flag set; its .text is
# the module's 0x970 bytes, trailing pad included, so it needs no trim. The
# former offset-zero rebinds of overlay87InitializeObject's resident calls
# are the generated _o087Reloc renames now. overlay87HasNearby is static
# (a JUMP record).
$(BUILD_DIR)/$(SRC_DIR)/overlays/o087/overlay_087.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 85 is one translation unit under the overlay flag set; its .text
# is the module's whole text, trailing pad included, so it needs no trim.
$(BUILD_DIR)/$(SRC_DIR)/overlays/o085/overlay_085.c.o: CFLAGS += -Wab,-r4300_mul
# Overlay 83 is one translation unit under the overlay flag set; its .text is
# the module's 0xB60 bytes, so it needs no trim. Its six unexported functions
# are static (JUMP records).
$(BUILD_DIR)/$(SRC_DIR)/overlays/o083/overlay_083.c.o: CFLAGS += -Wab,-r4300_mul
# Both functions are exact C. The state machine's nop between its paired
# owner->x products is the VR4300 multiply workaround. Its compiler pool --
# six float constants, the seven-entry state-switch table and 0.003f --
# duplicates the retained overlay data at data_rodata +0x8 (rodata-relative
# +0x8 after overlay90Initialize's two words, which the shipped %hi/%lo pairs
# encode). Rebind the sixteen references to a pool symbol and discard the
# digest-checked duplicate; no instruction or compiler addend is edited
# (overlay 86's metadata-only form), until the data moves into C
# (docs/OVERLAY_STRUCTURE_SPRINT.md phase 3). The seventeen resident callees
# go through the generated surface entries. The module is one translation
# unit: its .text is the module's 0xB20 bytes with the trailing pad, so no
# trim.
O90_OBJ := $(BUILD_DIR)/$(SRC_DIR)/overlays/o090/overlay_090.c.o
$(O90_OBJ): CFLAGS += -Wab,-r4300_mul
$(O90_OBJ): \
	$(TOOLS_DIR)/rebind_elf_relocations.py \
	$(TOOLS_DIR)/externalize_elf_section.py \
	config/normalizations/func_overlay_090_F00000FC_18D4BF4.rebind.spec
$(O90_OBJ): POSTPROCESS = \
	$(OBJCOPY) --add-symbol gOverlay90StatePoolReloc=0x8,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/func_overlay_090_F00000FC_18D4BF4.rebind.spec && \
	$(HOST_PYTHON) $(TOOLS_DIR)/externalize_elf_section.py $@ .rodata \
		sha256:c350971913605d505c513324a16e420965bd2f632cce4c770353b9c7fc954f99 && \
	$(OBJCOPY) --remove-section .rel.rodata $@

# Overlay 47 owns all source sections and links them at runtime section bases.
MODULE_LINKED_OVERLAYS += 047
$(BUILD_DIR)/$(SRC_DIR)/overlays/o047/overlay_047.c.o: CFLAGS += -Wab,-r4300_mul
