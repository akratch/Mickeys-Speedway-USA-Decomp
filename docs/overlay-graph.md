# Structural overlay comparison with Jet Force Gemini

`tools/overlay_graph_match.py` ranks JFG modules as investigation leads for
Mickey's overlays. It combines section sizes, function counts, dependency
shape and named resident-callee fingerprints. The current results and
calibration are in `config/overlay-graph.us.json`; names are never adopted by
this tool.

The report is derived from the build. Regenerate it after overlay source,
layout, symbol-name or reference-input changes:

```sh
gmake overlay-graph
gmake check-overlay-graph
```

The check is part of `tools/gates.sh --promotion`. It requires local builds
and reference ROMs, so it is separate from source-only `check-docs` and CI.
The report records `$JFG_ROOT` as the reference location placeholder rather
than publishing a workstation path. Set that environment variable to use a
reference checkout outside the default location documented in `references.md`.
Both write and check refuse missing reference inputs.

## Inputs and boundaries

Mickey's atlas supplies section sizes, BSS sizes, cross-overlay imports and
inbound relocation counts. Function boundaries come from the sized
`STT_FUNC` symbols in the compiled overlay objects, qualified by the atlas's
module and translation-unit offset. Local static functions count. Equality
aliases at the same boundary count once; data-only objects do not contribute
functions. A symbol escaping its owning TU is an error.

Atlas `text_ownership` entries are translation units, not functions. Counting
those entries made older reports change their function counts whenever a TU
was folded. The current implementation reads the functions themselves, so a
layout-only fold preserves that feature and the function-level call ranges.

JFG's published `jfg_us_syms_full.txt`, `build/jfg.us.map` and
`overly_refs.txt` supply module symbols, section sizes and dependency edges.
Its kiosk symbols support the cross-version calibration. The reference
material is permitted under `CLEANROOM.md`; the tool copies no game code.

Both games' relocation tables identify calls to the resident segment.
Resolving those table entries and mapping the target addresses to public
symbol names yields the named-callee fingerprints. No instruction is
decoded. Unknown resident targets remain opaque; they do not acquire names
from a structurally similar module.

## Scores and proposed correspondences

A weighted Jaccard comparison of named resident-call multisets contributes
0.55 of a score; section sizes, function counts and dependency degree shape
contribute 0.25, 0.12 and 0.08. When neither side has named calls, the tool
falls back to size/count/degree weights of 0.5, 0.3 and 0.2. Overlay numbers
are not comparable across games, so neighbour identity is not compared.

The legacy `confident` fields in the JSON mean only that the best score
clears the configured absolute floor and runner-up margin. They are scored
leads, not a probability or an authenticated module identity. An empty
module cannot pass the threshold.

For threshold-qualified pairs, function-level proposals first record shared
named resident calls. Their Tier B flag identifies that call-graph observation;
it does not prove equal semantics. Remaining pairs use rank order and size
similarity, marked Tier D. A proposal requires independent review against
Mickey's bytes, caller/callee contracts and the naming evidence rules in
`modules.md` before it can become an adopted name.

## Calibration and limitations

The JSON contains two controls: known byte-identical cross-game anchors and
JFG-US versus JFG-kiosk module recovery. Inspect both before using a ranked
lead. A method that recovers few known same-game modules cannot justify a
strong identity claim on the harder cross-game comparison.

`refractOutput` demonstrates a particular limitation: a function can remain
byte-identical while its surrounding module changes shape. Its module pair
can therefore rank poorly. `alSeqFileNew` is resident in JFG and an overlay
copy in Mickey; it supplies no JFG overlay pair to score. That absence is
recorded explicitly.

Names, incomplete symbol lists and differences in original module boundaries
all affect the rankings. The tool supports investigation and JFG orientation;
exact matching, relocation identity proofs and semantic review remain the
authorities for adopted code and names.
