<!-- plateau-handoff:overlay69DrawSortedGeometry:start -->
### `overlay69DrawSortedGeometry` plateau handoff

- source: `src/overlays/o069/overlay69DrawSortedGeometry.c`
- score: 57/359 words
- frame: 0x148
- relocations: 6
- first mismatch: +0x3DC
- summary: Listing rewrite at the target frame. Exact up to the fixed collect block, where the geometry store needs one more ring draw. A same-slot reload, the refs-geometry-keys order, a pointer index, and an early count increment were measured on 2026-10-05 and not kept.
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

<!-- plateau-handoff:overlay69DrawSortedGeometry:end -->
