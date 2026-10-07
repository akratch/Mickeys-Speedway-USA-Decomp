<!-- plateau-handoff:overlay69DrawSortedGeometry:start -->
### `overlay69DrawSortedGeometry` plateau handoff

- source: `src/overlays/o069/overlay69DrawSortedGeometry.c`
- score: 0 differing words
- frame: 0x148
- relocations: 6
- first mismatch: none
- summary: Matched. Geometry address as a word subscript scaled by 16, stores refs, geometry, keys; shared by overlays 69 and 88.

Summary before this remeasure: Listing rewrite at the target frame. Exact up to the fixed collect block, where the geometry store needs one more ring draw. A same-slot reload, the refs-geometry-keys order, a pointer index, and an early count increment were measured on 2026-10-05 and not kept.
#### 2026-10-02, lane x-sort: made measurable, rewritten, 140 to 57

The body was not in the queue: overlay 69 compiled it under a private guard
and overlay 88 included it, so no tool could score it. Overlay 69 now holds a
standard NON_MATCHING definition of the real symbol; overlay 88 renames the
function and its five callee proxies to its own symbols and includes the file,
and its NON_MATCHING object is byte-identical in .text to overlay 69's. Both
modules' relocation tables agree call for call (one camGetProjZ role at both
metric sites, the overlay 17 resource draw, the camera transform, camDoSprite,
the draw-cone call).

Measured steps (masked words, size delta), each a product cell:

- Natural rewrite, no register, no carrier copy of commands, no byte-offset
  stores, two scalar homes declared above order[]: frame 0x148, but 190 at -4.
- count++ before i++ in the dynamic collect loop (draw census): 187 at -4.
- Fixed-block statement order refs, keys, geometry: 135 at 0; with the
  increments on one line, 130.
- Bubble-sort swap through one temporary instead of left/right carriers (the
  target spends one more ring draw per inner iteration): 130 to 63.
- Submit loop forms the entry pointer before its order slot: 59.
- Dynamic collect loop as for (i = 0, count = 0; ...; i++): 57.

Everything before +0x3DC is now byte-exact. The residual is one ring position
in the fixed collect block, after the reference store. The freelist replay
shows the target's geometry store draws its address register from the list
head one position later than this source does. With the stores in the
target's emission order (refs, geometry, keys) this source recomputes
sp+count*4 into the same register the reference store used and as1 deletes
the copy (four bytes short); the target's copy lands in a fresh register.
Inert or worse, all measured: casts on the geometry value, Mtx-typed geometry
bases, pointer-form stores, do/while(0) and if(1) regions (frame moves),
count++ placement, five spellings of the active/reference test, while
against for. Next: find the source construct that spends one ring draw
between the reference store and the geometry store without emitting code
(an as1-deleted move), or read the fixed sort and final loop once that block
is exact; both later regions currently differ only by the inherited shift.
#### 2026-10-02, lane x-sort (cycle-46 pass): the residual is one ugen draw

Measured on the full TU with the stores in refs, geometry, keys order, the
freelist trace replayed per draw, and a small test TU with the same store
tree. Findings:

- ugen evaluates a store's value before its address in every case measured
  (stack array, pointer field and plain pointer targets; values of one to
  eight registers). So the target's geometry-store address register is not
  an address-first evaluation; it is one extra ring draw at the head of the
  geometry statement.
- Proof by construction: spelling the index load
  `*(state->fixedGeometryIndex + i)` makes uopt compute state + i afresh, which
  spends exactly that draw (one visible addu). With it, every register in the
  fixed block, the fixed sort and the final loop matches the target; what
  remains is that one addu, the schedule around it inside the block, and the
  state/resources colour swap its extra use of state causes. So the whole 57 is one invisible draw before the
  geometry index load, and nothing else.
- Spellings uopt normalises away (object byte-identical, 115 at -4 in this
  order): casts and round-trips on the stored value, the index, the base,
  the group and the count (s32, u32, s16, void * round-trips); OR-zero,
  XOR-zero, AND-minus-one and +0 probes on the index; 64 against 64U, 0x40,
  64 * idx, shifts; volatile and address-taken reads of the index, group,
  base and reference; self-assignments of count, i and reference between the
  stores; six reference types. Spellings that change size: (s16) casts,
  count - 1 indexing after an early count++, a shifted G index (+4 each);
  regions and slot copies (frame or size moves).
