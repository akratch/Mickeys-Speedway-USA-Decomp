# tools/nm_ranking.py: ranking the NON_MATCHING queue by closeness

Hundreds of kilobytes of code across many overlays and several `src/main` TUs sit behind
`#ifdef NON_MATCHING` (the compile-only escape hatch documented in the
Makefile and `docs/acceleration-survey.md` sec.13.2): a real C body compiled
under `gmake NON_MATCHING=1`, but not yet byte-identical, so the tree still
ships the `#pragma GLOBAL_ASM` fallback under normal `gmake`. Some of those
candidates are one register swap from matching; others are structurally
wrong. This tool ranks every queued function by how close its candidate
already is, so a fleet of workers can spend its time on near-misses first
instead of triaging the whole queue by hand.

## Method

### Configured full-TU measurement

The primary measurement compiles each original translation unit once with the
expanded `NON_MATCHING=1` Makefile command. Only the object output path changes.
The complete ordered compiler arguments, including per-TU `-D` and `-I`,
source filename, relative include resolution, and source-line metadata remain
the inputs the build actually uses. Compilation runs only the raw
asm-processor/IDO command, excluding POSTPROCESS. It never touches source
timestamps and never reuses a previous object after a failed compile.

For each queued function, the tool extracts that symbol's span from this full
TU object and compares it with its own assembled `asm/nonmatchings/**/<f>.s`
target. A shared TU therefore costs one compile per refresh, while the scoring
still concerns one exact function identity. Sibling source changes invalidate
the TU's measurements because they can change its compilation context.

The former isolated-import path discarded flags outside a small codegen
subset; its historical-declaration fallback could compile a different source
context from the one recorded as fresh. Neither supplies current ranking rows
now. A failed configured TU or absent compiled symbol is an unresolved
measurement requiring repair, not permission to substitute an older source.

These are object-level measurements. A zero raw or relocation-masked word
count is not a canonical match declaration: ownership, relocation identity,
configured linked output, and ROM verification still govern promotion.

### objdiff-cli, as supplementary context

An optional `--objdiff-report` contributes per-function
`fuzzy_match_percent` from objdiff-cli. Its coverage depends on which build
objects the report contains and which ELF objects objdiff can parse. It never
replaces the configured compile measurement or proves freshness of the report
itself. Omit it to keep `objdiff_match_pct` null. The generated region records
the actual retained coverage. `tools/gen_objdiff_config.py --base-dir` can point
objdiff at the separate `build_non_matching/` tree.

### Word diff and categorization

For each resolved function, both `.text` sections are read as big-endian
32-bit words (raw, in memory only -- see "Clean-room note" below) up to
each side's own symbol size:

- **`size_bytes`**: the target (ROM) function's size.
- **`size_delta`**: `base size - target size`.
- **`differing_words`** and **`first_mismatch_offset`**: the backward-compatible
  raw byte comparison. The count covers disagreeing positions in the shared
  prefix plus every word past the shorter side's end; the offset identifies
  the first such position (or the shared length for a pure size difference).
- **`relocation_masked_differing_words`** and
  **`relocation_masked_first_mismatch_offset`**: the same positional evidence
  after masking only bits owned by a relocation on either object. The mask
  removes 26 payload bits for `R_MIPS_26`, 16 for the ordinary 16-bit
  relocation families, and all 32 for `R_MIPS_32`; opcode and register bits
  still have to agree. Unknown relocation kinds fail closed and mask nothing;
  missing or extra words remain mismatches. A `null` masked count means a
  retained schema-v1/v2 measurement has not yet been refreshed, never that it
  is exact.
- **`category`**, in this precedence order:
  1. **`size-mismatch`** -- `size_delta != 0`. Nothing else is checked;  a
     size mismatch means the two objects aren't even comparable word-for-word
     past the point of divergence.
  2. *(equal size, no differing words at all)* -- reported as `other` with
     `differing_words: 0`: the selected configured-TU symbol's raw words
     agree with the assembled target. This is an object-level observation;
     relocation identity and linked ROM ownership still require proof.
  3. **`schedule-only`** -- equal size, and the target's words are a
     permutation of the candidate's (`Counter(base_words) ==
     Counter(target_words)`, order differs). A same-instructions,
     different-order mismatch is exactly the shape IDO's scheduler is
     sensitive to (`docs/tools.md`'s own permuter case study describes a
     `perm_sameline` fix of this shape).
  4. **`register-only`** -- equal size, and every differing word decodes
     to the same opcode/function/immediate once register-select fields
     are masked out (R-type rs/rt/rd, or I-type rs/rt; J-type is left
     unmasked since it has no register fields to begin with). This is a
     coarse approximation -- COP1/COP0 fields share the R-type layout and
     are masked the same way, undistinguished from GPR swaps -- but it's
     conservative in the direction that matters: it only ever
     under-counts `register-only`, never mis-labels a real semantic
     difference as one.
  5. **`reloc-mismatch`** -- equal size, at least one raw word differs, and
     masking the union of base/target linker-owned fields removes every
     difference. An opcode or register difference at a relocation site
     remains visible and therefore cannot receive this category.
  6. **`other`** -- equal size, everything else. This is the largest
     bucket in this run and is exactly the "needs a human/model
     look, no cheap mechanical explanation available" category.

Sort order for the table and the JSON: category rank (`register-only` <
`schedule-only` < `other` < `reloc-mismatch` < `size-mismatch`), then
relocation-masked differing words ascending within a category, then raw
`differing_words`. A legacy row with no masked measurement uses its raw count
for the masked sort position. Categories describe residual bytes, not expected
cost or source reachability. Exhausted register/schedule near-matches may need
a new measured lever; a size mismatch may be a small boundary problem. Read
current attempt history and the assignment gate before choosing another pass.

### Clean-room note

No instruction word, mnemonic, or hex byte from a decoded `.text` section
is ever written to `config/nonmatching-ranking.us.json`, printed, or
otherwise leaves the running process -- every decode exists only long
enough to produce raw and relocation-masked counts or first-mismatch offsets
before being discarded. `objdiff_match_pct` is a float already computed by
objdiff-cli, not ROM content.

## Reproducing this run

Before selecting by expected yield, audit the complete live queue with
`tools/nm_ranking.py --check-freshness` (add `--json` for exact identity lists).
This gate runs no compiler and fails on missing, retired, unresolved, or stale
rows. Context version 5 binds each measurement to its complete original TU,
expanded Makefile compile command, compiler and preparation-tool contents,
transitive literal headers, pragma assembly (including literal nested assembly
includes), literal command-file operands, and extracted target. It requires the configured
tools and extracted assembly. Source/header text retains comments and blank
lines because __LINE__ and IDO source-line metadata can affect output. Inactive
include branches are conservatively included, and macro includes fail closed.
Missing literal includes in inactive SDK branches are recorded as absent; if a
file appears at a searched location, the context changes. Unrelated headers
outside the include closure do not invalidate a TU. Updating a tool invalidates
all affected measurements, even when source text is unchanged.

The retained ranking is the refresh cache: exact full-context matches skip
compilation. Older context versions need a one-time remeasurement. Both
before and after a refresh, the tool recomputes commands, tools, headers,
targets, and live source membership; changed inputs abort publication. Recipe
expansion batches every distinct TU into one make dry run. Header parsing is
shared within each context pass. Neither operation touches source timestamps.

`tools/ready_queue.py --selection expected-yield` and `--selection
high-confidence` require this complete source coverage before applying focus,
scan, or top limits. Default selection remains available for maintenance and
reports coverage explicitly; `--format maintenance` includes every unranked
identity. Run `tools/nm_ranking.py --refresh-stale` to compile all changed, new,
and unresolved identities and remove retired rows. Pruning alone cannot prove
that the remaining measurements cover the current queue.

```sh
gmake extract
gmake -j$(sysctl -n hw.ncpu)
gmake verify
./tools/make_expected.sh
gmake NON_MATCHING=1 -k -j$(sysctl -n hw.ncpu)   # -k: some POSTPROCESS objects will fail, see above

# objdiff-cli report against build_non_matching/, with the standard
# POSTPROCESS exclusion (tools/objdiff_report.sh's own approach, pointed at
# the NON_MATCHING tree instead of build/):
grep -oE '^\$\(BUILD_DIR\)/\$\(SRC_DIR\)/[A-Za-z0-9_/]+\.c\.o: POSTPROCESS' \
    Makefile | sed -E 's#\$\(BUILD_DIR\)/\$\(SRC_DIR\)/#src/#; s/: POSTPROCESS$//' \
    | sort -u > tools/objdiff_exclude.txt
