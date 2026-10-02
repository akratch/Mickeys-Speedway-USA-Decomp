<!-- plateau-handoff:func_overlay_057_F0000000_18A3BF8:start -->
### `func_overlay_057_F0000000_18A3BF8` plateau handoff

- source: `src/overlays/o057/func_overlay_057_F0000000_18A3BF8.c`
- score: 0/597 words, promoted
- frame: 0x78
- relocations: 248
- first mismatch: none
- summary: Matched. Indexed for loops everywhere (i spans each preheader, so LFTR end s1 / index walk s2), s16 pair table by i << 1, state pointer local, byte0B after the pair.

Summary before this remeasure: 19 at 0, frame exact: locals reused (descriptor walks spawns, entry is loop end). Open: loop preheader lui order, 0x36 delay slot, one ring draw.

Summary before this remeasure: 84 at 0: pair table via (i<<2) keeps i; own counter for 0x36 loop. Open: s-reg order (i s1/ptr s0), frame 0x88 vs 0x78.

Summary before this remeasure: 100 at -8: target keeps i live in s1 in the final spawn loop, uopt here strength-reduces both arrays; frame 0x88 vs 0x78.

Summary before this remeasure: 282 to 276: every block-scoped register local merged to function scope and the packet declared sixth; the frame excess is now a local count.

Measured this lane:

- base 282 masked, 313 raw, size delta -20, frame 0x90, 17 homes against 24.
- the union split, initial declared first: 335 masked, 365 raw, delta -12,
  frame 0xA0, 23 homes against 24.
- the union split, final declared first: identical numbers, so the two packets'
  relative declaration order is inert at this stage.

Read the two ladders side by side with tools/frame_census.py before touching
anything here: the 0x18 frame excess and the 98 structural words are one
question, and the packet count is the half of it that the old comment gets
backwards.

## 2026-09-12 (lane `p12-o57`): the frame excess is a local COUNT, and both packet blocks are now reachable

  - before: 282 masked of 597 words, size delta -20, frame 0x90 against the
    target's 0x78, 17 stack homes against 24, byte-exact 410, register naming
    75, immediate only 23, really different 98.
  - after: 276 masked, size delta -20, frame 0x80, 17 homes against 24,
    byte-exact 421, register naming 75, immediate only 17, really different 94.

### Adopted, two edits

**Every block-scoped `register` local merged to function scope.** The seven
inner blocks declared fourteen locals between them, including three separate
`i` and two separate `spawned`. Each reserves its own home, so the scoping was
buying nothing and costing four slots. Twelve function-scope locals is frame
0x90 to 0x80 at 282 to 281, and it makes the ladder BELOW the packets exact:
six slots on each side at +0x2C +0x28 +0x24 +0x20 +0x1C +0x10.

**The packet declared sixth rather than first.** A move-one climb over all
thirteen declaration positions, 145 compiles, reaches 276 in one move and is
then a fixed point. What the move buys is the target's own packet offsets: the
union lands on +0x54 through +0x64 and the target's upper packet block is +0x54
through +0x64, slot for slot, where before it sat 0x24 too high.

### The union split is reachable and flat, which is a different statement from the last packet's

The last packet said the split "reproduces BOTH blocks" and costs 282 to 335
"because every home below it then shifts". Re-measured on the merged shape,
both halves need correcting.

The split reproduces the LOWER block exactly, and only when `final` is declared
**last** of the thirteen: its block then lands on +0x3C +0x3E +0x40 +0x42 +0x46
+0x47 +0x48, the target's offsets with only the target's +0x44 store
unaccounted. Declared anywhere else it does not.

But the cost is NOT a shift that an order sweep can recover. All eight
positions of `final` score exactly 335, and a separate move-one climb over all
fourteen positions of both packets, 170 compiles, is a fixed point at 335. The
split is flat across placement.

### The lever is the local count (L134), and it is measured

With the split the frame is 0x90 against 0x78 -- exactly six slots too many --
and this candidate declares twelve non-packet locals where the target's two
gaps, +0x4C through +0x53 and +0x6C through +0x77, leave room for five or six.

Merging `value` into `id` and `stride` into `i`, without the split, takes the
frame to 0x78 EXACTLY on both sides and costs 15 words (276 to 291 at the best
union position, 281 to 296 at the first). That is the measurement that proves
the count is the free parameter rather than any placement, and that ten locals
is two too few while the union is one object rather than two.

