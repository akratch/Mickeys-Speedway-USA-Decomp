# Tooling: decomp-permuter, objdiff, and the smoke test

This is reference documentation for the tools added in the `lane/tooling`
work (see `docs/acceleration-survey.md` sections 3-4 and 10 for the survey
that motivated them). For how to *use* each tool day-to-day, see
`skills/tools/permuter.md` and `skills/tools/objdiff.md` -- this file covers
setup, what's committed vs. gitignored, and why each piece is shaped the way
it is.

## decomp-permuter

Not a pip package: `~/Desktop/dev/decomp-permuter` (or wherever it's
checked out) has a `pyproject.toml` with no `[build-system]` table, only
`[tool.pyright]`/`[tool.black]` config, so `pip install -e` has nothing to
build. Instead:

```sh
ln -sfn /path/to/decomp-permuter tools/permuter   # gitignored, machine-specific
.venv/bin/pip install toml pynacl                 # its only real deps beyond stdlib
```

`toml` reads `tools/permuter_settings.toml`; `pynacl` is only used by
permuter@home (the distributed-compute mode, unused here) but `import.py`
imports it unconditionally at startup regardless. Both are pinned in
`requirements.txt`, along with the exact decomp-permuter commit this was
proven against (`requirements.txt`'s comment, since the checkout itself
isn't a package with its own version string).

### permuter_settings.toml: why the compiler command is hardcoded

`import.py`'s default `build_system = "make"` mode runs `gmake
--always-make --dry-run --debug=j PERMUTER=1 <target>` and parses the one
debug-trace line containing the source file to recover the compiler
invocation (`fixup_build_command()` in decomp-permuter's `import.py`). That
assumes the recipe is one line. This project's C rule is a shell line
*continuation*:

```make
$(BUILD_DIR)/%.c.o: %.c ... | $(ALL_DIRS) $(SPLAT_STAMP)
	$(ASM_PROCESSOR) $(CC) -- $(AS) $(ASM_PROC_ASFLAGS) -- \
		-c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<
```

`make --debug=j` echoes this as two physical lines (the trailing `\`
survives, and the tab-indented continuation prints separately). Only the
second line contains the `.c` path, so `import.py`'s line-scan finds a
"compiler" of just `-c ...` with `tools/ido/cc` missing, and fails with
`-c: command not found`. Verified by hand:

```sh
gmake --always-make --dry-run --debug=j PERMUTER=1 build/src/libultra/<f>.c.o
```

The fix, matching what `tools/permuter_settings.toml`'s own comment
documents in full: set `compiler_command`/`assembler_command` directly,
bypassing `find_build_command_line` entirely. Candidates the permuter
mutates are plain C with no `#pragma GLOBAL_ASM`, so asm-processor's
split-and-recombine step is a no-op for them -- calling `tools/ido/cc`
directly, with the same flags asm-processor would have passed through, is
equivalent. The `assembler_command` also deliberately omits
`include/asm_processor_prelude.inc`: that file and decomp-permuter's own
`prelude.inc` (always prepended to `target.s` by `import.py`) both define
`glabel`/`endlabel`/etc. as `.macro`, and assembling both together is a
"Macro already defined" error.

The hardcoded flags are the libultra-default group (`-O2 -mips1 -32`); the
toml's header comment lists the other three groups this Makefile defines
(overlay/`src/main` game code at `-O2 -mips2 -32`; two other libultra
sub-groups) and how to retarget for them.

### Proof: `__osContRamRead`

Ran end-to-end against `src/libultra/contramread.c` /
`asm/nonmatchings/libultra/contramread/__osContRamRead.s` (still
`#pragma GLOBAL_ASM`; a from-scratch candidate C body was written for the
test, adapted from Jet Force Gemini's published `libultra/src/io/contramread.c`
with a `PROVENANCE` note, per `docs/CLEANROOM.md` -- the same pattern
`src/libultra/pfsgetstatus.c` already uses for a sibling function -- then
reverted afterward; this was a tooling proof, not a matching attempt):

```
base score = 1330
best score (3-minute cap, -j 12, --stop-on-zero): 1055
```

No exact match (not required -- the point was a working loop). The winning
mutation joined a `for` loop's body onto one line, which is exactly the
kind of `perm_sameline`-shaped move IDO's scheduler is sensitive to and a
human would otherwise have to guess at.

### Check the scratch's word count before believing a base score, 2026-09-09

`permute.sh` fixes the *flags*, which is necessary and not sufficient: the
scratch is still an isolated single-function translation unit, and on
`src/main/models_5B300.c` it compiles `func_8005ABA8` at 112 words against the
real per-TU object's 111. Its reported base score of 400 was therefore a false
reading for a candidate whose real residual was two register names, and
nothing measured there would have transferred.

The cheap check takes one command: compile `build/permuter/<fn>/scratch/base.c`
with the flags from that scratch's own `compile.sh` and count the function's
instructions against the real object. If the counts differ, the scratch is
unfaithful for that TU and its scores are not evidence. On
`src/main/anim.c` the same check passed exactly (80 words, the same residual
as the full-TU build), and the run that followed found the two-carrier head
that took `func_8005716C` from 44 differing words to 10 -- so the check
separates a useful run from a wasted one rather than condemning the tool.

Note also that a permuter win is a *lead*, not a candidate: the same run's
best output reached its score through an uninitialised self-add inside a
`do {} while (0)`. Re-derive the effect as a semantics-preserving edit, as
`docs/ido-learnings.md` records for that function.

## objdiff

`tools/setup_objdiff.sh` fetches the `objdiff-cli` binary from
[encounter/objdiff releases](https://github.com/encounter/objdiff/releases)
(macOS arm64 asset `objdiff-cli-macos-arm64`) into `tools/objdiff/`
(gitignored, like `tools/ido/`/`tools/binutils/`) and records the resolved
version in `tools/objdiff/VERSION`. Proven against v3.8.0.

objdiff diffs *object files*, base vs. target. The target ("expected") side
here is a snapshot of a previously `gmake verify`-clean `build/` --
following DKR's/dp64's `expected/build/...` convention (dp64's own
`objdiff.json`, read for schema reference, uses exactly this shape:
`target_path`/`base_path` pairs per unit) -- not the baserom directly, since
objdiff needs linked, sectioned object files, not a ROM binary blob.
`tools/make_expected.sh` runs `gmake verify` and then `cp -R build
expected/build`; re-run it whenever `build/` changes, or the report compares
against a stale target.

`objdiff.json` (committed) is derived, not hand-edited, by
`tools/objdiff_config.py` (`gmake objdiff-config`) from `mickey.us.yaml`
(every `c` subsegment), the overlay atlas `config/overlays.us.json`
(each module's C sources), and the split tree (`asm/**/*.s`,
`assets/**/*.bin`, found the way the Makefile finds them). It does not read
`build/`, so it works before anything is compiled. C units carry a
`base_path`, `source_path`, `complete` (false while a `#pragma GLOBAL_ASM`
remains) and `scratch.c_flags` taken from a `gmake -nB` dry run, so per-TU
overrides are included. Assembly-only units carry no `base_path`, so objdiff
scores them 0 and does not count them as decompiled. Progress categories are
`resident`, `overlay`, `libultra`, plus `overlay-oNNN` per module. A `.c`
under `src/` that neither the yaml nor the atlas names is an orphan and gets
no unit. `gmake check-objdiff-config` fails on drift and is part of
`tools/gates.sh --promotion`. `tools/gen_objdiff_config.py` remains as a
wrapper that prints the config with the local exclusion list applied, for
`tools/objdiff_report.sh` and `tools/nm_ranking.py`; the report script
restores the canonical file when it exits.

### The trimmed-object exclusion list

686 of this project's ~832 C objects carry a Makefile `POSTPROCESS`
override -- overwhelmingly `trim_elf_section.py` (IDO aligns standalone
`.text` to 16 bytes; many of these reviewed overlay functions continue at a
4-byte boundary inside a larger module, so the trimmer shortens the section
header after the fact) or `normalize_elf_instructions.py` (patches
individual instruction words post-compile; see the Makefile's own comments
on both). objdiff-cli's `report generate` aborts its *entire* batch on the
first object it can't parse as a result -- `Section symbol without section`,
or for some objects, an unattributed `Symbol data out of bounds` with no
file name in the error at all, `-L debug` included.

Given that, per-object bisection to find every offender isn't worth it:
`tools/objdiff_report.sh` regenerates `tools/objdiff_exclude.txt` fresh on
every run with the full `POSTPROCESS`-override list:

```sh
grep -oE '^\$\(BUILD_DIR\)/\$\(SRC_DIR\)/[A-Za-z0-9_/]+\.c\.o: POSTPROCESS' \
    Makefile | sed -E 's#\$\(BUILD_DIR\)/\$\(SRC_DIR\)/#src/#; s/: POSTPROCESS$//' \
    | sort -u > tools/objdiff_exclude.txt
```

Deliberately gitignored rather than committed, unlike `objdiff.json`: it is
~700 lines of nothing but object-file basenames, no punctuation, no other
structure -- which `tools/cleanroom_detectors.py`'s base64-volume heuristic
reads as high-entropy text (834.5 base64-shaped chars/KiB against its
400/KiB threshold) even though every byte in it is a filename copied from
the Makefile, not ROM content. A real false positive on this specific file
shape, not a loophole worth routing around with a `CONTENT_EXEMPTIONS`
entry when "don't track the derived file, regenerate it" is simpler and
correct on its own merits regardless of the detector.

`tools/objdiff_report.sh` also retries with a newly-discovered offender
excluded on any *attributable* failure (`Failed to open ... .o`), as a
defensive fallback -- but the unattributed failure mode means this can't be
fully automatic. **This is a known scope limit**: objdiff currently reports
on the un-postprocessed objects only (719 of 1405 units as of this writing),
which is still most non-overlay code plus the overlay functions that don't
need trimming/normalization. Extending coverage to the rest would mean
teaching objdiff-cli's ELF reader about this project's post-linked object
shapes, which is out of scope for this lane.

### Proof: current build

```sh
gmake -j8 && gmake verify && ./tools/make_expected.sh && ./tools/objdiff_report.sh
```

719 units, 414 with nonzero code size, 711156/711156 bytes matched (100%) --
expected, since `expected/build/` was snapshotted from the same `build/`
being diffed. The report becomes informative once `expected/build/` is
refreshed from an *older* verified build (regressions), or a future
alternate target (a donor object, a different flag group) is compared
against the current one via a second `-2 <base>` pointed elsewhere with
`objdiff-cli diff` directly.

## mapfile_parser

`pip install mapfile_parser` (pinned `2.13.2` in `requirements.txt`).
Underlies decomp.dev-style progress reporting elsewhere in the splat/objdiff
ecosystem; not itself wired into a script here, but smoke-tested by
`tools/check_tools.sh` since it's a `gmake setup` dependency going forward.

## Function evidence preflight and workbench comparison

Run the evidence preflight before a flag sweep or allocator experiment:

```sh
tools/function_preflight.py overlay16ApplyGradient
tools/function_preflight.py func_overlay_016_F00001E0_1873678 --json
```

Either the friendly C name or splat's generated overlay name resolves to the
same identity. The command incrementally builds the canonical linked ELF and
the correct full-TU candidate object, automatically selecting
`build_non_matching/` for a guarded candidate. Each build uses the Makefile's
separate split and target phases, so regenerated assembly cannot be hidden by
Make's same-invocation timestamp scan. It then reports the exact
owned range and next ownership/padding boundary, ROM-table exports, inbound
call sites, and the candidate declaration and frame. For overlays it reports
every shipped runtime relocation record; for resident functions it reports
the authenticated canonical object's static relocation tuples separately from
the sparse resident startup-table records, where zero records is valid. It
then reports candidate static-relocation agreement and the current workbench
word score and first mismatch. Before those measurements it prints a concise
Git history of commits that materially changed this exact guarded function.
The history compares function token streams with each commit's parent, so
other-function edits in the same TU and comment/whitespace-only churn are not
reported. JSON consumers receive the same hash/subject rows in
`source_history`; no historical source or generated instruction text is
emitted.

`--no-build` makes existing artifacts a hard requirement. Before any
comparison, both the full-TU object and canonical linked ELF must be current
according to Make's real dependency graph. Preflight also checks the root
Makefile and checked-in policy/normalization fragments explicitly, because
Make does not age an output merely because its recipe changed. Ordinary
preflight refreshes missing or stale artifacts with separate low-priority
split and target invocations. Their job count defaults to the machine's CPU
count (one if unavailable), following ADR 0004. Set `MICKEY_BUILD_JOBS` to a
positive decimal integer for an explicit workstation or crew limit; malformed
values fail before a build starts. Both phases use the same resolved count.
When recipe/policy timestamp drift is
detected, the target phase uses Make's always-build mode so old objects cannot
survive the changed recipe. `--no-build` instead fails closed with an
actionable diagnosis. The command also fails when an alias, source, range, or
relocation identity is not unique; its output deliberately excludes
instruction listings, words, and hexdumps. For consolidated overlay TUs, a
candidate that has been shifted by earlier guarded bodies still fails closed,
but the error reports the candidate object extent, linked target extent,
prefix-size drift, and owner overrun. If a shared-TU definition's compiled
location disagrees with its generated alias identity, the error reports both
identities and their delta rather than only saying that the relocation symbol
is ambiguous. `wb_compare.sh` remains available for scalar source-shape
diagnostics; neither message authorizes relocation or ownership inference.

One narrower case preserves otherwise-valid analysis evidence. When the
candidate relocation surface is measurable but one or more static relocation
names have no provable runtime identity, including a name with multiple
possible identities, the report gains a small additive
`preflight` object. Its stable ingestion fields are `status` (`complete` or
`partial`), `action` (`continue_matching`,
`resolve_candidate_static_relocation_identities`,
`restore_linked_runtime_identity_proof`, or `run_promotion_proof`), named
relocation `counts`, and bounded `diagnostics`. `complete` means evidence
collection is complete, not that the function matches. Each unresolved
diagnostic carries the candidate-relative offset and relocation type; it never
supplies or guesses an identity. An ambiguous name remains unresolved at every
affected site; the tool never chooses among its candidates. The existing
`workbench` object still carries
target/candidate/matched word counts, frame sizes, verdict, and first mismatch.

Partial status is explicitly non-exact. Normal proof mode prints the full
human or JSON report and exits 1; hard preflight errors still exit 2 without a
report. `--analysis-only` changes a partial report's exit to 0 for experiment
ledger ingestion but leaves its status, action, error diagnostic, and missing
identity unchanged. A post-promotion report remains complete only when the
existing linked-ROM/runtime-table path proves exact effective identities; the
promotion proof wrapper also independently rejects any preflight status other
than `complete`.

For same-overlay data proxies, preflight may resolve an otherwise anonymous
name from a different canonical function only when that sibling is an exact,
function-sized atlas owner with fresh object/link evidence, linked-ROM byte
identity, and one unambiguous static/runtime relocation tuple. This adds the
identity to the candidate record but never moves its offset: a schedule-shifted
HI16/LO16 pair still fails the independent offset/type comparison. Conflicting
or merely nonmatching sibling evidence remains unresolved.

An undefined overlay proxy can also be bound directly from its owned shipped
runtime surface, but only for complete HI16/LO16 pairs. The candidate TU and
target range must already resolve to one atlas overlay owner; each pair must
align by function-relative offset and relocation type with one unique runtime
tuple; and every use of the proxy must derive the same stable overlay-selector
and base offset after removing the candidate REL addend. This is not a shared
synthetic-VMA inference: real overlays and reserved selectors remain distinct,
and duplicate, unpaired, missing, or conflicting tuples fail closed. Exact
promotion still independently requires exact code bytes, relocation shape and
identity, the linked owner/module range, and the full ROM.

The exact-sibling route also applies to an anonymous `R_MIPS_26` call proxy.
The sibling must supply one static call relocation at the same site as one
shipped runtime tuple; the proof subtracts the object's REL addend and records
only the resulting base identity. It does not rewrite the candidate call site,
infer through a missing tuple, or choose between conflicting sibling targets.

For an undefined same-overlay `R_MIPS_26` proxy with no sibling witness,
preflight also accepts a repeated-use proof. At least two distinct candidate
sites must name the same undefined call-only proxy, every site must align with
one unique shipped tuple for the authenticated owner overlay, and removing each
candidate REL addend must independently produce the same base. Single-site,
missing, mixed-type, resident, reserved-selector, and cross-overlay evidence
stays unresolved; duplicate or conflicting same-overlay evidence is ambiguous.
This route does not infer from the shared synthetic
VMA and does not weaken the independent relocation-shape, linked-overlay, or
full-ROM promotion gates.

Promotion does not make the preflight unusable when splat removes the
function's `asm/nonmatchings` fallback. With no fallback present, the resolver
enters `post_promotion` mode only for one unconditional requested C definition
with no matching `GLOBAL_ASM`. It then requires one agreeing tracked exact
identity: an overlay's friendly/generated alias plus either an exact
`text_ownership` row or function-sized `mixed_tu_exact_c_ranges` row in the
overlay atlas, or a resident symbol's single `type:func`, size-bearing
`matched C` row in `symbol_addrs.us.txt`. The tracked source and range must
agree with the unique definition and the linked ELF's value and size. Missing,
stale, conflicting, or merely C-looking evidence fails closed; in particular,
a guarded `NON_MATCHING` body cannot enter this path just because its extracted
fallback is absent. Post-promotion reports use the ordinary `build/` object and
automatically obtain their scalar score from the fully relocated ROM oracle.
This can reconfirm exactness but does not provide the relocation diagnostics of
the pre-promotion assembly comparison. JSON reports expose the distinction as
`resolution_mode` and `workbench.comparison_mode`.

After promotion, reduce that detailed report to one strict proof receipt with:

```sh
tools/promotion_proof.py overlay41SpawnItems
gmake promotion-proof SYMBOL=overlay41SpawnItems
```

`promotion_proof.py` runs and consumes the existing preflight JSON; it does not
reimplement symbol, ROM, or relocation analysis. It passes only when the
requested function resolves through `post_promotion`, uses the linked-ROM
oracle, has identical nonempty word counts with zero differing words, has an
identical frame (including the valid no-frame leaf case), and has exact
relocation counts, offsets/types, and effective runtime identities. The output
is a compact receipt containing only those proof totals and modes. Use
`--no-build` to require already-fresh preflight artifacts.

For a canonical integration proof, append `--canonical` (or set
`PROMOTION_PROOF_ARGS=--canonical` on the Make target). After the function
receipt passes, this runs `gmake verify` and `gmake check-overlay-syms` as two
explicit, sequential `nice -n 10`, `-j2` commands. This proves the full ROM and
the tracked overlay relocation surface without writing shared generated
artifacts. `--json` keeps the final receipt machine-readable and sends the
canonical commands' progress to standard error.

`tools/wb_compare.sh` uses the same resolver, so manual
`WB_CANDIDATE_SYMBOL`/`WB_CANDIDATE_BUILD_DIR` settings are no longer needed
for normal guarded functions:

```sh
tools/wb_compare.sh overlay16ApplyGradient --json
tools/wb_compare.sh --diagnose overlay16ApplyGradient --trace trace.log --trace-proc 0
tools/wb_compare.sh --no-build overlay16ApplyGradient --json
```

Wrapper options precede the symbol; all arguments after the symbol are passed
unchanged to `decomp-workbench compare` or, with `--diagnose`, to
`decomp-workbench diagnose`. `--rom` retains the linked-ROM final-oracle mode,
and can be combined with `--diagnose` to select `diagnose-dumps`. Invoke
`wb_compare.sh --rom <linked-C-name>` directly after promotion; ordinary
`function_preflight.py` chooses that mode automatically once its tracked
post-promotion checks pass. In assembly mode the wrapper performs the same
automatic two-phase refresh as ordinary preflight. Use wrapper-level
`--no-build` only when a diagnostic or test must prove existing evidence is
already fresh without compiling.

`--summary-json` emits the concise `mickey-wb-summary-v1` checkpoint input.
When its proof manifest's source, full-TU candidate object, and target artifact
still match their recorded hashes, the report carries one explicitly named
`relocation_surfaces` member:

- `fallback_static` for an assembly-fallback comparison; or
- `promoted_linked` for a ROM comparison whose promoted source/object owner,
  canonical linked ELF, and retail runtime surface can all be authenticated.

Each surface records its `evidence_mode`, candidate and target counts,
offset/type count, resolved candidate identities, exact candidate-to-target
identities, and computed `complete` state. Completeness means the whole
candidate/target shape is aligned and every candidate identity is resolved; it
does not mean every identity is equal. No field from one mode is inferred for
the other. Friendly requested names are valid artifact owners: the artifact
filename uses that requested spelling, while its object contents remain
authenticated against the generated target symbol.

The existing top-level `relocations` object remains unchanged for single-state
consumers. Its scalars are `candidate_relocations`, `target_relocations`, and
`exact_relocation_identities`; the last counts exact candidate-to-target
runtime identity alignments, not merely resolved candidate records. Missing
proof inputs omit all relocation fields. Malformed, stale, conflicting, or
ownership-inconsistent evidence fails closed.

Capture the two modes before committing the promotion, while the lane's HEAD
and branch still identify the same assignment base, then compose them without
transcribing counts:

```sh
tools/wb_compare.sh --summary-json symbol > build/wb/symbol.fallback.json
# Promote and rebuild the same C body, but do not commit or change branches yet.
tools/wb_compare.sh --rom --summary-json symbol > build/wb/symbol.promoted.json
tools/function_preflight.py --compose-relocation-summaries \
  build/wb/symbol.fallback.json build/wb/symbol.promoted.json
tools/function_preflight.py --compose-relocation-summaries \
  build/wb/symbol.fallback.json build/wb/symbol.promoted.json --json \
  > build/wb/symbol.paired.json
```

Composition accepts only fresh regular files under `build/`, requires distinct
fallback-static and promoted-linked modes, verifies each receipt's
source/object/target hashes, symbol identity, assignment base, branch, source
owner, and boundary, and rejects duplicate surfaces. Human output prints both
surfaces side by side. In paired JSON, the compatibility `relocations` object
deliberately remains the fallback-static view, so linked evidence cannot create
a false object-exact checkpoint. This is evidence reporting only; promotion
policy and the canonical ROM/relocation gates are unchanged.

## tools/check_tools.sh

Runs each tool's `--version`/`--help` and prints one line per tool:

```sh
./tools/check_tools.sh
```

Covers splat, spimdisasm, asm-differ, m2c, mapfile_parser, toml,
decomp-permuter (skipped with a note if `tools/permuter` isn't linked),
objdiff-cli (skipped if not fetched), and the gitignored IDO/binutils
binaries (skipped if `gmake setup` hasn't run). Exits nonzero if anything
that *is* present fails to start.

When the workbench, baserom, and an existing build are present, the smoke test
also runs `tools/wb_compare.sh --rom` on a matched overlay function; the
wrapper maps resident and overlay symbols to ROM offsets from their ELF
section VMA/LMA pairs and writes its retained dumps only under ignored
`build/wb/`. The default candidate is `build/`; set `WB_ROM_BUILD_DIR` to an
alternate build directory to compare a linked `NON_MATCHING=1` diagnostic ROM
without replacing the verified build tree.

## Overlay atlas release deltas

`python3 tools/overlay_atlas.py --delta` makes an overlay scoreboard
reconciliation reviewable without rebuilding or checking out the old tree.
With one state it compares that base with the current worktree atlas; with two
states it compares them directly:

```sh
python3 tools/overlay_atlas.py --delta HEAD^
python3 tools/overlay_atlas.py --delta release-base release-candidate
python3 tools/overlay_atlas.py --delta old-atlas.json ../candidate-checkout
python3 tools/overlay_atlas.py --delta release-base release-candidate --format json
```

A state may be a Git tree-ish, an atlas JSON file, or a checkout containing
`config/overlays.us.json`; an existing filesystem path takes precedence over a
same-named ref. The report lists exact-C promotions and retractions, their
individual byte ranges, gross byte totals, and the net exact-C change. JSON
output is schema-versioned and keeps overlay numbers, offsets, extents, and
sizes numeric for release automation.

Identity is always `(overlay, text, offset)`, never a shared synthetic VMA or
a source filename. The command refuses duplicate overlay rows, duplicate or
overlapping exact-C identities, inconsistent row sizes or atlas totals, and a
same-key extent that changed between states. This is deliberately fail-closed:
fix or explain the atlas boundary instead of allowing a release report to
guess whether one range was promoted, retracted, split, or renamed. The delta
audits atlas state transitions; the ordinary exact-object, linked-ROM, and
scoreboard gates remain the proof that a promotion is valid.

## Deterministic public-release reconciliation

`tools/public_release.py` is the final, push-incapable release preflight for a
checked-out publication branch. Both the branch and remote must be named on
the command line; the tool confirms the current branch, both remote URLs, the
local remote-tracking ref, and that the release is a fast-forward from that
ref. It deliberately does not fetch, merge, copy from another checkout, or
push. Fetching the comparison ref and publishing a proven commit remain
separate human actions.

The default is a read-only dry run:

```sh
gmake public-release \
  PUBLIC_RELEASE_ARGS="--remote public --branch master"
```

It regenerates the overlay atlas and post-process report in memory and checks
the donor digest, then composes the serial health-gated `verify`, tooling,
clean-room, documentation, scoreboard, and overlay-symbol checks. It scans
the complete resulting tree, every tree in the outgoing commit range, and
every outgoing commit message. Scanning every intermediate tree matters: text
introduced by one outgoing commit and deleted by a later one would still be
transferred. Operator-only paths, local absolute paths, release credentials,
and automated generator/co-author trailers fail closed. Untracked local setup
is ignored; tracked worktree or index changes are rejected.

Campaign maintainers with the out-of-tree donor farm can opt into an earlier
farm check:

```sh
gmake public-release \
  PUBLIC_RELEASE_ARGS="--remote public --branch master --check-reference-builds"
```

This runs `tools/verify_reference_builds.sh` before any reconciliation
generator, so a missing, stale, or locally divergent lock-pinned reference
build fails before donor-derived artifacts are considered. It checks every
title in `tools/reference-builds.lock`, including the canonical DKR and JFG
farms, and honors the verifier's existing `REFS_ROOT` environment override.
The option is deliberately off by default because ordinary public
contributors do not have the external farm. It can be combined with
`--write-derived` when a maintainer is intentionally refreshing generated
release artifacts.

The report compares the freshly generated scoreboard with the named
remote-tracking branch and prints exact numeric deltas. Overlay promotions and
retractions are additionally derived by interval-diffing the two canonical
atlases and are reconciled against `matched_overlay_c_bytes`; each changed
range is reported by overlay, offset, byte count, and owning source. Resident
changes are available as exact scoreboard metric deltas because the overlay
atlas does not own resident function boundaries.

If generated files actually need refreshing, opt in explicitly:

```sh
gmake public-release \
  PUBLIC_RELEASE_ARGS="--remote public --branch master --write-derived"
```

Write mode invokes only these existing public-safe generators, in order:

1. `overlay-atlas-write`;
2. the atlas digest refresh for the donor ledger;
3. `postprocess_audit.py --write`;
4. `overlay-syms`;
5. `scoreboard`.

It runs the same gates and scans against the generated worktree, but leaves
all changes unstaged for review. Commit only reviewed generated changes, then
rerun the default clean dry run. Neither mode contains a publication command;
the final success line always says `push=disabled`.

### Reviewed manifests across rewritten histories

The source and publication histories intentionally have different commit
identities. A source maintainer can create a scalar release manifest without
making the tool copy files or infer a relationship between those histories:

```sh
python3 tools/public_release.py \
  --remote SOURCE_REMOTE --branch SOURCE_BRANCH \
  --manifest-range LAST_RELEASED_SOURCE..HEAD
```

Manifest mode requires a clean checked-out source branch whose configured
upstream, named remote-tracking ref, and `HEAD` are identical. The range must
be exactly `BASE..TIP`, the base must be an ancestor, and the tip must be that
current `HEAD`. Merge commits, empty commits, and unresolved or ambiguous
endpoints are rejected. This makes every selected commit an integrated,
ordered source commit instead of an unreviewed side-branch object.

The deterministic output is
`build/public-release/manifest.json`. It contains one public commit unit per
source commit, retaining the commit's changed-path group and order. Each row
records status, old and new path where applicable, regular-file modes, exact
old/new Git blob identities, and one of these policy classifications:

- `public-safe`: a regular add or modification in a known public project path;
- `review-required`: an unknown path, deletion, rename/copy, or regular-file
  mode change that needs an explicit reviewer decision;
- `forbidden`: operating state or a ROM-derived, toolchain, scratch, or build
  artifact. A forbidden row stops generation and is never approvable.

The manifest also records the four canonical source/public gates on every
unit, adding the tooling and overlay-symbol gates when their paths make those
checks applicable. It records no file contents, diffs, disassembly, remote
URLs, timestamps, credentials, or workstation paths. Commit and blob IDs,
message digests, path/status metadata, gate names, and a SHA-256 seal are the
only release evidence. Source ref names and remote destinations are represented
only by SHA-256 identities. Re-running the command against unchanged refs
produces byte-identical JSON.

Generation also creates
`build/public-release/manifest.approval.json` once, with every path decision
set to `pending`. The tool preserves an existing approval file. A human reviews
each commit unit, its classification, path transition, and blob identities,
then changes only accepted path decisions to `approve` and chooses the plain
one-line `public_subject`. Validate that reviewed file against freshly derived
source evidence before recreating anything:

```sh
python3 tools/public_release.py \
  --remote SOURCE_REMOTE --branch SOURCE_BRANCH \
  --manifest-range LAST_RELEASED_SOURCE..HEAD \
  --approval build/public-release/manifest.approval.json
```

Approval is exact, not a glob. Missing, extra, reordered, stale, or altered
path/blob/mode rows fail, as do a stale manifest seal or changed source/remote tip.
Publication filters run over each selected commit message and each changed
text blob without echoing matched credentials. Absolute/noncanonical paths,
operating-only paths, binary/build artifacts, symlinks, gitlinks, automated
trailers, and unsafe messages stop the process before approval.

After the operator separately recreates those units on the publication branch,
place the two scalar JSON files in that checkout's ignored
`build/public-release/` directory and run the structural export check:

```sh
python3 tools/public_release.py \
  --remote public --branch master --validate-export \
  --approval build/public-release/manifest.approval.json
```

This intentionally does not require shared ancestry. It pairs the outgoing
publication commits with approved units in order and requires each commit's
plain subject and exact status/path/old-blob/new-blob/regular-mode transition
to agree.
Different commit IDs are expected; different file blobs are not. Then run the
ordinary read-only public-release preflight above to execute every recorded
gate and scan the complete resulting publication trees.

Manifest, approval, and export-validation modes never fetch, merge,
cherry-pick, copy source files, stage, commit, or push. They do not decide
whether a change is useful, prove a C match, replace clean-room review, or
prove the destination gates passed. The approval records what a reviewer
accepted; the export validator proves only that the manually recreated commit
units have those exact regular-file transitions.

## Map: the rest of the toolbox

The tools above (decomp-permuter, objdiff, mapfile_parser, check_tools.sh)
have their own detailed sections because this file started as their setup
doc. Everything below just points at where its own documentation lives —
this file is a map, not a manual, for the rest.

| Tool | What it does | Documented in |
|---|---|---|
| `tools/skeleton_scan.py` | Masked-instruction-shape ("skeleton") matching against the reference farm: finds a donor whose bytes changed but whose structure didn't, which the exact-match `find_known_objects.py` cannot do (ADR 0007). Prints candidates only; never writes ROM bytes to a file. | [`docs/skeleton-scan.md`](skeleton-scan.md) |
| `tools/flag_sweep.py` | Compiles one candidate under the full known compiler-flag lattice and ranks by objdiff score, before any hand permutation is attempted (ADR 0007). | [`docs/flag-sweep.md`](flag-sweep.md) |
| `tools/function_history.py` | Lists only commits that materially changed one exact guarded C function, excluding unrelated same-TU commits and comment/whitespace churn; preflight embeds the same hash/subject rows. | [Function evidence preflight](#function-evidence-preflight-and-workbench-comparison) above |
| `tools/overlay_graph_match.py` | Structural overlay-to-module matching against Jet Force Gemini by size, function count, and call graph, since byte identity mostly returns nothing against a differently-revised source tree. Writes `config/overlay-graph.us.json`. | [`docs/overlay-graph.md`](overlay-graph.md) |
| `tools/overlay_atlas.py --delta` | Audits exact-C overlay promotions, retractions, and net byte changes between refs, manifests, or checkouts using fail-closed `(overlay, offset)` identities; `--format json` is machine-readable. | [Overlay atlas release deltas](#overlay-atlas-release-deltas) above |
| `tools/permute.sh` | One bounded decomp-permuter run for one function: locates its C file and target `.s` (regenerating the target from the baserom via a temporary `GLOBAL_ASM` swap if the function already has a C body), imports both, and runs `permuter.py` under a wall-clock cap. Batch-only per ADR 0007 — never run inside an agent's own turn-by-turn reasoning loop. | this file, `## decomp-permuter` above |
| `tools/finalize_plateau.py` | Validates and preserves one guarded candidate, appends symbol-keyed EOF metadata without moving measured source lines, and writes a conflict-free `docs/matching-triage-handoffs/<symbol>.md` shard by default; explicit custom ledgers must already be tracked. | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Safe plateau finalization` |
| `tools/plateau_handoff_audit.py` | Audits structured source plateau markers against exact-symbol shards; `--check` reports drift and `--write` atomically reconciles only valid missing/stale shards without deriving metrics from prose or ROM data. | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Safe plateau finalization` |
| `tools/new_lane.sh`, `tools/merge_lane.sh`, `tools/codex_lane.sh` | Create, integrate, and (for Codex) launch a deadline-aware worker in an isolated lane worktree. | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Lane helpers` |
| `tools/integration_base.py` | Selects the newer linearly related local/remote campaign integration ref, fails on divergence, and prevents workers from silently starting on a stale checked-out branch. | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Lane helpers` |
| `tools/lane_status.py --symbols` | Screens many targets under one shared evidence scan and prints `verdict<TAB>symbol<TAB>source` per line, exiting 0 only when every symbol is assignable. A per-symbol `--symbol` call rebuilds the whole lane index, so screening a translation unit that way costs minutes and gets skipped; a 28-function unit screens in ~28s this way against ~7min sequentially. Accepts a comma list or `-` for one symbol per line on stdin. | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Lane helpers` |
| `tools/lane_status.py` | Read committed `lane/*` refs without opening sibling worktrees; reports advisory match claims and gives `--symbol` a fail-closed assignment verdict from exact source identity, batched descendant target-guard comparisons, plateau handoffs, per-symbol shards, and legacy target-specific triage history. Complete queue scans reuse one source-identity and lane-path index (ADR 0011). | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Lane helpers` |
| `tools/assignment_packet.py` | Composes one assignment packet for a queued target from the tree instead of by hand: the fail-closed assignment verdict, the retained configured measurement, the committed plateau handoff, the reopen authorization verbatim, and the workbench playbook its measured mismatch class routes to. Prints counts, offsets and paths only, and cites `decomp-workbench guide` rather than inlining lever prose, so the field guide stays the single source of truth. | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Lane helpers` |
| `tools/merge_lanes.sh` | Integrates several lane branches under one run of the heavy gates, using the same conflict rules and per-commit hooks as `merge_lane.sh`. A wave of finished lanes otherwise pays a full extract/build/verify per lane to answer one question about the combined tree. The tradeoff is attribution: a failing batch does not name the lane, so it prints the pre-batch commit for `git reset --hard` recovery and one-at-a-time retry. Integrate a lane carrying a new match on its own. | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Lane helpers` |
| `tools/fix_stale_externs.py`, `tools/refresh_atlas_digest.py`, `tools/resolve_modules_split.py` | Post-merge/integration housekeeping: stale `func_<VRAM>` externs, a stale atlas digest, and the `docs/modules.md`/`docs/overlays.md` split conflict. | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Integration housekeeping` and `## docs/modules.md / docs/overlays.md split` |
| `tools/postprocess_audit.py` | Classifies every object's `POSTPROCESS` build step as `altered` (forbidden, ADR 0002) or `metadata` (permitted); the mechanical check behind the scoreboard's decompiled line. | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Auditing post-compile steps` |
| `tools/public_release.py` | Regenerates/checks public-safe derived artifacts, emits and validates reviewed exact-blob release manifests across rewritten histories, reports exact release deltas, scans every outgoing tree/message, and composes all release gates. It never publishes or copies source files. | this file, `## Deterministic public-release reconciliation` |
| `tools/run_logged.py` | Runs one build command with complete output under `build/`, prints one compact PASS/FAIL receipt, and shows only a bounded tail on failure. Verification and progress/scoreboard targets use it for their noisy build prerequisites. | `gmake verify`, `gmake progress`, `gmake scoreboard` |
| `tools/reloc_identity.py` | Gives preflight and relocation-surface proof one fail-closed parser and canonical identity model for linker aliases, objcopy rename chains, addends, and ambiguity. | [`docs/reloc-surface.md`](reloc-surface.md) |
| `tools/experiment_ledger.py` | Appends compact, non-ROM-derived attempt metrics to an immutable local JSONL journal under ignored `build/`, then lists, ranks, or summarizes them without replacing canonical proof. | [`docs/experiment-ledger.md`](experiment-ledger.md) |