.venv/bin/python tools/gen_objdiff_config.py --base-dir build_non_matching > objdiff.json
tools/objdiff/objdiff-cli report generate -p . -o /tmp/nm_report.json -f json -d

# restore objdiff.json to the normal build/ tree afterward:
.venv/bin/python tools/gen_objdiff_config.py > objdiff.json

# the ranking itself:
.venv/bin/python tools/nm_ranking.py --jobs 12 \
    --out config/nonmatching-ranking.us.json --no-table
# Optional linked/whole-TU context:
.venv/bin/python tools/nm_ranking.py --objdiff-report /tmp/nm_report.json --jobs 12
.venv/bin/python tools/nm_ranking.py --show-retained --top 20 --markdown  # fleet-prompt excerpt; no compilation

# Fast maintenance between full compile passes: remove snapshot rows whose
# exact source/symbol identity is no longer guarded by NON_MATCHING.
.venv/bin/python tools/nm_ranking.py --prune-stale

# Incremental maintenance: retain source-proven rows, compile only changed,
# newly queued, or previously unresolved identities, then merge and re-sort.
.venv/bin/python tools/nm_ranking.py --refresh-stale --jobs 2

# A bounded checkpoint compiles at most five stale identities. Deferred old
# measurements stay explicitly unproven and deferred new identities stay in
# the unresolved list; rerun without --limit to finish the refresh.
.venv/bin/python tools/nm_ranking.py --refresh-stale --limit 5 --jobs 2

# Regenerate or verify the marked human-readable snapshot without compiling.
.venv/bin/python tools/nm_ranking.py --write-doc
.venv/bin/python tools/nm_ranking.py --check-doc
```

A complete pass can run long enough for another lane to promote functions
that were queued at startup. Immediately before publishing, the ranking tool
re-scans the canonical source and drops resolved and unresolved rows whose
`#ifdef NON_MATCHING` block no longer exists. The checked-in JSON is still a
historical snapshot after the ranking process exits; consumers that need the
current queue must intersect it with a fresh source scan. `--prune-stale`
performs that intersection in place without invoking the compiler or requiring
decomp-permuter. It keys every row by the exact `(source file, symbol)` pair,
validates that resolved and unresolved identities are unique, writes the JSON
atomically, and refuses malformed unresolved rows rather than guessing which
function they describe. It only removes stale rows and normalizes the retained
counts: newly added `NON_MATCHING` functions remain unranked and are reported.

`--refresh-stale` closes that gap without replaying the whole queue. Each
resolved schema-v3 row records a SHA-256 of the configured full-TU inputs
described above. The digest uses unpadded base64url split
into four-character groups, so the clean-room scanner cannot mistake a dense
table of hexadecimal digests for machine words. Shared declarations, macros,
local data, matched code, every candidate body, and physical source lines remain
covered. The refresh validates the complete retained
document, discovers the live exact `(file, symbol)` queue, and classifies rows
as follows:

- an embedded digest equal to current context is retained without compilation;
- older source-only rows require remeasurement before carrying a current
  configured-input receipt;
- changed rows, unresolved rows, and newly queued identities are compiled via
  the ordinary `process_item` path;
- identities no longer live are removed.

Schema v3 adds relocation-masked evidence and a derived coverage count.
Schema-v1/v2 documents remain readable. Missing masked evidence is itself a
bounded-refresh reason, so repeated incremental refreshes converge to full
coverage. A source-proven old row deferred by `--limit` retains its valid raw
measurement/context and receives explicit `null` masked fields; real masked
values appear only after that row is compiled, never by inference.

The tool re-discovers and re-hashes the complete queue after compilation. Any
membership/context race or selected per-item error exits nonzero before the
JSON or generated documentation is replaced. A bounded `--limit` applies only
to compile work: deferred stale measurements remain present without a context
digest, and deferred new identities are added to `unresolved_functions`, so
the snapshot still covers the complete live queue without claiming those rows
are current. The canonical JSON and marked documentation are fully rendered
before their atomic replacements. A supplementary `--objdiff-report` applies
to newly measured rows; omitted refreshes keep proven-fresh retained values and
leave refreshed values null.

Full canonical ranking writes and canonical `--prune-stale` and
`--refresh-stale` runs update the marked region below in the same invocation.
`--write-doc` is the source-only repair command; `--check-doc` validates the complete JSON schema and exact
`(file, symbol)` identity uniqueness before comparing the rendered region.
`gmake check-docs` runs that check, so edited prose, category summaries, ranked
rows, and unresolved tables cannot silently diverge from the persisted JSON.
Text outside the markers remains authored and is preserved byte-for-byte.

<!-- NM_RANKING_GENERATED_BEGIN -->
## Current generated ranking snapshot

> Generated by `tools/nm_ranking.py --write-doc` from
> `config/nonmatching-ranking.us.json`. Do not edit this region by hand;
> `--check-doc` and `gmake check-docs` fail on any drift.

The snapshot contains **262 queued identities**: **262 resolved measurements** and **0 unresolved identities**. Resolved target size totals **347,840 bytes (339.7 KiB)**.

Resolved rows span **61 overlays** and **2 resident TU groups** (`libultra`, `main`).

A supplementary objdiff report was not supplied; `objdiff_match_pct` covers **0 / 262** resolved rows.

Persisted configured-TU input evidence covers **262 / 262** resolved rows. Rows without it are retained legacy or bounded-refresh measurements and must be treated as requiring reproof.

Relocation-masked mismatch evidence covers **262 / 262** resolved rows. The raw count preserves literal object differences; the masked count removes only known linker-owned fields to expose the remaining code-generation mismatch. Neither replaces linked byte-identity proof.

### Category distribution

| Category | Count | Share of resolved |
|---|---:|---:|
| `register-only` | 12 | 4.6% |
| `schedule-only` | 1 | 0.4% |
| `other` | 113 | 43.1% |
| `size-mismatch` | 136 | 51.9% |

### Differing-word thresholds

| Threshold | Count |
|---|---:|
| raw `differing_words <= 5` | 7 |
| relocation-masked `differing_words <= 5` | 8 |
| raw `differing_words <= 10` | 12 |
| relocation-masked `differing_words <= 10` | 14 |
| raw `differing_words <= 20` | 24 |
| relocation-masked `differing_words <= 20` | 24 |

### Complete ranked queue

Rank is the persisted `functions` array order. The exact identity is
(`file`, `symbol`), so repeated symbol spellings in different translation
units remain distinct.

