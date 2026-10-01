<!-- plateau-handoff:func_overlay_011_F0001E4C_186A694:start -->
### `func_overlay_011_F0001E4C_186A694` plateau handoff

- source: `src/overlays/o011/func_overlay_011_F0001E4C_186A694.c`
- score: 0/295 words, promoted
- frame: 0x40
- relocations: 87
- first mismatch: none
- summary: Matched. Each object the relocation records name is its own extern, the option handles are a one-based table indexed directly, the action is read from its object, the mode test is a plain or, the status clear is a plain loop, and the configuration bytes are unsigned.

#### 2026-10-01, lane c-ovl2: ROM-exact closure

The retained candidate reproduced 63 masked differences at 1,180 bytes. It
was rewritten from the listing and the module's relocation records and
measured on the direct configured compile:

- The records separate what the candidate reached as offsets from one menu
  base: a controller index and an option index in the overlay's
  uninitialized storage, and a highlight, an action, a done flag and the
  handle table in its initialized data. The action shares its offset with
  the controller index but lives in the other section.
- Written as separate externs, with the action read from its object at each
  test (no local, no volatile pointer), the constant compares take the
  shipped operand order.
- The six status fields are cleared by a plain loop. The compiler unrolls
  it into the shipped two-then-four form, which the candidate had spelled
  out by hand with an index carrier.
- The two configuration bytes are unsigned. Signed, the two 4s share one
  register and the function is four bytes short (167); unsigned, 26.
- The two status modes as an ordinary or of two reads: 26 becomes 2. The
  earlier range-fold that motivated the volatile reads does not occur on
  this shape; a switch with two case labels keeps both tests but ranks the
  constant 1 ahead of the saved pointer.
- The last two words were the order of the two saves around the handle
  call. With a declared walking pointer the pointer is saved second; with
  the table indexed by the option number the compiler's own cursor is saved
  first, as shipped. The cursor starts at the table base only if the table
  is one-based, which the record's addend confirms: the table symbol is
  four bytes below the first handle.
- Five locals put that cursor's temporary at the shipped home.

Proof: overlay 11 text +0x1E4C, 1,180 executable bytes / 295 words, frame
0x40, 87 of 87 relocation identities. Ten resident callees are renamed to
their generated surface entries; the two cross-overlay callees and the one
same-module callee whose site is a SYMBOL record are placeholder
declarations; the switch table is bound to the retained overlay table at
rodata +0x54 and the compiler's copy is dropped by digest. None of these
steps touches an instruction. The older filter specification for this
symbol is unreferenced and was left as found. Prior measurements below
remain historical negatives for the menu-base shape.

Commands: direct configured compile scored with `tools/score_symbol.py
--object`, `gmake overlay-atlas-write`, `tools/refresh_atlas_digest.py`,
`gmake extract`, `gmake overlay-syms`, `gmake verify`,
`gmake check-overlay-syms`, and
`gmake promotion-proof SYMBOL=func_overlay_011_F0001E4C_186A694`.

Lane `w18-o011` on base `cbaed235`. Re-measure: ranking size -4 is right;
the prior V0 "-1 word" / first `+0x10` were stale. Configured IDO 5.3
`-O2 -mips2 -32` emits 294 words / 1,176 bytes against 295 / 1,180. Both
frames are `0x40`. Masked 186 / raw 189 / artifact 3. Aligned 190 exact,
53 naming, 11 immediate, 44 structural. Candidate-only `+0x23C` (1) and
`+0x2A4` (2); target-only `+0x1FC` (1) and `+0x288` (3). First naming
`+0xD8` (handle vs index in `v0`/`v1`); first immediate `+0x14C`; first
structural `+0x200`. Candidate has nine stack slots, target eight: extra
`+0x28` is 1 store / 1 load.

