<!-- plateau-handoff:func_overlay_007_F0000324_185C1AC:start -->
### `func_overlay_007_F0000324_185C1AC` plateau handoff

- source: `src/overlays/o007/func_overlay_007_F0000324_185C1AC.c`
- score: 10/348 words
- frame: 0x78
- relocations: 61
- first mismatch: +0x120
- summary: Colour floor 10: web 99 denied f0 (L142); a distinct f0 name costs a twelfth home or size. as1 delays are besttime, not lineno.

## 2026-09-11 frame closed (lane `lane/o11-frames`)

The frame identity, solved from both objects rather than guessed.  Argument
build, the five saved-register words and the `ra` slot sit at identical
absolute offsets on the two sides, so the whole 8-byte gap was the declaration
block: this candidate's was 56 bytes for fourteen locals, the target's is 44 for
eleven.  What pins the count at eleven rather than twelve is the compiler
temporary that carries the first measurement across the second call -- it is the
*second* temp below the block on both sides, so an eleven-cell block puts it at
-52 and a twelve-cell block at -56.  The twelve-declaration control, with a
separate `scale`, was built and measured: frame back to 0x80, immediate-only
bucket back to 24.

Three declarations came out, none of them by merging into a live carrier:

- `handleObject`: deleted, the call reading `overlay7RuntimeLastObjectReloc->x/y/z`
  directly.  uopt commons the global load into one `v0` temp, which is what the
  target does.  Byte-identical code, one declaration fewer.
- `cursor`: deleted, the loops indexing `objects[remaining]`.  Strength reduction
  rebuilds the identical walking pointer in `s1`.
- the nested `scale`: deleted, its hoisted loop invariant reusing the `difference`
  carrier, whose live range is the first object loop and therefore disjoint.

Then declaration order, which is free: `objects` 6th, `object` 7th, `difference`
11th.  That reproduces the target's stack ladder exactly -- all fourteen slots,
the compiler temp included.  The other eight positions are byte-inert; four
random permutations of them scored identically.

Measured, aligner buckets before and after:

- before: 238 masked, frame 0x80 against 0x78, 229 byte-exact, 45 naming,
  28 immediate-only, 50 really-different, first mismatch +0x0
- after: 129 masked, frame 0x78, 280 byte-exact, 30 naming, 5 immediate-only,
  33 really-different, first mismatch +0x7C

The immediate-only bucket is the one that tracked the frame: 28 to 5.

One further structural edit is folded in: `entry` is assigned inside the
`D_290->nested != NULL` arm rather than above the test, which is what makes the
target's `move` of the commoned load into the `entry` carrier appear.

### What is left: three words, both named

The candidate is now twelve bytes short, and the alignment names all three
missing words.

1. The `D_844` scan.  The target counts a register down from 43 and walks the
   table pointer; this candidate's `index` dies at strength reduction and the
   loop compares the pointer against a precomputed end.  The target also copies
   its counter to `v0` every iteration, dead on the path that takes the call, so
   its counter is live across the call in uopt's view and this one's is not.
   That accounts for one word.
2. The sixth argument of `overlay7CreateRuntimeOwnerReloc`.  This candidate
   passes `NULL` and stores `zero`; the target materialises a relocated address
   into a register and stores that, which is two words.  The argument is not
   null, and identifying what it points at is reconstruction, not allocation.

Thirty naming rows remain after that.  They are not a frame problem any more.

## 2026-09-18 size closed (lane `lane/w17-o007`)

The three missing words closed at frame 0x78.  Aligner after the two source
edits: size delta 0, 348 words, 10 masked, 338 byte-exact, 3 naming, 0
immediate, 7 structural, first mismatch +0x120.

1. The `D_844` scan is `index = 0x2B; do { D_844[0x2B - index] } while
   (index--)`.  That is the countdown plus walking pointer, and it emits the
   extra copy of the counter that keeps the index live across the adjust call.
2. The sixth argument is `&overlay7RuntimeHandleReloc`.  Runtime LOCAL HI16/LO16
   at function +0x410/+0x420 share the data-section base and stored addend 0
   with the handle load, so the call takes the address of that slot (the
   `func_80002FE0` handle-pointer argument), not NULL.