| Rank | File | Symbol | Overlay/TU | Category | Target bytes | Raw diff | Masked diff | Raw first | Masked first | Size delta | Objdiff% |
|---:|---|---|---|---|---:|---:|---:|---:|---:|---:|---:|
| 1 | `src/overlays/o001/overlay_001_tail.c` | `overlay1UpdateRangeFlags` | `o001` | `register-only` | 480 | 2 | 2 | 400 | 400 | 0 | — |
| 2 | `src/overlays/o020/overlay20RemoveEntry.c` | `overlay20RemoveEntry` | `o020` | `register-only` | 212 | 2 | 2 | 108 | 108 | 0 | — |
| 3 | `src/main/level.c` | `levelInit` | `main` | `register-only` | 2,064 | 6 | 6 | 808 | 808 | 0 | — |
| 4 | `src/overlays/o071/func_overlay_071_F0000870_18CA390.c` | `func_overlay_071_F0000870_18CA390` | `o071` | `register-only` | 728 | 9 | 9 | 92 | 92 | 0 | — |
| 5 | `src/main/block_4F4E0.c` | `func_8004EC60` | `main` | `register-only` | 328 | 15 | 15 | 44 | 44 | 0 | — |
| 6 | `src/main/models.c` | `func_80020D8C` | `main` | `register-only` | 192 | 15 | 15 | 56 | 56 | 0 | — |
| 7 | `src/main/diCpu.c` | `func_80046BCC` | `main` | `register-only` | 424 | 16 | 16 | 44 | 44 | 0 | — |
| 8 | `src/main/track.c` | `func_8000FAE0` | `main` | `register-only` | 248 | 16 | 16 | 28 | 28 | 0 | — |
| 9 | `src/overlays/o044/overlay44UpdateFrameCache.c` | `overlay44UpdateFrameCache` | `o044` | `register-only` | 748 | 18 | 18 | 500 | 500 | 0 | — |
| 10 | `src/main/objects.c` | `func_80007C68` | `main` | `register-only` | 472 | 25 | 25 | 148 | 148 | 0 | — |
| 11 | `src/overlays/o001/overlay_001_tail.c` | `overlay1ConsumeNearbyPending` | `o001` | `register-only` | 276 | 31 | 31 | 64 | 64 | 0 | — |
| 12 | `src/main/particles.c` | `func_8003D25C` | `main` | `register-only` | 672 | 68 | 68 | 80 | 80 | 0 | — |
| 13 | `src/overlays/o019/overlay19BuildSpatialMasks.c` | `overlay19BuildSpatialMasks` | `o019` | `schedule-only` | 908 | 2 | 2 | 192 | 192 | 0 | — |
| 14 | `src/main/models_5B300.c` | `func_8005ABA8` | `main` | `other` | 444 | 2 | 2 | 60 | 60 | 0 | — |
| 15 | `src/overlays/o014/overlay14CreateValue.c` | `overlay14CreateValue` | `o014` | `other` | 384 | 3 | 2 | 84 | 344 | 0 | — |
| 16 | `src/overlays/o001/overlay_001.c` | `overlay1ActivateObject` | `o001` | `other` | 352 | 14 | 3 | 100 | 108 | 0 | — |
| 17 | `src/overlays/o057/overlay57UpdateModeState.c` | `overlay57UpdateModeState` | `o057` | `other` | 1,416 | 5 | 5 | 264 | 264 | 0 | — |
| 18 | `src/overlays/o009/overlay_009.c` | `func_overlay_009_F0000540_1866BB8` | `o009` | `other` | 516 | 9 | 8 | 76 | 76 | 0 | — |
| 19 | `src/overlays/o043/overlay43ComputeMotion.c` | `func_overlay_043_F00010A8_188B078` | `o043` | `other` | 220 | 9 | 9 | 116 | 116 | 0 | — |
| 20 | `src/main/main.c` | `func_80028FCC` | `main` | `other` | 108 | 10 | 10 | 28 | 28 | 0 | — |
| 21 | `src/overlays/o007/func_overlay_007_F0000324_185C1AC.c` | `func_overlay_007_F0000324_185C1AC` | `o007` | `other` | 1,392 | 19 | 10 | 132 | 288 | 0 | — |
| 22 | `src/overlays/o001/overlay_001.c` | `overlay1FindPreviousUsable` | `o001` | `other` | 160 | 14 | 12 | 4 | 20 | 0 | — |
| 23 | `src/overlays/o015/overlay_015.c` | `overlay15DrawRain` | `o015` | `other` | 216 | 18 | 13 | 8 | 116 | 0 | — |
| 24 | `src/main/objects.c` | `func_80006534` | `main` | `other` | 820 | 17 | 17 | 284 | 284 | 0 | — |
| 25 | `src/overlays/o015/overlay_015.c` | `overlay15InitStarsAndPalette` | `o015` | `other` | 988 | 20 | 19 | 112 | 112 | 0 | — |
| 26 | `src/overlays/o001/overlay_001_tail.c` | `overlay1BendPathPoint` | `o001` | `other` | 428 | 21 | 21 | 12 | 12 | 0 | — |
| 27 | `src/main/fx.c` | `func_80046EC4` | `main` | `other` | 440 | 23 | 23 | 104 | 104 | 0 | — |
| 28 | `src/overlays/o013/overlay13DrawActive.c` | `overlay13DrawActive` | `o013` | `other` | 664 | 26 | 24 | 8 | 64 | 0 | — |
| 29 | `src/overlays/o092/overlay92FindNearestCourse.c` | `func_overlay_092_F0000068_18D5F88` | `o092` | `other` | 672 | 26 | 26 | 4 | 4 | 0 | — |
| 30 | `src/main/charControl.c` | `func_8001D880` | `main` | `other` | 144 | 28 | 28 | 4 | 4 | 0 | — |
| 31 | `src/main/objects.c` | `func_80008028` | `main` | `other` | 240 | 28 | 28 | 36 | 36 | 0 | — |
| 32 | `src/overlays/o041/overlay41EnqueueTransition.c` | `func_overlay_041_F000195C_1888C94` | `o041` | `other` | 420 | 29 | 29 | 8 | 8 | 0 | — |
| 33 | `src/overlays/o098/overlay98CollectUniqueY.c` | `overlay98CollectUniqueY` | `o098` | `other` | 324 | 32 | 32 | 104 | 104 | 0 | — |
| 34 | `src/overlays/o003/overlay3RunCachedModeAction.c` | `overlay3RunCachedModeAction` | `o003` | `other` | 452 | 34 | 34 | 132 | 132 | 0 | — |
| 35 | `src/overlays/o101/overlay101DrawPanel.c` | `overlay101DrawPanel` | `o101` | `other` | 1,072 | 36 | 36 | 180 | 180 | 0 | — |
| 36 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F0002640_1860398` | `o008` | `other` | 732 | 38 | 37 | 240 | 240 | 0 | — |
| 37 | `src/main/main.c` | `func_80029274` | `main` | `other` | 348 | 39 | 39 | 8 | 8 | 0 | — |
| 38 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F0004CF0_1862A48` | `o008` | `other` | 1,080 | 44 | 39 | 452 | 452 | 0 | — |
| 39 | `src/overlays/o002/overlay2QueryNode.c` | `overlay2QueryNode` | `o002` | `other` | 1,012 | 51 | 39 | 64 | 88 | 0 | — |
| 40 | `src/main/lights.c` | `func_80018F08` | `main` | `other` | 820 | 41 | 41 | 32 | 32 | 0 | — |
| 41 | `src/overlays/o073/overlay73Draw.c` | `func_overlay_073_F0000D70_18CB830` | `o073` | `other` | 312 | 42 | 42 | 28 | 28 | 0 | — |
| 42 | `src/main/matrix.c` | `MatrixMultiplyVec4` | `main` | `other` | 212 | 47 | 47 | 0 | 0 | 0 | — |
| 43 | `src/main/joy.c` | `joyRead` | `main` | `other` | 636 | 48 | 47 | 44 | 44 | 0 | — |
| 44 | `src/overlays/o027/overlay_027.c` | `func_overlay_027_F0000064_187BA3C` | `o027` | `other` | 1,472 | 49 | 48 | 148 | 148 | 0 | — |
| 45 | `src/overlays/o017/overlay17AdvanceChain.c` | `overlay17AdvanceChain` | `o017` | `other` | 588 | 49 | 49 | 24 | 24 | 0 | — |
| 46 | `src/main/diCpu.c` | `render_epc_lock_up_display` | `main` | `other` | 1,376 | 51 | 51 | 24 | 24 | 0 | — |
| 47 | `src/main/audio_manager_36D0.c` | `func_80003480` | `main` | `other` | 376 | 53 | 53 | 0 | 0 | 0 | — |
| 48 | `src/overlays/o046/overlay46UpdateSequence.c` | `func_overlay_046_F0000120_188E518` | `o046` | `other` | 1,268 | 70 | 54 | 4 | 120 | 0 | — |
| 49 | `src/main/spranim.c` | `effectboxControl` | `main` | `other` | 772 | 57 | 57 | 72 | 72 | 0 | — |
| 50 | `src/main/objects.c` | `func_8000BB84` | `main` | `other` | 260 | 61 | 61 | 0 | 0 | 0 | — |
| 51 | `src/overlays/o037/overlay37Render.c` | `overlay37RenderEffect` | `o037` | `other` | 856 | 61 | 61 | 84 | 84 | 0 | — |
| 52 | `src/main/objects.c` | `func_80006FA0` | `main` | `other` | 376 | 63 | 63 | 12 | 12 | 0 | — |
| 53 | `src/overlays/o058/overlay58DrawSegmentStrip.c` | `overlay58DrawSegmentStrip` | `o058` | `other` | 804 | 68 | 68 | 240 | 240 | 0 | — |
| 54 | `src/main/objects.c` | `func_80004FE0` | `main` | `other` | 1,384 | 70 | 70 | 308 | 308 | 0 | — |
| 55 | `src/overlays/o036/overlay36ChooseWeightedState.c` | `func_overlay_036_F0000A60_1883F18` | `o036` | `other` | 680 | 71 | 71 | 60 | 60 | 0 | — |
| 56 | `src/main/particles.c` | `func_80040B88` | `main` | `other` | 1,208 | 72 | 72 | 72 | 72 | 0 | — |
| 57 | `src/overlays/o083/overlay83DrawStrip.c` | `overlay83DrawStrip` | `o083` | `other` | 308 | 73 | 73 | 4 | 4 | 0 | — |
| 58 | `src/overlays/o040/overlay40BuildFrame.c` | `overlay40BuildFrame` | `o040` | `other` | 324 | 75 | 75 | 4 | 4 | 0 | — |
| 59 | `src/overlays/o058/overlay58FinalizePackedStatus.c` | `overlay58FinalizePackedStatus` | `o058` | `other` | 1,216 | 78 | 77 | 24 | 24 | 0 | — |
| 60 | `src/overlays/o001/overlay_001_tail.c` | `overlay1AdvancePath` | `o001` | `other` | 648 | 81 | 81 | 16 | 16 | 0 | — |
| 61 | `src/overlays/o007/overlay_007_tail.c` | `overlay7UpdateOwnerMode` | `o007` | `other` | 556 | 81 | 81 | 16 | 16 | 0 | — |
| 62 | `src/overlays/o025/overlay_025.c` | `overlay25UpdateEffect` | `o025` | `other` | 1,036 | 82 | 82 | 60 | 60 | 0 | — |
| 63 | `src/overlays/o031/overlay31BuildLookupTables.c` | `func_overlay_031_F0000000_187F520` | `o031` | `other` | 744 | 86 | 86 | 52 | 52 | 0 | — |
| 64 | `src/main/fx.c` | `func_800470B0` | `main` | `other` | 596 | 89 | 89 | 68 | 68 | 0 | — |
| 65 | `src/overlays/o015/overlay_015.c` | `overlay15InitStars` | `o015` | `other` | 760 | 89 | 89 | 124 | 124 | 0 | — |
| 66 | `src/main/shadows.c` | `func_800180B4` | `main` | `other` | 824 | 91 | 91 | 52 | 52 | 0 | — |
| 67 | `src/main/objects.c` | `func_80007E40` | `main` | `other` | 488 | 92 | 92 | 24 | 24 | 0 | — |
| 68 | `src/overlays/o038/func_overlay_038_F000047C_188618C.c` | `func_overlay_038_F000047C_188618C` | `o038` | `other` | 876 | 93 | 93 | 192 | 192 | 0 | — |
| 69 | `src/overlays/o002/func_overlay_002_F0000C90_1857A88.c` | `func_overlay_002_F0000C90_1857A88` | `o002` | `other` | 1,420 | 104 | 94 | 152 | 220 | 0 | — |
| 70 | `src/overlays/o034/overlay34SortAndDraw.c` | `overlay34SortAndDraw` | `o034` | `other` | 760 | 99 | 97 | 80 | 80 | 0 | — |
| 71 | `src/overlays/o083/overlay83BuildBatch.c` | `overlay83BuildBatch` | `o083` | `other` | 672 | 98 | 98 | 0 | 0 | 0 | — |
| 72 | `src/main/models.c` | `func_80020B10` | `main` | `other` | 636 | 102 | 102 | 12 | 12 | 0 | — |
| 73 | `src/main/weather_tail.c` | `func_8003C80C` | `main` | `other` | 472 | 104 | 104 | 20 | 20 | 0 | — |
| 74 | `src/overlays/o050/overlay50Initialize.c` | `func_overlay_050_F0000000_1896970` | `o050` | `other` | 740 | 112 | 104 | 60 | 192 | 0 | — |
| 75 | `src/overlays/o101/overlay101TailC6E8.c` | `func_overlay_101_F000C6E8_18E7F08` | `o101` | `other` | 1,268 | 113 | 106 | 76 | 76 | 0 | — |
| 76 | `src/main/weather.c` | `rain_render_splashes` | `main` | `other` | 1,616 | 113 | 113 | 148 | 148 | 0 | — |
| 77 | `src/overlays/o052/overlay52Initialize.c` | `func_overlay_052_F0000000_189A670` | `o052` | `other` | 1,264 | 119 | 115 | 44 | 44 | 0 | — |
| 78 | `src/main/objects.c` | `func_8000B3CC` | `main` | `other` | 1,976 | 116 | 116 | 328 | 328 | 0 | — |
| 79 | `src/overlays/o002/func_overlay_002_F0001364_185815C.c` | `func_overlay_002_F0001364_185815C` | `o002` | `other` | 756 | 125 | 125 | 8 | 8 | 0 | — |
| 80 | `src/overlays/o009/overlay_009.c` | `func_overlay_009_F0000000_1866678` | `o009` | `other` | 1,344 | 131 | 125 | 60 | 60 | 0 | — |
| 81 | `src/overlays/o017/overlay17CreateChain.c` | `overlay17CreateChain` | `o017` | `other` | 784 | 130 | 130 | 0 | 0 | 0 | — |
| 82 | `src/overlays/o101/overlay101BuildPresentationD.c` | `overlay101BuildPresentationD` | `o101` | `other` | 824 | 131 | 130 | 16 | 16 | 0 | — |
| 83 | `src/main/fx.c` | `func_8004A10C` | `main` | `other` | 628 | 132 | 132 | 0 | 0 | 0 | — |
| 84 | `src/overlays/o017/overlay17CalculateEndpoints.c` | `overlay17CalculateEndpoints` | `o017` | `other` | 792 | 133 | 133 | 36 | 36 | 0 | — |
| 85 | `src/overlays/o063/overlay63UpdateEffects.c` | `overlay63UpdateEffects` | `o063` | `other` | 1,400 | 142 | 139 | 88 | 364 | 0 | — |
| 86 | `src/overlays/o079/func_overlay_079_F0000FA0_18CDF40.c` | `func_overlay_079_F0000FA0_18CDF40` | `o079` | `other` | 736 | 144 | 143 | 56 | 60 | 0 | — |
| 87 | `src/overlays/o068/overlay68DrawSortedEntries.c` | `overlay68DrawSortedEntries` | `o068` | `other` | 852 | 144 | 144 | 76 | 76 | 0 | — |
| 88 | `src/main/objects.c` | `func_8000A39C` | `main` | `other` | 656 | 154 | 154 | 4 | 4 | 0 | — |
| 89 | `src/main/menu.c` | `func_80039E34` | `main` | `other` | 1,048 | 156 | 156 | 20 | 20 | 0 | — |
| 90 | `src/main/camera.c` | `func_80022FD4` | `main` | `other` | 1,476 | 157 | 157 | 156 | 156 | 0 | — |
| 91 | `src/overlays/o092/func_overlay_092_F0000308_18D6228.c` | `func_overlay_092_F0000308_18D6228` | `o092` | `other` | 1,832 | 158 | 157 | 148 | 148 | 0 | — |
| 92 | `src/main/fx.c` | `func_80047304` | `main` | `other` | 740 | 158 | 158 | 4 | 4 | 0 | — |
| 93 | `src/overlays/o038/overlay38UpdateParticles.c` | `func_overlay_038_F0000154_1885E64` | `o038` | `other` | 808 | 160 | 159 | 32 | 32 | 0 | — |
| 94 | `src/main/level.c` | `levelGetCounts` | `main` | `other` | 1,036 | 160 | 160 | 56 | 56 | 0 | — |
| 95 | `src/overlays/o100/overlay100DrawMotion.c` | `overlay100DrawMotion` | `o100` | `other` | 972 | 161 | 161 | 0 | 0 | 0 | — |
| 96 | `src/overlays/o057/overlay57UpdateSelection.c` | `overlay57UpdateSelection` | `o057` | `other` | 1,132 | 166 | 162 | 48 | 76 | 0 | — |
| 97 | `src/main/track.c` | `func_8001398C` | `main` | `other` | 1,320 | 163 | 163 | 96 | 96 | 0 | — |
| 98 | `src/main/anim.c` | `func_80056DD8` | `main` | `other` | 916 | 177 | 177 | 36 | 36 | 0 | — |
| 99 | `src/main/track.c` | `func_8000F198` | `main` | `other` | 996 | 185 | 185 | 0 | 0 | 0 | — |
| 100 | `src/main/objects.c` | `func_80007118` | `main` | `other` | 1,836 | 186 | 186 | 44 | 44 | 0 | — |
| 101 | `src/main/track.c` | `func_800140CC` | `main` | `other` | 868 | 187 | 187 | 0 | 0 | 0 | — |
| 102 | `src/main/track.c` | `func_8001357C` | `main` | `other` | 1,040 | 189 | 189 | 8 | 8 | 0 | — |
| 103 | `src/overlays/o027/overlay_027.c` | `func_overlay_027_F0000624_187BFFC` | `o027` | `other` | 1,016 | 196 | 195 | 8 | 8 | 0 | — |
| 104 | `src/overlays/o079/func_overlay_079_F0000134_18CD0D4.c` | `func_overlay_079_F0000134_18CD0D4` | `o079` | `other` | 3,528 | 198 | 198 | 124 | 124 | 0 | — |
| 105 | `src/overlays/o068/overlay68UpdateAnimation.c` | `overlay68UpdateAnimation` | `o068` | `other` | 1,424 | 208 | 208 | 28 | 28 | 0 | — |
| 106 | `src/overlays/o057/func_overlay_057_F0004E18_18A8A10.c` | `func_overlay_057_F0004E18_18A8A10` | `o057` | `other` | 4,832 | 278 | 209 | 132 | 256 | 0 | — |
| 107 | `src/main/rcpFast3d.c` | `func_8002EBE0` | `main` | `other` | 1,020 | 218 | 218 | 0 | 0 | 0 | — |
| 108 | `src/overlays/o012/func_overlay_012_F00003A8_186D628.c` | `func_overlay_012_F00003A8_186D628` | `o012` | `other` | 1,384 | 226 | 226 | 80 | 80 | 0 | — |
| 109 | `src/overlays/o011/func_overlay_011_F00022E8_186AB30.c` | `func_overlay_011_F00022E8_186AB30` | `o011` | `other` | 1,068 | 231 | 227 | 4 | 4 | 0 | — |
| 110 | `src/main/models.c` | `func_8002057C` | `main` | `other` | 1,368 | 229 | 229 | 60 | 60 | 0 | — |
| 111 | `src/overlays/o036/overlay36UpdateInteractiveEntity.c` | `overlay36UpdateInteractiveEntity` | `o036` | `other` | 1,220 | 238 | 235 | 24 | 44 | 0 | — |
| 112 | `src/overlays/o066/overlay66SmoothAndDraw.c` | `func_overlay_066_F0000040_18C64A8` | `o066` | `other` | 1,184 | 251 | 249 | 4 | 132 | 0 | — |
| 113 | `src/overlays/o057/func_overlay_057_F00060F8_18A9CF0.c` | `func_overlay_057_F00060F8_18A9CF0` | `o057` | `other` | 1,764 | 268 | 252 | 72 | 124 | 0 | — |
| 114 | `src/main/diCpu.c` | `func_80045D34` | `main` | `other` | 1,836 | 270 | 254 | 12 | 12 | 0 | — |
| 115 | `src/overlays/o058/func_overlay_058_F0000000_18AF1E8.c` | `func_overlay_058_F0000000_18AF1E8` | `o058` | `other` | 1,472 | 305 | 295 | 0 | 0 | 0 | — |
| 116 | `src/main/charControl.c` | `func_8001DD70` | `main` | `other` | 2,132 | 299 | 299 | 84 | 84 | 0 | — |
| 117 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F00034A0_18611F8` | `o008` | `other` | 3,592 | 336 | 308 | 28 | 28 | 0 | — |
| 118 | `src/overlays/o099/func_overlay_099_F0000DDC_18DA38C.c` | `func_overlay_099_F0000DDC_18DA38C` | `o099` | `other` | 1,408 | 331 | 331 | 4 | 4 | 0 | — |
| 119 | `src/overlays/o098/overlay98RenderReflections.c` | `overlay98RenderReflections` | `o098` | `other` | 1,556 | 333 | 332 | 0 | 0 | 0 | — |
| 120 | `src/overlays/o029/func_overlay_029_F00005C4_187D874.c` | `func_overlay_029_F00005C4_187D874` | `o029` | `other` | 2,332 | 376 | 374 | 8 | 8 | 0 | — |
| 121 | `src/overlays/o064/overlay64GenerateTexture.c` | `func_overlay_064_F0000000_18C3B28` | `o064` | `other` | 1,680 | 401 | 401 | 0 | 0 | 0 | — |
| 122 | `src/overlays/o101/overlay101TailAB4C.c` | `func_overlay_101_F000AB4C_18E636C` | `o101` | `other` | 2,552 | 501 | 499 | 16 | 16 | 0 | — |
| 123 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F0001294_185EFEC` | `o008` | `other` | 5,036 | 654 | 636 | 0 | 0 | 0 | — |
| 124 | `src/overlays/o054/overlay54TailA.c` | `func_overlay_054_F00005AC_189F24C` | `o054` | `other` | 6,376 | 867 | 851 | 0 | 0 | 0 | — |
| 125 | `src/overlays/o050/func_overlay_050_F0000334_1896CA4.c` | `func_overlay_050_F0000334_1896CA4` | `o050` | `other` | 6,300 | 1,129 | 1,115 | 0 | 0 | 0 | — |
| 126 | `src/overlays/o052/overlay52TailB.c` | `func_overlay_052_F000063C_189ACAC` | `o052` | `other` | 6,748 | 1,338 | 1,337 | 304 | 304 | 0 | — |
| 127 | `src/libultra/block_6F3E0.c` | `func_8006E7E0` | `libultra` | `size-mismatch` | 4 | 2 | 2 | 0 | 0 | 4 | — |
| 128 | `src/main/vehicle_sounds.c` | `func_80058250` | `main` | `size-mismatch` | 88 | 19 | 19 | 0 | 0 | 16 | — |
| 129 | `src/overlays/o015/overlay_015.c` | `overlay15MoveStars` | `o015` | `size-mismatch` | 216 | 33 | 30 | 48 | 48 | 16 | — |
| 130 | `src/main/matrix.c` | `func_8002B040` | `main` | `size-mismatch` | 136 | 34 | 34 | 0 | 0 | 4 | — |
| 131 | `src/main/objects.c` | `func_80006EE4` | `main` | `size-mismatch` | 188 | 40 | 40 | 8 | 8 | -4 | — |
| 132 | `src/overlays/o048/overlay48InitializeState.c` | `overlay48InitializeState` | `o048` | `size-mismatch` | 228 | 47 | 47 | 0 | 0 | -16 | — |
| 133 | `src/overlays/o046/func_overlay_046_F0000874_188EC6C.c` | `func_overlay_046_F0000874_188EC6C` | `o046` | `size-mismatch` | 1,800 | 56 | 50 | 112 | 508 | 8 | — |
| 134 | `src/main/saves.c` | `func_8002CF6C` | `main` | `size-mismatch` | 352 | 54 | 54 | 32 | 32 | -8 | — |
| 135 | `src/main/frontend_37D50.c` | `func_80037414` | `main` | `size-mismatch` | 580 | 57 | 57 | 16 | 16 | 4 | — |
| 136 | `src/main/objects.c` | `func_80006B04` | `main` | `size-mismatch` | 316 | 63 | 63 | 20 | 20 | -4 | — |
| 137 | `src/main/track.c` | `func_8000D820` | `main` | `size-mismatch` | 344 | 65 | 65 | 52 | 52 | -8 | — |
| 138 | `src/main/camera.c` | `func_80024978` | `main` | `size-mismatch` | 332 | 71 | 71 | 8 | 8 | 4 | — |
| 139 | `src/main/objects.c` | `func_8000831C` | `main` | `size-mismatch` | 424 | 81 | 81 | 64 | 64 | -4 | — |
| 140 | `src/main/matrix.c` | `func_8002AB78` | `main` | `size-mismatch` | 268 | 83 | 83 | 0 | 0 | 68 | — |
| 141 | `src/overlays/o015/overlay_015.c` | `overlay15UpdateMovingStars` | `o015` | `size-mismatch` | 412 | 87 | 84 | 36 | 48 | 28 | — |
| 142 | `src/overlays/o089/overlay89UpdateStateAndParticles.c` | `overlay89UpdateStateAndParticles` | `o089` | `size-mismatch` | 544 | 85 | 85 | 8 | 8 | 4 | — |
| 143 | `src/main/matrix.c` | `func_8002AA50` | `main` | `size-mismatch` | 296 | 90 | 90 | 0 | 0 | 68 | — |
| 144 | `src/main/track.c` | `func_800133FC` | `main` | `size-mismatch` | 384 | 98 | 98 | 0 | 0 | 12 | — |
| 145 | `src/main/sched.c` | `func_80030610` | `main` | `size-mismatch` | 768 | 105 | 104 | 140 | 140 | 4 | — |
| 146 | `src/main/font.c` | `func_8004C690` | `main` | `size-mismatch` | 584 | 105 | 105 | 0 | 0 | -8 | — |
| 147 | `src/main/track.c` | `func_8000D1B8` | `main` | `size-mismatch` | 512 | 114 | 114 | 48 | 48 | 8 | — |
| 148 | `src/main/fx.c` | `fxScreenEffect` | `main` | `size-mismatch` | 588 | 117 | 116 | 8 | 8 | -16 | — |
| 149 | `src/main/matrix.c` | `func_8002AC84` | `main` | `size-mismatch` | 396 | 118 | 118 | 0 | 0 | 80 | — |
| 150 | `src/overlays/o041/overlay41ProcessEntry.c` | `func_overlay_041_F0001464_188879C` | `o041` | `size-mismatch` | 492 | 120 | 120 | 16 | 16 | 20 | — |
| 151 | `src/main/fx.c` | `wakeDraw` | `main` | `size-mismatch` | 708 | 121 | 121 | 0 | 0 | -4 | — |
| 152 | `src/main/runlink.c` | `ProcessRelocationEntry` | `main` | `size-mismatch` | 584 | 126 | 126 | 0 | 0 | 4 | — |
| 153 | `src/overlays/o054/overlay54Initialize.c` | `func_overlay_054_F0000000_189ECA0` | `o054` | `size-mismatch` | 972 | 144 | 127 | 84 | 240 | 4 | — |
| 154 | `src/main/spranim.c` | `func_8001B798` | `main` | `size-mismatch` | 700 | 131 | 131 | 0 | 0 | -16 | — |
| 155 | `src/main/matrix.c` | `func_8002AE10` | `main` | `size-mismatch` | 348 | 138 | 138 | 0 | 0 | 208 | — |
| 156 | `src/overlays/o101/overlay101BuildPresentationA.c` | `overlay101BuildPresentationA` | `o101` | `size-mismatch` | 832 | 144 | 143 | 16 | 16 | 4 | — |
| 157 | `src/overlays/o101/overlay101BuildPresentationB.c` | `overlay101BuildPresentationB` | `o101` | `size-mismatch` | 832 | 144 | 143 | 16 | 16 | 4 | — |
| 158 | `src/overlays/o101/overlay101BuildPresentationC.c` | `overlay101BuildPresentationC` | `o101` | `size-mismatch` | 832 | 144 | 143 | 16 | 16 | 4 | — |
| 159 | `src/main/frontend_37D50.c` | `func_800371BC` | `main` | `size-mismatch` | 600 | 144 | 144 | 44 | 44 | 20 | — |
| 160 | `src/main/track.c` | `func_8000DB34` | `main` | `size-mismatch` | 688 | 144 | 144 | 84 | 84 | -12 | — |
| 161 | `src/overlays/o010/overlay10Initialize.c` | `overlay10Initialize` | `o010` | `size-mismatch` | 688 | 147 | 147 | 0 | 0 | 156 | — |
| 162 | `src/main/audiomgr.c` | `func_80001740` | `main` | `size-mismatch` | 836 | 154 | 153 | 0 | 0 | -24 | — |
| 163 | `src/main/fx.c` | `func_80049B14` | `main` | `size-mismatch` | 824 | 154 | 154 | 8 | 8 | 4 | — |
| 164 | `src/main/track.c` | `func_80012658` | `main` | `size-mismatch` | 708 | 154 | 154 | 0 | 0 | -8 | — |
| 165 | `src/main/lights.c` | `func_80019AB8` | `main` | `size-mismatch` | 736 | 155 | 155 | 32 | 32 | -4 | — |
| 166 | `src/main/track.c` | `func_800103D4` | `main` | `size-mismatch` | 640 | 158 | 158 | 0 | 0 | -16 | — |
| 167 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F0001000_185ED58` | `o008` | `size-mismatch` | 660 | 159 | 159 | 8 | 8 | -4 | — |
| 168 | `src/overlays/o001/overlay_001_tail.c` | `overlay1TransitionState` | `o001` | `size-mismatch` | 948 | 160 | 160 | 32 | 32 | -4 | — |
| 169 | `src/main/track.c` | `func_80010654` | `main` | `size-mismatch` | 684 | 162 | 162 | 0 | 0 | 4 | — |
| 170 | `src/main/fx.c` | `func_800479D4` | `main` | `size-mismatch` | 772 | 167 | 167 | 0 | 0 | -4 | — |
| 171 | `src/main/objects.c` | `func_800084C4` | `main` | `size-mismatch` | 1,372 | 168 | 168 | 96 | 96 | 4 | — |
| 172 | `src/main/textures_354C8.c` | `func_80036544` | `main` | `size-mismatch` | 608 | 172 | 172 | 0 | 0 | 80 | — |
| 173 | `src/main/textures_354C8.c` | `func_800355A0` | `main` | `size-mismatch` | 1,076 | 177 | 177 | 0 | 0 | -4 | — |
| 174 | `src/overlays/o020/func_overlay_020_F0001148_1877720.c` | `func_overlay_020_F0001148_1877720` | `o020` | `size-mismatch` | 828 | 180 | 178 | 32 | 32 | -8 | — |
| 175 | `src/overlays/o001/func_overlay_001_F0003258_184F638.c` | `func_overlay_001_F0003258_184F638` | `o001` | `size-mismatch` | 800 | 183 | 182 | 0 | 0 | -48 | — |
| 176 | `src/main/track.c` | `func_8000E5EC` | `main` | `size-mismatch` | 820 | 185 | 185 | 0 | 0 | 16 | — |
| 177 | `src/overlays/o020/overlay20UpdateGrid.c` | `overlay20UpdateGrid` | `o020` | `size-mismatch` | 860 | 185 | 185 | 0 | 0 | 4 | — |
| 178 | `src/overlays/o014/func_overlay_014_F0001830_1871108.c` | `func_overlay_014_F0001830_1871108` | `o014` | `size-mismatch` | 804 | 186 | 186 | 0 | 0 | 4 | — |
| 179 | `src/overlays/o011/func_overlay_011_F0001E4C_186A694.c` | `func_overlay_011_F0001E4C_186A694` | `o011` | `size-mismatch` | 1,180 | 189 | 186 | 216 | 216 | -4 | — |
| 180 | `src/main/track.c` | `func_80012234` | `main` | `size-mismatch` | 832 | 187 | 187 | 12 | 12 | 12 | — |
| 181 | `src/overlays/o087/func_overlay_087_F0000128_18D3090.c` | `func_overlay_087_F0000128_18D3090` | `o087` | `size-mismatch` | 1,896 | 195 | 193 | 44 | 44 | -16 | — |
| 182 | `src/overlays/o066/func_overlay_066_F00004E0_18C6948.c` | `func_overlay_066_F00004E0_18C6948` | `o066` | `size-mismatch` | 816 | 202 | 202 | 0 | 0 | -52 | — |
| 183 | `src/overlays/o099/overlay99ApplySegment.c` | `overlay99ApplySegment` | `o099` | `size-mismatch` | 920 | 203 | 202 | 0 | 0 | -4 | — |
| 184 | `src/main/main.c` | `func_80026FB4` | `main` | `size-mismatch` | 1,652 | 207 | 207 | 56 | 56 | 24 | — |
| 185 | `src/main/track.c` | `func_80011980` | `main` | `size-mismatch` | 860 | 211 | 211 | 0 | 0 | 8 | — |
| 186 | `src/main/track.c` | `func_800115E4` | `main` | `size-mismatch` | 924 | 217 | 217 | 0 | 0 | 24 | — |
| 187 | `src/main/fx.c` | `fxSPDPRipple` | `main` | `size-mismatch` | 928 | 224 | 224 | 8 | 8 | 8 | — |
| 188 | `src/main/lights.c` | `func_8001953C` | `main` | `size-mismatch` | 1,016 | 225 | 225 | 0 | 0 | 32 | — |
| 189 | `src/main/charControl.c` | `func_8001EC44` | `main` | `size-mismatch` | 952 | 231 | 231 | 0 | 0 | 12 | — |
| 190 | `src/main/anim.c` | `func_80051364` | `main` | `size-mismatch` | 1,148 | 251 | 251 | 0 | 0 | 4 | — |
| 191 | `src/overlays/o020/func_overlay_020_F000038C_1876964.c` | `func_overlay_020_F000038C_1876964` | `o020` | `size-mismatch` | 1,080 | 256 | 256 | 0 | 0 | -24 | — |
| 192 | `src/overlays/o035/func_overlay_035_F00001E0_1881EC0.c` | `func_overlay_035_F00001E0_1881EC0` | `o035` | `size-mismatch` | 1,424 | 262 | 262 | 188 | 188 | -8 | — |
| 193 | `src/main/camera.c` | `func_80023598` | `main` | `size-mismatch` | 1,136 | 263 | 263 | 0 | 0 | 8 | — |
| 194 | `src/main/textures_354C8.c` | `func_800349A4` | `main` | `size-mismatch` | 1,088 | 265 | 265 | 8 | 8 | 8 | — |
| 195 | `src/overlays/o101/overlay101TailA6BC.c` | `overlay101TailA6BC` | `o101` | `size-mismatch` | 1,168 | 267 | 267 | 0 | 0 | 4 | — |
| 196 | `src/main/shadows.c` | `func_80017BCC` | `main` | `size-mismatch` | 1,256 | 270 | 270 | 4 | 4 | 8 | — |
| 197 | `src/main/rcpFast3d.c` | `func_8002FB34` | `main` | `size-mismatch` | 1,436 | 272 | 272 | 64 | 64 | -20 | — |
| 198 | `src/overlays/o057/func_overlay_057_F0000000_18A3BF8.c` | `func_overlay_057_F0000000_18A3BF8` | `o057` | `size-mismatch` | 2,388 | 307 | 276 | 8 | 8 | -20 | — |
| 199 | `src/overlays/o101/func_overlay_101_F0002510_18DDD30.c` | `func_overlay_101_F0002510_18DDD30` | `o101` | `size-mismatch` | 1,172 | 291 | 291 | 0 | 0 | 8 | — |
| 200 | `src/main/models.c` | `func_8001FC50` | `main` | `size-mismatch` | 1,332 | 299 | 299 | 0 | 0 | -12 | — |
| 201 | `src/main/shadows.c` | `func_80017140` | `main` | `size-mismatch` | 1,312 | 300 | 300 | 68 | 68 | -12 | — |
| 202 | `src/main/track.c` | `func_8000DFBC` | `main` | `size-mismatch` | 1,584 | 304 | 304 | 72 | 72 | 8 | — |
| 203 | `src/main/main.c` | `func_80028564` | `main` | `size-mismatch` | 1,956 | 307 | 306 | 4 | 4 | 8 | — |
| 204 | `src/overlays/o043/func_overlay_043_F0000BE4_188ABB4.c` | `func_overlay_043_F0000BE4_188ABB4` | `o043` | `size-mismatch` | 1,220 | 307 | 307 | 4 | 4 | 28 | — |
| 205 | `src/main/frontend_37D50.c` | `func_80037C74` | `main` | `size-mismatch` | 1,308 | 314 | 314 | 52 | 52 | -64 | — |
| 206 | `src/main/track.c` | `func_80011CDC` | `main` | `size-mismatch` | 1,368 | 327 | 327 | 0 | 0 | 8 | — |
| 207 | `src/overlays/o044/func_overlay_044_F0000580_188BDE0.c` | `func_overlay_044_F0000580_188BDE0` | `o044` | `size-mismatch` | 1,396 | 338 | 338 | 8 | 8 | -4 | — |
| 208 | `src/overlays/o001/overlay_001_tail.c` | `func_overlay_001_F0003750_184FB30` | `o001` | `size-mismatch` | 1,784 | 346 | 341 | 76 | 88 | 4 | — |
| 209 | `src/main/fx.c` | `wakeAllocate` | `main` | `size-mismatch` | 1,404 | 345 | 345 | 12 | 12 | -32 | — |
| 210 | `src/main/audiomgr.c` | `func_80001BF4` | `main` | `size-mismatch` | 1,344 | 350 | 350 | 0 | 0 | 144 | — |
| 211 | `src/main/models.c` | `func_8001F520` | `main` | `size-mismatch` | 1,604 | 365 | 365 | 8 | 8 | -20 | — |
| 212 | `src/main/frontend_37D50.c` | `func_80038190` | `main` | `size-mismatch` | 1,472 | 366 | 365 | 0 | 0 | -104 | — |
| 213 | `src/main/fx.c` | `wakeUpdate` | `main` | `size-mismatch` | 1,592 | 367 | 367 | 0 | 0 | -4 | — |
| 214 | `src/main/textures_354C8.c` | `func_80035F48` | `main` | `size-mismatch` | 1,532 | 371 | 371 | 0 | 0 | -84 | — |
| 215 | `src/main/anim.c` | `func_80054B3C` | `main` | `size-mismatch` | 1,480 | 374 | 374 | 8 | 8 | 40 | — |
| 216 | `src/main/models_5B300.c` | `func_8005AF14` | `main` | `size-mismatch` | 1,840 | 377 | 377 | 0 | 0 | 12 | — |
| 217 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F00042A8_1862000` | `o008` | `size-mismatch` | 1,788 | 384 | 384 | 68 | 68 | -68 | — |
| 218 | `src/main/charControl.c` | `func_8001C4C0` | `main` | `size-mismatch` | 1,612 | 386 | 386 | 0 | 0 | -8 | — |
| 219 | `src/overlays/o002/func_overlay_002_F0001DF8_1858BF0.c` | `func_overlay_002_F0001DF8_1858BF0` | `o002` | `size-mismatch` | 1,840 | 387 | 387 | 40 | 40 | 4 | — |
| 220 | `src/main/fx.c` | `func_800475E8` | `main` | `size-mismatch` | 1,004 | 390 | 390 | 0 | 0 | 780 | — |
| 221 | `src/main/objects.c` | `func_80009414` | `main` | `size-mismatch` | 1,684 | 398 | 398 | 0 | 0 | 20 | — |
| 222 | `src/main/charControl.c` | `func_8001E5C4` | `main` | `size-mismatch` | 1,664 | 411 | 408 | 0 | 0 | -16 | — |
| 223 | `src/overlays/o046/func_overlay_046_F0001228_188F620.c` | `func_overlay_046_F0001228_188F620` | `o046` | `size-mismatch` | 1,844 | 416 | 415 | 92 | 92 | -96 | — |
| 224 | `src/main/anim.c` | `func_80055104` | `main` | `size-mismatch` | 1,780 | 420 | 420 | 56 | 56 | -56 | — |
| 225 | `src/overlays/o001/func_overlay_001_F0002B4C_184EF2C.c` | `func_overlay_001_F0002B4C_184EF2C` | `o001` | `size-mismatch` | 1,804 | 433 | 433 | 0 | 0 | -16 | — |
| 226 | `src/overlays/o008/overlay_008.c` | `func_overlay_008_F0000058_185DDB0` | `o008` | `size-mismatch` | 2,108 | 442 | 438 | 44 | 112 | -64 | — |
| 227 | `src/main/shadows.c` | `shadowGenerate` | `main` | `size-mismatch` | 2,040 | 445 | 445 | 12 | 12 | 40 | — |
| 228 | `src/overlays/o055/func_overlay_055_F000031C_18A1E34.c` | `func_overlay_055_F000031C_18A1E34` | `o055` | `size-mismatch` | 2,324 | 445 | 445 | 60 | 60 | -24 | — |
| 229 | `src/overlays/o057/func_overlay_057_F0004460_18A8058.c` | `func_overlay_057_F0004460_18A8058` | `o057` | `size-mismatch` | 1,976 | 447 | 446 | 4 | 4 | -144 | — |
| 230 | `src/overlays/o022/func_overlay_022_F00002B0_18783B8.c` | `func_overlay_022_F00002B0_18783B8` | `o022` | `size-mismatch` | 1,996 | 448 | 448 | 28 | 28 | -12 | — |
| 231 | `src/overlays/o084/func_overlay_084_F0000314_18D07F4.c` | `func_overlay_084_F0000314_18D07F4` | `o084` | `size-mismatch` | 1,856 | 450 | 449 | 60 | 60 | 16 | — |
| 232 | `src/main/track.c` | `func_8000E920` | `main` | `size-mismatch` | 2,168 | 459 | 459 | 0 | 0 | 20 | — |
| 233 | `src/main/textures_354C8.c` | `func_80034E54` | `main` | `size-mismatch` | 1,868 | 461 | 461 | 0 | 0 | -32 | — |
| 234 | `src/main/font.c` | `func_8004B1DC` | `main` | `size-mismatch` | 2,224 | 466 | 465 | 48 | 48 | -32 | — |
| 235 | `src/overlays/o001/overlay_001_head.c` | `overlay1LoadBuildRecords` | `o001` | `size-mismatch` | 2,288 | 491 | 470 | 52 | 52 | -92 | — |
| 236 | `src/overlays/o061/func_overlay_061_F0000B84_18BFF4C.c` | `func_overlay_061_F0000B84_18BFF4C` | `o061` | `size-mismatch` | 2,548 | 499 | 476 | 0 | 0 | -64 | — |
| 237 | `src/overlays/o035/func_overlay_035_F0000B40_1882820.c` | `func_overlay_035_F0000B40_1882820` | `o035` | `size-mismatch` | 2,112 | 486 | 486 | 4 | 4 | -48 | — |
| 238 | `src/overlays/o043/func_overlay_043_F0000324_188A2F4.c` | `func_overlay_043_F0000324_188A2F4` | `o043` | `size-mismatch` | 2,240 | 495 | 494 | 96 | 96 | -4 | — |
| 239 | `src/main/track.c` | `func_8001291C` | `main` | `size-mismatch` | 2,192 | 499 | 499 | 0 | 0 | -40 | — |
| 240 | `src/overlays/o026/func_overlay_026_F00001A0_187A598.c` | `func_overlay_026_F00001A0_187A598` | `o026` | `size-mismatch` | 2,424 | 510 | 510 | 156 | 156 | -16 | — |
| 241 | `src/overlays/o019/overlay19BuildPlanes.c` | `overlay19BuildPlanes` | `o019` | `size-mismatch` | 2,128 | 512 | 512 | 72 | 72 | -40 | — |
| 242 | `src/overlays/o011/func_overlay_011_F0000150_1868998.c` | `func_overlay_011_F0000150_1868998` | `o011` | `size-mismatch` | 2,248 | 538 | 535 | 0 | 0 | -120 | — |
| 243 | `src/overlays/o056/overlay_056.c` | `func_overlay_056_F00001A0_18A2F18` | `o056` | `size-mismatch` | 2,324 | 538 | 538 | 80 | 80 | 40 | — |
| 244 | `src/overlays/o047/func_overlay_047_F0000000_1890E18.c` | `func_overlay_047_F0000000_1890E18` | `o047` | `size-mismatch` | 2,512 | 553 | 548 | 104 | 200 | 16 | — |
| 245 | `src/main/shadows.c` | `func_80016890` | `main` | `size-mismatch` | 2,224 | 563 | 563 | 4 | 4 | 60 | — |
| 246 | `src/overlays/o090/overlay_090.c` | `func_overlay_090_F00000FC_18D4BF4` | `o090` | `size-mismatch` | 2,592 | 575 | 575 | 0 | 0 | -36 | — |
| 247 | `src/overlays/o012/func_overlay_012_F0000910_186DB90.c` | `func_overlay_012_F0000910_186DB90` | `o012` | `size-mismatch` | 2,444 | 581 | 581 | 0 | 0 | -20 | — |
| 248 | `src/overlays/o001/func_overlay_001_F0001D78_184E158.c` | `func_overlay_001_F0001D78_184E158` | `o001` | `size-mismatch` | 2,508 | 587 | 584 | 0 | 0 | -112 | — |
| 249 | `src/overlays/o057/func_overlay_057_F0001020_18A4C18.c` | `func_overlay_057_F0001020_18A4C18` | `o057` | `size-mismatch` | 2,392 | 585 | 585 | 0 | 0 | -32 | — |
| 250 | `src/overlays/o045/func_overlay_045_F0001158_188D5B0.c` | `func_overlay_045_F0001158_188D5B0` | `o045` | `size-mismatch` | 2,696 | 593 | 593 | 0 | 0 | 28 | — |
| 251 | `src/overlays/o057/func_overlay_057_F0001AE8_18A56E0.c` | `func_overlay_057_F0001AE8_18A56E0` | `o057` | `size-mismatch` | 3,532 | 645 | 614 | 4 | 352 | -132 | — |
| 252 | `src/overlays/o053/func_overlay_053_F0000240_189DBE8.c` | `func_overlay_053_F0000240_189DBE8` | `o053` | `size-mismatch` | 2,544 | 636 | 636 | 0 | 0 | -112 | — |
| 253 | `src/main/anim.c` | `func_800563B4` | `main` | `size-mismatch` | 2,596 | 637 | 637 | 28 | 28 | -160 | — |
| 254 | `src/main/track.c` | `func_80010B4C` | `main` | `size-mismatch` | 2,712 | 662 | 662 | 0 | 0 | 56 | — |
| 255 | `src/overlays/o065/overlay65UpdateParticles.c` | `overlay65UpdateParticles` | `o065` | `size-mismatch` | 2,880 | 696 | 689 | 12 | 12 | -40 | — |
| 256 | `src/main/vehicle_sounds.c` | `func_8005830C` | `main` | `size-mismatch` | 3,048 | 698 | 697 | 8 | 8 | -16 | — |
| 257 | `src/overlays/o058/func_overlay_058_F00005FC_18AF7E4.c` | `func_overlay_058_F00005FC_18AF7E4` | `o058` | `size-mismatch` | 3,316 | 730 | 722 | 0 | 0 | 4 | — |
| 258 | `src/overlays/o073/func_overlay_073_F0000190_18CAC50.c` | `func_overlay_073_F0000190_18CAC50` | `o073` | `size-mismatch` | 3,040 | 756 | 755 | 16 | 16 | 24 | — |
| 259 | `src/main/anim.c` | `func_80053868` | `main` | `size-mismatch` | 4,820 | 1,170 | 1,169 | 12 | 12 | -240 | — |
| 260 | `src/overlays/o001/overlay_001_tail.c` | `func_overlay_001_F000438C_185076C` | `o001` | `size-mismatch` | 6,168 | 1,203 | 1,196 | 36 | 36 | -4 | — |
| 261 | `src/overlays/o047/func_overlay_047_F0000B30_1891948.c` | `func_overlay_047_F0000B30_1891948` | `o047` | `size-mismatch` | 8,672 | 1,593 | 1,580 | 4 | 4 | 4 | — |
| 262 | `src/main/anim.c` | `func_800517E0` | `main` | `size-mismatch` | 7,232 | 1,785 | 1,782 | 0 | 0 | -332 | — |