Call test: p1 only (the body issues calls). Instrumented IDO at
`ido-instrumented` `.text` is byte-identical to stock. `CDX_PROC=0`, 21
p1 decisions, 15 coloured webs. Unforced `forced=-2`. Cross-kind `s0`
forces move size by +/-8 or +/-12 and destroy the score. `p1:w39=c5`
and `action = D_1C4` each reach size 0 by inserting a copy/`lui`, not
by recovering the missing word; the same four-word insert/delete pair
remains. Colour / `web_footprint --every-colour` is L155 insertion
shadow until size is honestly 0.

The missing word is the unfilled delay of `bne mode, 6` (target `nop`)
together with rematerialized `addiu 2` for `action * 2`. The candidate
spills `action` across `func_80005820`, reloads it at `+0x28`, and
fills that delay with speculative `action << 1`. Target materialises
2 in the mode block and keeps the scale `index * 0x28`.

Tried, all against this 294-word baseline unless noted:

- drop volatile on `menuInput`: size -24, still nine slots
- `index = 2`: extra slot gone (8=8), size -28 because uopt folds
  `status[2]` to a constant offset and deletes the scale
- hoist `action * 2` before the call, `action + action`, L97 `if (1)`
  around the stores, L144 `menuInput = &D_1C4`, sibling do-while,
  generated cursor `D_1CC - 1`, `D_1B8`/`D_204` scalars, `mode` local:
  flat at -4 or a frame regression (`D_1CC - 1` is frame `0x50`)
- sibling `void *` / `void * volatile *`: size +20
- `action = D_1C4` (non-volatile global): coincidental size 0, extra
  `lui`, slot remains, 88 masked

Ownership, ABI, donor scan, and the 87-site reloc surface are unchanged
from the prior shard. The retired carrier-schedule rewrite is still not
admissible.

Lane `w30-o011` on base `8eb96979`. Identity-gate PASS on stock vs
instrumented IDO (proc 0). The live size fact is now 0: 295/295 words,
frame 0x40, slots 8=8 with identical ladders (extra +0x28 gone). Masked
63 / raw 68 / artifact 5. Aligned 241 exact, 44 naming, 5 immediate, 7
structural. Displacement tax 7. Candidate-only +0x280 (2); target-only
+0x240 (1) and +0x29C (1). First naming +0xD8; first immediate +0x21C;
first structural +0x260. Register census: 44 substitutions, 79 percent
coherence, 5 windows starting +0x1FC, +0x248, +0x2A4, +0x330; not one
ring phase.

The closer for size and the extra slot is the adopted body: volatile
reloads of `status->mode` so consecutive equals-5 or equals-6 does not
range-fold, plus `index = 2` with overlay22 empty-if and L109 OR-zero so
the 0x28 scale survives without spilling `action` across
`func_80005820`. Bare `index = 2` still folds to size -28. Empty-if or
OR-zero alone folds (-12 / -20). Bitwise or of the two compares also
reaches size 0 (65 masked) but keeps a 3+3 insert/delete and is worse
than volatile (2+2). Empty-if on `updateRate`, do-while-zero around
OR-zero, and OR-zero inside the empty-if are byte-identical to the
adopted occupancy. Comma-assign, L97 around GEPs, L144, unused L99
f32/pointer, leftover updateRate OR-zero, sibling do-while, and
D_1C4/D_1B8/D_204 scalars are flat at 63 on this shape (L146 re-test).
Non-volatile `menuInput` regresses to size -20.

Remaining 2+2: candidate occupancy `beq` plus `addiu 2` at +0x280;
target `addiu 4` at +0x240 and `addiu 2` at +0x29C. The two `addiu 2`
are the same rematerialised scale, scheduled 4 bytes apart because of
the extra `beq`. Colour / `web_footprint --every-colour` is still L155
insertion shadow until those two words are gone. Do not restore the
retired rewrite or weaken reloc checks. Do not drop the empty-if /
OR-zero pair without a replacement that keeps the scale.

Next lever: occupancy that keeps `index = 2` from folding into constant
GEP offsets without emitting the extra `beq`, so the rematerialised 2
lands at the target +0x29C and `addiu 4` realigns. Then colour.
<!-- plateau-handoff:func_overlay_011_F0001E4C_186A694:end -->
