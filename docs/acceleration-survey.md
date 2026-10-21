# Acceleration survey: where the big strides are

## Status, 2026-08-24 evening

This survey's ten findings drove same-day action; this note records what
was actioned and what is still open, so the findings below read as history,
not as an open task list. Every number here is quoted from `README.md`'s
generated Progress block (`gmake scoreboard`/`gmake check-scoreboard`) or
counted directly from the tree in this lane, not carried forward — see the
file-level note below on why this file itself stays a snapshot.

**Actioned:**

- **Finding 1** (instruction-editing inflated the matched count) became
  [ADR 0001](adr/0001-matching-standard.md) (matching standard, the
  objdiff/DKR byte-identity definition) and
  [ADR 0002](adr/0002-no-post-compile-instruction-editing.md) (no
  post-compile instruction editing). `tools/postprocess_audit.py`
  classifies every object's `POSTPROCESS` step; `config/postprocess-audit.us.json`
  records the result (`summary.by_class`), `tools/progress.py` withdraws the
  credit of any `altered` object, and `gmake check-docs` fails while a
  credited one exists — so the conversion is held for every object, not just
  the ones this lane's prose describes. The 274 affected
  functions moved to `#ifdef NON_MATCHING` over `#pragma GLOBAL_ASM`, per
  ADR 0002's Consequences.
- **Finding 3/4/6** (no near-match oracle, permuter uninstalled, JFG
  structural matching unused) became [ADR 0007](adr/0007-matching-tools.md).
  decomp-permuter is installed and batch-only (`tools/permute.sh`), objdiff-cli
  is the per-object oracle, and `tools/skeleton_scan.py`
  ([`docs/skeleton-scan.md`](skeleton-scan.md)),
  `tools/flag_sweep.py` ([`docs/flag-sweep.md`](flag-sweep.md)), and
  `tools/overlay_graph_match.py` ([`docs/overlay-graph.md`](overlay-graph.md))
  are installed and documented.
- **Finding 5** (the two-job compile ceiling) became
  [ADR 0004](adr/0004-build-parallelism.md): the ceiling is removed, and
  contention is handled structurally by per-worker lane worktrees
  (`tools/new_lane.sh`, `tools/merge_lane.sh`, `tools/codex_lane.sh`) rather
  than by a job-count policy.
- **Finding 7** (three weeks uncommitted) is closed by the lane workflow
  itself: every worker now commits on its own `lane/<name>` branch, in
  small commits, with hooks on ([ADR 0010](adr/0010-commit-discipline.md)).
- **Finding 9** (per-function-file overlay layout) became
  [ADR 0006](adr/0006-overlay-source-layout.md): one translation unit per
  overlay, consolidated overlay-by-overlay once it passes ~5 matched
  functions, starting with overlays 50, 52, 54, and 101. This is
  **in progress**, not complete — see "What remains" below.
- **§13.3's `n_audio` ruling** (adopt PD/BK/JFG bodies, [ADR 0008](adr/0008-provenance.md))
  is largely executed: **30 of 45** `n_audio` translation units under
  `src/libultra/` now have C bodies with no `GLOBAL_ASM` remaining
  (counted directly from `src/libultra/n_*.c`, `sl.c`, `slHeap.c`, and the
  `LIBULTRA_NAUDIO_BARE_TUS` flag-group list in `Makefile`, which by its own
  comment only gains an entry once that TU's compiled bytes are checked
  against the ROM). 15 TUs remain.
- **Finding 10** (stale toolchain) — objdiff-cli and mapfile_parser are
  installed and covered by `tools/check_tools.sh`; the splat/spimdisasm
  version bump is not part of this pass.

**The honest scoreboard, right now** (`README.md`'s generated block; do not
copy these numbers forward into a later document — recompute):

```
functions      312 / 1457    21.41%   matched to C, byte-identical
whole resolved 163164 / 947972  17.21%   resident C + verified asm + overlay C
```

```
decompiled              146060 / 947972  (15.41%)
handwritten asm          17104 / 947972  ( 1.80%)
GLOBAL_ASM remaining    584412 / 947972  (61.65%)
NON_MATCHING            200396 / 947972  (21.14%)
NON_EQUIVALENT               0 / 947972  ( 0.00%)
```

