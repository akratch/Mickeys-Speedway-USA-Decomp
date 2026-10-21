# Overlay consolidation: ADR 0006 pilot recipe and measurements

Status: pilot complete for overlays 39, 77, 85, 95; post-consolidation
match-loss audit complete for the 13 affected mixed translation units;
initialized-data ownership wired for overlay 77.
Written for whoever (human or agent) applies ADR 0006 to other overlays.

## Scope of the pilot

ADR 0006 picked overlays 50, 52, 54, and 101 to go first because they share
code shapes (the n-gram measurement in `docs/acceleration-survey.md` §9).
This pilot instead took the four smallest fully-matched, no-instruction-editing
overlays -- 39, 77, 85, 95 -- to prove the mechanical recipe cheaply before
spending it on the harder, larger targets. Selection criteria, checked
against `config/postprocess-audit.us.json` before starting:

- every function in the overlay already matched (`matched_c: true` for
  every row; `c_owned: true` alone is insufficient because mixed and guarded
  TUs remain source-owned C while their fallback bytes are uncredited)
- no row in the overlay has `"class": "altered"` -- i.e. no
  `normalize_elf_instructions.py`, `normalize_o63_*.py`,
  `resize_elf_function.py`, or `extend_elf_function_to_text.py` in its
  POSTPROCESS recipe (ADR 0002 forbids adopting those as part of a
  consolidation; they'd have to convert to `NON_MATCHING` first, which is
  out of scope for a layout-only pilot)

All four overlays qualified: every POSTPROCESS row for them was
`trim_elf_section.py` only.

## The recipe

1. **Read every function's C file and the atlas's `TEXT_SUBSEGMENTS[overlay]`
   entry** (`tools/overlay_atlas.py`) to get the ROM order and each
   function's offset/size. Function order in the new single file must match
   ROM order; splat's `text_ownership` regenerates offsets from the file's
   actual compiled output, but starting from the wrong order changes which
   function symbol lands at which address.

2. **Check every function's own `POSTPROCESS`/`CFLAGS`/`OPT_FLAGS` line in
   `mk/overlays.mk`.** This is the step that decides whether the overlay folds
   into one TU or has to stay split:
   - If every function shares identical `CFLAGS`/`OPT_FLAGS` overrides (or
     none), the module folds into exactly one TU. `overlay_039` (both
     functions used `-O2 -Wo,-loopunroll,0`, unchanged in the merge) and
     `overlay_095` (no overrides at all) fell into this case.
   - If some functions carry an override and the merge with an override-free
     neighbor changes that neighbor's compiled size, the module cannot fold
     into one TU without moving bytes off the ROM (ADR 0002 forbids editing
     compiled instructions to force a match). `overlay_085` merged cleanly
     because *both* of its functions already carried `-Wab,-r4300_mul`.
     `overlay_077` did not: `overlay77Init` and `overlay77Update` both
     needed `-Wab,-r4300_mul`, but the two-function tail
     (`overlay77EnsureSelection`/`overlay77RunCallback`) did not, and
     merging all four functions produced a fixed-size `.text` (IDO compiled
     it 16 bytes bigger than the ROM's real bytes at that offset --
     `trim_elf_section.py` refused the nonzero discard). The fix was two
     TUs, not one: `overlay_077.c` (the two flagged functions) and
     `overlay_077_tail.c` (the two unflagged ones). **This is the pilot's
     one real blocker**, and it will recur: any overlay whose functions
     were matched with a mix of per-function `CFLAGS`/`OPT_FLAGS` overrides
     needs this same split-by-flag-group treatment, not a single TU. A
     `-Wab,-r4300_mul` per-TU flag can silently change a *different*
     function's byte count if it's merged into the same object as a
     function that needs the flag; the only detection method that worked
     here was building and letting `trim_elf_section.py` refuse the
     nonzero trim, then confirming the flag was the cause by rebuilding
     without it and watching `gmake verify` fail on the flagged functions
     instead.

