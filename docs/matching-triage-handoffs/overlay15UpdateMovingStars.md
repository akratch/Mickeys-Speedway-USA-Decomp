<!-- plateau-handoff:overlay15UpdateMovingStars:start -->
### `overlay15UpdateMovingStars` plateau handoff

- source: `src/overlays/o015/overlay_015.c`
- score: 0/103 words, promoted
- frame: 0x58
- relocations: 39
- first mismatch: none
- summary: Matched. The rain field is a locally-defined static struct reached through a pointer taken at entry; all uses sit behind the camera call, so every access is direct and shares high halves.

#### 2026-10-01, lane b-o101: matched on the overlay15MoveStars form

Promoted at 103 of 103 words, delta 0, frame 0x58, 39 of 39 relocations.
Before: 84 masked words and 28 bytes long.

The 28-byte deficit was seven extra high halves from the scalar-symbol
candidate. With the rain field defined in this unit as a static struct (bounds
at 0x50, movement at 0x80, previous camera at 0x8C, colours at 0x98) and
reached through a pointer local, every use sits behind the camera call, so
uopt forwards the address into each access rather than keeping a base
register; the accesses are direct, and as1 shares one high half per aligned
pair of them, which is the shipped shape. The pointer is declared last so
deltaZ keeps its spill home; declared first it moves the frame 0x10 and costs
13 words. The three position products are written in x, y, z order (the old
x, z, y order was 2 words). Two plain blend statements score the same as the
nested assignment.

The section-relative records are dropped in mk/overlays.mk with
filter_elf_relocations.py, as for overlay15MoveStars.

Gates: gmake verify, check-overlay-syms and promotion-proof pass.


#### 2026-09-13, lane l1: measured schedule controls

Fresh procedure-8 baseline has 110 candidate words against 103 target
words, delta plus 28, with 84 masked differences from plus 0x30. Alignment is
62 exact, nine naming, fifteen immediate and sixteen paired structural rows,
plus eight candidate-only and one target-only words. The frame and complete
stack traffic agree at 0x58. Candidate static relocations number 46, versus two
in the target object; these are different relocation representations, not an
identity proof. The census records 27 draws and 132 emissions.

Remove only the trailing named update-rate conversion and repeat that same
conversion at the three position multiplications; preserve the earlier
reciprocal scale carrier and every operation's types and order. This tests
release of that trailing scale before the scalar-bound accesses. It removes
one location emission at the old definition, but every draw count and the
entire draw sequence remain fixed. Stock text is byte-identical and the aligned
windows and gaps do not move. Restore the original. Stop early: the named
carrier is not a pressure lever in this shape, and the recorded colour axis
is closed. Next action requires changing the scalar-address lowering without
adding a temporary draw or conflating distinct storage identities.

Named Ucode mapping and full stock/capture fidelity pass for the baseline and
each retained experiment. Sources, stock objects, scores, frame/relocation
censuses and aligned deltas remain under ignored build/l1/overlay15UpdateMovingStars. Commands:
configured compilation, allocator_trace_receipt mapping, draw_census profile
and comparison, residual_map object comparison, finalize_plateau, and
tools/gates.sh verify cleanroom check-docs. No matching credit is claimed.

<!-- plateau-handoff:overlay15UpdateMovingStars:end -->