### Next edit, named

Find six reuses among `current`, `descriptor`, `resourceIndex`,
`descriptorEnd`, `i`, `value`, `stride`, `spawned`, `path`, `result`, `entry`
and `id` that cost less than the split buys, then take the split with `final`
declared last. The pointer set is where the room is: `path`, `result` and
`spawned` are used in three disjoint blocks and `resourceIndex` and `entry` in
two more, so five of the six reuses are legal under L115 and need only a type
that serves both sites.

Do not re-run: the thirteen-position and fourteen-position declaration climbs,
the eight positions of `final`, the 66-cell two-packet placement sweep, or the
block-scope shape.

Validation: `gmake verify` printed
507341c0a40ca3e9a7cee969b396ee53facfb548 and `tools/gates.sh --staged` passed
all four gates. The candidate remains `NON_MATCHING`, so no bytes are credited.

## 2026-10-01 (lane `d-o057`): 276 at -20 -> 100 at -8

The "frame excess is a local count" reading below was made on the union
shape and does not survive the rewrite. Every step rewrites inherited shape;
measured with `tools/fast_score.py` and `tools/shape_product.py`.

  - The 0x134 and 0x138 stores are the bss fog pair (`D_134`, as in the
    matched F0001AE8), not the data-section id list the switch walks: the
    relocation records carry different section bases for the two. Two packet
    locals instead of the union, the final packet storing `z = 0` (the
    target's +0x44 store), and the f32 prototype for overlay57SetNodeValue:
    264 at delta 0.
  - Declaration move-one climb on that shape: 257.
  - The id walks as `while (*entry != -1)` reading the entry at each use
    (matched F0004E18 and F00060F8 idiom): the switch block is exact, 161 at
    -16.
  - The choice mask as a compound OR-assign of `1 << i` in a for: 115 at -8 (two
    shipped words recovered). func_8005AD64's last argument is a float zero
    (`addiu a3, zero, 0` in the target): 114.
  - The initial packet assigns scale before kind and state (24 orders): 100.

Open, with the decision variable named: the -8 is the final spawn loop. The
target keeps `i` live (s1) and indexes the pair table from it every pass,
strength-reducing only the spawned array (s0); here uopt reduces both and
drops `i` by linear test replacement. Loop form (do-while `!=`, for `<`, for
`!=`) and the spawned carrier against array reads were measured flat or
worse. The frame is 0x88 against 0x78 with every packet and the choice mask
at the target's offsets: two reserved cells below the final packet that no
declared local accounts for (removing code moves them, they never carry
traffic). Bare blocks were removed at no change.

## 2026-10-02 (lane `j-o057`): 84 -> 19 at delta 0, frame 0x78 exact

Every step is a local-set or carrier rewrite measured with `tools/fast_score.py`
and `tools/shape_product.py`; the uopt records (`p1dec` save/nocs) were read
with the instrumented toolchain to pick each one.

  - Locals cut to the frame: `path`/`spawned`/`result` are one pointer
    (`object`, one struct type covering +0x8/+0x16/+0x3C/+0x68), `stride`
    folded into `i`, the 0x36-loop counter is `value`. Seven scalars plus the
    two packets land every home at the target's offset and frame 0x78:
    84 -> 76 (frame alone is 20 words).
  - Flag test `((flags & 0x1C0) >> 6) >= 3` instead of `(< 3) ^ 1`, and the
    final packet storing z, state, byte0B, byte0A: 76 -> 57.
  - `descriptor` reused as the final loop's walk over gO57Spawned150Reloc:
    the walk becomes part of the highest-save symbol web and takes s0, so `i`
    falls to s1 in the choice and final loops as in the target: 57 -> 45.
    (Records: `i` 27.67 against the SR walk temporary 20.5 before.)
  - `entry` reused as every descriptor loop's end pointer (end and the id walk
    are s1 in the target) and the index tables subscripted by `value`, so the
    index walk is an SR temporary that sits in s2 below the end: 45 -> 41.
    A `resourceIndex` symbol always outranks the end (124/5 = 24.8 against
    137/8 = 17.1) unless assigned before the singles, which costs 49 words.
  - Preheader statement order `value = 0; entry = end; descriptor = start;`:
    41 -> 29 (12 cells).
  - The 0x27C..0x31C fill is one 10-row loop (`O57Row`, stride 0x10); IDO
    peels the two remainder rows, which are the 27C/28C stores. With value10
    then read at each use (no carrier) the seed block is exact: 29 -> 21.
    Neither edit alone moves (value10 shares block 37 with the fill walk).
  - The mask compare with the local first, and `i = 0` before the walk's
    assignment: 21 -> 19.

