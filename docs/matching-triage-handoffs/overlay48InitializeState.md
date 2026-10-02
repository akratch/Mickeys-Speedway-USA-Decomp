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

#### 2026-10-02, lane w7-o48: insertion-pair products, size still -16

The object rule is only the POSTPROCESS redefine of
func_overlay_048_F0000060_1895468 to overlay48InitializeState, plus the
text trim. There is no CFLAGS or OPT_FLAGS entry, so no -Wo,-loopunroll,0,
-Olimit, or -O2 -g3 was present to remove, and -Wab,-r4300_mul is not set.
No flag was added.

Fresh fast_score of the guarded body: 228 target bytes, 47 raw and 47
masked, artifact 0, size delta -16, first mismatch +0x0, category
size-mismatch. insertion_pairs (identity gate passed, proc 0) reports
frame delta +0, aligned residual 42 after shadow 5. Pair 1, +0xC through
+0x84, is extra-ILOD with shadow 0, owned by iloadistore on the header
seed store and the indexed record stores. Pair 2, +0xB0 through the end,
is missing-CSE with shadow 5, owned by loadstore and eval on the finished
store and the script pointer store. That missing-CSE label is the
function label. Colour, draw-census, and last-mile were not run.

Three shape products, jobs 2, each one construct those pairs name. None
moved size toward 0, so none was adopted. The guarded body is unchanged.

Attempt 1, extra-ILOD. The index is assigned before the loop, on the
hypothesis that strength reduction then keeps the scaled cursor instead
of folding it. Three dest spellings: an entry walk, a separate slot
subscript, and D_10 indexed by index minus one. Floor 60 masked at delta
+16 (entry walk). The separate slot was 69 at +48. The index-minus-one
cell reproduced the already recorded +52. The entry-walk object contains
no shift and no add-unsigned. Eliminated: a pre-loop index definition
does not survive as the scaled cursor.

Attempt 2, missing-CSE of the seed base. Seed reads are a cursor
induction rather than an indexed load of the global. Four spellings:
post-increment from a folded values-plus-one pointer, the same walk
after index equals 1, cursor subscript with an entry walk, and cursor
subscript with direct D_10 indexing. Floor 57 masked at delta +16. The
folded pointer and the index-initialized pointer scored the same, and
neither object contains a shift or an add-unsigned, so the index was
constant-folded. Direct indexing was 66 at +52. Eliminated: a cursor
loop still loses the scaled setup and grows by the same 16 bytes as the
recorded pointer loops.

Attempt 3, extra-ILOD peel. The entry and the seed cursor are born in
the for-init, so no separate start-value copy is live across the head
for the rotator to peel. Three pointer for-inits, including one whose
body stores lifetime before active, scored 57 masked at delta +16, the
same floor as the separated-start cursor walk. Direct D_10 subscript
again scored 66 at +52. Eliminated: folding those starts into the
for-init does not keep record 0 on the D_10 pointer and does not emit
the single end bump.

ADR 0018: these three attempts produced no better residual. They did
eliminate the pre-loop index, the cursor induction, and the for-init
counter as ways to obtain that cursor. Resume only with a form that
keeps the scaled seed cursor and leaves every record, including the
first, on the D_10 pointer. Do not repeat the pre-loop index, the
cursor walk, or the for-init counter. Do not run a colour sweep while
size delta is nonzero. Retained source is the original guarded body.

<!-- plateau-handoff:overlay48InitializeState:end -->
