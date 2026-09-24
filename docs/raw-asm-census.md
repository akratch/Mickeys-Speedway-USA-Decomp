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
whole-object match (`mickey.us.yaml`). `main/shadows_fp`,
`main/weather_snow_asm`, and the two unnamed odd-register blocks are the
files `docs/modules.md` already closes: IDO cannot emit them, and they do
not become verified assembly without a byte-identical permitted reference
object. `main/refractOutputAssembler` is the one compiler-shaped near miss
in the list. Its symbol note records a same-sized Jet Force Gemini object
with a nonzero masked difference, so it is not admissible as verified
assembly either. The four-byte unnamed range is a single `nop` between two
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

`func_overlay_009_F0000744_1866DBC` is the exception that is not bare
assembly. It is ordinary C in a mixed translation unit, it is absent from
the non-matching queue, and the ROM still matches, so those bytes are
already the compiler's output. It is missing from
`MIXED_TU_EXACT_C_RANGES`, which is why an atlas-minus-ranking subtraction
still counts them. A fresh promotion proof does not accept the credit:
the translation unit's postprocess contains a shell conditional the proof
tool refuses, while the neighbouring exact functions only pass from an
existing receipt. The range stays unlisted until that proof can be
issued. The scoreboard is unchanged by this finding.
