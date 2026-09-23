# Mickey's Speedway USA (US) — clean-room decompilation build
#
# This is one ordinary host build graph. Splat describes the ROM layout and
# emits the linker script; matched functions compile from C, unmatched
# functions enter those same C objects through GLOBAL_ASM, and retained data
# enters as extracted binary inputs. A single final link places resident code
# and every overlay back at their original ROM offsets. The game's runtime
# overlay loader is source under src/main/runlink.c, not Make machinery.
#
#   gmake            build/mickey.us.z64
#   gmake verify     build + SHA1 compare against the baserom hash
#   gmake setup      venv + toolchain + baserom check + splat extraction + git hooks
#   gmake hooks      point git at .githooks (clean-room commit/push gates)
#   gmake cleanroom  clean-room sweep (CLEANROOM_ARGS=--staged, --range A..B)
#   gmake audit-decoders  assert no clean-room decoder is inventing words
#   gmake overlay-tables  decode the four overlay ROM blocks and check the layout
#   gmake overlay-atlas   check the generated overlay manifest and yaml block
#   gmake overlay-atlas-write  refresh those two tracked generated artifacts
#   gmake overlay-syms    regenerate the overlay relocation surface
#   gmake check-overlay-syms   fail if that generated block has drifted
#   gmake overlay-donors  validate the exhaustive DKR/JFG donor ledger
#   gmake overlay-donors-write  rescan the out-of-tree donor builds
#   gmake prune-asm  delete asm/ files splat orphaned (also run by every split)
#   gmake reference-builds        rebuild the out-of-tree reference decomp farm
#   gmake check-reference-builds  prove that farm is the one the names came from
#   gmake scoreboard        regenerate README.md's progress block from the tree
#   gmake check-scoreboard  fail if that block has gone stale
#   gmake forced-floor-census  regenerate docs/forced-floor-census.md from the handoffs
#   gmake system-health     read-only campaign load/memory/process summary
#   gmake check-tooling     focused safety/provenance/tooling regressions
#   gmake small-delta-census  insertion-pair census -> docs/small-delta-census.md
#   gmake promotion-proof SYMBOL=name  strict post-promotion exactness receipt
#   gmake release-gate      serial, niced release checks with compact output
#   gmake public-release    dry-run reconciliation/preflight; never pushes
#   gmake clean      remove build/
#   gmake distclean  also remove splat's generated output

BASENAME := mickey
VERSION  := us

# ---------------------------------------------------------------------------
# Directories
# ---------------------------------------------------------------------------

# Compile-only escape hatch for the NON_MATCHING/GLOBAL_ASM functions (see
# docs/acceleration-survey.md sec.13.2): `gmake NON_MATCHING=1` takes every
# converted TU's real C body instead of its GLOBAL_ASM fallback, so the
# functions queued for source restructuring keep compiling under IDO without
# being claimed as matched. It never produces a byte-identical ROM -- `verify`
# refuses to run under it, exactly DKR's guard.
NON_MATCHING ?= 0

# Separate build tree for NON_MATCHING=1: objects compiled with -DNON_MATCHING
# are never byte-identical, so they must never sit next to (or be mistaken
# for, via stale timestamps) the objects `verify` checks.
ifeq ($(NON_MATCHING),0)
BUILD_DIR := build
else
BUILD_DIR := build_non_matching
endif
SRC_DIR   := src
# Every directory under src/ that holds .c files. Discovered rather than listed
# so adding a new source subdirectory (src/libultra, src/main, ...) needs no
# Makefile edit -- splat decides the layout via the yaml's subsegment names.
SRC_DIRS  := $(shell find $(SRC_DIR) -type d 2>/dev/null)
# Same idea as SRC_DIRS: naming a subsegment `libultra/foo` makes splat write
# asm/libultra/foo.s, and a hardcoded list would silently drop it from the link
# (no error -- just a wrong ROM). `asm` stays literal so `distclean` still has
# something to remove on a tree that was never split.
#
# asm/nonmatchings is deliberately excluded: those files are assembled *into* C
# objects by asm-processor via `#pragma GLOBAL_ASM`, so assembling them again
# here would define every one of their symbols twice.
ASM_DIRS  := asm $(shell find asm -mindepth 1 -type d -not -path 'asm/nonmatchings*' 2>/dev/null)
# Overlay data/relocation tails are deliberately nested by module. Discover
# binary directories just like asm/source directories so adding a generated
# module never requires a second, hand-maintained object list.
BIN_DIRS  := assets $(shell find assets -mindepth 1 -type d 2>/dev/null)
TOOLS_DIR := tools

# ---------------------------------------------------------------------------
# Tools
# ---------------------------------------------------------------------------

CROSS   := $(TOOLS_DIR)/binutils/mips64-elf-
AS      := $(CROSS)as
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy
OBJDUMP := $(CROSS)objdump
STRIP   := $(CROSS)strip

HOST_CC := cc
HOST_PYTHON := python3
VENV    := .venv
PYTHON  := $(VENV)/bin/python
SHA1    := shasum -a 1

# IDO 5.3. These are the project-default C flags; per-file overrides live in the
# "Per-file compiler flags" block further down and must be justified in the
# file's header comment (see docs/CLEANROOM.md's sibling rule in the plan).
CC      := $(TOOLS_DIR)/ido/cc
OPT_FLAGS := -O2
MIPSISET  := -mips1 -32
DEFINES   := -D_LANGUAGE_C -D_FINALROM -DTARGET_N64 -DVERSION_$(VERSION) \
             -D_MIPS_SZLONG=32
ifneq ($(NON_MATCHING),0)
DEFINES += -DNON_MATCHING
endif
INCLUDE_CFLAGS := -I . -I include -I include/libc -I include/PR -I assets
CFLAGS  := -non_shared -G 0 -Xcpluscomm -fullwarn -woff 649,838 -nostdinc \
           $(DEFINES) $(INCLUDE_CFLAGS)
POSTPROCESS := @:

# Report-and-skip for the digest-guarded POSTPROCESS passes, for
# tools/promotion_trial.py only. Unset (the default) every guard aborts the
# build as it always has; set, a guard prints a `PROMOTION-TRIAL: ...` marker
# and skips its pass, so a candidate whose codegen is the wrong *size* yields
# `text-size-differs (+N bytes)` and a linked ROM to diff instead of a bare
# build failure. The resulting ROM is not a valid build and is never verified.
# Exported so it reaches the tools; see tools/postprocess_guard.py.
PROMOTION_TRIAL ?=
export PROMOTION_TRIAL

# Every per-file POSTPROCESS below is a post-compile ELF normalization -- a
# section trim, a relocation rebind or filter, an added relocation guarded by a
# .text prefix hash. All of them encode the *matching* object's exact layout,
# so none of them can succeed against an object compiled with -DNON_MATCHING:
# the text is a different size and the relocations sit at different offsets.
# The escape hatch is compile-only by design (see NON_MATCHING above), so the
# normalizations are simply skipped in the build_non_matching tree. Recursive
# assignment on purpose: it has to expand in each target's context so the
# target-specific POSTPROCESS override is the one that runs.
ifeq ($(NON_MATCHING),0)
RUN_POSTPROCESS = $(POSTPROCESS)
else
RUN_POSTPROCESS = @:
endif

# asm-processor (simonlindholm) is what makes `#pragma GLOBAL_ASM("...")` work
# with IDO: it strips the pragmas out, compiles the remaining real C, assembles
# the referenced .s files with $(AS), and splices the result back into the
# object so hand-written asm and compiled C share one translation unit in the
# right order. Invocation shape is
#   build.py <compiler...> -- <assembler...> -- <compile args...> <input.c>
# i.e. the compiler and assembler command lines are passed through verbatim.
ASM_PROCESSOR := $(PYTHON) $(TOOLS_DIR)/asm-processor/build.py

CRC := $(TOOLS_DIR)/n64crc

# ---------------------------------------------------------------------------
# Flags
# ---------------------------------------------------------------------------

# Verified working assembler invocation (Task 4).
ASFLAGS := -march=vr4300 -32 -mabi=32 -G0 -I include

# asm-processor's GLOBAL_ASM path needs a couple of macros that its own prelude
# doesn't define; gas takes several input files, so the extra prelude is simply
# handed to it ahead of asm-processor's temporary .s. See the file's comment.
ASM_PROC_ASFLAGS := $(ASFLAGS) include/asm_processor_prelude.inc

# splat's ld script names every input object explicitly, so nothing is passed
# on the command line; --no-check-sections because segments deliberately share
# VRAM ranges (overlays / assets all follow main's vram).
LD_SCRIPT := $(BASENAME).$(VERSION).ld
LDFLAGS   := -T $(LD_SCRIPT) \
             -T undefined_funcs_auto.$(VERSION).txt \
             -T undefined_syms_auto.$(VERSION).txt \
             -T libultra_undefined_syms.$(VERSION).txt \
             -T overlay_undefined_syms.$(VERSION).txt \
             -Map $(BUILD_DIR)/$(BASENAME).$(VERSION).map --no-check-sections

# rom_fill already carries the trailing 0xFF region, so --pad-to is a belt-and
# -braces guarantee of the 32MiB image size rather than a real fill step.
ROM_SIZE     := 0x2000000
OBJCOPYFLAGS := -O binary --pad-to=$(ROM_SIZE) --gap-fill=0xFF

# ---------------------------------------------------------------------------
# Files
# ---------------------------------------------------------------------------