This is a smaller "decompiled" figure than the pre-ADR headline number, on
purpose (ADR 0001's Consequences said as much before the fact): it no longer
credits an object an instruction-editing step touched. The `NON_MATCHING`
line is the real, honest size of the queue those functions now sit in.

**What remains:**

- The **NON_MATCHING queue**: 200,396 bytes across the functions ADR
  0001/0002 demoted, now the decomp-permuter's first batch per ADR 0007 —
  a bounded batch job per function, never inside an agent's own reasoning
  loop, or closed by source restructuring the way DKR's own endgame case
  studies were (ADR 0002's Consequences).
- **Overlay consolidation** (ADR 0006) is still per-function-file outside
  the four overlays named as the first cohort; the JFG/dp64-style
  `elf2dll`-equivalent consolidation step for those four has not yet
  landed in this lane.
- **Plateaus**: `README.md`'s per-area breakdown still shows "game code, not
  yet split" at 5.1% (0 matched of 590 functions) — the bulk of the game's
  own logic, as opposed to the libultra corridor (96.2%) and TU-identified
  game code (45.2%), is still unclaimed text with no owning translation
  unit at all.

Written 2026-08-24 from a review of the working tree, the reference farm,
and the current external tooling. Every number below was measured during the
review (the commands are named where they matter); none is carried forward
from an earlier document. Numbers in this file are a snapshot and are not
covered by `gmake check-docs`.

The question asked was: what have we not tried, out of ignorance of standard
tools, failure to mine sibling games, or stones not turned, that would
massively speed the campaign up under agentic workflows. The short answer is
that the campaign has been optimising the wrong denominator with the wrong
oracle, and the fixes are mostly cheap.

## 0. The ten findings, ranked by expected gain

| # | Finding | Gain |
|---|---|---|
| 1 | **63.5% of the "overlay C" bytes come from objects whose compiled instructions were rewritten after compilation** (`normalize_elf_instructions.py` and friends: 13,889 instruction-field edits, 250 reorders, 31 deletions across 274 objects). Only 10.3% is untouched compiler output. The scoreboard, `progress.py` and the README do not distinguish the two. | Credibility of every headline number; redirects ~175 KB of "done" work back onto the queue |
| 2 | **The resident segment is where the donors are, and it has been ignored.** 21.7% of resident game code (98 KB) has an exact instruction-skeleton twin in the reference builds; 86 unambiguous donors covering 28 KB are not even named yet. The overlays, where all August effort went, have 0.3%. | Names, TU boundaries and in many cases matched source for ~100 KB, at near-zero matching cost |
| 3 | **The project has no near-match oracle at all.** Donor scans are exact-modulo-relocation only. A 60-line masked-skeleton scanner written for this review runs in 3 s and found the above. `coddog` (opcode-hash + Levenshtein, indexes whole repos) and `objdiff` 3.8 ("find similar functions") do this properly and are unused. | Turns every sibling build into a searchable corpus |
| 4 | **decomp-permuter is on disk, not installed, and replaced by hand-written brute-forcers and object patching.** The register-swap histogram in the normalization files (`v0↔v1` ×672, `t6↔t7`, `t7↔t8`, `t8↔t9`) is exactly the class of mismatch the permuter's source mutations resolve without touching the object. | Converts "normalized" functions into real matches; removes the need for most of the 309 `.ops` files |
| 5 | **The two-compiler-job ceiling throttles a 14-core M3 Max to ~1/7 of its throughput** for no measured reason. One TU compiles in 0.09–0.11 s. | ~7× on every compile-and-compare loop |
| 6 | **JFG's 748 named overlay functions (with in-module offsets) and its 76-overlay cross-reference graph have never been cross-referenced against Mickey.** One of 748 names is adopted. Byte identity cannot find them (JFG's overlays are 0% matched C, compiled from a different revision); topology and offset matching can. | Real names and module identities for the overlay half of the game |
| 7 | **Three weeks of work is uncommitted.** One commit since 2026-08-01; 23 modified tracked files (+13,583 lines), 704 untracked paths, including every normalization tool and all of `config/normalizations/`. The pre-commit clean-room hook has therefore not run on any of it. | Loss and review risk; also blocks per-function bisection |
| 8 | **The local model lane was measured and lost** (5 targets, 18 samples, 2 compiling candidates, 0 improvements; server stopped 2026-08-23). The published fine-tune results agree (10/53 exact for a LoRA'd 9B; frontier models with compiler feedback at 74%). | Stop spending slots on it |
| 9 | **Overlay code is written as one function per translation unit with private struct typedefs per file** (747 files, 447 carrying `u8 pad[]` structs, 107 using `volatile` casts, aliases like `D_1DA0Read` to defeat CSE). This cannot converge to the shipped source layout, cannot own data/rodata/BSS, and re-derives every struct per function. | Consolidating per overlay is what makes the second half of each overlay cheaper than the first |
| 10 | **The toolchain is a year stale where it matters:** splat 0.35.2 (0.50.0 rewrote overlay symbol lookup across `exclusive_ram_id` segments and added `prioritized_segments`), spimdisasm 1.36 vs 1.42, no `objdiff`, no `mapfile_parser`, `ido-static-recomp` submodule uninitialised, `permuter_settings.toml` pointing at a path that does not exist. | Removes a class of friction that is currently handled with ELF surgery |

Each is expanded below, with the measurement and the concrete action.
§13 records what the gold-standard projects do on every open question, and
the rulings that follow from the user's rule that those projects' provenance
and matching standards apply here.

## 1. What the scoreboard is actually counting

### 1.1 Measurement

The working-tree `Makefile` (7,680 lines; HEAD's is 558) carries 693 object
targets with a `POSTPROCESS` assignment. By tool:

| Tool | Objects |
|---|---:|
| `trim_elf_section.py` (drop trailing alignment bytes) | 657 |
| `normalize_elf_instructions.py` (rewrite register/immediate fields, reorder, delete) | 274 |
| `objcopy --redefine-sym` | 140 |
| `filter_elf_relocations.py` | 106 |
| `rebind_elf_relocations.py` | 103 |
| `externalize_elf_section.py`, `add_elf_relocations.py`, `order_o*`, `normalize_o63_*`, `extend_elf_function_to_text.py`, `resize_elf_function.py`, `set_elf_symbol_size.py` | 12, 8, 3, 3, 3, 2, 1 |

Instruction-level operations applied at build time: **13,889** (13,332
`fields:`, 270 `set:`, 250 `reorder:`, 20 `drop-li`, 10 `drop-copy`, 6
`commute`, 1 `drop-branch`), from 537 inline Makefile lines and 266
`config/normalizations/*.ops` files (309 exist; 43 are no longer referenced).
The largest single file is `overlay1UpdateObjectPhysics.ops` at 1,135
operations for one function. A further 128 relocation `.spec` files (1,654
lines) filter, rebind or reorder relocations.

Attributing the atlas's 740 matched-C ranges to their objects (the `source`
field joins the Makefile targets 692/693):

| Category | Bytes | Share of 276,600 |
|---|---:|---:|
| instruction words altered after compilation | 175,724 | 63.5% |
| relocation/symbol table rewritten only | 8,324 | 3.0% |
| trailing trim only | 64,100 | 23.2% |
| untouched compiler output | 28,452 | 10.3% |

78 of the 106 overlays with matched C contain at least one altered object;
in 25 of them the altered share exceeds 60% (o009 99.7%, o098 100%, o017
97.6%, o086 95.5%, o015 94.7%).

`tools/progress.py` does not look at objects. `matched` is
`kind == "c"` over the hand-maintained `TEXT_SUBSEGMENTS` dictionary in
`tools/overlay_atlas.py`; the only byte-level check is the whole-ROM SHA1,
which the normalizations are constructed to satisfy. `README.md` contains no
occurrence of "normaliz", "postprocess" or "guarded".

### 1.2 What the edits are

The register-field rewrites are dominated by adjacent-register swaps:
`v0↔v1` (672 operations), `t6↔t7` (426), `t7↔t8` (424), `t8↔t9` (336),
`s0↔s1` (204), and so on down the allocation order. That histogram is the
fingerprint of source whose temporaries are created in a different order
from retail's, not of a compiler picking colours at random: IDO's allocator
is deterministic, and when the C is right the registers are right. The
remaining operations, adjacent-instruction exchanges with branch-displacement
fixups, frame-size and stack-slot rewrites, and outright deletion of a `move`
or `li` the source produced and retail did not, are cases where the source
produces different code and the object is edited until it agrees. (One
class is benign: `imm=0@0xC`-style rewrites at filtered relocation sites are
storing an overlay relocation addend, which is a linker-model workaround, not
a codegen difference.)

Neither `docs/CONTRIBUTING.md` nor `docs/workbench-improvement-log.md`
mentions normalization; the justification lives as per-function prose in
`docs/modules.md` §5.13–5.16 and `docs/campaigns.md`, in vocabulary
("complete private register web", "fail-loud", "selects retail's equivalent
allocation") that is defined nowhere. By the standard every published decomp
uses (decomp.me, objdiff, frogress, decomp.dev), and by this repository's own
tier A definition, these functions are not matched.

### 1.3 Action

1. Split the scoreboard into three overlay states: **matched** (untouched
   compiler output, trailing trim allowed), **normalized** (source plus
   reviewed object edits), **asm**. `progress.py` can derive the middle
   category mechanically from the Makefile's `POSTPROCESS` assignments; it
   should refuse to count a range as matched if its object has any
   instruction-altering step. Add a tier to `docs/modules.md` §1 for it.
2. Treat the 274 normalized functions as a queue, not a result. Each has a
   natural source that is close; that is precisely the permuter's input (§4).
   Every one that reaches an exact match retires an `.ops` file.
3. Freeze new `normalize_elf_instructions` rules. A fail-loud SHA guard makes
   an edit reproducible, not correct.

## 2. The resident segment has the donors; the overlays do not

### 2.1 Measurement

A masked-skeleton fingerprint (opcode plus funct/fmt/regimm field kept;
registers, immediates and jump targets masked) makes two functions equal when
they are the same source compiled by the same compiler, regardless of
register colouring, addresses or constants. Applied to every function in the
five reference builds (13,229 unique skeletons ≥10 words) and searched over
Mickey's ROM (`scratchpad/fingerprint.py`, 3 s):

| Region | Bytes | Skeleton-covered | Unambiguous donor | Donor bytes by project |
|---|---:|---:|---:|---|
| resident game code 0x1000–0x6F420 | 451,616 | 97,976 (21.7%) | 239 hits, 87,368 B | JFG 91,752 · PD 31,796 · DKR 5,744 · BK 556 |
| libultra corridor 0x6F420–0x76D10 | 30,960 | 24,932 (80.5%) | 85 hits | JFG 24,384 · DKR 20,896 · PD 9,464 |
| resident tail 0x76D10–0x86990 | 64,640 | 10,960 (17.0%) | 2 hits | JFG 10,960 |
| **all 106 overlays** | 469,264 | **1,256 (0.3%)** | 11 hits | |

Of the resident hits, **86 unambiguous donors covering 27,960 bytes start at
an address that is not yet in `symbol_addrs.us.txt`**. The largest are three
JFG `n_csplayer`/`gsSnd` bodies (8,132, 4,860 and 2,172 bytes), a run of JFG
matrix/path/spline routines (`matrix_XYZ_YPR_SCL`, `matrix_XYZ_YPR`,
`matrix_RPY_XYZ`, `Arctanf`, `mathXZInTri`, `pointListRPY`, `splinePos`,
`matrixTransposeVectorMultiply`, `matrixYaw`, `shadowBoxPolyOverlap`,
`rcpInit`), and PD's `osCreatePiManager`. Many JFG donors are themselves
still assembly in JFG's tree, so what they donate is the name and the TU
(`gsSnd`, `n_csplayer`, `n_load`, `n_drvrNew`, `rcpFast3d`, `shadows`); the
PD and BK `n_audio` donors are matched C.

An 8-word n-gram kinship measure (fraction of a region's masked 8-grams that
occur anywhere in a reference build) calibrates this:

| Region | DKR | JFG | PD | BK | any |
|---|---:|---:|---:|---:|---:|
| reference DKR vs JFG (same engine family) | | 17.3% | 4.9% | 3.5% | |
| reference PD vs BK (unrelated engines) | 2.4% | 6.1% | | 3.6% | |
| Mickey resident game code | 6.8% | **31.7%** | 19.8% | 5.2% | 37.8% |
| Mickey resident tail | 10.8% | 23.6% | 0.1% | 9.6% | 23.6% |
| Mickey all overlay text | 3.1% | 7.9% | 5.7% | 4.3% | 13.9% |

The resident segment is closer to JFG than DKR is. The overlays sit at the
level of an unrelated Rare game. Mickey's overlay code is new code; its
resident segment is JFG's engine.

### 2.2 The mismatch with the campaign plan

The README table says it: *game code, not yet split: 1,048 functions, 2.5%
identified*. `docs/campaigns.md` Epochs 3 through 12 are overlay campaigns
ranked by "expected exact bytes per unit time", and the resident segment's
progress since July is seven matched functions. The 50 KB `n_audio` block
(45 TUs, 10.9% of unmatched resident text, brief in
`scratchpad/n-audio-brief.md` from July) has matched C in PD and BK and
appears in the skeleton hits above; it has been waiting on a provenance
ruling about the SGI legend in those files since 2026-07-31.

### 2.3 Action

1. Adopt the 86 unnamed unambiguous donors as tier A/B names now, with
   `PROVENANCE` notes, and use the JFG TU names to draw the resident split
   (`gsSnd`, `n_csplayer`, `rcpFast3d`, `shadows`, `matrix`, `paths`).
2. Rule on `n_audio`. If the SGI legend blocks PD/BK bodies, JFG's latest
   commit ("Finish matching libultra naudio files") and decompals' `ultralib`
   (no licence file; note that too) are the alternatives. Either way the
   block is 50 KB of resident text with a matched donor and it is the largest
   single item on the board.
3. Re-rank the campaign queue on **donor availability first, bytes second**.
   The resident segment's remaining 1,048 functions include ~240 with a donor
   skeleton; those are the cheapest bytes in the ROM.

## 3. A near-match oracle, and why the exact one keeps returning zero

`tools/find_known_objects.py` masks relocated words and anchors on the
longest fixed run; that is the right tool for the libultra corridor and it
delivered 190 names there. It cannot see a function whose constants,
register colouring or call targets changed, which is every function in a
game compiled from a revised source tree. `config/overlay-donors.us.json`
records the result: 104 of 107 overlays "none" against DKR, 96 of 107 against
JFG, three usable names in total.

Three tools do this properly, none in use:

- **coddog** (ethteck, Rust, 2026-07): three hashes per function (exact
  bytes, opcodes+some operands, opcodes only), bounded Levenshtein over
  opcode sequences, `compare-raw <binary> <yamls…>` for a raw overlay against
  indexed projects, a Postgres DB that ingests repos via `decomp.yaml`, and
  `submatch <window>` for a function buried in a larger blob. Indexing DKR,
  JFG, PD, BK, Conker and dp64 is an afternoon.
- **objdiff 3.8.0** (2026-08-06): "find similar functions" across an
  object set, plus a one-shot JSON CLI (3.6+) that reports per-function
  match percentage, which is the number `progress.py` should be reading
  instead of `TEXT_SUBSEGMENTS`.
- **The review's `scratchpad/fingerprint.py`** is a 60-line stand-in for
  both and already produced §2. Promote it to `tools/skeleton_scan.py` with
  `--emit-symbols` (the backlog item in `workbench-improvement-log.md`) until
  coddog is wired in.

Related backlog: `find_known_objects.py` has no `--exclude`, no
`--min-unmasked-words`, no `--unambiguous`, and prints bytes where the
threshold is defined in functions. All are on the improvement log; none has
been done.

## 4. The permuter, and what the normalizations are standing in for

`~/Desktop/dev/decomp-permuter` is checked out (HEAD
2026-07-27), not installed in `.venv`, and referenced by nothing;
`tools/permuter_settings.toml` names a compiler at
`tools/ido-static-recomp/build/5.3/out/cc`, which does not exist. In its
place the tree has accumulated per-function brute-forcers
(`nonmatchings/*/search_{shapes,loops,stack,locals,layouts,body_lines}.py`,
each an `itertools.product` over hand-listed C spellings), about 700
`.tmp-o*` objects in the repository root, and the object-edit machinery of §1.

The permuter has 35 randomization passes, an `[ido]` weight profile, `-j N`,
`--stack-diffs`, `--stop-on-zero`, and `import.py --decompme` that builds a
self-contained scratch from the project's flags and context. Its passes
(`perm_temp_for_expr`, `perm_reorder_stmts`, `perm_sameline`,
`perm_float_literal`, `perm_commutative`, `perm_add_self_assignment`, ...)
are the source-level moves whose absence shows up as the `v0↔v1`, `t6↔t7`
swaps in the `.ops` files.

One caution from the field: the Snowboard Kids 2 project (100% in May
2026, driven mostly by Claude and Codex) removed the permuter from the
*agent* loop because agents mistook permuter artefacts (`do{}while(0)`,
nested assignments) for signal. The right shape is: agent proposes a typed,
structurally plausible candidate; permuter runs as a bounded batch job on
the candidate; a human-readable diff of the winning mutation comes back to
the agent, which rewrites it idiomatically. That is a tool call, not a lane.

Action: `pip install -e` the permuter into `.venv`, fix the toml to
`tools/ido/cc`, and run the 274 normalized functions through it as one batch
(`-j 12`, `--stop-on-zero`, 20-minute cap each). Every exact result deletes
an `.ops` file and moves bytes from "normalized" to "matched".

## 5. Throughput: the compile-token ceiling

`tools/with_compile_token.sh` allows two compiler processes at a time, at
`nice -n 10`, and `AGENTS.md` makes the ceiling policy. Measured through the
exact Makefile recipe on this machine (M3 Max, 14 cores, 36 GiB): a small
libultra TU compiles in **0.09 s**, `src/main/matrix.c` in **0.11 s**; a no-op
`gmake -n` is 0.62 s. The workbench's own campaign runner already uses
`jobs: 6`. There is no recorded contention measurement. Raise the ceiling to
`hw.ncpu - 2` and keep the token script only as a mutex for the two-phase
`gmake verify`.

## 6. JFG's overlay names and graph

`~/Desktop/dev/decomp-refs/jfg/jfg_us_syms_full.txt` lists 157 module
sections with `+offset  name` lines: 798 symbols, 760 real Rare names
(`AIJunctionControl`, `squadsGetClosestFood`, `RecceNodeMover`,
`refractInit`, `bloodSpurtAdd`, ...). `overly_refs.txt` is a cross-overlay
call graph over 76 overlays with per-call-site offsets. One of these names
(`refractOutput`) is in Mickey's symbol file; four appear anywhere in the
tree. JFG also ships the overlay toolchain (`extract_overlays.py`,
`overlay_reloc.py`, `overlay_linker.py`, `overlay_build.py`,
`read_relocs.py`) for the same 32-byte header and 8-byte relocation entry
`runlink.h` documents, and a ROM-resident symbol-name table (2,371 names in
JFG US) that Mickey's runtime linker also reads.

Because JFG's overlay code is compiled from a different revision, bytes will
not match and did not. What transfers is structure: an overlay's import
list, export offsets, entry function shape and cross-overlay edges. Mickey's
atlas already has the 97-edge cross-overlay graph and every import/export.
Matching the two graphs (by degree sequence, shared resident imports, and
function count/size profile) is a scripting task and would name the modules
that Mickey's kart-racing rewrite kept from JFG's object, AI, path, particle
and audio-line systems.

The same structural approach applies to Dinosaur Planet's 453 DLLs (425
fully C), but `docs/CLEANROOM.md` permits dp64 for file-format facts only,
and the local checkout's baserom is the 2000-12-01 proto rather than a retail
ROM, which is the premise the exception rests on. That needs a ruling before
anyone reads a dp64 name. Its 18 DLL scripts (`elf2dll.py`, `dll_split.py`,
`dllsyms2ld.py`, `dllimports.py`, `dlldiff.py`, ...) are format tooling and
are permitted; `dlldiff.py` in particular is the per-overlay oracle §1.3
asks for.

## 7. Process: commit, and let the hooks run

State on 2026-08-24: last commit 2026-08-22 (a doc); before that, 2026-07-31.
`git status` shows 23 modified tracked files (+13,583/−284) and 704 untracked
paths. `git log -S normalize_elf_instructions` and `-S patch_elf_words`
return nothing: the entire object-editing apparatus, `config/normalizations/`
(491 files), `docs/campaigns.md` (5,091 lines), 747 overlay C files and 46
new libultra TUs exist only in the working directory. `AGENTS.md` forbids
agents from committing, which is correct, but nothing has replaced it.

Consequences: no clean-room sweep has run on any of it (the pre-commit hook
scans the index); `gmake check-docs` cannot pin the numbers in
`docs/campaigns.md`; nothing can be bisected; a `git clean` or a bad
`checkout` loses August. The repository root also holds `-o`, `-o.unlinked`,
`attempt*`, `candidate*`, `variant*`, `preprocessed_*` and ~700 `.tmp-*`
files, which are the untracked residue of the hand-rolled search loops.

Action: commit in function-sized units (one exact function, its symbol
lines, its atlas row), which is also the granularity the permuter and
objdiff report on. Give each agent a worktree
(`EnterWorktree`/`git worktree add`), which SK2 found necessary for the same
reason. `.claude/` is empty; a `settings.json` allowlist for the read-only
commands (`objdump`, `nm`, `grep`, `wb_compare.sh`) is generated by
`/fewer-permission-prompts`.

## 8. Model routing

The local `n64dllm-v1` lane (llama.cpp, ~17 GiB Q4_K_M, 49k context,
`build/wb/epoch12/local_agent_eval/LEDGER.md`): 5 targets, 18 samples, 8
protocol-valid, 2 compiling, 0 improvements over the manual basin; every lane
graded "Harmful"; server stopped 2026-08-23. This agrees with the published
data: a LoRA on LLM4Decompile-9B over 23.4k byte-exact N64 scratches reached
10/53 exact, while the Macabeus benchmark (Sonnet 4.6, 12 attempts, compiler
feedback) reached 74% overall on Animal Forest (IDO): 90/70/50% on
easy/medium/hard. SK2's last ten functions fell to "Codex 5.5 xhigh". The
winning pattern everywhere is a frontier model, compiler feedback in the
loop, a hard attempt cap (10–12), and a similarity-first schedule that puts
the closest already-matched function in context as the example. Route cheap
mechanical passes (renames, header consolidation, scoreboard) to a small
model; route matching to the largest.

## 9. Overlay translation units

Every overlay function is its own `.c` with its own typedefs
(`Overlay1OwnerState { u8 pad000[0x37C]; s16 recordIndex; u8 selector; }`),
its own externs, and where needed a second extern name for the same address
(`D_1DA0Read`) or a `volatile` cast to force a reload. 747 files; 447 carry
pad-structs; 107 use `volatile`; 59 use `register`. Only nine shared headers
exist under `include/overlays/`. JFG's layout is one `.c` per overlay;
dp64's is one directory per DLL with a shared `dll.h`.

Per-function TUs were the fastest way to bank a first function in an
overlay. They are the slowest way to bank the tenth: every function
rediscovers the same struct, no data/rodata/BSS can be owned (the atlas
records text ownership only; its 61,312 initialised and 77,680 BSS bytes
have no owning object), and the
relocation model needs the `filter`/`rebind` spec files of §1 because
addends cannot be expressed. The n-gram scan shows where consolidation pays
first: overlays 50, 52 and 54 share ~40% of their code shapes with other
overlays (a shared record/offset family), overlay 101's unmatched remainder
shares 26% with code already matched.

Action: for each overlay with more than ~5 matched functions, fold into one
TU with a header, build it as a relocatable object through a
JFG/dp64-style `elf2dll` step (format facts, permitted), and compare with
`dlldiff`/objdiff per section. Do it first for 50/52/54 and 101.

## 10. Toolchain freshness

| Component | Here | Current | Why it matters |
|---|---|---|---|
| splat64 | 0.35.2 | 0.50.0 (2026-07-27) | overlay symbol lookup rewritten for symbols used across overlays sharing an `exclusive_ram_id`; `prioritized_segments`; vram+rom naming for overlay symbols; stricter overlap checks |
| spimdisasm | 1.36.1 | 1.42.4 | |
| rabbitizer | 1.16.2 | 1.16.2 | |
| objdiff | absent | 3.8.0 | per-object match %, find-similar, JSON one-shot |
| coddog | absent | 2026-07 | cross-project function search |
| mapfile_parser | absent | 2.13.2 | objdiff/decomp.dev report generation |
| decomp-permuter | on disk, not installed | 2026-07 | §4 |
| ido-static-recomp | submodule empty; prebuilt 5.3 only | v1.2: 5.3 and 7.1, native arm64 | PD compiles its naudio/mp3 TUs with **7.1 -g**; dp64 downloads both; a 7.1 basin remains a legitimate per-TU flag group to test on the normalized functions. The former `xprintf`/`xldtob` O3 deferrals are resolved by scoped direct-IDO recipes |

`gmake setup` should smoke-test each tool it installs (backlog item; both
vendored Python tools have broken at start-up before).

## 11. Things that are settled and should stay settled

- **Odd single-precision FP registers.** Re-tested across six projects'
  pure-C objects (no `GLOBAL_ASM`): 43,282 IDO single-precision operations,
  zero on an odd register. The signature appears only in unmatched assembly,
  and it clusters the same way in JFG (147 of 253 affected files are
  overlays) and dp64 (329 lines). `docs/modules.md` §6.2's conclusion holds
  and is now a lineage-wide result. Two notes: the section's "1727 operands
  across 9 files" has drifted to 1,983 across 11 with the current split, and
  the test has three silent-zero pitfalls worth recording (objdump separates
  mnemonic and operands with a tab; zsh does not word-split `$(...)`; dp64's
  disassembly uses `$ft0f`-style ABI names). One bounded experiment remains
  cheap and unrun: compile one of the unrolled float loops with the GCC
  family decomp.me offers (`gcc2.7.2kmc`, `egcs`) purely to see whether the
  interleaved odd/even shape is a compiler idiom or a human one. No Rare
  title is documented as using GCC, so the prior is low; the cost is an hour.
- **`-fp32regs` via `-Wc,`, per-file only.** Stays.
- **Reloc tables are flat in ROM.** Stays.

## 12. A suggested order of operations

1. Commit what exists, in a branch, so the hooks and `check-docs` run
   (§7). Convert the 274 instruction-altered functions to
   `#ifdef NON_MATCHING` over `GLOBAL_ASM` and adopt DKR's five-line
   scoreboard (§13.2).
2. Raise the compile ceiling (§5). Install the permuter and objdiff; pin
   splat 0.50 in a branch and re-extract (§4, §10).
3. Adopt the 86 resident donor names; draw the resident TU split from JFG's
   TU names; start `n_audio` (§2.3, unblocked by §13.3).
4. Index the reference farm in coddog (or run `skeleton_scan.py`) and put
   the top similar function in context for every resident target (§3, §8).
5. Batch the 274 normalized functions through the permuter; retire `.ops`
   files as they match (§4).
6. Match JFG's overlay graph to Mickey's atlas graph and name the modules
   (§6). Consolidate overlays 50/52/54 and 101 into real TUs (§9).
7. Try IDO 7.1 as a per-TU basin on whatever survives step 5 (§10).

Steps 1–2 are a day. Steps 3–4 are where the next 100 KB of honest progress
is. Step 5 is what turns 176 KB of provisional credit into real credit.

## 13. Gold-standard answers: what DKR actually does

The user's standing rule is that this project follows the same provenance
and matching rules as the existing N64 decomps. The DKR repository (100%,
2020-05 to 2026-07, ~20 contributors, 1,864 commits) was surveyed for each
open question above, with JFG, Banjo-Kazooie, Perfect Dark and Dinosaur
Planet as corroboration. Paths are under `~/Desktop/dev/decomp-refs/`.

### 13.1 What counts as matched

DKR's `tools/python/score.py` is purely textual: it parses `src/**.c` and
`libultra/src/**.c`, weights every function by its size from the map file,
and subtracts every `GLOBAL_ASM`. Its `WIP_REGEX` rewrites any
`#ifdef NON_MATCHING … #else GLOBAL_ASM … #endif` block back to a bare
`GLOBAL_ASM` before counting, so **NON_MATCHING and NON_EQUIVALENT functions
count as unmatched**, exactly like extracted assembly. The README block
reports five lines: decompiled, handwritten ASM, GLOBAL_ASM remaining,
NON_MATCHING, NON_EQUIVALENT. Project-level verification is the ROM SHA1
(`Makefile:342-350`, `VerifyBuild.yml`), with separate CI jobs that build
`NON_MATCHING=1` and `NON_EQUIVALENT=1` compile-only.

Hand-written original assembly is a first-class, permanently-asm category
(`src/hasm/*.s`, `hasm_in_src_path: True` in the splat yaml), counted toward
100% and reported on its own line ("Handwritten ASM functions: 85"). Mickey's
`verified_asm.us.txt` ledger is the same idea and can stay.

### 13.2 Post-compile object editing

Every object-level step in DKR, JFG, BK, PD and dp64 was enumerated:

| Project | Step | Touches |
|---|---|---|
| DKR, PD, dp64 | `patchmips3.py` | one byte of the ELF header `e_flags`, so a `-mips3` object links |
| BK | `set_o32abi_bit.py`, `objcopy --prefix-symbols`, `strip` | ELF flags and symbol table |
| DKR | `calc_func_checksums.py`, `n64crc` | post-link data words: the game's own anti-tamper checksums and the header CRC, which the original build also computed |
| PD | `mkrom` | the same: piracy-checksum placeholders and the header CRC |
| dp64 | `elf2dll.py` symbol-binding and GOT overrides | relocation metadata for DLLs that still contain `GLOBAL_ASM`, marked "hacks … should only be used by the decomp" |
| dp64 | `recomp_rom_patcher.py` | instruction bytes, but only in the deliberately non-matching static-recomp output |

**No gold-standard project edits an instruction word after compilation to
reach a match.** Mickey's `set_elf_flags.py` on `ll.c` is the DKR
`patchmips3.py` case and is fine. `normalize_elf_instructions.py`,
`normalize_o63_*.py`, `resize_elf_function.py`,
`extend_elf_function_to_text.py` and the `drop-*`/`reorder`/`fields` ops are
outside the rule. Relocation filtering/rebinding and section trimming touch
metadata only and have a partial precedent in dp64's `elf2dll` overrides, but
they exist because of the per-function TU model (§9) and should retire with
it.

DKR's endgame (`tools/decomp-workbench/docs/case-studies/`) is the proof
that the hard cases are solvable the honest way: `trackbg_render_flashy` was
a `uopt` global-allocator tie between two float registers, `func_80049794`
a temp-register FIFO recovered by splitting one expression, `func_8008FF1C`
an `as1` scheduling decision hinging on one missing `.noalias` fact. All
four were solved by source restructuring guided by compiler-internals
instrumentation, none by touching the object. The workbench's own
`scope-and-claims.md` says its forced-compiler controls "are diagnostic
only."

**Ruling for Mickey.** The 274 instruction-altered functions become
`#ifdef NON_MATCHING` C over a `GLOBAL_ASM` fallback, exactly DKR's form;
the matching build stops running the ops. The scoreboard adopts DKR's five
lines. Matched overlay C drops from 276,600 to at most 100,876 bytes
(28,452 untouched plus 72,424 whose objects had only metadata steps, pending
per-object review), and the difference is the honest queue for §4.

### 13.3 SDK library source with the SGI legend

DKR ships 108 files carrying "Copyright … Silicon Graphics" (76 with the
"unpublished proprietary" paragraph) under `libultra/src/{audio,gu,io,libc,
os,sc}`, in a CC0 repository, with no provenance statement anywhere. JFG has
64, BK 210 (via a `mariob92/ultralib` submodule plus `include/n_audio/`), PD
74 under MIT, dp64 30. This is the universal N64 decomp practice, and
Mickey's `docs/CLEANROOM.md` already permits "library source as distributed
in existing public decomp projects."

**Ruling.** `n_audio` is unblocked. Take the 45 TUs from PD/BK/JFG bodies
with `PROVENANCE` notes, JFG's headers (already verified as the right SDK
vintage: 3-argument `n_alEvtqPostEvent`), and the measured bare-`OPT_FLAGS`
`-mips2 -32` group. It is 50 KB of resident text, 10.9% of what is
unmatched there, with matched donors for every function.

### 13.4 Name provenance, dp64 and sfadebug

DKR documents no naming source at all; names live in a hand-curated
`ver/symbols/symbol_addrs.*.txt` with inline "Official Name:" comments where
one was known, and the only decomp.me mention is a 2025 commit updating code
"to match scratches on decomp.me." JFG tracks two symbol dumps
(`jfg_us_syms_full.txt`, `jfg_kiosk_syms_full.txt`). None of the five repos
carries a written leak policy; the community line in practice is that
leaked *source code* is off-limits and dumped *builds* are ordinary ROMs.
dp64's README describes its baserom neutrally: "Dinosaur Planet … as
released by Forest of Illusion on Feb. 20, 2021."

Two statements in `docs/CLEANROOM.md` need attention:

- The dp64 exception says the project is "built from a retail ROM, not the
  leaked build." There is no retail Dinosaur Planet; dp64 is built from the
  2000-12-01 development-cartridge dump. The exception's *scope* (format
  facts only) can stand, but its justification cannot.