3. **Trace every global that's `extern`-declared with a different
   qualifier across files.** `overlay_077`'s `gOverlay77Handle` was declared
   `extern volatile s32` in the tail file and plain `extern s32` in
   `overlay77Init`/`overlay77Update`; unifying it to one declaration in the
   shared header (either qualifier) changed the *other* file's group's
   compiled bytes by 8 bytes once it landed in the same TU as the flag
   change above. The fix: leave that one declaration duplicated, once per
   TU, with a comment explaining why -- the shared header holds everything
   else, and just documents that this one symbol is intentionally not in
   it. This is a real, permanent exception to "one header, no duplicate
   declarations," not scaffolding to remove later.

4. **Reconcile per-file struct typedefs into the shared header, one
   definition per real struct.** Two distinct situations came up:
   - **True duplicates**: `overlay85Configure.c` and `overlay85Update.c`
     each had their own `Overlay85Trigger` typedef, identical field layout,
     different field names (`active`/`strength` vs `enabled`/`scale`).
     These collapse into one typedef; whichever file's names survive, the
     *other* file's body needs a mechanical field-rename (not a logic
     change) to match.
   - **Name collisions between genuinely different structs**: both
     `overlay85Configure.c` and `overlay85Update.c` also declared a type
     named `Overlay85State`, but they were two different real structures at
     different call sites (Configure's large per-effect state block vs. a
     4-byte-plus-a-flag status struct Update reaches through
     `Object::state`). Unifying these under one name would have been
     *wrong* -- they aren't the same memory layout. The fix was renaming
     the second one (`Overlay85ObjectState`), keeping its field offsets
     exactly as they were, and noting the collision in the header comment
     so the next person doesn't try to merge them for real.
   - A related case in `overlay_077`: `overlay77Init.c` and
     `overlay77Update.c`'s private `Overlay77Object` typedefs padded over
     whatever fields that file didn't touch, and agreed on every field they
     shared -- except `overlay77Update` read `x`/`y`/`z` through a
     bit-reinterpreting union (`Overlay77Coord`) where `overlay77Init` only
     ever read the float. The canonical struct uses the union (the
     strictly more general view), so `overlay77Init`'s body picked up a
     mechanical `object->x` -> `object->x.value` rewrite. This counts as
     "typedef/extern consolidation," not a body change, but it's worth
     flagging explicitly since it touches every field access line.

5. **Compute the merged TU's trim offset.** Before consolidation, every
   function's own object gets `trim_elf_section.py`'d down to its own size
   (IDO pads *each object's* `.text` to a 16-byte boundary; the trim removes
   that so the next function's object starts flush against it, with zero
   gap, because the real ROM has zero gap between two functions from the
   same original translation unit). Once merged into one TU, there is only
   one boundary left: the end of the whole module's `.text`, right before
   the next non-`c` subsegment (a hand-written `asm` padding entry, or a
   `bin` data/rodata/reloc blob). The new trim size is simply the sum of the
   merged functions' sizes -- equivalently, the offset of that next
   subsegment. **Do not assume this trim can be dropped just because the
   size lands on a 16-byte boundary already**: `overlay_077.c` proves size
   arithmetic alone is not sufficient evidence (see point 2's blocker); the
   real trim size has to come from an actual build and byte-compare, not a
   pen-and-paper alignment check.

6. **Update `TEXT_SUBSEGMENTS[overlay]`** in `tools/overlay_atlas.py` to one
   `(0x0, "c", "overlay_NNN")` entry (two, for a split-TU case like 77),
   keeping any existing `asm` padding entries verbatim -- they're still
   real, still hand-written, and still correct regardless of how many `c`
   objects precede them. Run `gmake overlay-atlas-write` to regenerate
   `config/overlays.us.json` and the generated block in `mickey.us.yaml`.

7. **Collapse the Makefile.** Replace every per-function `POSTPROCESS`
   (and any `CFLAGS`/`OPT_FLAGS` override) rule for the overlay with one
   rule per surviving TU, and collapse the object list
   (`OVERLAY_TRIMMED_OBJECTS` and its per-overlay siblings) the same way.

8. **Build, then `gmake verify`.** This is the only real gate; everything
   above is process for getting to a build that either matches or doesn't.
   If it doesn't, the almost-certain cause is one of: wrong function order,
   a dropped/added `CFLAGS` override, a unified `extern` qualifier that
   should have stayed split, or a struct field reinterpreted wrong.

9. **`gmake check-docs`.** `tools/overlay_donor_scan.py --check` validates
   a *stored* `sha256` of `config/overlays.us.json` against the live file;
   consolidating an overlay changes `config/overlays.us.json` (the
   `text_ownership` breakdown collapses to fewer, larger entries) even
   though it doesn't change any donor-scan *content* (module `.text` sizes
   and "empty" classification are unaffected by a pure layout change). The
   correct fix is `python3 tools/overlay_donor_scan.py --write`, but that
   tool also re-scans the pinned reference decomps on disk
   (`~/Desktop/dev/decomp-refs/...`) and will fail if that clone has moved
   past the commit pinned in `DEFAULT_REFERENCES` -- an environment problem
   unrelated to the consolidation, reproducible on an unmodified tree in
   this pilot's sandbox. When that happens, the atlas digest can be patched
   directly (`config/overlay-donors.us.json`'s `"atlas": {"sha256": ...}`
   field, recomputed as `sha256(config/overlays.us.json)`) without touching
   `donors`/`semantic_findings`, since those sections are unaffected by the
   layout change. Don't do this silently for a change that *does* affect
   scan content (a real function move, a `.text` size change, an
   empty-module flip) -- only for the mechanical case this pilot hit.