- A bounded permuter run (25 minutes, the body is now importable) found
  nothing better under the ranking's scorer; its best cell is the -4 order.

Next: a construct that makes ugen compute the geometry address (sp plus
count*4) at the head of the statement into a free temporary, so the store's
own recomputation lands in the same register and as1 deletes it; or one that
copies the index load through a second temporary as1 renames away. Either
spends the draw without a word.

#### 2026-10-05: four folded-address attempts stay off the 57-word body

Configured full-TU baseline remains 57 masked and 57 raw words, target 1436 bytes, size delta 0, first mismatch +0x3DC. The kept store order is refs, keys, geometry.

- A second store of fixedGeometry[count] from itself, the store-then-reload, scored 122 masked words at size delta +8. The copy survived as a real store. Not kept.
- The target store order, refs then geometry then keys, scored 115 masked words at size delta -4. as1 still deletes the repeated stack address. Not kept. Do not repeat this order on the current body.
- That order plus a pointer-plus-index load of the geometry byte scored 123 masked words at size delta +4. The added address and the deleted stack address do not cancel. Not kept.
- Incrementing count between the geometry store and a count-minus-one keys store scored 171 masked words at size delta +4. Not kept.

The 57-word body stays. No bytes are credited. A later pass needs a zero-word draw that this reload, this order, and this pointer form do not produce.

#### 2026-10-06: indexing the geometry store through a copy of count

Configured full-TU baseline: 1436 bytes, 57 raw and 57 masked words, size delta 0, first mismatch +0x3DC. The copy is the existing unused `right` local, assigned from `count` and used only as the geometry-store index. The candidate scores 57 raw and 57 masked words at size delta 0, and the first mismatch stays +0x3DC. The object is not the baseline object. The masked count does not fall, so the 57-word body stays. Do not repeat this index copy.

#### 2026-10-06: a pointer-plus-index geometry load on the kept order grows the function

The unmodified body scores 1436 bytes, 57 raw and 57 masked words, size delta 0, first mismatch +0x3DC. The stores stay refs, keys, then geometry.

Loading the geometry index as a pointer plus i, instead of a subscript, scores 129 masked and 129 raw words at size delta +4. The candidate grows by 4 bytes. The first mismatch moves to +0x44. Not kept. The 57-word body stays. Do not repeat this pointer form on the kept order. The count-index copy and the target store order stay closed.

#### 2026-10-06: an all-ones mask on the fixed reference is folded

The unmodified body scores 1436 bytes, 57 raw and 57 masked words, size delta 0, first mismatch +0x3DC. The ROM recomputes the stack address for the geometry store. This body reuses the address from the reference store.

Masking the reference with all-ones before that store scores the same 57 masked and 57 raw words at size delta 0. The mismatch list is unchanged. The identity is folded. Not kept. The 57-word body stays. Do not repeat this mask. The pointer form stays closed.

#### 2026-10-06: masking the geometry subscript count grows the function

The unmodified body scores 1436 bytes, 57 raw and 57 masked words, size delta 0, first mismatch +0x3DC. The ROM recomputes the stack address for the geometry store. This body reuses the address from the reference store.

Indexing that store with count masked by all-ones scores 115 masked and 115 raw words at size delta +4. The candidate grows by 4 bytes. The first mismatch moves to +0x44. Not kept. The 57-word body stays. Do not repeat this subscript mask. The reference mask stays closed.

#### 2026-10-06: a byte truncation of the key grows the function

The unmodified body scores 1436 bytes, 57 raw and 57 masked words, size delta 0, first mismatch +0x3DC. The ROM recomputes the stack address for the geometry store. This body reuses the address from the reference store.

Storing the key and then truncating that stored half to a byte scores 156 masked and 156 raw words at size delta +12. The candidate grows by 12 bytes. Not kept. The 57-word body stays. Do not repeat this truncation. The subscript mask stays closed.

#### 2026-10-07: canonical callee identities and private byte-mask controls

