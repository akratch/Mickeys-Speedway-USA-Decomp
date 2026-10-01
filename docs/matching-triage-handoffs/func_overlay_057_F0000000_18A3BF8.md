<!-- plateau-handoff:func_overlay_057_F0000000_18A3BF8:start -->
### `func_overlay_057_F0000000_18A3BF8` plateau handoff

- source: `src/overlays/o057/func_overlay_057_F0000000_18A3BF8.c`
- score: 100/597 words
- frame: 0x88
- relocations: 246
- first mismatch: +0x8
- summary: 100 at -8: target keeps i live in s1 in the final spawn loop, uopt here strength-reduces both arrays; frame 0x88 vs 0x78.

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

<!-- plateau-handoff:func_overlay_057_F0000000_18A3BF8:end -->