10. **Commit per overlay**, `gmake cleanroom` clean, one commit per module
    (`docs/adr/0010-commit-discipline.md`).

## Post-consolidation match-loss audit

The apparent fall from 19.32% to 18.38% was a reporting-granularity defect,
not a compiler-codegen regression. The consolidation commits combined some
byte-exact C objects with `#ifdef NON_MATCHING`/`GLOBAL_ASM` fallbacks. The
atlas's old `is_nonmatching_source()` predicate saw one guard and marked the
merged source's entire ownership row NON_MATCHING, including the exact C that
had shared the new TU. The ROM continued to verify.

History was checked immediately before and after every `Consolidate overlay`
commit. Candidates were excluded if their pre-consolidation object used an
instruction-altering postprocess or if their body had since been edited. The
remaining set is 73 independently owned exact ranges, totaling 8,944 bytes in
13 overlays. At the 947,932-byte whole-program denominator, that is 0.9435
percentage points: the complete explanation for the reported 0.94-point
drop after rounding.

One representative was rebuilt from the pre-consolidation source and compared
with the corresponding current merged object in every affected overlay. The
ordinary overlay flags reproduced all representatives except overlay 12,
whose historical `-Wo,-loopunroll,0` override was retained for its standalone
rebuild.

| Overlay | Representative | Standalone versus merged result |
|---|---|---|
| 1 | `overlay1PreviousPointer` | Instructions and relocation layout exact |
| 4 | `overlay4InitializeObjectMotion` | Instructions and relocation type/offset exact; the raw symbol spelling reflects the canonical synthetic alias, while the resolved linked target and ROM bytes are exact |
| 5 | `overlay5InitSequence` | Instructions and relocation layout exact |
| 7 | `overlay7CreateEntry` | Instructions and relocation layout exact |
| 8 | `overlay8Ignore` | Instructions and relocation layout exact |
| 9 | `overlay9Ignore` | Instructions and relocation layout exact |
| 12 | `overlay12Initialize` | Instructions and relocation layout exact with the historical loop-unroll override |
| 15 | `overlay15GetResource4` | Instructions and relocation layout exact |
| 16 | `overlay16BuildGradient` | Instructions and relocation layout exact |
| 25 | `overlay25SetVectorFlags` | Instructions and relocation layout exact |
| 27 | `overlay27Init` | Instructions and relocation layout exact |
| 28 | `overlay28ResetBuffer` | Instructions and relocation layout exact |
| 49 | `refractOutput` | Instructions and relocation layout exact |