The retained source remains 57 masked words at the original extent and frame.
The five opaque callee declarations now name Mickey's actual strip renderer,
projection-depth helper, transform preparation, sprite submission, and cone
renderer. The independent linked definitions agree with the runtime call
records; the strip call belongs to overlay 17's export. Per-object symbol-only
renames retain separate overlay 69 and 88 relocation names. Renaming these
references leaves every owned instruction byte and all six relocation slots
unchanged. Overlay 88 still compiles its own wrapper and needs separate proof.

A private fixed-key byte mask reduced the residual to 48 words at unchanged
size and frame. Signed versus unsigned mask constants, a destination-width
cast, and unsigned address-addition spelling retained 48. A source-width byte
cast gave 49 with a different temporary sequence; combining that cast with
retail store order again lost one instruction and gave 115. Earlier guard-mask
controls gave 65 or 66, and signed geometry-byte masking grew by eight bytes.
These nonexact inert diagnostics remain private; none replaces the retained
source or earns credit. Manual controls stopped without further improvement.

Search preparation exposed unresolved opaque callee identities, a stale
canonical-object snapshot after editing, and missing independent proof for a
friendly cross-overlay callee name. Build the canonical fallback before taking
search receipts. The friendly-name proof and included-wrapper proof must pass
before a bounded search may consume the retained private 48-word candidate.
Do not repeat the closed store-order or index-copy controls without new evidence.

#### 2026-10-07: authenticated searches close the byte-phase packet

Both wrappers now resolve through configured candidate preprocessing, and all
six call identities and slots pass independent preflight. The overlay 88 object
also explicitly depends on the included body. A fresh canonical callee rebuild
and full fallback ROM verification passed before the bounded search.

The first six-minute search authenticated all six candidate bindings and kept
compiler context unchanged. Its proxy winner reproduced the already-closed
retail store order: 115 words, one instruction short. Replaying its next result
in the configured full TU gave a private best of 46 raw and masked words at the
original extent and frame, first mismatch +0x3C4. That variant orders the stores
geometry, references, keys and retains the inert key mask. A copied command
cursor result gave 151 words and again lost one instruction. A second five-minute
search from the 46-word seed returned the same rejected size-short winner;
neither search found an exact candidate.

A duplicated nonvolatile byte-field condition shared a later load and shortened
the candidate by three instructions. An independent initialized flags-field
condition kept the extent but regressed to 275 words. These controls did not
supply the required temporary without disturbing other allocation. Stop this
phase mechanism: do not repeat its masks, cast/order interactions, duplicated
conditions or either bounded search without new structural evidence.

The canonical 57-word source was restored exactly. Fresh configured builds of
both overlay owners reproduce 57 raw differences, first +0x3DC, with 1,436 owned
bytes each. The 46-word candidate and all search/capture evidence remain private;
no executable bytes or matching credit were added by this packet.

#### 2026-10-07, lane a-ovl1: draw arithmetic for the geometry store

The kept body re-scores 57 at delta 0, first +0x3DC. From `cc -S` on the
refs, geometry, keys order, the geometry statement's draws are t5 t6 t7 t8
t9 t2 t4 t3 (index load first, address last, landing on the refs store's t3,
which as1 then deletes: -4). The target's registers for the same statement
read t6 t7 t8 t9 t2 t4 t3 t5: the same free list with t5 consumed and
returned to the tail before the index load. Replayed by hand, the keys
statement after it would then draw t6 and t8, which is what ships; that
favours refs, geometry, keys as the source order and argues against an
extra draw in the keys store (inferred from the free list, not compiled).
So the target needs one draw-and-free between the refs store and the index
load, or a move after the index load that as1 deletes by renaming the load
(L150) -- either gives the shipped registers exactly.

Measured, none kept:

- The index through an existing s16 local (`j = index;` then `j * 64`) in
  refs, geometry, keys order: 57 at delta 0, and the fixed block's
  instruction sequence equals the target's word for word (the address is
  no longer deleted), but j is coloured a1 so the index spends no ring
  draw and every following register is one position early. Through `slot`
  62, `right`/`left` worse.
- Index conversions (s8 field read as `(s8)` of a u8 field, `(s8)(u8)`,
  `(s8)(x & 0xFF)`, `(s32)(s8)(s16)`): uopt removes all of them; with the
  key mask, 48 as already recorded.
