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

The snapshot contains **14 queued identities**: **14 resolved measurements** and **0 unresolved identities**. Resolved target size totals **33,516 bytes (32.7 KiB)**.

Resolved rows span **4 overlays** and **1 resident TU group** (`main`).

A supplementary objdiff report was not supplied; `objdiff_match_pct` covers **0 / 14** resolved rows.

Persisted configured-TU input evidence covers **14 / 14** resolved rows. Rows without it are retained legacy or bounded-refresh measurements and must be treated as requiring reproof.

Relocation-masked mismatch evidence covers **14 / 14** resolved rows. The raw count preserves literal object differences; the masked count removes only known linker-owned fields to expose the remaining code-generation mismatch. Neither replaces linked byte-identity proof.

### Category distribution

| Category | Count | Share of resolved |
|---|---:|---:|
| `other` | 10 | 71.4% |
| `reloc-mismatch` | 1 | 7.1% |
| `size-mismatch` | 3 | 21.4% |

### Differing-word thresholds

| Threshold | Count |
|---|---:|
| raw `differing_words <= 5` | 0 |
| relocation-masked `differing_words <= 5` | 1 |
| raw `differing_words <= 10` | 3 |
| relocation-masked `differing_words <= 10` | 5 |
| raw `differing_words <= 20` | 5 |
| relocation-masked `differing_words <= 20` | 6 |

### Complete ranked queue

Rank is the persisted `functions` array order. The exact identity is
(`file`, `symbol`), so repeated symbol spellings in different translation
units remain distinct.

| Rank | File | Symbol | Overlay/TU | Category | Target bytes | Raw diff | Masked diff | Raw first | Masked first | Size delta | Objdiff% |
|---:|---|---|---|---|---:|---:|---:|---:|---:|---:|---:|
| 1 | `src/main/spranim.c` | `func_8001B798` | `main` | `other` | 700 | 6 | 6 | 244 | 244 | 0 | — |
| 2 | `src/main/anim.c` | `func_80051364` | `main` | `other` | 1,148 | 10 | 7 | 96 | 276 | 0 | — |
| 3 | `src/overlays/o020/func_overlay_020_F000038C_1876964.c` | `func_overlay_020_F000038C_1876964` | `o020` | `other` | 1,080 | 10 | 10 | 548 | 548 | 0 | — |
| 4 | `src/main/track.c` | `func_80011980` | `main` | `other` | 860 | 11 | 10 | 192 | 192 | 0 | — |
| 5 | `src/main/fx.c` | `wakeAllocate` | `main` | `other` | 1,404 | 16 | 16 | 148 | 148 | 0 | — |
| 6 | `src/overlays/o045/func_overlay_045_F0001158_188D5B0.c` | `func_overlay_045_F0001158_188D5B0` | `o045` | `other` | 2,696 | 96 | 96 | 1,816 | 1,816 | 0 | — |
| 7 | `src/main/anim.c` | `func_80054B3C` | `main` | `other` | 1,480 | 344 | 344 | 4 | 4 | 0 | — |
| 8 | `src/main/anim.c` | `func_80053868` | `main` | `other` | 4,820 | 354 | 354 | 112 | 112 | 0 | — |
| 9 | `src/overlays/o064/overlay64GenerateTexture.c` | `func_overlay_064_F0000000_18C3B28` | `o064` | `other` | 1,680 | 371 | 371 | 0 | 0 | 0 | — |
| 10 | `src/main/anim.c` | `func_800517E0` | `main` | `other` | 7,232 | 1,107 | 1,106 | 200 | 208 | 0 | — |
| 11 | `src/overlays/o047/func_overlay_047_F0000B30_1891948.c` | `func_overlay_047_F0000B30_1891948` | `o047` | `reloc-mismatch` | 8,672 | 54 | 0 | 124 | — | 0 | — |
| 12 | `src/main/matrix_2BC40.c` | `func_8002B040` | `main` | `size-mismatch` | 136 | 34 | 34 | 0 | 0 | 4 | — |
| 13 | `src/main/matrix.c` | `func_8002AA50` | `main` | `size-mismatch` | 296 | 90 | 90 | 0 | 0 | 68 | — |
| 14 | `src/main/shadows.c` | `func_80017140` | `main` | `size-mismatch` | 1,312 | 292 | 292 | 76 | 76 | 12 | — |

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