The cause taxonomy for this loss is therefore: status/accounting granularity,
13 overlays; per-TU flags, static/file-scope ordering, rodata/literal-pool
ordering, and dropped volatile/alias declarations, zero overlays each. Those
other causes remain real consolidation hazards (the overlay 77 pilot above
demonstrates two of them), but they did not cause this particular regression.
Every currently guarded body found by this audit was already an ADR 0002
demotion of an instruction-altered object before consolidation.

### Corrected mixed-TU recipe

Status, 2026-10-21: every overlay TU is complete, so the table this recipe
fills is empty. Its 182 rows each sat inside a fully matched TU, where the
generator already skipped them; removing them left `config/overlays.us.json`
and the `mickey.us.yaml` block byte-identical. The recipe below stays correct
for a consolidation that produces a mixed TU again.

Do not split a byte-exact consolidated TU merely to restore scoreboard credit.
After proving the standalone object, merged object, relocations, linked owned
range, and ROM, record its exact subranges in
`MIXED_TU_EXACT_C_RANGES` in `tools/overlay_atlas.py`. The generator requires
each range to be ordered, nonempty, and contained in exactly one broad C row
that is source-level NON_MATCHING. It emits `mixed_tu_exact_c_ranges` evidence
in `config/overlays.us.json` and moves only those bytes from NON_MATCHING to
matched C; it does not create another splat subsegment, object, or linked copy.

Use that exception only for independently established, metadata-only exact
objects. If standalone and merged compiler output really differ, follow the
original recipe: split at the flag boundary, restore the per-TU declaration,
or preserve the required file-scope/literal ordering, then re-prove the linked
range and full ROM. A source file containing a NON_MATCHING guard remains a
conservative object-level signal everywhere outside the reviewed range map.

## What did *not* come up in this pilot

- **Relocation filter/rebind steps.** None of the four pilot overlays used
  `filter_elf_relocations.py`/`rebind_elf_relocations.py` to begin with
  (checked against `config/postprocess-audit.us.json`: every row was
  `trim_elf_section.py` only). ADR 0006's claim that these retire
  "overlay-by-overlay as each one consolidates" is **not exercised by this
  pilot** and needs a target that actually carries them (o61, o70, o84 in
  this tree's Makefile all have relocation-filter/rebind or
  `normalize_elf_instructions.py` steps visible in the grep this pilot ran
  incidentally -- none of them are matching-clean enough to pilot next
  without first resolving their `altered` rows under ADR 0002).
- **BSS ownership.** All four pilot overlays have `bss_size: 0x0` in the
  generated yaml. The pilot's data-ownership experiment (next section) only
  covers initialized data; BSS ownership needs a target overlay with
  nonzero BSS.

## Measurements

| Overlay | Files before | Files after | Makefile lines (net) | POSTPROCESS steps before -> after |
|---|---|---|---|---|
| 39 | 2 `.c` | 1 `.c` + 1 header | 4 insertions / 8 deletions | 2 trims -> 1 trim |
| 77 | 3 `.c` | 2 `.c` + 1 header | 4 insertions / 8 deletions | 3 trims -> 2 trims (one still carries `-Wab,-r4300_mul`) |
| 85 | 2 `.c` + 1 nested header | 1 `.c` + 1 flat header | 3 insertions / 6 deletions | 2 trims -> 1 trim |
| 95 | 2 `.c` | 1 `.c` + 1 header | 4 insertions / 7 deletions | 2 trims -> 1 trim |

Every trim step remaining after consolidation is `trim_elf_section.py`
trimming the merged object's own trailing IDO 16-byte alignment tail down to
the real function-content size -- exactly `overlay_006`'s existing pattern,
now the norm rather than a curiosity. No `normalize_elf_instructions.py`,
relocation filter/rebind, or `objcopy` symbol-rename step was needed or
removed for any of the four (none used one before consolidation either).

All four rebuilt byte-identical to
`507341c0a40ca3e9a7cee969b396ee53facfb548` after their commit.

## Data ownership (ADR 0006's "atlas gains real data ownership" claim)

The overlay 95 pilot first established that declaration order matters: its
fade-rate definition followed by its disabled flag reproduced the 16-byte
initialized section, while reversing the declarations did not. That result
was object-level only; overlay 95 still imports the range as a raw bin.