- Re-reading `state->fixedRefs[i]` for the refs store: delta 0 in the
  shipped order, but the reference no longer spills and the frame moves
  (112). `&base[index * 64]`, u32 arithmetic, index-first operand order
  and a store-then-reload of the geometry slot: 112 to 168.

#### 2026-10-07, lane a-ovl1 (resumed): zero-word draw constructs are all removed by uopt

In the shipped order (refs, geometry, keys), a 22-cell product placed one
statement between the refs store and the geometry statement: `(void)` of a
field load, a dead store to an s16 local, `j++`, `left = count`, an unused
compare, `reference = fixedRefs[count]`, a dead store to an address-taken
local, `vector = vector`, `i = i`, a pointer copy. Every one compiles to the
same object as the order without it (-4, the deleted address); `j++` is
+36. None reaches ugen, so none draws a register.

Read the free-list arithmetic again: the base order's geometry draws are
t5 t6 t7 t8 t9 t2 t4 t3. The shipped registers equal the same list with the
store address taken FIRST (t5) and the value after (t6 .. t3), with no extra
draw. So the open question can also be stated as "what makes ugen evaluate
this stack store's address before its value". A mini TU confirms ugen puts
a stack-array element's `sp + index` address after the value for `a[i]`,
`*(a + i)`, a byte-offset cast, a 2-D array and a struct member array. The
only address-first case was a uopt address CSE shared by two stores, and
that lands in a coloured register, not the ring. The s16 index local (j)
gives the shipped instruction sequence at delta 0 but spends no draw (57).

#### 2026-10-07, lane f-o069: the ring arithmetic settled, five cell families closed

Kept body re-scores 57 at delta 0, first +0x3DC. The refs, geometry, keys
order (115 at -4) is the base for every cell below, because the ugen trace
puts the target's statement order there: replaying the free list as ugen
keeps it (a draw takes the head and re-appends it to the tail; a free MOVES
the register to the tail, so a register held across other draws comes out
later) reproduces every target register after the geometry store, the keys
lbu t6 and address t8, the count narrowing's surviving sll temp t9, the i
narrowing's t7 and the post-loop t3 t5 t6, from one event: between the refs
store's address draw (t3) and the geometry index load, one register is
drawn and freed, or the geometry address is drawn first and held. Both
readings give identical registers everywhere.

Measured on the full TU:

- Address-first is closed. ugen's f_eval_2ops evaluates the store VALUE
  before the address even when the address needs its own draws (index
  `count + i`: the sll and addu for the address are drawn after the value,
  172 at +16). Two stores to the same stack element make uopt CSE the full
  address into a COLOURED temp (`addu v1, v0, sp+0xC0`, 2 words: a NULL
  store first 122 at +8, a duplicate identical store 121 at +36 with the
  duplicate eliminated), never a ring draw.
- Product-by-zero probes (`+ i * 0`, `+ count * 0`, `(idx + i * 0) * 64`,
  `0 * i +`, `(s32)reference * 0`, on the group index, on the refs value):
  seven cells, every object byte-identical to the base. uopt folds an
  integer product by zero.