# There is no second, JFG-derived overlay build hidden below. Splat classifies
# every generated input into the same three ordinary lists: C, assembly, or
# binary. Overlay paths are simply the members under */overlays/oNNN/. The
# generated linker script, not a parallel Make pipeline, places those objects
# back into each module's [.text][.data][reloc1][reloc2] ROM range. See the
# final-link comment and docs/CONTRIBUTING.md's "Overlay build flow" section.
S_FILES   := $(foreach dir,$(ASM_DIRS),$(wildcard $(dir)/*.s))
BIN_FILES := $(foreach dir,$(BIN_DIRS),$(wildcard $(dir)/*.bin))
C_FILES   := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

# Matching tools deliberately compile the guarded C bodies in files that
# contain NON_MATCHING candidates.  They normally use build_non_matching/ or
# build/wb/, but a manual full-TU probe can still leave one of those objects in
# build/.  Timestamps cannot tell that its preprocessor mode was wrong.  The
# successful-verify receipt lets the guard force only objects whose content
# changed since the last byte-identical ROM proof.
NONMATCHING_C_FILES := $(shell grep -l '#ifdef NON_MATCHING' $(C_FILES) 2>/dev/null)
CANONICAL_CANDIDATE_O_FILES := $(addprefix build/,$(addsuffix .o,$(NONMATCHING_C_FILES)))
CANONICAL_CANDIDATE_RECEIPT := build/.canonical-candidate-objects.json

# Every header, as a blunt prerequisite for every object: there are only a
# handful of them and IDO's dependency output is awkward to wire in, so
# "recompile all C when any header changes" is the cheap correct answer.
H_FILES   := $(shell find include -name '*.h' 2>/dev/null)

O_FILES := $(foreach f,$(S_FILES),$(BUILD_DIR)/$(f).o) \
           $(foreach f,$(C_FILES),$(BUILD_DIR)/$(f).o) \
           $(foreach f,$(BIN_FILES),$(BUILD_DIR)/$(f).o)

ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(ASM_DIRS) $(BIN_DIRS) $(SRC_DIRS))

TARGET   := $(BUILD_DIR)/$(BASENAME).$(VERSION)
BASEROM  := baseroms/$(BASENAME).$(VERSION).z64
EXPECTED_SHA1 := $(firstword $(shell cat $(BASENAME).$(VERSION).sha1))

# splat.yaml is the single source of truth for what gets extracted/split from
# the baserom (asm/, assets/, the generated .ld script...). This stamp makes
# that split step a real Make dependency: anything that reads split output
# (the .s/.bin -> .o pattern rules, the final link) order-depends on it, so
# an edited yaml re-splits before those steps run instead of silently
# building against stale disassembly. See the .o pattern rules and
# $(TARGET).elf below for how the ordering is wired up.
SPLAT_STAMP := $(BUILD_DIR)/.splat-stamp

# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------

default: all

# Two-phase build, driven by a fresh recursive $(MAKE) per phase.
#
# Why: a single `make` invocation decides whether build/asm/FOO.s.o needs
# rebuilding by stat()-ing its prerequisites once, in the order they're
# listed on the rule -- `%.s` (normal) comes before `$(SPLAT_STAMP)`
# (order-only), so that stat happens *before* the split recipe (which is
# what actually rewrites %.s) has run, even though make correctly runs the
# order-only prereq's recipe first. Net effect, verified empirically: within
# one invocation, order-only alone does NOT retrigger .o rebuilds after a
# same-invocation re-split -- `gmake` would silently link against
# pre-re-split objects. (Minimal repro kept in the Task 2 fix notes.)
#
# The reliable fix is the classic two-pass recursive-make idiom: run the
# split to completion as its own `make` invocation (phase 1), then start a
# genuinely *new* `make` invocation for the real build (phase 2) -- its
# dependency scan stats files from disk fresh, after phase 1's writes, so
# object files correctly see their .s as newer and rebuild. The .o/.elf
# pattern rules below still order-depend on $(SPLAT_STAMP) too, as a
# best-effort guard for anyone invoking a build target directly instead of
# via `all`/`verify`.
all:
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	@$(MAKE) --no-print-directory $(TARGET).z64

setup: $(PYTHON) hooks
	$(PYTHON) -m pip install -q -r requirements.txt
	git submodule update --init $(TOOLS_DIR)/asm-processor \
		$(TOOLS_DIR)/asm-differ $(TOOLS_DIR)/m2c
	$(TOOLS_DIR)/setup_toolchain.sh
	$(TOOLS_DIR)/verify_baseroms.sh
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_atlas.py --check
	$(PYTHON) -m splat split $(BASENAME).$(VERSION).yaml
	@$(MAKE) --no-print-directory prune-asm
	@mkdir -p $(BUILD_DIR)
	@touch $(SPLAT_STAMP)

# Unconditional re-split, e.g. after hand-editing yaml and wanting the result
# immediately without going through the dependency graph. Keeps the stamp in
# sync so a following `gmake` doesn't redundantly split again.
extract:
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_atlas.py --check
	$(PYTHON) -m splat split $(BASENAME).$(VERSION).yaml
	@$(MAKE) --no-print-directory prune-asm
	@mkdir -p $(BUILD_DIR)
	@touch $(SPLAT_STAMP)

# splat writes asm/ and never prunes it, so converting a subsegment from `asm`
# to `c`, or naming a function, leaves a file behind that nothing produces any
# more. Neither breaks the build -- the linker script simply stops naming the
# object -- but tools/progress.py counts a function as unmatched while any
# glabel for it survives anywhere under asm/, so the matched count silently
# under-reports until the file is deleted. It ran after every split rather than
# being remembered.
prune-asm:
	@$(PYTHON) $(TOOLS_DIR)/prune_stale_asm.py $(BASENAME).$(VERSION).yaml

verify:
ifneq ($(NON_MATCHING),0)
	$(error verify does not run under NON_MATCHING=1 -- it never produces a byte-identical ROM; unset NON_MATCHING and rebuild)
endif
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	@stale="$$($(HOST_PYTHON) $(TOOLS_DIR)/canonical_candidate_guard.py \
		--manifest $(CANONICAL_CANDIDATE_RECEIPT) dirty \
		$(CANONICAL_CANDIDATE_O_FILES))"; \
	if [ -n "$$stale" ]; then \
		count=$$(printf '%s\n' $$stale | wc -w | tr -d ' '); \
		echo "canonical candidate guard: rebuilding $$count changed/unproven object(s)"; \
		$(HOST_PYTHON) $(TOOLS_DIR)/run_logged.py \
			--repo . --log build/verify/canonical-candidates.log \
			--label "canonical candidate rebuild ($$count objects)" -- \
			$(MAKE) --no-print-directory --always-make \
				--assume-old=$(PYTHON) --assume-old=$(SPLAT_STAMP) $$stale || exit $$?; \
	fi
	@$(HOST_PYTHON) $(TOOLS_DIR)/run_logged.py \
		--repo . --log build/verify/rom-build.log \
		--label "canonical ROM build" -- \
		$(MAKE) --no-print-directory $(TARGET).z64
	@got=$$($(SHA1) $(TARGET).z64 | cut -d' ' -f1); \
	echo "expected $(EXPECTED_SHA1)"; \
	echo "built    $$got"; \
	if [ "$$got" = "$(EXPECTED_SHA1)" ]; then \
		$(HOST_PYTHON) $(TOOLS_DIR)/canonical_candidate_guard.py \
			--manifest $(CANONICAL_CANDIDATE_RECEIPT) write \
			$(CANONICAL_CANDIDATE_O_FILES); \
		echo "OK  $(TARGET).z64 matches the expected US ROM hash"; \
	else \
		echo "FAIL $(TARGET).z64 does not match the expected US ROM hash"; \
		exit 1; \
	fi

# Points git at the tracked hook directory. Separate from `setup` so it can be
# re-run on its own, and so a clone that only wants the gates does not have to
# build a toolchain to get them. core.hooksPath is per-clone config, not a
# tracked file, so every fresh clone needs this once.
hooks:
	git config core.hooksPath .githooks
	@echo "hooks active: .githooks/pre-commit, .githooks/pre-push"

# Clean-room sweep. Extra arguments pass through, so the same target serves
# every mode:
#   gmake cleanroom                                  worktree
#   gmake cleanroom CLEANROOM_ARGS=--staged          the index
#   gmake cleanroom CLEANROOM_ARGS="--range A..B"    a commit range
cleanroom:
	bash $(TOOLS_DIR)/cleanroom_check.sh $(CLEANROOM_ARGS)

system-health:
	$(HOST_PYTHON) $(TOOLS_DIR)/system_health.py $(SYSTEM_HEALTH_ARGS)

# Discovers and runs every tools/test_*.py and tests/test_*.py file, so a
# new test file is never silently excluded. Special-cased interpreters and
# class restrictions (a build-needing class skipped, a venv-only import)
# live in run_tool_tests.py itself, not here -- see its docstring.
check-tooling:
	$(HOST_PYTHON) $(TOOLS_DIR)/run_tool_tests.py $(CHECK_TOOLING_ARGS)
	$(HOST_PYTHON) $(TOOLS_DIR)/test_force_lattice.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_web_footprint.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_score_symbol.py ForcedObjectTests
	$(HOST_PYTHON) $(TOOLS_DIR)/test_draw_census.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_insertion_pairs.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_lineage_census.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_function_probe.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_progress.py
	$(PYTHON) $(TOOLS_DIR)/test_raw_asm_census.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_merge_transaction.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_check_shard_metrics.py
	$(HOST_PYTHON) tests/test_make_layout.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_check_match_regression.py
	$(HOST_PYTHON) tests/test_flag_sweep.py
	$(HOST_PYTHON) tests/test_tu_flag_impact.py
	$(HOST_PYTHON) tests/test_overlay_atlas.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_reloc_identity.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_reloc_surface.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_cleanroom_detectors.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_proof_provenance.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_metadata_filter_proof.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_elf_metadata_contracts.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_function_history.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_function_preflight.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_canonical_candidate_guard.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_promotion_proof.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_allocator_trace_receipt.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_integration_base.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_lane_status.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_check_donor_claims.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_mul_scheduler_scan.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_authorize_reopen.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_resolve_target_asm.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_check_isa_overrides.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_check_paths.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_ready_queue.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_skeleton_scan.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_wb_compare.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_nm_ranking.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_align_symbol.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_frame_census.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_triage.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_reclaim_worktrees.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_register_census.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_permute_batch_deadline.py
	$(PYTHON) $(TOOLS_DIR)/test_candidate_context.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_permute_sweep_cli.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_sweep_receipts.py
	$(PYTHON) $(TOOLS_DIR)/test_source_fidelity.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_promotion_transaction.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_finalize_plateau.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_crew_heartbeat.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_release_gate.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_run_logged.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_experiment_ledger.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_plateau_remeasure.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_resolve_comment_hunks.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_lane_cache.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_public_release.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_blockclimb.py
	$(HOST_PYTHON) $(TOOLS_DIR)/test_residual_map.py

# Insertion-pair reading of every 0 < |size_delta| <= 12 ranking row, written
# to docs/small-delta-census.md. Needs the extracted tree and, for ownership,
# the instrumented IDO toolchain; one configured and one traced compile per TU.
.PHONY: small-delta-census
small-delta-census:
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	$(PYTHON) $(TOOLS_DIR)/small_delta_census.py

# Ownership-only inventory; does not count padding/scaffolds as matched C.
.PHONY: check-raw-asm
check-raw-asm:
	$(PYTHON) $(TOOLS_DIR)/raw_asm_census.py --check-overlays

promotion-proof:
	@test -n "$(SYMBOL)" || { echo "usage: gmake promotion-proof SYMBOL=name [PROMOTION_PROOF_ARGS='--canonical']"; exit 2; }
	$(HOST_PYTHON) $(TOOLS_DIR)/promotion_proof.py "$(SYMBOL)" $(PROMOTION_PROOF_ARGS)

release-gate:
	$(HOST_PYTHON) $(TOOLS_DIR)/release_gate.py $(RELEASE_GATE_ARGS)

# Public release reconciliation has no push operation.  The default rerenders
# derived artifacts through their read-only checks, computes exact deltas from
# the explicitly named remote-tracking branch, scans every outgoing commit
# tree/message, and composes the ordinary release gate.  Explicit
# --write-derived invokes only the documented in-tree artifact generators and
# leaves their output for review; a clean dry-run must follow the commit.
#
#   gmake public-release \
#     PUBLIC_RELEASE_ARGS="--remote public --branch master"
public-release:
	$(HOST_PYTHON) $(TOOLS_DIR)/public_release.py $(PUBLIC_RELEASE_ARGS)

# Asserts that no clean-room decoder is inventing words -- that every stage
# which exists to DECODE something contributes nothing to a tree whose content
# is not encoded. Five false-decode defects were found by hand before this was
# automated, two of them only by re-running the audit after a "fix" had
# re-routed the phantom to a different decoder. Read the header of
# tools/audit_decoders.py before changing a threshold it complains about.
#
#   gmake audit-decoders                       tracked files (~0.2s)
#   gmake audit-decoders AUDIT_ARGS=--all      every blob reachable from all refs
#   gmake audit-decoders AUDIT_ARGS=--verbose  print the per-stage totals
#
# Deliberately NOT folded into `cleanroom`. The hooks run `cleanroom` on every
# commit and it must stay fast and must fail only for clean-room reasons; this
# check answers a different question -- "is the detector still measuring
# reality?" -- and it is aimed at whoever edits a decoder, not at whoever
# writes a commit. Folding it in would make an unrelated red bar block commits,
# which is precisely how a gate gets bypassed with --no-verify.
audit-decoders:
	$(PYTHON) $(TOOLS_DIR)/audit_decoders.py $(AUDIT_ARGS)

# Decodes the four overlay blocks the yaml splits (main relocation table, ROM
# table, header table, module images) out of the baserom and re-asserts the
# layout: the count word, 370 of 375 reloc entries landing on a real
# `jal TrapDanglingJump`, the per-module byte-gap arithmetic across all 107
# headers, the final module's end at 0x18F1FE0, and clone.c's strings falling
# inside overlay 43's .data. Reads the ROM at run time and writes nothing, so
# like `verify` it needs a baserom and cannot run in CI.
#
#   gmake overlay-tables                       human-readable module map
#   gmake overlay-tables OVERLAY_ARGS=--json   machine-readable, same fields
overlay-tables:
	$(PYTHON) $(TOOLS_DIR)/overlay_tables.py $(OVERLAY_ARGS)

# The atlas is the canonical, reviewable projection of all 107 module headers,
# 18,542 relocation records, exports, resident references, and cross-overlay
# edges. `overlay-atlas` is read-only and fails on drift. Updating the tracked
# JSON and generated yaml block is explicit so an ordinary build never edits
# source files as a side effect.
overlay-atlas:
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_atlas.py --check

overlay-atlas-write:
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_atlas.py --write

# overlay_undefined_syms.$(VERSION).txt is generated, not maintained. Every one
# of its lines is derivable: a value line is the stored relocation addend read
# from the baserom at the site the module's own relocation table names, and an
# alias line is the generated splat identity for a module offset pointed at the
# friendly name the adopted C defines there. Both come from
# config/overlays.$(VERSION).json's text_ownership rows plus the compiled
# objects, so the surface regenerates on every promotion instead of being
# hand-derived per function. See docs/reloc-surface.md.
#
# It needs the overlay objects compiled (not linked -- the link is exactly what
# is missing when a promotion fails to resolve), so both targets build them
# first. `check-overlay-syms` fails on drift and is an explicit release gate;
# unlike `check-docs`, it also requires the complete compiled overlay set.
OVERLAY_SYM_OBJECTS := $(filter $(BUILD_DIR)/$(SRC_DIR)/overlays/%,$(O_FILES))

overlay-syms:
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	@$(MAKE) --no-print-directory $(OVERLAY_SYM_OBJECTS)
	$(HOST_PYTHON) $(TOOLS_DIR)/reloc_surface.py generate --write

check-overlay-syms:
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	@$(MAKE) --no-print-directory $(OVERLAY_SYM_OBJECTS)
	$(HOST_PYTHON) $(TOOLS_DIR)/reloc_surface.py generate --check

# Every overlay decomp pass starts with the DKR v77/v80 and JFG object scans in
# this ledger. The ordinary target validates the committed 107-row-per-donor
# report without needing the out-of-tree builds. Refresh and reproducibility
# checks are explicit because they do need the pinned reference farm.
overlay-donors:
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_donor_scan.py --check

overlay-donors-write:
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_donor_scan.py --write

overlay-donors-scan-check:
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_donor_scan.py --scan-check

# The other direction, and the one `audit-decoders` is structurally blind to: a
# decoder that quietly STOPS producing words looks exactly like a decoder
# behaving, so every number in the audit stays green while the gate becomes a
# no-op. This synthesizes real-ROM fixtures in every encoding at every wrap
# width from baseroms/mickey.us.z64 AT RUN TIME, asserts each is still caught,
# and writes nothing to disk -- a committed fixture that proves the detectors
# catch ROM data would itself be ROM data, and `cleanroom` would rightly reject
# it. Needs the baserom, so like `verify` it cannot run in CI; run it after
# touching tools/cleanroom_detectors.py, alongside `audit-decoders`, not
# instead of it.
check-fixtures:
	$(PYTHON) $(TOOLS_DIR)/false_negative_fixtures.py $(FIXTURE_ARGS)

# The reference decompilations 190 of this tree's tier-A names were mined from.
# `reference-builds` rebuilds that farm from the commits and baserom checksums
# tools/reference-builds.lock pins; `check-reference-builds` proves a farm on
# this machine is the one those names came from, by re-deriving each title's
# aggregate digest and comparing it. Neither can run in CI: the farm needs
# retail ROMs, which this project does not ship and never will, so this is the
# same honest split as `verify` and `check-fixtures`.
#
#   gmake reference-builds REFS_ARGS=jfg     one title
#   gmake reference-builds REFS_ARGS="--root DIR --jobs 2"
#
# Reference material stays out of the tree (docs/CLEANROOM.md); what is
# committed is the recipe, the pins and the digests. docs/references.md says
# what each build yielded and what remains uncheckable.
reference-builds:
	bash $(TOOLS_DIR)/setup_reference_builds.sh $(REFS_ARGS)

check-reference-builds:
	bash $(TOOLS_DIR)/verify_reference_builds.sh $(REFS_ARGS)

# Re-derives the arithmetic the docs claim -- VRAM/ROM conversions, segment
# size subtractions, the MiB column of the top-level map, the jump-table count
# -- and fails on a mismatch. A count audit found several of these stale at
# once; they are all recomputable, so they are recomputed here rather than at
# the next review. The jump-table count needs asm/, so it is skipped (not
# failed) before `gmake extract`.
check-docs:
	$(HOST_PYTHON) $(TOOLS_DIR)/lane_status.py --check-reopen-schema
	$(HOST_PYTHON) $(TOOLS_DIR)/check_donor_claims.py
	$(HOST_PYTHON) $(TOOLS_DIR)/check_isa_overrides.py
	$(HOST_PYTHON) $(TOOLS_DIR)/check_paths.py
	$(PYTHON) $(TOOLS_DIR)/check_derived_numbers.py
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_donor_scan.py --check
	$(HOST_PYTHON) $(TOOLS_DIR)/postprocess_audit.py --check-redefines
	$(HOST_PYTHON) $(TOOLS_DIR)/nm_ranking.py --check-doc
	$(HOST_PYTHON) $(TOOLS_DIR)/nm_ranking.py --check-retired
	$(HOST_PYTHON) $(TOOLS_DIR)/plateau_handoff_audit.py --check

# Proved colour floors recorded in plateau handoffs, summarised. Reads the
# handoff shards, source PLATEAU-HANDOFF blocks and the ranking; no build.
# tools/triage.py computes the same census live and withholds the
# colour-exhausted rows from colour routing; this file is the tracked summary.
forced-floor-census:
	$(HOST_PYTHON) $(TOOLS_DIR)/forced_floor_census.py --write docs/forced-floor-census.md

# Keep the shared linked-ELF prerequisite quiet for progress consumers while
# retaining complete compiler/linker diagnostics on disk.
QUIET_ELF_BUILD = $(HOST_PYTHON) $(TOOLS_DIR)/run_logged.py \
	--repo . --log build/progress/elf-build.log \
	--label "linked ELF build" -- \
	$(MAKE) --no-print-directory $(TARGET).elf

# Builds just far enough to have a linked ELF (no crc/z64 round-trip needed --
# tools/progress.py only reads the ELF's symbol table plus the current asm/
# and symbol_addrs.$(VERSION).txt state), then reports the derived progress
# numbers. Same two-phase split-then-build shape as `all`/`verify`, for the
# same reason (see the big comment on `all` above).
progress:
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	@$(QUIET_ELF_BUILD)
	$(PYTHON) $(TOOLS_DIR)/progress.py --version $(VERSION)

# Rewrites README.md's scoreboard block, between its SCOREBOARD_BEGIN /
# SCOREBOARD_END markers, from the same derivation `progress` prints. The
# README is the most-read page in the repo and therefore the worst place for a
# hand-maintained count: this project has already caught summary numbers that
# had drifted from the tree they described, and the block this replaces was
# itself stale by two functions and a hundred and ninety symbols. So it is
# generated, and `check-scoreboard` below proves it stayed generated.
scoreboard:
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	@$(QUIET_ELF_BUILD)
	$(PYTHON) $(TOOLS_DIR)/progress.py --version $(VERSION) --update-readme

# Fails if README.md's scoreboard block is not what the tree generates right
# now, printing the diff. Deliberately a separate target from `check-docs`:
# that one re-derives arithmetic *stated in prose* and needs nothing built,
# while this one needs a linked ELF, so folding them together would make
# `check-docs` require a toolchain and a build to answer a question that has
# nothing to do with one.
check-scoreboard:
	@$(MAKE) --no-print-directory $(SPLAT_STAMP)
	@$(QUIET_ELF_BUILD)
	$(PYTHON) $(TOOLS_DIR)/progress.py --version $(VERSION) --check-readme

clean:
	rm -rf $(BUILD_DIR)

distclean: clean
	rm -rf $(ASM_DIRS) $(BIN_DIRS)
	rm -f $(LD_SCRIPT) undefined_funcs_auto.$(VERSION).txt undefined_syms_auto.$(VERSION).txt
	rm -f $(CRC)

# ---------------------------------------------------------------------------
# Recipes
# ---------------------------------------------------------------------------

$(ALL_DIRS):
	@mkdir -p $@

# Bootstrap the venv on a fresh clone. Only runs when .venv/bin/python is
# missing, so an existing environment is never rebuilt from under you.
$(PYTHON):
	$(HOST_PYTHON) -m venv $(VENV)

$(CRC): $(TOOLS_DIR)/n64crc.c
	$(HOST_CC) -O2 -w -o $@ $<

# $(SPLAT_STAMP) is an order-only prereq here: it guarantees the split has
# run at least once before this rule's recipe executes. It is *not* enough
# on its own to retrigger a rebuild after a same-invocation re-split (see
# the big comment on the `all` target above for why, and why the real
# correctness mechanism is the two-phase recursive `make` in `all`/`verify`)
# -- but it's a harmless, cheap safety net for anyone building a specific
# .o/.elf target directly instead of going through `all`/`verify`.
$(BUILD_DIR)/%.s.o: %.s | $(ALL_DIRS) $(SPLAT_STAMP)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.bin.o: %.bin | $(ALL_DIRS) $(SPLAT_STAMP)
	$(OBJCOPY) -I binary -O elf32-bigmips -B mips $< $@

# The C rule. Everything goes through asm-processor unconditionally -- a file
# with no GLOBAL_ASM pragmas is passed through to IDO untouched, so there is no
# reason to maintain two recipes.
#
# $(SPLAT_STAMP) is a *normal* (not order-only) prerequisite here, unlike the
# .s.o/.bin.o rules below: a #pragma GLOBAL_ASM in a .c file names a .s under
# asm/nonmatchings/ that splat regenerates on every re-split, and that .s is
# not itself listed as a prerequisite of this object (there is no per-TU
# dependency file naming exactly which GLOBAL_ASM paths one .c references).
# Without a real dependency on the stamp, a re-split that renames a symbol
# (splat re-numbers auto names, or a friendly name changes) leaves an
# already-built .c.o holding a stale reference and an incremental build can
# link a mismatched object, or fail outright, while a clean build passes --
# this happened for real (overlay1GetEntry's pilot conversion sits right next
# to this comment; runlink.c.o hit exactly this when another lane's re-split
# renamed func_8002B768). Depending on the stamp for real means *every*
# .c.o rebuilds after any re-split, not just the ones that reference a moved
# name -- coarser than necessary, but a full rebuild is ~17s and correctness
# beats precision here.
$(BUILD_DIR)/%.c.o: %.c $(H_FILES) $(TOOLS_DIR)/normalize_elf_instructions.py $(SPLAT_STAMP) | $(ALL_DIRS)
	$(ASM_PROCESSOR) $(CC) -- $(AS) $(ASM_PROC_ASFLAGS) -- \
		-c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<
	$(RUN_POSTPROCESS)

# The DKR-exact rmonprintf source has no GLOBAL_ASM. Sending it through
# asm-processor changes IDO's line metadata and produces a seven-word schedule
# residual; direct IDO compilation reproduces the whole source object.
$(BUILD_DIR)/$(SRC_DIR)/libultra/rmonprintf.c.o: $(SRC_DIR)/libultra/rmonprintf.c $(H_FILES) $(SPLAT_STAMP) | $(ALL_DIRS)
	$(CC) -c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<
	$(RUN_POSTPROCESS)

# __osEepStatus is the exact tail of DKR's SDK conteepwrite source. Like the
# direct rmonprintf object above, it has no GLOBAL_ASM and retains the donor's
# direct-IDO line schedule.
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepstatus.c.o: $(SRC_DIR)/libultra/eepstatus.c $(H_FILES) $(SPLAT_STAMP) | $(ALL_DIRS)
	$(CC) -c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<
	$(RUN_POSTPROCESS)

# DKR's whole xprintf object is exact, including its anonymous static helper,
# data and rodata. Compile the pragma-free donor directly at its SDK preset.
$(BUILD_DIR)/$(SRC_DIR)/libultra/xprintf.c.o: $(SRC_DIR)/libultra/xprintf.c $(H_FILES) $(SPLAT_STAMP) | $(ALL_DIRS)
	$(CC) -c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<
	$(RUN_POSTPROCESS)

# Same whole-object result for DKR's xldtob conversion source and constants.
$(BUILD_DIR)/$(SRC_DIR)/libultra/xldtob.c.o: $(SRC_DIR)/libultra/xldtob.c $(H_FILES) $(SPLAT_STAMP) | $(ALL_DIRS)
	$(CC) -c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<
	$(RUN_POSTPROCESS)

# ---------------------------------------------------------------------------
# Per-file compiler flags
#
# Deviations from the project defaults above. Each one must be forced by
# evidence (the file does not match otherwise) and explained in the source
# file's header comment, never guessed at.
# ---------------------------------------------------------------------------

# Mickey's object-system TU uses the R4300 multiply-hazard scheduler: target
# func_8000A62C reproduces three delay nops only with this measured flag.
# It does NOT carry -Wo,-loopunroll,0: that override was added for a
# NON_MATCHING candidate (func_80005548, commit 7adbef15), is byte-inert on
# every matched function in the TU (lane s2-a, 2026-09-16: only six
# GLOBAL_ASM-backed candidates change without it), and is refuted by
# func_8000590C, whose target copy loop is the unroller's own output (the
# remainder loop, the bypass of the `i != n` test on the no-remainder path
# and the 4x body) from a plain `while (i < n)` loop.
$(BUILD_DIR)/$(SRC_DIR)/main/objects.c.o: CFLAGS += -Wab,-r4300_mul

# The shadow TU needs the same scheduler. The target bytes of two of its
# unmatched functions carry the FP multiply-hazard nop between adjacent
# mul.s instructions that only this flag emits, and adding it reproduces
# that nop at the same instruction index without disturbing any of the
# four functions this TU already matches (gmake verify still passes).
$(BUILD_DIR)/$(SRC_DIR)/main/shadows.c.o: CFLAGS += -Wab,-r4300_mul

# Three more resident TUs on the same scheduler, kept on measurement rather
# than on the signature alone.
#
# tools/mul_scheduler_scan.py finds the nop-separated adjacent multiply pair
# in 51 of 51 TUs that have such a pair, which means the signature is
# necessary but does NOT discriminate -- it cannot tell a unit that needs the
# flag from one that does not. Setting it on the six resident TUs the scan
# named and re-measuring the queue split them evenly: fx.c -15 masked words,
# frontend_37D50.c -23 and block_506D0.c -27, against matrix.c +28,
# models_5B300.c +13 and spranim.c +8. The three that regressed are not set.
#
# What the three below buy is a class change, which is the part that matters:
# func_80048080 and func_80047304 (fx.c), func_80037BF4 (frontend_37D50.c)
# and func_8004FAD0 (block_506D0.c) all crossed size-mismatch into a class
# permutation can actually close.
#
# The safety property is the ROM: a flag disturbing any function these units
# already match would break the byte-identical rebuild. `gmake verify` passes.
$(BUILD_DIR)/$(SRC_DIR)/main/fx.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/main/frontend_37D50.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/main/block_506D0.c.o: CFLAGS += -Wab,-r4300_mul

# One overlay TU on the same evidence (2026-09-12, lane p9-tight). The flag
# buys the same class change here: overlay1FindType47ByAngle goes 18 masked
# words to 10 at size delta 0, and what it removes is the whole structural
# residual -- as1 stops hoisting the second float argument out of the
# unsigned-to-float conversion block, so every instruction lands on the
# target's offset and only two register decisions are left. The other two
# guarded functions in the TU are bit-for-bit unmoved at 3 and 12, and
# `gmake verify` passes, which is the safety property above.

# libultra's libc string TU needs branch-likely instructions (bnel/beql), which
# IDO only emits at -mips2; -mips1 produces a 0x90-byte .text instead of the
# ROM's 0xA0. Consistent with how the DKR decomp builds its libultra tree.
$(BUILD_DIR)/$(SRC_DIR)/libultra/string.c.o: MIPSISET := -mips2 -32

# These libultra TUs are built at -O1 -mips2, not the project default -O2 -mips1.
# Measured, not assumed: at -O2 IDO folds their stack frames away entirely and
# the .text comes out the wrong size, while -O1 -mips2 reproduces every one of
# them byte for byte.
#
# The list is NOT "libultra's io/ directory", despite most of it coming from
# there: getactivequeue is os/getactivequeue.c and needs the same flags. DKR
# happens to build its io/ tree at -O1 and its os/ tree at -O1 as well, so the
# per-directory framing would have been a coincidence rather than a rule -- the
# grouping here is simply "TUs measured to need -O1 -mips2", which is the only
# thing the evidence actually supports. Add a file when you have measured it.
#
# splat writes every libultra subsegment into one flat src/libultra/, so this
# cannot be a directory-scoped pattern rule and each file is named. Keep the
# list sorted.
LIBULTRA_O1_TUS := ai aigetlen aisetfreq aisetnextbuf controller contreaddata crc createmesgqueue destroythread \
                   dp dpsetnextbuf dpsetstat getactivequeue getthreadpri \
                   gettime jammesg pfsdeletefile recvmesg resetglobalintmask sendmesg \
                   seteventmesg setthreadpri settimer settime si siacs \
                   sirawdma sirawread sirawwrite sp sprawdma spgetstat \
                   spsetpc spsetstat sptask sptaskyield \
                   sptaskyielded startthread stopthread thread timerintr vi viblack \
                   vigetcurrcontext vigetcurrframebuf vigetnextframebuf \
                   visetevent visetmode visetspecial viswapbuf \
                   viswapcontext virtualtophysical yieldthread
$(foreach f,$(LIBULTRA_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: OPT_FLAGS := -O1))
$(foreach f,$(LIBULTRA_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: MIPSISET := -mips2 -32))

# osCreateThread is an SDK C TU whose target schedule is MIPS-II-specific:
# -mips1 leaves the cleanup-thread address materialization and epilogue in the
# wrong order, while -mips2 reproduces all 58 target instruction words.
$(BUILD_DIR)/$(SRC_DIR)/libultra/createthread.c.o: MIPSISET := -mips2 -32

# This old SDK source uses `__GNUC__` as a version-path selector even when IDO
# compiles it. JFG's matching object defines it for this TU only.
$(BUILD_DIR)/$(SRC_DIR)/libultra/destroythread.c.o: CFLAGS += -D__GNUC__

# -g3 -mips2: a THIRD libultra flag group, found in Phase 2 Task 3.
#
# WHAT IS MEASURED AND WHAT IS NOT -- read the two apart, because they are not
# the same strength of claim.
#
#   MEASURED: `-g3` and `-mips2` are both forced by Mickey's bytes. All twelve
#   combinations of {-O0,-O1,-O2} x {with,without -g3} x {-mips1,-mips2} were
#   compiled and compared against ROM 0x730A0 and 0x730F0. Exactly two produce
#   the ROM's 0x48-byte function: `-O1 -g3 -mips2` and `-O2 -g3 -mips2`. Drop
#   -g3 and the function is 0x44; drop -mips2 and it is 0x50 or 0x4C; -O0 gives
#   0x60. (-O3 could not be tested: this IDO recomp build dies in `uld` on any
#   -O3 invocation, an environment failure rather than a fact about the code.)
#
#   ALSO MEASURED, on a later TU: `-O2` rather than `-O1`. epiread and epiwrite
#   are byte-identical at both, so those two files never discriminated the
#   optimisation level and -O2 was borrowed from Jet Force Gemini's published
#   Makefile. `pidma` (ROM 0x6FB90) settles it from Mickey's own bytes: 48
#   instructions at -O2 -g3 -mips2, matching word for word, against 68 at
#   -O1 -g3 -mips2 and 78 at -O0.
#
# What -g3 changes, structurally, confirmed in disassembly: without it IDO
# hoists the third argument's spill (`sw a2,0x28(sp)`) into the first jal's
# delay slot and leaves that slot empty in the ROM's version, and it emits
# `lw ra` before `lw v0` in the epilogue. The ROM does neither.
#
# Reading a permitted public decompilation's build configuration is the same
# permission as reading its source (docs/CLEANROOM.md). The part of this that
# is a fact about MICKEY is the part that was measured here.
#
# Every TU in the list below was measured before it was added, one at a time.
# The list is not "libultra's io/ and pfs/ trees"; it is the set that has been
# measured, and nothing joins it on a neighbour's evidence.
LIBULTRA_O2_G3_TUS := contpfs contramread contramwrite devmgr epidma epilinkhandle epirawdma epirawread \
                      epirawwrite epiread epiwrite pfsallocatefile pfschecker \
                      initialize motor pfsfilestate pfsfreeblocks pfsgetstatus pfsinit \
                      pfsisplug pfsnumfiles pfsreadwritefile pfssearchfile \
                      pfsselectbank piacs pidma pigetcmdq pirawdma
$(foreach f,$(LIBULTRA_O2_G3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: OPT_FLAGS := -O2 -g3))
$(foreach f,$(LIBULTRA_O2_G3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: MIPSISET := -mips2 -32))

# JFG's whole initialize object matches Mickey's and selects the 2.0J source
# path plus Rare's external osViClock ownership.
$(BUILD_DIR)/$(SRC_DIR)/libultra/initialize.c.o: CFLAGS += -DBUILD_VERSION=7 -DRAREDIFFS
$(BUILD_DIR)/$(SRC_DIR)/libultra/controller.c.o: CFLAGS += -DRAREDIFFS
$(BUILD_DIR)/$(SRC_DIR)/libultra/contreaddata.c.o: CFLAGS += -DRAREDIFFS
$(BUILD_DIR)/$(SRC_DIR)/libultra/contramread.c.o: CFLAGS += -DBUILD_VERSION=7
$(BUILD_DIR)/$(SRC_DIR)/libultra/contramwrite.c.o: CFLAGS += -DBUILD_VERSION=7
$(BUILD_DIR)/$(SRC_DIR)/libultra/pfsdeletefile.c.o: CFLAGS += -DBUILD_VERSION=6 -DMICKEY_PFS_OLD_SIGNATURE -DRAREDIFFS
$(BUILD_DIR)/$(SRC_DIR)/libultra/motor.c.o: CFLAGS += -DBUILD_VERSION=7 -DJFGDIFFS

# The Transfer Pak bank-fill loop remains rolled only with the explicit uopt
# switch below. The default unroll pass grows this 0xD0 TU by 0x30 bytes.
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakselectbank.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakselectbank.c.o: CFLAGS += -Wo,-loopunroll,0

# Mickey's EEPROM long-read object uses Banjo-Kazooie's O1/MIPS II group
# with R4300 multiply-hazard scheduling and the older long-write delay idiom.
$(BUILD_DIR)/$(SRC_DIR)/libultra/eeplongread.c.o: OPT_FLAGS := -O1
$(BUILD_DIR)/$(SRC_DIR)/libultra/eeplongread.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/eeplongread.c.o: CFLAGS += -Wab,-r4300_mul

# Mickey's EEPROM read object uses the SDK O1/MIPS II group with R4300
# multiply-hazard scheduling.
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepread.c.o: OPT_FLAGS := -O1
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepread.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepread.c.o: CFLAGS += -Wab,-r4300_mul

# Mickey's EEPROM probe uses the O1/MIPS II object recipe and Banjo-Kazooie's
# VERSION_I selection; the later VERSION_L read-state reset is absent.
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepprobe.c.o: OPT_FLAGS := -O1
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepprobe.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepprobe.c.o: CFLAGS += -Wab,-r4300_mul -DBUILD_VERSION=6

# Mickey's Transfer Pak read/write TU contains the VERSION_K+ zero-size guard
# and uses the ordinary O2 MIPS II group with R4300 hazard scheduling.
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakreadwrite.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakreadwrite.c.o: CFLAGS += -Wab,-r4300_mul

# Mickey's Transfer Pak power TU uses the O2/MIPS II group with R4300 hazard
# scheduling and IDO's default loop-unroll mode.
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakpower.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakpower.c.o: CFLAGS += -Wab,-r4300_mul

# Mickey's Transfer Pak initialization TU uses the O2/MIPS II group with R4300
# hazard scheduling and IDO's default loop-unroll mode.
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakinit.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakinit.c.o: CFLAGS += -Wab,-r4300_mul

# Mickey's EEPROM long-write object uses Banjo-Kazooie's O1/MIPS II group
# with R4300 multiply-hazard scheduling.
$(BUILD_DIR)/$(SRC_DIR)/libultra/eeplongwrite.c.o: OPT_FLAGS := -O1
$(BUILD_DIR)/$(SRC_DIR)/libultra/eeplongwrite.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/eeplongwrite.c.o: CFLAGS += -Wab,-r4300_mul

# Mickey's EEPROM write object uses the SDK O1/MIPS II group with R4300
# multiply-hazard scheduling.
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepwrite.c.o: OPT_FLAGS := -O1
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepwrite.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepwrite.c.o: CFLAGS += -Wab,-r4300_mul

# Perfect Dark's matching Transfer Pak status object uses the default O2
# loop-unroll mode, unlike the rolled bank-selector TU immediately below.
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakgetstatus.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakgetstatus.c.o: CFLAGS += -Wab,-r4300_mul

# Perfect Dark's matching Transfer Pak connector-check object uses the rolled
# O2 loop group and the R4300 multiply-hazard scheduler.
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakcheckconnector.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakcheckconnector.c.o: CFLAGS += -Wab,-r4300_mul -Wo,-loopunroll,0

# Mickey's Transfer Pak ID reader contains the VERSION_K+ reset/retry path and
# uses the ordinary O2 MIPS II loop-unroll group with R4300 hazard scheduling.
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakreadid.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/gbpakreadid.c.o: CFLAGS += -Wab,-r4300_mul

# -Xphase,uopt,+ -Xphase,uopt,-O1: a FOURTH libultra flag group, and the only
# one that does not go through the `cc` driver.
#
# WHY A WRAPPER AT ALL. `tools/ido/cc` runs `uopt` only at -O2 and above --
# checked at -O1 x {-g0,-g1,-g2,-g3,none}, where `cc -v` never lists a uopt
# stage. So the configuration "uopt ran, at -O1" cannot be produced by the
# driver. It cannot be faked with -Wo,-O1 either: -W<pass>,-O<n> is inserted
# BEFORE the driver's own -O and the last -O wins. (ISA options are appended
# after, which is why -Wc,-mips3 below does work and -Wo,-O1 does not.)
#
# ido-phases.py drives cfe/uopt/ugen/as1 itself, taking each phase's command
# line from `cc -v` rather than reimplementing the driver's flag translation,
# so it cannot drift from it. With no -Xphase, option it is byte-identical to
# `cc`: verified on string.c (-O2 -mips2), epidma.c (-O2 -g3 -mips2), si.c
# (-O1 -mips2, where the driver skips uopt), pfsreadwritefile.c (-O2 -g3
# -mips2), and on a GLOBAL_ASM TU under asm-processor.
#
# MEASURED, on setglobalintmask: __osSetGlobalIntMask is byte-identical to ROM
# 0x75080 once uopt has run at -O1 -- 19 of 19 words, with only the six the
# linker fills in differing. Without the uopt stage the same source gives 20
# instructions with the first jal's delay slot empty, which is what the file's
# header used to describe. -g3 is NOT needed (identical with and without);
# -mips2 IS (-mips1 gives 9 of 19); -O2 destroys it (the frame drops to 0x40).
#
# HOST_PYTHON, not $(PYTHON): the wrapper is stdlib-only, and using the venv
# would make every C object depend on `gmake setup`.
IDO_PHASES := $(HOST_PYTHON) $(TOOLS_DIR)/ido-phases.py

LIBULTRA_UOPT_O1_TUS := setglobalintmask
$(foreach f,$(LIBULTRA_UOPT_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: CC := $(IDO_PHASES)))
$(foreach f,$(LIBULTRA_UOPT_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: OPT_FLAGS := -O1))
$(foreach f,$(LIBULTRA_UOPT_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: MIPSISET := -mips2 -32))
$(foreach f,$(LIBULTRA_UOPT_O1_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: CFLAGS += -Xphase,uopt,+ -Xphase,uopt,-O1))

# libultra's compiler-runtime 64-bit helpers use MIPS III instructions and
# were built at -O1. JFG's whole `libc/ll.c` object fixes both choices against
# Mickey's bytes; this is not a directory-wide libc assumption.
$(BUILD_DIR)/$(SRC_DIR)/libultra/ll.c.o: $(TOOLS_DIR)/set_elf_flags.py
$(BUILD_DIR)/$(SRC_DIR)/libultra/ll.c.o: OPT_FLAGS := -O1
$(BUILD_DIR)/$(SRC_DIR)/libultra/ll.c.o: MIPSISET := -mips3 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/ll.c.o: POSTPROCESS = $(HOST_PYTHON) \
	$(TOOLS_DIR)/set_elf_flags.py $@ 0x10000000

# Preserve the extracted fallback's endlabel: its symbol spans 4 bytes,
# followed by 60 bytes outside the label in the 0x40-byte block. asm-processor
# incorrectly assigns the entire block to the first glabel. Restore only
# st_size; keep all section bytes and the NON_MATCHING verdict unchanged.
# This placeholder boundary is not evidence of a four-byte C function.
$(BUILD_DIR)/$(SRC_DIR)/libultra/block_6F3E0.c.o: $(TOOLS_DIR)/set_elf_symbol_size.py
$(BUILD_DIR)/$(SRC_DIR)/libultra/block_6F3E0.c.o: POSTPROCESS = $(HOST_PYTHON) \
	$(TOOLS_DIR)/set_elf_symbol_size.py $@ func_8006E7E0 0x40 0x4

# --- n_audio flag group (lane/naudio) -------------------------------------
# The n_audio synthesis library (ROM 0x5E6B0-0x6ACF0, docs/modules.md 4.2) was
# built unoptimised with debug codegen: bare `OPT_FLAGS := -g` (no -O at all,
# distinct from -O0), `-mips2 -32`. Verified byte-exact per TU as each is
# matched; see docs/acceleration-survey.md 13.3 for the ruling that adopted
# JFG's n_audio bodies. Add a TU's object name to this list only once its
# compiled bytes have been checked against the ROM.
LIBULTRA_NAUDIO_BARE_TUS := n_cspsetvol n_csplayer n_cspgetstate n_cspmessage slHeap sl \
	n_cseq n_cseqnextdelta n_synsetpriority n_cspsetchlvol n_cspsetseq n_cspplay \
	n_cspstop n_cspsendmidi n_sl n_syndelete n_synsetpan n_synsetpitch \
	n_synsetfxmix n_synstopvoice n_synfreevoice n_alsynsetlpffreq n_alsynsetlpfgain \
	n_alsynsetdistort n_synsetfxparam n_synallocfx n_reverb n_seqplayer n_resample \
	n_alcspchan n_syngetfxref n_synsetvol n_synstartvoiceparam n_synaddplayer \
	n_synallocvoice alsurround n_mainbus n_auxbus n_event n_load n_alLPFilter \
	n_drvrNew n_synthesizer n_env
# n_alcspchan uses the Rare-added MIDI control-change codes (AL_MIDI_UNK_FC,
# AL_MIDI_FADEEND_CTRL, AL_MIDI_FADESTART_CTRL), guarded by RAREDIFFS like the
# other Rare-diffed libultra TUs above.
$(BUILD_DIR)/$(SRC_DIR)/libultra/n_alcspchan.c.o: CFLAGS += -DRAREDIFFS
# JFG applies Rare's extended MIDI controller definitions and the R4300
# multiply-hazard pass globally; n_csplayer is the other Mickey TU whose
# compiled text proves both are required.
$(BUILD_DIR)/$(SRC_DIR)/libultra/n_csplayer.c.o: CFLAGS += -DRAREDIFFS -Wab,-r4300_mul
# The reverb sources select the naudio microcode command layout explicitly;
# its final multiply also needs the R4300 hazard scheduling pass.
$(BUILD_DIR)/$(SRC_DIR)/libultra/n_reverb.c.o: CFLAGS += -DN_MICRO -Wab,-r4300_mul
# The driver configuration stores Rare's per-bus effect arrays.
$(BUILD_DIR)/$(SRC_DIR)/libultra/n_drvrNew.c.o: CFLAGS += -DRAREDIFFS -Wab,-r4300_mul
# The synthesizer uses the same per-bus layout and the n_audio microcode ABI.
$(BUILD_DIR)/$(SRC_DIR)/libultra/n_synthesizer.c.o: CFLAGS += -DRAREDIFFS -DN_MICRO
# The envelope mixer emits n_audio microcode commands; its rate helper needs
# the R4300 multiply scheduler's hazard spacing.
$(BUILD_DIR)/$(SRC_DIR)/libultra/n_env.c.o: CFLAGS += -DN_MICRO -Wab,-r4300_mul
# The resampler uses the naudio microcode command encoding found in Mickey.
$(BUILD_DIR)/$(SRC_DIR)/libultra/n_resample.c.o: CFLAGS += -DN_MICRO
# The N64DD/mobile microcode branches in the SDK source do not describe the
# n_audio ABI used by Mickey's decoder object.
$(BUILD_DIR)/$(SRC_DIR)/libultra/n_load.c.o: CFLAGS += -DN_MICRO
$(foreach f,$(LIBULTRA_NAUDIO_BARE_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: OPT_FLAGS := -g))
$(foreach f,$(LIBULTRA_NAUDIO_BARE_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: MIPSISET := -mips2 -32))
# --- end n_audio flag group -------------------------------------------------

# Remaining SDK C groups measured against the pinned JFG objects.
$(BUILD_DIR)/$(SRC_DIR)/libultra/cents2ratio.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/cents2ratio.c.o: OPT_FLAGS := -g
$(BUILD_DIR)/$(SRC_DIR)/libultra/sinf.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/sinf.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/libultra/vimgr.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/vimgr.c.o: OPT_FLAGS := -O2
# osFlashClearStatus ends at the measured 0x4C-byte split boundary; IDO adds
# one zero instruction solely to align its standalone .text section to 0x10.
$(BUILD_DIR)/$(SRC_DIR)/libultra/osFlashClearStatus.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x4C
# The five-function flash block ends at the measured 0x3B4-byte boundary;
# discard IDO's trailing section-alignment words before the next subsegment.
$(BUILD_DIR)/$(SRC_DIR)/main/flash_5885C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3B4
# The compiled five-function body plus IDO's natural trailing alignment ends
# at 0x3B0; the final four target bytes are linker padding before the next
# subsegment.
$(BUILD_DIR)/$(SRC_DIR)/main/flash_5885C.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x3B0
$(BUILD_DIR)/$(SRC_DIR)/libultra/aisetnextbuf.c.o: CFLAGS += -DRAREDIFFS
$(BUILD_DIR)/$(SRC_DIR)/libultra/sptask.c.o: CFLAGS += -DRAREDIFFS
$(BUILD_DIR)/$(SRC_DIR)/libultra/vi.c.o: CFLAGS += -DRAREDIFFS

LIBULTRA_O3_TUS := xlitob
$(foreach f,$(LIBULTRA_O3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: CC := $(IDO_PHASES)))
$(foreach f,$(LIBULTRA_O3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: OPT_FLAGS := -O2))
$(foreach f,$(LIBULTRA_O3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: MIPSISET := -mips2 -32))
$(foreach f,$(LIBULTRA_O3_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/libultra/$(f).c.o: CFLAGS += -Xphase,cfe,-O3 \
	-Xphase,uopt,-O3 -Xphase,ugen,-O3 -Xphase,as1,-O3))

# IDO's driver cannot complete an -O3 build in this environment, but the phase
# wrapper can drive the same cfe/uopt/ugen/as1 pipeline directly.  The base -O2
# invocation supplies all four stages, then each phase is promoted to the -O3
# pass that produced libultra's ldiv object (including its function ordering).
$(BUILD_DIR)/$(SRC_DIR)/libultra/ldiv.c.o: CC := $(IDO_PHASES)
$(BUILD_DIR)/$(SRC_DIR)/libultra/ldiv.c.o: OPT_FLAGS := -O2
$(BUILD_DIR)/$(SRC_DIR)/libultra/ldiv.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/ldiv.c.o: CFLAGS += -Xphase,cfe,-O3 \
	-Xphase,uopt,-O3 -Xphase,ugen,-O3 -Xphase,as1,-O3

# GAME code (as opposed to libultra) is -mips2, not the project default -mips1.
# Measured on the first game TU decompiled, main/runlink.c: the ROM's code does
# `lw t7,0(a3)` immediately followed by `addu v1,v1,t7`, i.e. it uses the loaded
# register in the very next instruction. That is only legal without a nop from
# -mips2 onwards; at -mips1 IDO's assembler inserts a load-delay nop after every
# such pair, and main/runlink.c came out 0x14 bytes long over five functions
# before this line existed.
#
# Scoped to src/main/ rather than made the global default so the libultra
# findings above stay untouched and so the claim stays exactly as wide as the
# evidence. Widen it when the next game module is measured, not before.
$(BUILD_DIR)/$(SRC_DIR)/main/%.c.o: MIPSISET := -mips2 -32
# DKR's libultra libc build supplies the exact rmonprintf object and uses
# mips2 for this source family; mips1 changes its late instruction schedule.
$(BUILD_DIR)/$(SRC_DIR)/libultra/rmonprintf.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/rmonprintf.c.o: CFLAGS := -G 0 -non_shared -verbose \
	-Xcpluscomm -nostdinc -Wab,-r4300_mul $(DEFINES) $(INCLUDE_CFLAGS) -w
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepstatus.c.o: OPT_FLAGS := -O1
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepstatus.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/eepstatus.c.o: CFLAGS := -G 0 -non_shared -verbose \
	-Xcpluscomm -nostdinc -Wab,-r4300_mul $(DEFINES) $(INCLUDE_CFLAGS) -w
$(BUILD_DIR)/$(SRC_DIR)/libultra/xprintf.c.o: OPT_FLAGS := -O3
$(BUILD_DIR)/$(SRC_DIR)/libultra/xprintf.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/xprintf.c.o: CFLAGS := -G 0 -non_shared -verbose \
	-Xcpluscomm -nostdinc -Wab,-r4300_mul $(DEFINES) $(INCLUDE_CFLAGS) -w
$(BUILD_DIR)/$(SRC_DIR)/libultra/xldtob.c.o: OPT_FLAGS := -O3
$(BUILD_DIR)/$(SRC_DIR)/libultra/xldtob.c.o: MIPSISET := -mips2 -32
$(BUILD_DIR)/$(SRC_DIR)/libultra/xldtob.c.o: CFLAGS := -G 0 -non_shared -verbose \
	-Xcpluscomm -nostdinc -Wab,-r4300_mul $(DEFINES) $(INCLUDE_CFLAGS) -w

# IDO's `-dollar` extension is required for the named stack-register source;
# invoke the compiler directly because asm-processor does not expose it.
$(BUILD_DIR)/$(SRC_DIR)/main/get_stack_pointer.c.o: OPT_FLAGS := -dollar
$(BUILD_DIR)/$(SRC_DIR)/main/get_stack_pointer.c.o: MIPSISET := -mips1 -32
$(BUILD_DIR)/$(SRC_DIR)/main/get_stack_pointer.c.o: CFLAGS += -Wab,-r4300_mul
$(BUILD_DIR)/$(SRC_DIR)/main/get_stack_pointer.c.o: $(SRC_DIR)/main/get_stack_pointer.c $(H_FILES) | $(ALL_DIRS)
	$(TOOLS_DIR)/ido/cc -c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<
# This nine-instruction accessor ends at the measured 0x24-byte boundary;
# discard only IDO's three trailing section-alignment words.
$(BUILD_DIR)/$(SRC_DIR)/main/amAudioMgrSetScheduleMode.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x24
# texEnableModes ends at the measured 0x1C-byte boundary; discard only IDO's
# trailing section-alignment word.
$(BUILD_DIR)/$(SRC_DIR)/main/texEnableModes.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1C
# texLoadTextureAddr ends at the measured 0x28-byte boundary; discard only
# IDO's two trailing section-alignment words.
$(BUILD_DIR)/$(SRC_DIR)/main/texLoadTextureAddr.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x28
# These three C subsegments are 4-byte-aligned resident slices inside the
# original textures translation unit. IDO rounds each standalone .text
# section to 0x10, so discard only the section padding after the exact slice.
$(BUILD_DIR)/$(SRC_DIR)/main/textures_34E60.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x1A8
$(BUILD_DIR)/$(SRC_DIR)/main/textures_35024.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x47C
$(BUILD_DIR)/$(SRC_DIR)/main/textures_354C8.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0x2188
# The audiomgr block ends at the measured 0xD7C boundary; discard only IDO's
# trailing section-alignment word before the following 0x30BC TU.
$(BUILD_DIR)/$(SRC_DIR)/main/audiomgr.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .text 0xD7C
# Mickey's three maths objects are byte-identical to JFG's matching objects,
# whose per-directory rule uses bare `-g` (no optimisation flag). The -O2
# game default changes atan2f from 0x1F4 to 0x134 bytes, so keep this override
# limited to the three measured TUs.
MAIN_MATH_BARE_TUS := math_atan math_acosf math_arc
$(foreach f,$(MAIN_MATH_BARE_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/main/$(f).c.o: OPT_FLAGS := -g))
$(foreach f,$(MAIN_MATH_BARE_TUS),$(eval \
	$(BUILD_DIR)/$(SRC_DIR)/main/$(f).c.o: CFLAGS += -Wab,-r4300_mul))
# The reconstructed resident main loop reproduces its target frame with uopt capped.
$(BUILD_DIR)/$(SRC_DIR)/main/main.c.o: CFLAGS += -Wo,-Olimit,100
ifeq ($(NON_MATCHING),1)
MAIN_THREAD_TEXT_SIZE := 0x2B10
MAIN_THREAD_TEXT_SHA256 := ba62e894f3b04c9359aea1b9806045208745c9321dd76e52dd59283234d88c3c
else
MAIN_THREAD_TEXT_SIZE := 0x2AF0
MAIN_THREAD_TEXT_SHA256 := d6a4eb4e0e95dfbbf5944ed336e6dda98f930939d0f0dbf69d2e07537cb2c306
endif
# Keep the literal RAM-end expression's instruction words intact, then add
# the two target relocation records using a digest-guarded metadata pass. The
# source already carries the linked HI16/LO16 addends, so the temporary
# relocation carrier is zero-valued; the linker script retains the final
# D_803FFFFC absolute symbol value. Rename it only after the pass, before
# folding the rain callback's typed alias back to the shipped carrier.
$(BUILD_DIR)/$(SRC_DIR)/main/main.c.o: $(TOOLS_DIR)/add_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/main/main.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol mainThreadRamEndAnchor=0x0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/add_elf_relocations.py $@ .text \
		$(MAIN_THREAD_TEXT_SIZE) $(MAIN_THREAD_TEXT_SHA256) \
		0x120:HI16:mainThreadRamEndAnchor:0x8040 \
		0x130:LO16:mainThreadRamEndAnchor:0xFFFC && \
	$(OBJCOPY) --redefine-sym mainThreadRamEndAnchor=D_803FFFFC \
		--redefine-sym mainCPUeffectsRainDraw=TrapDanglingJump $@
# joyInit's enabled-pad bytes are one contiguous array for IDO's exact loop
# schedule, while the retail relocation table names the adjacent byte labels.
# Add global aliases at their actual .bss offsets and local relocation carriers
# at the compiler's array base. Rebind existing records to the carriers, then
# rename them to the retail labels; no instruction or data byte is changed.
$(BUILD_DIR)/$(SRC_DIR)/main/joy.c.o: config/normalizations/joyInit.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/main/joy.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol D_800CF3B5=.bss:0x5,global,object \
		--add-symbol D_800CF3B6=.bss:0x6,global,object \
		--add-symbol D_800CF3B7=.bss:0x7,global,object \
		--add-symbol joyInitRelocBC=.bss:0x8,local,object \
		--add-symbol joyInitRelocB5=.bss:0x4,local,object \
		--add-symbol joyInitRelocB6=.bss:0x4,local,object \
		--add-symbol joyInitRelocB7=.bss:0x4,local,object $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/joyInit.rebind.spec && \
	$(OBJCOPY) --redefine-sym joyInitRelocBC=D_800CF3BC \
		--redefine-sym joyInitRelocB5=D_800CF3B5 \
		--redefine-sym joyInitRelocB6=D_800CF3B6 \
		--redefine-sym joyInitRelocB7=D_800CF3B7 $@
# The resident formatter's integer multiply/divide schedule uses R4300 timing.
$(BUILD_DIR)/$(SRC_DIR)/main/diprint.c.o: CFLAGS += -Wab,-r4300_mul
# The scheduler TU owns osScGetTaskType's table and __scSchedule's table;
# IDO's trailing four zero bytes follow the combined 0x38-byte input section.
# The initialized retrace counter owns eight data bytes; the remaining eight
# are IDO's section alignment padding, not another variable.
$(BUILD_DIR)/$(SRC_DIR)/main/sched.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .rodata 0x38 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .data 0x8
# objects owns five switch tables and three literal-pool floats in an exact
# aligned input section. The three default branches are already resolved:
# site-bound PC16 records preserve their fields
# and the exact relocation identities of the assembled fallback functions.
# Name the input rodata base so table identities remain unambiguous in the ELF.
# func_80009220's compiler-owned float literal uses the same proved input base.
$(BUILD_DIR)/$(SRC_DIR)/main/objects.c.o: $(TOOLS_DIR)/add_elf_relocations.py \
    $(TOOLS_DIR)/trim_elf_section.py $(TOOLS_DIR)/rebind_elf_relocations.py
$(BUILD_DIR)/$(SRC_DIR)/main/objects.c.o: POSTPROCESS = \
	$(OBJCOPY) --add-symbol objectsSizeDefaultBranch=.text:0x6500,local \
	    --add-symbol objectsInitDefaultBranch=.text:0x6718,local \
	    --add-symbol objectsControlDefaultBranch=.text:0x6C00,local \
	    --add-symbol objectsSwitchTablesBase=.rodata:0,global $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .rodata 0x690 && \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
	    0x4F94:.rodata:objectsSwitchTablesBase \
	    0x4F9C:.rodata:objectsSwitchTablesBase \
	    0x6508:.rodata:objectsSwitchTablesBase \
	    0x6510:.rodata:objectsSwitchTablesBase \
	    0x6720:.rodata:objectsSwitchTablesBase \
	    0x6728:.rodata:objectsSwitchTablesBase \
	    0x6C08:.rodata:objectsSwitchTablesBase \
	    0x6C10:.rodata:objectsSwitchTablesBase && \
	$(HOST_PYTHON) $(TOOLS_DIR)/add_elf_relocations.py $@ .text 0x708C \
	    04dcd22184d1b53507977efc4868c715fea7ba6a86fa32c29b086d53cb076668 \
	    0x6500:PC16:objectsSizeDefaultBranch:0x76 \
	    0x6718:PC16:objectsInitDefaultBranch:0x120 \
	    0x6C00:PC16:objectsControlDefaultBranch:0x114
# JFG's source-level string migration reproduces diRcp's complete diagnostic
# string block followed by the 0x100-byte switch-table span. The following
# four zero bytes are output-section padding.
$(BUILD_DIR)/$(SRC_DIR)/main/diRcp.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .rodata 0xC3C
# func_80038BC4 owns nineteen table words; the next menu table starts immediately.
$(BUILD_DIR)/$(SRC_DIR)/main/menu.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .rodata 0x4C
# Both measured FP helpers in this TU require the R4300 multiply schedule.
$(BUILD_DIR)/$(SRC_DIR)/main/lights.c.o: CFLAGS += -Wab,-r4300_mul
# IDO rounds the five-entry switch table's 0x14-byte input section to 0x20;
# discard only that trailing input-section padding before the linker lays out
# the following shared resident rodata.
$(BUILD_DIR)/$(SRC_DIR)/main/lights.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .rodata 0x14
# The camera projection-depth dot product requires the R4300 multiply schedule.
$(BUILD_DIR)/$(SRC_DIR)/main/camera.c.o: CFLAGS += -Wab,-r4300_mul

# The general-particle velocity magnitudes require the R4300 multiply schedule.
$(BUILD_DIR)/$(SRC_DIR)/main/particles.c.o: CFLAGS += -Wab,-r4300_mul
# The matched switch table and the following particle constants own 0x28
# bytes; discard only IDO's trailing zero section alignment.
$(BUILD_DIR)/$(SRC_DIR)/main/particles.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .rodata 0x28

# The vehicle logarithm-series helper needs the R4300 multiply-hazard pass.
$(BUILD_DIR)/$(SRC_DIR)/main/vehicle_sounds.c.o: CFLAGS += -Wab,-r4300_mul

# The track plane builder proves the resident TU's R4300 FP hazard schedule.
$(BUILD_DIR)/$(SRC_DIR)/main/track.c.o: CFLAGS += -Wab,-r4300_mul

# func_8000D018's camera-position dangling call needs a typed alias to pass its
# three f32 args single-precision (the unprototyped TrapDanglingJump promotes
# them to double). Canonicalize only the undefined symbol name to the shared
# TrapDanglingJump target (0x800333A0); section contents are unchanged.
$(BUILD_DIR)/$(SRC_DIR)/main/track.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym trackCamPosTrap=TrapDanglingJump $@

# levelInit's typed weak aliases preserve the seven runtime-loaded ABIs while
# IDO emits their calls against the shared TrapDanglingJump identity directly.
# No postprocess is needed.

# The gsSnd flag lattice reproduces its debug-shaped epilogues only with bare -g.
$(BUILD_DIR)/$(SRC_DIR)/main/gsSnd.c.o: OPT_FLAGS := -g

# The animation-cache source requires a clean current full-TU baseline under
# this override; retained isolated C omitted it, and historical lattice outputs
# are not retained.
$(BUILD_DIR)/$(SRC_DIR)/main/models_5B300.c.o: CFLAGS += -Wo,-loopunroll,0

# The resident animation TU's reset loops use IDO's non-unrolled form. The
# canonical setting otherwise expands the 0x40-byte light-record reset by four;
# the flag lattice selects this setting before any source permutation.
# -Wab,-r4300_mul is the same lattice's selection for func_800573C8: with it
# the configured full-TU object is instruction-word identical to the target
# over all 233 words, and without it two scheduler-tied words at +0xDC differ
# under every source spelling tried. Impact review: the whole ROM still
# rebuilds byte-identically, so no already-matched function in this TU moves.
$(BUILD_DIR)/$(SRC_DIR)/main/anim.c.o: CFLAGS += -Wo,-loopunroll,0 -Wab,-r4300_mul
# The path reset trap needs a typed alias to preserve its f32 argument.
# Canonicalize only the undefined symbol name; section contents are unchanged.
# func_800508D4's 0.01f literal owns one word of the anim literal pool; the
# rest of IDO's 0x10-byte input section is alignment padding, and the still
# anonymous pool (0.02f onward) begins immediately after it.
$(BUILD_DIR)/$(SRC_DIR)/main/anim.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym animResetTrap=TrapDanglingJump $@ && \
	$(OBJCOPY) --redefine-sym hitCopyFirstTrap=TrapDanglingJump $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .rodata 0x4

# menu.c compiles at the plain game-code preset. It once carried
# -Wo,-loopunroll,0 "for func_80038878"; that flag was measured byte-inert for
# every other function in the TU and wrong for that one, whose target unrolls
# both the pointer-table clear (4 stores per iteration) and the play-choice
# reset (fully). Its matched body needs the default unroller.
#
# The adjacent menu tail, menu_3B1A0.c, carried the same flag "for the scalar
# record-reset loop". That was never a measurement: the loop in question lives
# in func_8003A754, which was and still is under GLOBAL_ASM, so the flag could
# only ever reach the three tiny loop-free bodies the TU actually compiles,
# where it is byte-inert. func_8003A7D0's target, by contrast, unrolls its
# accumulation loop four ways, which the flag made unreachable from any C at
# all. Removing it matched that function exactly and moved nothing else.
# func_80038750's five-entry language jump table (0x14) precedes three
# consecutive 0x4C-byte switch tables (func_800389CC, func_80038BC4,
# func_80038E1C's mode dispatch at 0x800827E0); the ROM pads that 0xF8 by one
# zero word before the next 16-aligned float table, and IDO rounds the input
# section up to 0x100, so keep 0xFC and discard only the rest of the
# input-section padding.  The array-shaped aliases stay external to
# IDO so func_80039720 retains its target induction-pointer allocation; bind
# their metadata back to the individually owned BSS labels before linking.
# D_800D3044 (the four play-choice bytes) is DEFINED in menu.c because
# func_80038878 stores all four through one `lui $at`, which IDO emits only
# for an object the TU owns. IDO lays it at .bss+0x2C inside the 0x30-byte
# carve, so the section keeps its size, but the retail link placed the object
# at 0x800D3044, which asm/data's bss gap still labels. Weakening menu.c's
# definition lets the gap's label win; the symbol-relative relocations then
# resolve to the retail address and the linked words match.
$(BUILD_DIR)/$(SRC_DIR)/main/menu.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym menuRepeatX=D_800D3198 \
	--redefine-sym menuRepeatY=D_800D319C \
	--redefine-sym menuPreviousButtons=D_800D31A0 \
	--weaken-symbol=D_800D3044 $@ && \
	$(HOST_PYTHON) $(TOOLS_DIR)/trim_elf_section.py $@ .rodata 0xFC

# The saves slot-reset loop is scalar in the target; the 119-combination flag
# lattice otherwise expands four 0x20-byte records into each loop iteration.
$(BUILD_DIR)/$(SRC_DIR)/main/saves.c.o: CFLAGS += -Wo,-loopunroll,0

# Preserve the SDK end-label spelling that gives IDO the shipped address web,
# then restore the target's equivalent Fast3D-start relocation identity.
$(BUILD_DIR)/$(SRC_DIR)/main/rcpFast3d.c.o: \
	config/normalizations/rcpFast3d.rebind.spec
$(BUILD_DIR)/$(SRC_DIR)/main/rcpFast3d.c.o: POSTPROCESS = \
	$(HOST_PYTHON) $(TOOLS_DIR)/rebind_elf_relocations.py $@ .text \
		@config/normalizations/rcpFast3d.rebind.spec

# The charControl target carries the R4300 multiply scheduling nops around its
# single-precision smoothing helpers; the flag lattice isolates this assembler
# mode without changing the resident TU's O2/MIPS-II compiler output.
$(BUILD_DIR)/$(SRC_DIR)/main/charControl.c.o: CFLAGS += -Wab,-r4300_mul
# func_8001C4C0's effect-spawn dangling call needs its observed twelve-argument
# prototype without changing the other shared TrapDanglingJump call sites.
# Canonicalize only the undefined alias name; section contents are unchanged.
$(BUILD_DIR)/$(SRC_DIR)/main/charControl.c.o: POSTPROCESS = \
	$(OBJCOPY) --redefine-sym charControlEffectSpawnTrap=TrapDanglingJump $@

# The positional-audio distance loops retain the R4300 multiply schedule;
# the full flag lattice selects this mode for amPlayAudioMap.
$(BUILD_DIR)/$(SRC_DIR)/main/audio_manager_36D0.c.o: CFLAGS += -Wab,-r4300_mul
ifeq ($(NON_MATCHING),1)
# The candidate's update-entry scan is scalar in the target; retain IDO's
# rolled loop without changing the verified canonical TU flags.
$(BUILD_DIR)/$(SRC_DIR)/main/audio_manager_36D0.c.o: CFLAGS += -Wo,-loopunroll,0
endif

# The oscillator TU uses the VR4300 multiply scheduling mode. The exact BK
# depth2Cents body reaches Mickey's instruction schedule only with this flag;
# the flag lattice leaves canonical -O2/-mips2 otherwise unchanged.
$(BUILD_DIR)/$(SRC_DIR)/main/audio_manager_4C50.c.o: CFLAGS += -Wab,-r4300_mul

# ---------------------------------------------------------------------------
# Overlay object policy is kept in one dedicated include so this root graph
# remains readable: one source/asset graph, one final link, one ROM. The
# include contains only target-specific compiler and ELF-metadata settings;
# runtime overlay loading remains game code in src/main/runlink.c.
include mk/overlays.mk

# One final link builds the complete ROM image. For overlays the dependency
# flow is:
#
#   config/overlays.us.json -> generated block in mickey.us.yaml
#   -> splat C/asm/bin inputs under */overlays/oNNN/
#   -> the ordinary compile/assemble/binwrap rules -> O_FILES
#   -> splat's mickey.us.ld places every module range -> this ELF
#
# src/main/runlink.c is Mickey's runtime loader reconstructed as game code; it
# does not participate in this host-side build graph. JFG is evidence for parts
# of that runtime lineage, not the source of a separate overlay Make pipeline.
$(TARGET).elf: $(O_FILES) $(LD_SCRIPT) overlay_undefined_syms.$(VERSION).txt | $(ALL_DIRS) $(SPLAT_STAMP)
	$(TOOLS_DIR)/link_diagnose.sh $(LD) $(LDFLAGS) -o $@

# symbol_addrs is a real input to the split, not just documentation: naming a
# function there changes the labels in the generated .s and, for `c`
# subsegments, the *filenames* under asm/nonmatchings/ that the source's
# GLOBAL_ASM pragmas point at. Leaving it off this list meant adding a symbol
# name did not re-split, and the build failed with a missing nonmatchings file.
#
# Order-only prereq on $(PYTHON) so the split never runs against a nonexistent
# venv; `setup` is what actually installs splat into it.
$(SPLAT_STAMP): $(BASENAME).$(VERSION).yaml symbol_addrs.$(VERSION).txt \
                config/overlays.$(VERSION).json $(TOOLS_DIR)/overlay_atlas.py \
                $(TOOLS_DIR)/overlay_tables.py requirements.txt | $(ALL_DIRS) $(PYTHON)
	$(HOST_PYTHON) $(TOOLS_DIR)/overlay_atlas.py --check
	$(PYTHON) -m splat split $(BASENAME).$(VERSION).yaml
	@$(MAKE) --no-print-directory prune-asm
	@touch $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@

# n64crc is expected to be a no-op: every byte, CRC words included, came out of
# the baserom via splat. If it rewrites the header the build is wrong upstream,
# so say so loudly instead of silently "fixing" it.
$(TARGET).z64: $(TARGET).bin $(CRC)
	@cp $(TARGET).bin $@
	@before=$$($(SHA1) $@ | cut -d' ' -f1); \
	$(CRC) $@ >/dev/null; \
	after=$$($(SHA1) $@ | cut -d' ' -f1); \
	if [ "$$before" != "$$after" ]; then \
		echo "ERROR: n64crc rewrote the CRC words ($$before -> $$after);"; \
		echo "       the linked image did not reproduce the ROM's own checksums."; \
		echo "       This means the build is wrong upstream -- failing loudly"; \
		echo "       instead of silently shipping a 'fixed' ROM."; \
		exit 1; \
	else \
		echo "n64crc: checksums already correct (no-op)"; \
	fi
	@ls -l $@

.PHONY: default all setup hooks extract prune-asm verify cleanroom system-health check-tooling forced-floor-census promotion-proof release-gate public-release audit-decoders overlay-tables overlay-atlas overlay-atlas-write overlay-syms check-overlay-syms overlay-donors overlay-donors-write overlay-donors-scan-check check-fixtures check-docs reference-builds check-reference-builds progress scoreboard check-scoreboard clean distclean
.SECONDARY:
SHELL = /bin/bash -e -o pipefail

# Every candidate-bearing TU must still compile with -DNON_MATCHING, or its
# candidates silently drop out of the permuter sweep (fx.c, overlay 1 and
# overlay 8 were locked out this way for days). Compile-only: the
# build_non_matching tree skips the matching-only ELF normalizations.
check-nonmatching-builds:
	@fail=0; for f in $$(grep -l '#ifdef NON_MATCHING' -r src --include='*.c'); do \
	  if ! $(MAKE) -s NON_MATCHING=1 build_non_matching/$$f.o >/dev/null 2>&1; then echo "FAIL $$f"; fail=1; fi; \
	done; [ $$fail -eq 0 ] && echo "check-nonmatching-builds: OK ($$(grep -l '#ifdef NON_MATCHING' -r src --include='*.c' | wc -l | tr -d ' ') TUs)" || exit 1