Call test: p1 only.  Instrumented IDO `.text` is byte-identical to stock.
`CDX_PROC=0` (one procedure, 40 p1 decisions, 0 p2).  Scale web 99 took
colour 26; its `p1cost` list starts at 26 and `forbidden0` has the two
caller-float bits, so `CDX_FORCE=p1:w99=c24` is `forced=-2` and never
applied (L142).  L139 splits on other float webs are not this residual.

Tried and rejected on the remaining 10, all against this size-0 baseline:

- Nested `Overlay7Pair *pair` cursor: frame 0x80.
- `state->scale *= overlay7RuntimeScaleReloc` with no carrier: size +4.
- Two block-scoped `difference` symbols: frame 0x80, naming 3 to 1 (the f0
  split is real) but the twelfth home is the cost.
- L144 address-form of `difference` at the multiply: size +8.
- L144 address-form of the scale global: flat 10.
- L109 D_844 address probes: flat or 43 masked.
- L59 same-line folds: flat 10.
- L97 `if (1)` around the scale loop: flat 10.
- Pun through `found`: size +32 and an extra slot.

The 10 are three as1 scheduling windows (remaining versus the objects reload,
the scale address versus the index shift, and `li 43` versus `%hi(D_844)` in
the same delay) plus f12 versus f0 on the scale web.  Do not reopen the
declaration block.

## 2026-09-18 colour and as1 closed (lane `lane/w20-o007`)

Identity gate: instrumented IDO `.text` is byte-identical to stock.
`CDX_PROC=0`, 40 p1, 0 p2.  Reconfirmed web 99 (`difference`, type-3,
`bbs=26,27,28,45,46,47,48,107`) `p1cost` starts at colour 26, `forbidden0=0xC0`
(the two caller-float bits).  `CDX_FORCE=p1:w99=c24` is not on the offer list.

Offered caller colours of web 99, all accepted at size delta 0:

- c26 (incumbent f12): 10
- c27: 16
- c28: 26
- c29: 25

The incumbent is the best colour this web is offered.  L142 arity cannot
help: the denied bits are the float return registers, which any spanned call
sets, not f12/f14.

Forced split `p1:w99=s` accepted (`forced=-1`).  A remainder row then
reports `bestcolor=24 forbidden=0 totalsave=11 nocs=2` and would colour at
f0, at size +8.  Nested `f32 scale` for the multiply creates web 138, which
takes c24 with empty forbidden, at size 0 but 32 masked (24 immediate-only:
the twelfth home).  `register f32` is byte-identical to that nested form.
No-carrier global, puns through `found`/`index`, store-first, and first-loop
expression doubling all reopen size (plus 4 to plus 40).

`--every-colour` landscape: 264 probes over 30 coloured webs, out directory
untracked.  173 accepted at size delta 0.  None beat 10; the best ok cells
are 13 (web 90 at c2-c6, web 202 at c1/c2/c4-c6).  No cell scores 0.  L140:
the wanted f0 colour belongs to a web this body does not have.

as1 `-R` on the three delay windows: lever 42 / overlay40 line-join of the
initializer onto the loop header is flat (or plus 1 for the scale join).
The remaining-versus-objects pick is `besttime` 0 on the stack reload
(latency 3) against `besttime` 3 on `count - 1`, not lineno.  The `li 43`
versus `%hi(D_844)` pick is `aftercycles` 0 against 1; tying lineno leaves
that key in place.  L97 around the first objects loop, remaining-in-condition,
index hoisted above `if (found)`, nested `if (1)` around the countdown,
`D_844` discarded probe, and value/index/do on one line: all flat 10.

Decision variable: `difference` is one IR name whose hull spans the measure
and record calls, so the scale fragment is denied f0.  A second name that
does not span a call colours f0, and every such name measured here is either
a twelfth home or a size change.  The three as1 windows sit below
globalcolor (the 264-cell floor is the incumbent 10).  Next lever is a
source form that gives the scale a distinct IR name without a new home and
without aliasing `overlay7RuntimeScaleReloc` through `state->scale`.
<!-- plateau-handoff:func_overlay_007_F0000324_185C1AC:end -->