- Under the user's rule (same provenance as the existing projects), a
  public decomp of a publicly dumped build is a published retail-derived
  decomp in every respect that matters to the community, and debug-build
  symbols from a dumped build are how OoT, Majora's Mask and Paper Mario
  were named. That would make dp64's 2,291 names and the sfadebug symbol
  set admissible. `CLEANROOM.md` currently prohibits both by name. This is
  a policy choice the user makes in writing, not something an agent decides
  mid-campaign; until it is rewritten, both stay prohibited.

decomp.me: acceptable per the user; unnecessary in an agent-driven loop,
since the permuter, coddog and objdiff all run locally against
`tools/ido/cc`.

### 13.5 Source organisation

DKR is one `.c` per original translation unit, with libultra TUs interleaved
at their true ROM offsets in the splat yaml (`- [0xB310, c,
../libultra/src/audio/mips1/seqplayer]`), `migrate_rodata_to_functions:
True`, and asm-processor swapped in as the driver for any whole file that
still contains `GLOBAL_ASM`. There is no per-function file anywhere and no
`.late_rodata` in the tree. JFG's overlays are one `src/overlays/oN/
overlay_N.c` each over shared `include/{common,functions,structs,
variables}.h`. This confirms §9: per-overlay TUs with shared headers are the
target layout, and the atlas's `(overlay, offset)` ownership becomes the
ordinary splat subsegment list.

### 13.6 Tools and workflow

DKR's tooling is small: asm-differ, asm-processor, m2c (all submodules),
`permuter_settings.toml` (the permuter run from a separate checkout),
`first_diff`, `livediff.sh` (`diff.py -smwo3`), `m2c_all.sh`,
`generate_ctx.sh`, `score.sh`/`update-score.sh` (CI opens a README PR), a
`.clang-format` (4-space, 120 columns, right-aligned pointers) and
`.clang-tidy` run by `format.sh`, and the late-stage `decomp-workbench`.
`asm/` and `assets/` are gitignored. Flags: game code `-O2 -mips1` with
`-Wab,-r4300_mul`, libultra `-mips2` (audio `-O3`, os `-O1`), three `-mips3`
TUs via `patchmips3.py`. No `-Wo,-loopunroll,0` anywhere in DKR or JFG.
Nothing here contradicts §3–§5 and §10; DKR simply never needed a
near-match oracle because it had no sibling to mine, and Mickey does.

## Appendix: measurements not covered elsewhere

- Matched overlay text 276,600 B, unmatched 192,664 B. Of the unmatched
  region's 33,576 distinct masked 8-grams, 6.7% occur in matched overlay C
  and 4.1% in resident game code.
- Reference farm (`~/Desktop/dev/decomp-refs/`): DKR 100% (1,939/1,939
  functions, 243 objects); JFG 20.8% functions / 8.3% bytes, overlays 1,274
  `GLOBAL_ASM` and 1 C function; PD 100% (IDO 5.3 and 7.1); BK ~full;
  Conker ~23%. Outside the farm: dp64 at `~/Desktop/dev/dp64/dinosaur-planet`
  (453 DLLs, 425 fully C, ninja build, IDO 5.3 `-O2 -g3 -mips2` with per-DLL
  `-O1 -g2`). `sfadebug` is not on this machine and is prohibited by
  `docs/CLEANROOM.md`.
- Compiler flag groups in use: 69 TUs with `-Wab,-r4300_mul` (DKR/JFG's
  default), 7 at `-O1`, 7 driven by `ido-phases.py`, 4 with all phases at
  `-O3`, 3 at `-O2 -g3`, overlay 5's audio group at `-O3 -mips2`. No
  systematic per-function flag sweep exists; the three flag families were
  found by hand. A script that compiles a natural candidate under the
  whole lattice and ranks by objdiff score belongs before any hand
  permutation.
- `.decomp-workbench/` campaign manifests already run `jobs: 6`.
- `build/wb/` is 82,272 files and 2.5 GB.
- decomp.me is acceptable per the user (2026-08-24) and unnecessary here:
  the permuter, coddog and objdiff all run locally against `tools/ido/cc`.
  make, not a default to assume.