- 27 zero-cost constructs (empty `if (reference) {}` 307 at -8, `do {}
  while (0)` around or before the statement 300 at -8, `<< 6`, `* 8 * 8`,
  `(s32)` on the product, `+ (reference == NULL)` +8, `* 64 * 1`, `(s8)(x *
  1)` +8, `slot = count` indexing, `(void *)(u32)` casts, `fixedRefs[count]
  = fixedRefs[count]`, comma expressions discarding count, reference, the
  key byte, metrics[count] or the index, `reference ? reference : NULL`
  +36, an or-zero on the pointer): all 115 at -4 except `metrics[count] =
  metrics[count]` (61 at 0: uopt moves the metrics store after the refs
  store, not a fix) and the s16 index local (57, a-ovl1's cell).
- `slot = (s32)count` with slot used by the geometry, keys or refs index,
  dead, or copied on (eight cells): byte-identical; uopt folds a narrowing
  of a declared s16.

Two zero-word draws exist in this function and both are measured, not
inferred, from the trace against the object: (1) the sort's `left =
order[j]` is a dead narrowing of the 2-use load temp a0 (`sll a2; sra t4;
move a2`) that as1 deletes as a dead chain, one draw and no word; (2) every
s16 `count++`/`i++` narrowing draws two temps and as1 renames the second
away. Neither has a host between the refs store and the index load: (1)
needs a coloured load temp known s16-ranged, and every load after the call
in that block is a single-use ring load; (2) is pinned to the count and i
increments at the block end. The L129 reload needs a use of the stored
value, and the target reads v1 nowhere after the refs store.

Cycle-21 line: the decision variable is one ring draw at the head of the
geometry statement. The next thing to measure is a draw that ugen spends on
a *value* evaluated before the index load whose instruction is deleted
downstream: write the geometry value with the index through a chained
assignment into a declared s16 (`(j = state->fixedGeometryIndex[i]) * 64`)
and read the trace, expecting the dead narrowing's draw after the lb (one
late, 115 + rotation) unless uopt orders the CVT first; and instrument
f_eval_2ops directly for the operand-order rule.

#### 2026-10-07, lane f-o069 (resumed): chained assignments are normalised

Traced on the refs, geometry, keys order: `(j = state->fixedGeometryIndex[i])
* 64` compiles exactly as the plain `j = ...;` cell (the lb goes straight to
j's colour, no draw, 57 at delta 0), and `geometryBases[slot =
resources->geometryGroup]` likewise (lh into slot's colour, 99). uopt turns
an embedded assignment into a variable definition with a direct load and
folds the narrowing, so no dead CVT temp exists for a load that fits the
variable. The dead-narrowing generator needs an int-typed two-use load
temp, which this block has none of after the call. Cycle-21 line unchanged:
instrument f_eval_2ops for its operand-order rule, since the value-first
order and one draw are all that separate this block from the target.

#### 2026-10-07, lane g-5: the geometry value through a reused pointer local

Kept body re-scores 57 at delta 0, first +0x3DC (fast_score). One product,
none kept: the geometry value computed into an existing pointer local
(`vector` or `entry`, both dead after the call's arguments, so no new frame
cell) and stored from it, with that assignment placed before the refs
store, after it, or embedded in the geometry store, and the stores in
refs-geometry-keys or refs-keys-geometry order. Eight cells: 281 to 316 at
+8 or +12. The local is coloured and its definition and store become
separate statements, so the block grows instead of gaining a ring draw.

Read (no build): in the decompiled ugen, `f_eval_2ops` orders a binary
node's operands by the register-need byte at node offset 22, but the ISTR
case of `f_eval` does not use it: it calls `f_eval` on the value operand
unconditionally first and only then evaluates the address's register part.
So value-first is the code path, not a measured tendency; address-first is
closed for every store spelling, as f-o069 found.

Cycle-21 line unchanged: one ring draw at the head of the geometry
statement, which must come from a value-side construct (a draw-and-free on
the value path before the index load). Next: instrument a value whose
evaluation ugen begins with a register it frees without emitting (a CSE
reload f_load_cse declines, or a cvt that f_eval_int_int_cvt folds), and
read the trace on the refs-geometry-keys order.

#### 2026-10-07, lane i-7: matched (scaled word subscript for the geometry address)

The missing draw before the geometry index load is checklist item 43's
scaled subscript. Writing the geometry value as an element of a word array
scaled by 16 (`&((s32 *)base)[index * 16]`) makes ugen shift the index by 4
and then scale the subscript by 2 into a second register; as1 folds the two
shifts into the shipped single `sll 6`, but the second draw stays spent, so
the store address lands one ring position later, on the register the target
uses. With the stores in the order refs, geometry, keys (the order the
earlier free-list replay pointed to), 0 masked at delta 0, aligned exact 359.

Product (one cycle, 14 cells), masked at size delta:
  - u32, u16 (index * 32), 16-byte struct (index * 4), 8-byte command
    (index * 8) and f32 (index * 16) element types: 0 at 0 in the
    refs, geometry, keys order, 48 at 0 in the kept refs, keys, geometry;
  - the byte form (`(u8 *)base + index * 64`) and a 64-byte struct element
    (`&((Geom64 *)base)[index]`): 57 at 0 kept order, 115 at -4 shipped
    order, as before. A single scaled multiply spends no extra draw.

Overlay 88 includes the same body; both objects pass promotion proof (359
words, frame 0x148, 6 of 6 relocations).
<!-- plateau-handoff:overlay69DrawSortedGeometry:end -->