Open, 19 words, decision variables named:

  - Descriptor loops, 8 words: the target's preheader emits the index-table
    `lui` first and the `addiu`s in reverse (LIFO), ours FIFO with the SR
    init last. Folding statements onto one line, all six orders, and a
    one-line (macro-like) loop body were measured flat or worse; a pointer
    symbol for the index gets the order but outranks the end pointer.
  - 0x36 loop, 4 words: delay slot holds the counter step instead of the
    store, and the two `addiu`s order. Folding `value = 0; descriptor = ...;
    entry = ...;` onto one line fixes the order (2 words) but is unnatural.
  - Final loop, 7 words: preheader `addiu` order, and one ring draw. The
    freelist trace shows ours draws t5 for 0x80 where the target's sequence
    (kind t3, mode t2, x t4/t7, y t6, 0x80 t9, state t8, 2 t0) needs t5 spent
    before the loop and x/y emitted before byte0B. Store-order products (120
    and 96 cells) are flat; the missing earlier draw is not located.

Do not re-run: the declaration-order sweeps (560 and 210 cells, all flat once
the set is right), seed carrier products, index-loop line folding.

## 2026-10-02 (lane `l-o057`): 19 -> 0 at delta 0, promoted

The previous lane's reuse of `descriptor` and `entry` across regions was a
priority stand-in for the shape below; with it gone both are unreferenced
(`descriptor` stays as `pad0`, a frame cell). Measured with
`tools/fast_score.py` and `tools/shape_product.py --jobs 3`, records read with
the instrumented `uopt` (`p1dec`) and the ugen freelist trace.

  - The 0x36 loop as a `for` whose header holds init, bound and both steps
    (one source line, L59): delay slot and address order exact, 19 -> 15. Not
    kept: superseded by the next edit.
  - Every descriptor loop as `for (i = 0; i < N; i++)` subscripting both
    arrays (dl x counter x 0x36-form product, 12 cells): 15 -> 7. uopt
    creates the index walk, the descriptor walk and the LFTR end in the
    target's order (index table first, LIFO addius). The register order is
    the counter's web: `i` spans each loop preheader (records: block 10, 15,
    20, 25, 30), so it forbids s1 to the index walk (bbs 10-13) but not to
    the end (bbs 11-13); with `value` as the counter it is a 2.5-save v1 web
    and the index walk takes s1. With every loop indexed and `value` as the
    counter, no end pointer is hoisted at all (565-585 at -20/-24): s2 is
    then unpaid when the end webs (save 5) are decided.
  - Final loop as `for (i = 0; i < 4; i++)` with `gO57Spawned150Reloc[i]`
    (6 forms x 2 pair spellings): 7 -> 5. `[i].first` is strength-reduced
    and drops `i` (580 at -24).
  - The spawned state pointer in a local (`spawnState`, which takes the
    unreferenced `value` cell): it is p1-coloured v1 as in the target, so it
    leaves the ring and `2` draws t0: 5 -> 6 alone.
  - The pair table read as an s16 array, `[i << 1]` and `[(i << 1) + 1]`,
    with byte0B stored after the pair (6 spellings x 2 positions): 6 -> 2
    (0x80 in t9).
  - Store order product (24 orders of z/state/byte0B/byte0A x 7 pair
    positions, 168 cells): 12 exact cells; kept kind, mode, x, y, z, state,
    byte0B, byte0A. 2 -> 0.

Promotion: overlay 45 calls through `overlay45CreateDescriptor_o057Reloc` and
`overlay45SetMode_o057Reloc`; ten resident callees renamed in POSTPROCESS; the
mode switch table is the retained overlay rodata at +0x6C (data +0x5AC..0x5E8),
bound by `gOverlay57InitModeJumpTableReloc` with a rebind spec, the private
copy externalised by digest, and an atlas ownership row. `gO57Rows27CReloc`
binds to 0x27C. `gmake verify` and `promotion-proof` (248/248 relocations)
pass.

<!-- plateau-handoff:func_overlay_057_F0000000_18A3BF8:end -->
