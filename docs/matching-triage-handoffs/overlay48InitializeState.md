<!-- plateau-handoff:overlay48InitializeState:start -->
### `overlay48InitializeState` plateau handoff

- source: `src/overlays/o048/overlay48InitializeState.c`
- score: 47 differing words
- frame: 0x18
- relocations: 24
- first mismatch: +0x0
- summary: Hand-unroll folds index=1 to addiu+2; loops peel record 0. Missing sll+addu cursor and v0+=0x30. Colour refused at nonzero size.
#### 2026-09-13, lane k1: authenticated draw-census follow-up

Fresh configured stock compilation reproduces 228 target bytes,
size delta -16, 47 raw and
47 masked differences, first +0x0.
Candidate frame is 0x18; the procedure-0 census records
6 draws and 156 emission records. Stock and traced full-TU text
compare identically. Static relocation counts are 24 candidate and
8 extracted target, with 1 identical offset/type/symbol tuples.
These are static measurements; overlay runtime identity proof remains separate.

This replaces the stale ten-word positional description in the legacy triage
row with the current comparator's 47 masked differences: 53 candidate versus
57 target words. Paired alignment has 19 exact, six naming, two immediate and
22 structural rows, plus four candidate-only and eight target-only words.
The net 16-byte deficit is structural; a colour-only reading would miss it.
The matching frame and six-draw baseline do not repair that deficit.

There was no per-symbol shard on arrival. The legacy matching-triage row and
source annotation record natural direct-array, pointer-loop, hand-unrolled,
aggregate/scalar-BSS, declaration and qualifier controls. Those forms either
lose the target's retained index/base setup or add address producers. No new
source-authenticated declaration or TU fact was found to change that constraint.
ADR 0018 early evidence stop applies with zero new source attempts. No colour
sweep or known-failed loop/unroll control was repeated. This is a current
baseline receipt and creation of the missing dedicated shard, not a new
exhaustion claim. Runtime relocation identity remains unresolved.

The original guarded body and assembly fallback are retained. Sources, stock
and traced objects, frame and scalar measurements, draw profiles and aligned
maps remain ignored under build/k1/overlay48InitializeState.
Commands: lane_status.py, configured stock compilation, draw_census.py,
residual_map.py --object/--against where compared, finalize_plateau.py and
tools/gates.sh. No executable bytes are newly credited.

#### 2026-09-19, lane w29-o048: reconstruction sweep, size still -16

Re-measure of the retained body: 228 target bytes, candidate 53 words,
size delta -16, 47 raw and 47 masked, first +0x0, frame 0x18 both sides
with one shared slot at +0x14. Align: 19 exact, 6 naming, 2 immediate,
34 really different. residual_map: 22 structural plus 4 candidate-only
(+0x64, +0x68, +0x7C, +0x80) and 8 target-only (+0xC, +0x1C, +0x20,
+0x28, +0xB0, +0xB4, +0xBC, +0xC0). Displacement tax 5. Identity-gate:
configured stock .text is byte-identical to instrumented IDO; CDX_PROC=0
with 2 allocator decisions. Colour was not run (L155 until size 0).

The four missing words are a retained index copy, its strength-reduced
cursor, and a leftover dest bump: addiu a0,1; sll a0,1; addu onto the
D_274 base; then addiu v0, 0x30 after the unrolled stores. Header seed
is a lui-plus-lh of D_274[0], then a separate lui-plus-addiu of the same
symbol for that cursor. The second jal delay is nop (no second 0x16).

Hand-unroll keeps the interleaved D_10 store schedule but copy-props
index=1 into addiu v1,2 and DCE's the dest bump. L109 OR-with-zero on index
or arg0 still folds to +2 and lands at size -12, masked 52 (F, J, S, T, X). Empty
if (index) / if (entry) and L97 if (1) around the cursor add extra D_10
lui rematerializations and overshoot to size +4, masked 52-54 (G, H).
Comma-assign of index and cursor is byte-flat on size at masked 46 (I).
do/for/while loops unroll by peeling record 0 as global D_10 stores and
rematerializing D_274 per seed; leftover dest math appears as +0x10 then
+0x20, net size +16, masked 56-57 (A, B, K, O_unroll4). One-index
D_10[index-1] rematerializes every field (N, +52). Walking hand-increments
reach size -4, masked 51, but 31 aligned structural rows versus the
baseline 22 (Q, V). Unused pointer/f32 (L99) and local-array length (L112)
do not emit the bump at frame 0x18. loopunroll,0 under-emits (-52).
Volatile entry does not keep entry += 3.

The named stall is the same peel-versus-fold trade the k1 shard recorded,
now priced: no tried spelling produces the sll+addu cursor and v0+=0x30
without peel or extra D_274/D_10 address producers. Resume only with a
form that strength-reduces the index before unroll and keeps record 0 on
the D_10 pointer. Do not run --every-colour until size_delta is 0.
Retained source is the original guarded body. ADR 0018: three later
batches (copy, fallthrough, entries, values[0]-only) neither improved
the best residual nor opened a new axis.

<!-- plateau-handoff:overlay48InitializeState:end -->
