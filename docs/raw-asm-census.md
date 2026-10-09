# Raw assembly ownership census

Run `gmake check-raw-asm` before populating a raw-assembly carve queue, or
`.venv/bin/python tools/raw_asm_census.py --json` for exact range identities.
The tool checks the baserom against the configured SHA1 and derives extents
from parsed splat subsegments, including dictionary-form and unnamed rows.
It never prints instruction text or ROM bytes.

The overlay gate fails for every raw range except explicitly named padding
or tails whose bytes are all zero and whose end is a text/data or module
boundary. A small nonzero tail remains work. A zero-filled unnamed range also
remains work: zero bytes alone do not establish padding ownership. Missing,
reversed, unaligned, or out-of-ROM boundaries fail rather than disappear.

Resident raw ranges are reported separately. The existing
`verified_asm.us.txt` ledger identifies handwritten subsegments; the remainder
are **unclassified**, not automatically compiler-generated or ready to carve.
Review the YAML evidence and exact function boundaries before assigning them.
Reported subsegment bytes include alignment, unlike the scoreboard's
function-owned executable bytes. Neither raw ownership nor a new C scaffold
earns matching credit.

An unclassified census row is a gap in this tool's ledger coverage, not
necessarily an unexplored decompilation opportunity. Consult the settled
odd-FP/handwritten exclusions in [modules.md](modules.md#62-odd-floating-point-registers-settled--those-files-stay-assembly),
the YAML ownership comments and the function evidence in
[resident.md](resident.md) before scheduling work. A reference object's exact
assembly fallback is not evidence that a compiler can emit that body. Likewise,
padding established by an adjacent function's boundary proof must not become
a new tiny-function assignment. Keep these distinctions explicit rather than
loosening the census gate or adding unproved matching credit to its ledger.

The gate requires the local baserom and is separate from source-only
`check-docs` and clean-room hooks. Synthetic census regressions run through
`gmake check-tooling`. Matching and ROM proof remain governed by ADR 0001 and
the carve-before-decomp workflow in ADR 0016.

## What the current unclassified ranges are

The census's resident-unclassified rows are not a hidden matching queue.
`main/trackasm` is a Jet Force Gemini handwritten run whose Mickey routines
are shorter revisions, so it is a file-boundary attribution and not a
whole-object match (`mickey.us.yaml`). `main/shadows_fp_19144` (the one
shadows routine that is a shorter JFG revision; its neighbours are verified),
`main/weather_snow_asm`, and the two unnamed odd-register blocks are the
files `docs/modules.md` already closes: IDO cannot emit them, and they do
not become verified assembly without a byte-identical permitted reference
object. `main/refractOutputAssembler`, once listed here as a compiler-shaped
near miss, is verified assembly since 2026-10-07: its whole `.text` agrees
with Jet Force Gemini's hand-written `src/hasm/refractOutputAssembler.s`
on every word outside the 16 HI16/LO16 relocated fields, and it saves
`$ra` in a leaf, which IDO never emits. The four-byte unnamed range is a single `nop` between two
C translation units, with no callers. It is not a function, and the census
does not call it padding only because that class is reserved for named
overlay tails.

Overlay raw assembly is already carved: the census reports no overlay-raw
rows. Text that the atlas still calls nonmatching, and that the ranking
does not score, is a bare `GLOBAL_ASM` with no `NON_MATCHING` candidate.
That is the overlay 51 middle function, the overlay 69 and overlay 88
sorted-draw functions, the leading overlay 14 function, and the overlay 1
function that shares another function's `#else` and therefore has no
candidate of its own. A four-byte difference on a few other translation
units is alignment, not a missing function. Those bare functions become
ranking rows only when a candidate exists. They are not promoted by this
census.

`func_overlay_009_F0000744_1866DBC` is ordinary C in a mixed translation
unit and is absent from the non-matching queue. The matched build's
`.rodata`, after the renames, filter, and text trim, has one digest, so
the old shell `case` is a straight
`externalize_elf_section.py` step. `gmake verify` still matches the US
ROM. A fresh `tools/promotion_proof.py` run does not accept a credit.
With no exact-range row it reports: expected one tracked exact atlas range for func_overlay_009_F0000744_1866DBC, found 0 (none).
The range stays unlisted. Listing it would still not prove: the eight
`--redefine-sym` steps converge on one symbol, and
`canonicalize_redefine_aliases` already rejects that fan-in
(`ambiguous metadata symbol rename identities`, covered by
`tools/test_reloc_identity.py`). No new proof step was added. The
scoreboard is not hand-edited.

## Retail revision reference search (2026-10-09)

A diagnostic search extended the existing five-project object census to owned
retail revisions. It compared nine established contiguous ranges from
`trackasm`, `shadows_fp_19144`, `weather_snow_asm`, `conv_mult_matrix`, and
`gen_anim_data`: 9,380 executable bytes in total, excluding their reviewed
alignment. The four track routines and two snow routines were compared
separately. Mickey's configured objects supplied only the R_MIPS_26, HI16 and
LO16 masks; resolved PC-relative branch fields stayed fixed. Each target first
passed comparison against its own linked ROM range.

The initial search of uncompressed bytes covered sixteen retail archives:
three Banjo-Kazooie revisions, two Conker revisions, five Diddy Kong Racing
revisions, Jet Force Gemini Europe and Star Twins Japan, and four Perfect Dark
revisions. It found no exact masked target. That result alone says nothing
about compressed code, so a second search decoded the relevant retail streams:

| Reference | Compressed-code coverage | Result |
|---|---|---|
| Banjo-Kazooie US and Europe | All 32 code/data streams listed for each revision by the reference ROM decompressor; also all successfully decoded, declared-size-checked 1172 streams | No target hit |
| Banjo-Kazooie US Rev 1 | Successfully decoded, declared-size-checked 1172 streams; no independent complete code-layout claim | No target hit |
| Conker US and Europe | All 507 code chunks in each retail offset table, joined in table order; terminal offsets agree with each reference configuration | No target hit |
| Perfect Dark US, US Rev 1, Europe and Japan | Complete game offset tables (442, 442, 444 and 443 pages respectively), every page checksum checked, plus reconstructed libraries | No target hit |

The Conker and Perfect Dark comparisons included functions crossing compressed
page boundaries. The same scanner found the already-verified 160-byte
`mtxf_transform_point` in both reconstructed Conker code images as a positive
control. Decompression required a complete deflate stream and agreement with
its declared output size; searching for a compression marker alone was not
accepted as coverage.

Format authority was the permitted public projects' tooling:
Banjo-Kazooie `tools/bk_rom_compressor/src/decomp/main.rs` and
`tools/rareunzip.py` at `6eaae281481c9e4b367dc161faabfc3c79fe8733`;
Conker `tools/splat_ext/rzip.py`, `tools/rareunzip.py`, and the US/EU game
configurations at `3adf229175c037c771f251f169f9dd80ca306924`;
Perfect Dark `tools/extract-segment` and `tools/mkrom/game.c` at
`169ed48bdcbfb3b568b028bd5bebb27680073514`. These supplied format facts,
not replacement Mickey source. Diagnostic scripts, archive identities,
decoded images and reports remain ignored under `build/research/` in the
reference-research lane.

This closes a specific compressed-retail search gap, not the authentication
question. It does not prove absence from every revision, every compression
format, or every possible reference routine boundary. A supplemental diagnostic
also searched `gen_anim_data` using its current
object-symbol extents: the decoder and three helper ranges, with the final
bare return retained in its owning helper. These searches found no target in
the same raw retail images, decoded streams, reconstructed game code or
Perfect Dark libraries. This does not establish new independently creditable
function or translation-unit boundaries. No function, boundary, provenance
tier, or verified-assembly credit changed. Another search needs a new permitted
revision, a demonstrated
coverage gap, or a separately justified routine extent; repeating the same
covered images is not a new lead.