Overlay 77 is the first ownership result wired through the complete build.
Its 48-byte initialized range is four 32-bit state slots followed by five
single-precision constants and compiler alignment. The TU's uses and local
relocation groups establish the declaration order. `gOverlay77Count` and
`gOverlay77Sequence` were two names for the same slot, so the tail now uses
the single defined `gOverlay77Sequence` object.

IDO emits the complete range in `overlay_077.c`'s `.data` with no `.rodata`.
`INITIALIZED_DATA_OWNERSHIP` in `tools/overlay_atlas.py` records the reviewed
range and its existing text-owning TU. The generated manifest exposes that
row as `data_rodata_ownership`, and the YAML generator omits only the former
`overlay_077_data_rodata` bin row. Both relocation tails remain raw and in
their original positions: all 10 symbol relocations and 36 local relocations
are unchanged.

The configured object, linked overlay range, and full ROM are byte-identical;
`gmake verify` prints the expected US SHA1. This closes 48 of the atlas's
61,312 initialized bytes with ordinary C definitions and provides the generic
recipe for later full-range owners. A partial initialized-section owner still
needs an explicit ordering model before the generator can mix C-owned and raw
fragments safely.

### Overlay 42 implementation

Overlay 42 is the first module to wire that experiment into the canonical
split. Its one 16-byte initialized range is now emitted by
`overlay_042.c` as an ordinary initialized-state object: two buffer slots,
one buffer-index word, and one image-alpha byte, in that declaration order.
IDO emits a 16-byte `.data` section that is byte-identical to the former raw
asset. The module's 0x700-byte `.text`, its compiled text relocation surface,
and both shipped relocation tails remain unchanged, and the full linked ROM
still verifies to the expected SHA1.

`DATA_RODATA_OWNERSHIP` in `tools/overlay_atlas.py` records leading,
contiguous initialized ranges owned by C sources already present in the
overlay's text ownership. The generator validates source and declaration
order, records the range in `config/overlays.us.json`, omits the covered raw
`bin` row, and retains any uncovered initialized tail plus both relocation
tails as raw rows. Overlay 42's full initialized range is covered, so only its
relocation tails remain raw. Its separate 16-byte BSS range is unchanged and
is not claimed by this data-only pass.

## Environment note (unrelated to consolidation, hit during every commit)

`gmake check-docs` calls `tools/overlay_donor_scan.py --check`, which
compares a stored `sha256` of `config/overlays.us.json` against the live
file. Any atlas-affecting change (this pilot's consolidations included)
goes stale and needs `--write` to refresh -- but `--write` also re-scans the
pinned JFG reference clone at `~/Desktop/dev/decomp-refs/jfg`, and in this
sandbox that clone is checked out past the commit pinned in
`DEFAULT_REFERENCES` (`c75c270d...` on disk vs. `c82afff...` expected),
which is unrelated to any change in this repo and reproduces on an
unmodified tree. Point 9 above is the workaround used for this pilot; the
actual fix (re-pinning `DEFAULT_REFERENCES` to the clone's current commit,
or pinning the clone back to the expected commit) is outside this pilot's
scope and should be flagged to whoever owns the reference-clone pins.

## Handoff

**Commits** (branch `lane/consolidate-pilot`):
- `e3e5c25` Consolidate overlay 39 into one translation unit
- `b0abdae` Consolidate overlay 95 into one translation unit
- `19be80e` Consolidate overlay 85 into one translation unit
- `cf8d7f2` Consolidate overlay 77 into two translation units

Each commit builds and `gmake verify`s byte-identical on its own; `gmake
cleanroom` and `gmake check-docs` pass after each.

**For the next batch**: pick targets the same way this pilot did --
`matched_c: true` and no `altered` rows in
`config/postprocess-audit.us.json` -- and budget extra time for any overlay
whose Makefile shows per-function `CFLAGS`/`OPT_FLAGS` overrides that don't
apply uniformly across the whole module; that's the one case in this pilot
that didn't fold to a single TU, and it will recur elsewhere. The relocation
filter/rebind retirement ADR 0006 predicts still needs its own pilot target;
none of these four exercised it.