### Unresolved identities

None.
<!-- NM_RANKING_GENERATED_END -->

## Recommended batching for the fleet

1. Check complete ranking freshness and the current lane/plateau ledger before
   assigning work. A near-exact residual with an exhausted attempt history is
   not a ready target until a new mechanism or authorized bounded sweep exists.
2. Use register/schedule categories to choose the next diagnostic tool, not to
   promise cheap wins. Preserve exact extent, frame, relocation evidence, and
   prior flat experiments alongside the scalar word score.
3. Group independent work by useful shared context while keeping symbol
   ownership disjoint. Prefer measured new levers and source-hash-new sweeps;
   stop bounded attempts with a reproducible plateau.
4. Separate relocation/boundary reproof from creative source reconstruction.
   A zero object score still needs real-offset linked proof. Structural work
   receives an explicit budget and stop condition under ADR 0009.

## Authored campaign notes

The observations below are retained campaign context, not generated ranking
rows. Current queue membership and measurements live only in the marked region
above and its JSON source.

At the time of its recorded investigation, `overlay1FindBestRecord` was a
size-exact register-allocation near miss that the snapshot classified as
`other`: the extracted target
object omits the selected-type HI16/LO16 pair that the shipped runtime table
and candidate both retain. Current configured full-TU V0 is frameless and
18/30 words with 12 `a1`/`a3` pool-register differences from `+0x04`; it
belongs beside the 12-word rows operationally. All 119 flag configurations
were attempted, seven O2/MIPS-II rows tie V0, and none is exact. A
fidelity-clean proc-38 allocator trace plus all three permitted natural
declaration/scope forms leave the same object, so the fallback is parked
pending a new allocator-order mechanism. The earlier exact claim depended on
prohibited post-compile field edits and remains inadmissible.

