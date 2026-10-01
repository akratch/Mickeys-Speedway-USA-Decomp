<!-- plateau-handoff:overlay57UpdateModeState:start -->
### `overlay57UpdateModeState` plateau handoff

- source: `src/overlays/o057/overlay57UpdateModeState.c`
- score: 0/354 words, promoted
- frame: 0x30
- relocations: 59
- first mismatch: none
- summary: Matched. One symbol for the timer at every site, no timer local, and the function-scope eligibility local reused as the loop's dead post-decrement copy.

#### 2026-10-01, lane b-o057: ROM-exact closure, and the recorded floor of 2 was a property of the shape

The section below this one records a floor of 2 and says no further pass is
worth a lane. Both of its residual terms were real, and both belonged to the
inherited shape rather than to the function.

The inherited candidate reached the timer through eight names: the plain
global at three sites, and seven alias arrays, one per store in the dispatch
arms, one for the early store and one for the reload after the call. The
aliases kept the address web down to three sites in seven blocks, which the
allocator coloured whole. Its re-entry after the call is then emitted at the
head of the block, ahead of the aliased reload, and the scheduler tie the old
closure analysed follows from that emission order. Hand-swapping the two
statements in the compiler's own listing and assembling it reproduced the
target's order exactly, so the tie was never a line-number question: the
target emits the value reload first.

Steps, each measured with the configured flags at size delta 0 unless stated:

- inherited shape: 5
- every timer site names the one global, test and decrement read the global
  with no timer local and no written reload: 19. The address is now one web
  across the dispatch, the allocator splits it, and the piece that covers the
  timer block re-enters after the call with the value reload first and the
  address second. The four words after the call are exact. What is left is
  one colour swap between the address piece and the eligibility flag, plus
  the saved-flag store sinking into both arms as before.
- the same with a volatile saved flag: 17, the store lands and only the swap
  remains.
- the same, non-volatile, with the loop's dead post-decrement copy held in
  the function-scope eligibility local instead of a per-arm local: 12, and all
  12 are frame displacement. Redefining the eligibility local in the arms
  stops the copy into the saved flag from being propagated, so the store stays
  at its statement; the eligibility web then spans the dispatch and is decided
  after the address piece, which is the target's colour order.
- declaration order eligibility, saved flag, entries, with no timer local: 0.

Also measured on the unified shape: a timer local with a written reload is
four bytes short at 247; a timer local without the reload is four bytes long;
reusing the saved flag or the eligibility local for the choice or count
carriers is 28 or a size mismatch. Natural and byte-inert on the final shape:
the in-place countdown decrement, an if and else dispatch in place of two
gotos, the call result stored straight into the choice global, and the gate
declared as a scalar.

Verified by the ROM hash, the overlay alias drift check and the per-symbol
promotion proof.

#### 2026-09-10, lane w8-bigclose: the floor is 2, so this function cannot match

Both residual terms are now read off the compilers' own traces rather than
inferred, and one of them is unreachable. **No further pass on this function is
worth a lane.**

**The +0x108 pair cannot be reached from any source.** `cc -Wa,-R` prints as1's
selection chain. Replaying this function's whole trace reproduces **244 of 244**
multi-candidate selections with zero mispredictions under

    (start time, -aftercycles, -latency, node addr, lineno, ready-list position)

with the ready list **LIFO** — a node that just became ready is examined before
an older one. That is [L59]'s chain re-confirmed on a third function, with the
last key pinned as LIFO rather than emission order.

The block after the timer call holds exactly four nodes in two dependent pairs:
the block-entry rematerialisation of the timer's address, and the reload. Both
pairs carry equal `aftercycles` and equal `latency`, so only `lineno` and list
position can separate them — and the two nodes of a pair necessarily share one
line, because an address materialisation is a single ugen line. The target needs
the reload's **first** node to beat the remat's first node *and* the remat's
**second** node to beat the reload's second node. Those are opposite demands on
one per-pair key, so no assignment of line numbers satisfies both. Measured over
the key's entire reachable space, using `#line` to reach the two positions no
legal statement order can:

- reload's line above the call's, the natural order: 5, wrong at the two
  address materialisations;
- reload's line equal to the call's: 7, all four rows wrong;
- reload's line below the call's, reached with `#line`: 5, wrong at the two
  second halves.

The target's row order is none of the three. The one remaining degree of freedom
would be ugen's emission order, and it is fixed: ugen emits the block-entry
rematerialisation immediately after the call and before the next statement's
`.loc` **whatever follows it** — probed with an extra statement placed before
and after the reload, and it does not move. So the earlier closure's "no legal
statement order can reverse it" was right, and the stronger form holds: no legal
line *numbering* reaches it either, because the residual is a ready-list
ordering that source cannot address ([L79]).

**The +0x15c store lands, and the 16 it costs is one colour swap.** `volatile
s32 savedEligible` declared between `timer` and `eligible` puts the store
exactly where the target has it; the other 16 words are register names. An
instrumented `uopt` (CDX log, object byte-identical to the tree's, which is the
identity gate) records **473 p1 decisions and zero p2** for this procedure, so
[L106]'s ascending-web-number axis does not exist here and only [L100]'s
`save = totalsave/nocs` orders anything — [L108]'s question, asked first, and
worth asking first.

In the base, `eligible`'s web spans the dispatch: `nocs` 19, `totalsave` 5,
`save` 0.263158, `decision=split`; its surviving piece is re-decided at `save`
0.666667, **exactly tied** with the timer address web at 0.666667, and the
address web is scanned first, so it keeps its colour and eligible's piece takes
the next. That tie *is* the target's pair, which is why the plain form is right.
`volatile` retires the split — `nocs` 2, `totalsave` 5, `save` 2.5 — so eligible
is decided ahead of the address web, takes its colour, and the address web falls
one further. Every one of the 16 words is that swap.

Eliminated this pass, each measured: restoring the long span while keeping the
store (final test on `eligible` 16, on both 28 and 29, a read-back 291);
[L104]'s redefinition route, reusing `eligible` as an arm carrier so the copy
survives its source, 29 with the choice and count carriers and 14 with prev; all
four `volatile` declaration positions (18, 20, 27, 29); and a 119-combination
flag sweep on *this* base, which still ranks `-O2 -mips2` first.

<!-- plateau-handoff:overlay57UpdateModeState:end -->