Nine rows from that historical run were subsequently promoted and ceased to
be search candidates: `overlay3FindClosestObject`, `overlay40AddEntry`,
`overlay43SubmitChildren`, `func_80038750`, `partUpdateTriggers`,
`func_8001A154`, `overlay1UpdateValueCache`,
`func_overlay_041_F0000000_1887338`, and `overlay80UpdateContact`. Their
canonical source and function-specific ledgers carry the exact proofs; stale
generated ranking entries must not put them back into the ready queue.

`func_80021504` is unguarded matched C and no longer a ranking candidate.
Retained configured C owns 133 words with frame `0x28` and 43 candidate
relocation tuples. Its linked range, complete camera TU, and resident `.main`
section are byte-identical to ROM. The retained full `.bin` predates the
object and independent target relocation metadata is absent, so this is a
reproof-only integrity target rather than living search.

`func_80021718` is already canonical C: retained configured C owns 37
words, frame `0x28`, and 14 candidate relocation tuples. Those tuples
reproduce all 37 linked ELF words, and the post-object linked range and
complete camera TU are byte-identical to ROM. No caller is proven;
ROM-table row 453 is an unreferenced export, not inbound evidence. The
full `.bin` predates the object and independent target relocation metadata
is absent, so this remains a reproof-only target rather than living search.

`func_800219D0` is a reproof-only matched function, not a ranking candidate.
The retained pre-comment configured object owns 104 words with frame `0x8`
and eight HI16/LO16 records. Its raw function agrees with the retained
`NON_MATCHING=1` object, and its linked range, complete camera TU, and resident
`.main` section are byte-identical to ROM. The later canonical change added
comments only, so no source/codegen search is warranted. The exact retained
whole `.bin` predates the object and is historical rather than causal proof;
fresh current-source object, link, and full-bin comparisons remain due.

`func_800320F0` (the function formerly routed under the JFG donor alias
`runlinkEnsureJumpIsValid`) has since been promoted: retained canonical C is
101 words with 21 relocations, and its linked ROM range
`0x32CF0..0x32E84` is byte-identical. It is no longer an unmatched ranking
candidate.
