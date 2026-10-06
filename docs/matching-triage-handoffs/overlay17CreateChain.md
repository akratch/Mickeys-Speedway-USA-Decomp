<!-- plateau-handoff:overlay17CreateChain:start -->
### `overlay17CreateChain` plateau handoff

- source: `src/overlays/o017/overlay17CreateChain.c`
- score: 45 differing words
- frame: 0x80
- relocations: 7
- first mismatch: +0x3C
- summary: Authenticated allocation/material/endpoint calls: 65 to 45; all three call identities align. The independently owned template extent is 16 records; faithful pass captures retain one baseline address definition and two in the branch-local diagnostic. Pre-call size is already coloured a2; five candidate relocations still differ from seven target records.

Summary before this remeasure: Declaration order and chain->red masked with 0xFF (ring phase): 115 to 83. Template-loop webs rotate one position; halfBufferBytes in a3 not t7.

Summary before this remeasure: Template pointer is source on the no-material arm; source-before-destination; while(x--) loops: 130 to 115, frame 0x80 closed. Ring colour rotation remains.

Summary before this remeasure: Fresh V0 retains 130 differences; proxy evidence cannot authenticate the shifted template HI16/LO16 pair, leaving no new source lever.

- geometry: Target and configured C remain exactly `0x310`/784 bytes/196 words; the owned Overlay 17 range is `+0x318..+0x628`, ROM `0x1873CD0..0x1873FE0`, followed immediately by `overlay17ReleaseChain`. Target frame is `0x80`; candidate frame is `0x88`.
- ABI/flags: The configured constructor takes owner/count/material/scale, four `f32` geometry values, and four color bytes, returns `Overlay17Chain *`, and uses overlay game-code `-O2 -mips2 -32`.
- relocation proof: Target has seven runtime records and candidate five static records. Three offsets/types and one stable/effective identity align. The target template pair is at `+0xA4/+0xB0` with identity `overlay:17:+0xA90`; candidate `gOverlay17TemplateReloc` remains unresolved at `+0xA8/+0xB4`.
- diagnosis: Workbench reports 66/196 matching words, 130 raw/masked differences from `+0x0`, 41 opcode mismatches, and no instruction-count delta. Its acceptance basis is relocation-symbol mismatch.
- caller/donor evidence: One resident `R_MIPS_26` caller at VRAM `0x8001C70C` and the Overlay 17 export at `+0x318` authenticate the entry. The retained donor scan remains weak (best Conker Jaccard 0.0574), with no source or relocation witness.
- proxy recheck: Canonical proxy tooling does not infer an identity from the target runtime identity alone and does not normalize the four-byte relocation offset displacement. No exact same-overlay matched sibling supplies a unique witness for this template proxy.
- history: Authorization commit `d5ccd88015299d96c20e3c793d6dfc52bf593eb9` permits only fresh V0 and structured finalization. The guarded C body, prior source forms, flags, and permutation remain closed and unchanged.
- next action: Reopen only if a source-authentic mechanism explains both the frame/schedule difference and the shifted template relocation sites, or if independent exact same-overlay ownership evidence authenticates the proxy without target-assisted circularity.

### 2026-10-01, lane d-ovl2: frame closed, 130 to 115

Priced edits, each measured with tools/shape_product.py at size delta 0:

- `chain->template = source` on the no-material arm (it was 0): the target
  stores the template address there. Frame 0x88 to 0x80 with `vertexCount`
  inlined as `count * 2` and `while (index--)`/`while (buffer--)` loops
  (`!= 0` post-decrement spelling emitted `sltu` and a branch-likely): 130 to 123.
- `source = ...` assigned before `destination = ...`: 123 to 115.
- Flat: `while (vertex--)` against `vertex-- != 0` (same count, matches the
  target's copy shape), halfBufferBytes type, header-size local (worse),
  inline `count * 20` (+8 bytes), loop spellings (index = 16 while, for-down).
- Remaining residual is colour only: template-loop webs rotate one position
  (cand v1,v0,a1,a2,a3,a0 against target v0,a1,a2,a3,a0,v1) and halfBufferBytes
  takes a3 where the target holds it in a ring temp (t7).

### 2026-10-01, lane d-ovl2 (second pass): declaration order and a ring no-op, 115 to 83

- Declaration order, best of 150 random permutations of the eight locals
  (floor 109 reached by several orders): source, vertex, buffer, destination,
  halfBufferBytes, index, the six floats, chain. 115 to 109.
- `chain->red = (red & 0xFF)`: the u8 mask the peephole deletes still spends a
  ring temp (L127); 109 to 83 at size delta 0. It is the best of 37 single
  `& 0xFF` wraps of the u8 reads and stores (next best 85, chain->blue), and
  no pair of them beats it. Wraps on the s32 locals (count, index, buffer,
  vertex, halfBufferBytes) with `& -1`, or-zero, `* 1` and `(s32)` all fold early
  and are flat.
- Left: 58 naming rows, 10 structural; the template-loop webs still rotate one
  position and halfBufferBytes takes a3 where the target holds a ring temp.
#### 2026-10-02, lane g-ovl5: no change, 83

- Template loop as `do/while (index--)` or `for (index = 15; index >= 0; ...)`:
  both 83 (canonicalised); count-up loops 190 at +128.
- Declaration-order hill climb over the 6 locals: floor 83.

#### 2026-10-02, lane x-ovlb: one shared loop counter, 83 -> 65

The decision records ranked the two loop counters and the template loop's
post-decrement copy as separate webs. In the target the template-copy loop
and the alpha-clearing loop use the same registers for counter and copy
(a0 and v1), and uopt colours a symbol as one web. Writing both loops with
`index`, so the separate `vertex` counter goes away, gives the template loop
the target's v0/a1/a0/a2/a3 assignment. 65 at delta 0. Aligned buckets:
141 exact, 31 naming, 7 immediate, 21 structural. The one-sided words are
at +0xC0 and +0x190.

Measured and rejected:

- Five spellings of the half-buffer size and the alloc argument. Computing
  it after the call, inline, or through sizeof was 84 to 184.
- An 8-cell product over where `source`/`destination` are initialised and
  whether the else arm stores the template constant directly. The cells that
  stay well-defined are 65, 90 and 149. The 64 cell reads `source` before it
  is set in the else arm and is rejected.
- A fully natural rewrite, with indexed template copy, resident alloc and
  texture calls on their real arity, and a separate `size` local, is 185 at
  +12. The target keeps the header size in s0, which is the chain variable,
  so the size and the chain must be one symbol.

Still open, each one a named web:

- `halfBufferBytes` before the alloc call. The target computes it in ring
  temp t7 and stores it straight to its home at sp+0x38. Ours colours that
  segment a3.
- The else arm's template address. The target forms it in ring temp t2 in
  the block before the branch and rematerialises it in the textured arm
  (v0). Ours has one coloured web plus a copy.

### 2026-10-03, lane codex-chain-constructor-20261003: sibling constant-identity packet

The newly exact `overlay17AdvanceChain` establishes that signed and unsigned
constant identities can separate a shared scaling constant from an immediate
multiply. This packet tested that mechanism on constructor allocation size,
rather than repeating the constructor's prior full natural rewrite or flag
lattice. Assignment base `5d6feb466` authorized the source/handoff pair
`8158e70087a5322a62fa3db4401bc926b7c37eab`.

The configured full-TU baseline reproduces **65 raw/masked differences, zero
size delta, 784 bytes, and a 128-byte frame**; its first mismatch remains
`+0x34`. The actual IDO-preprocessed compiler input compared with itself is
`unchanged`. A direct configured stock compile reproduces the configured
candidate's measurement. The canonical fallback ROM rebuild also passes the
expected hash. None of these baseline proofs accepts the guarded candidate.

Measured causes and outcomes, preserved privately in `build/causal-chain/`:

- Changing the multiply to `count * 20U`, or casting its index to `u32`,
  is byte-identical to the baseline: both remain 65 at zero delta. The exact
  sibling's constant-type separation does not explain this allocation web.
- Removing the size local and repeating the signed or unsigned product
  distributes the allocation multiply and loses the shared half-buffer
  expression: both score 190 at delta +8. Keeping explicit pair indexing
  restores the target extent, but scores 71 and shrinks the frame to 120
  bytes. This isolates expression sharing from stack-home ownership; neither
  removes the pooled allocation-size web.
- A diagnostic initial template snapshot stored before the material branch,
  overwritten by the destination on the textured arm, scores 132 at delta
  -12. It retains one template address pair. The initialization's earlier
  lifetime does not produce the target's independent rematerialization. This
  nonexact diagnostic remains ignored and is not adopted.

The data-owner review supplies an independent route for a future template
binding: YAML already owns the 256-byte Overlay 17 initialized-data range
with its binary-wrapper object, whose start/end symbols span exactly that
range. This extent agrees with the constructor's sixteen 16-byte template
records and starts at the required module data boundary. No other committed
Overlay 17 source or runtime relocation directly references that owner.
The route is evidence for a future owner-backed binding, not permission to
normalize a mismatching relocation schedule or create a target-assisted proxy.

Acceptance is still blocked by **seven target runtime records versus five
candidate static records**: two template address pairs are required, while
one is emitted. The first two calls also require distinct authenticated
resident identities and ABI declarations; the baseline's one old-style thunk
currently conflates allocation, material lookup, and endpoint calculation.
The target's template sites are `+0xA4/+0xB0` and `+0xC4/+0xCC`; the
candidate's sole pair is `+0xA4/+0xA8`.

No source body changed and no matching credit is claimed. Stop early under
ADR 0018: the sibling's available signedness mechanism is now ruled out and
the prior placement/index/declaration grids remain closed. A useful reopening
must explain the allocation-size web's pre-call temporary versus post-call
colored reload, or trace the template address's PRE/rematerialization decision
with a faithful compiler capture. Mere source placement permutations are not
new evidence. A future near-exact candidate must additionally bind the
independently owned data symbol and prove distinct call identities, exact
relocation multiplicity/sites, linked ownership, and full ROM identity.


### 2026-10-03 follow-up: authenticated calls, 65 to 45

The runtime-table identities independently resolve the allocation call to
`func_8002B280` in `src/main/memory.c` and the material lookup to
`func_80034448` in `src/main/textures_35024.c`; their committed definitions
establish `void *(s32, u32)` and `struct TextureHeader *(s32)`.
Opaque declarations retain the helpers' real structure tags; explicit casts
bridge the constructor's local layout facades without changing pointer bits.
The endpoint call names the now exact same-overlay `overlay17CalculateEndpoints`
and returns `void`. The allocation's artificial third argument is removed;
the material token is converted to the helper's 32-bit integer argument.
The constructor's public ABI, size expression/local, call order, loops and
field accesses remain unchanged. This call-only correction is source
reconstruction, independently reviewed against Mickey's own helper definitions.

Configured full-TU measurement improves **65 to 45 raw/masked differences**
at **784 bytes, zero size delta, frame 128**, first mismatch `+0x3C`.
Stable relocation identity alignment improves **one to three**: all three
call identities now agree. The template mismatch remains: candidate five
records versus target seven, with the same sole candidate template pair.
Thus this is an improved `NON_MATCHING` body, not a match or byte credit.
Baseline and corrected source/object/relocation receipts remain privately
preserved beside the earlier diagnostics.

Three focused continuations tested stack-home and constant identity:
addressing the size local through a pointer gives 138 at zero delta;
self-assignment before multiplication and an unsigned size local each remain
45. Addressing the local does not provide the target's home/topology; the
self-assignment and unsigned spellings add no information. Stop with 45 as
the admissible best and no further placement or typing permutations. The
next causal packet needs faithful optimizer evidence for the pre-call size
web split or template rematerialization, followed by the independently
owned-data binding and complete relocation proof before promotion.

### 2026-10-03, lane codex-constructor-trace-20261003: owned extent and phase boundary

The actual configured baseline reproduces **45 raw/masked differences, 784
bytes, zero size delta, frame 128**, first mismatch `+0x3C`. Captured actual
asm-processor compiler input accepts self-context comparison. Stock pass
captures agree with configured output in executable sections, relocations and
symbols; instrumented index/detail captures pass the same fidelity gates.
Named Ucode and the complete procedure index map the constructor to ordinal
zero, with 25 allocator decisions. These are diagnostic compiler receipts,
not matching proof.

The canonical YAML named binary owner independently fixes the template range
at Overlay 17 `+0xA90..+0xB90`. Its unique wrapper and linked start/end symbols,
whole 256-byte initialized section, asset and private ROM range agree.
Stock compiler sizeof probes prove a 16-byte POD element and 256-byte array,
with executable fidelity. The accepted owner witness additionally proves
alignment two and binds current source, compiler recipe/dependencies and
owner inputs. This authorizes the sole reconstruction change: the existing
external array now has its proven extent `[16]`. Generated code remains at
45 differences with the same size, frame and first mismatch.

The full opt-in witness uses an ignored values file and a **separate diagnostic
ELF** whose carrier is bound to absolute zero. This proves all five candidate
identities without deriving owner identity from target relocation sites. It
still refuses the five-versus-seven record count and displaced sites, with
four offset/type and stable-identity alignments. The zero-bound ELF is not a
physical-owner promotion link and earns no matching credit; a future exact
candidate still needs reviewed real-owner binding and linked/full-ROM proof.

Measured phase findings:

- The baseline CFE stream has one template address definition; optimized UGEN
  input retains one. It does not contain two surviving definitions that UGEN
  later collapses.
- A defined, private branch-local diagnostic initializes the same template
  cursor inside the material arm and assigns the global template directly in
  the else arm. It has two CFE address definitions and retains both after
  optimization. Fresh procedure mapping and stock/instrumented fidelity pass;
  its 142 differences at delta +4 remain ignored. This eliminates address
  elimination before UGEN for that form, without repeating a score search.
- The baseline optimized input already assigns the pre-call half-buffer
  product to `a2`; the discrepancy precedes UGEN. This does not authenticate a
  source-to-allocator-web join or a producer-emitted final stack home.

The installed profile emits no PRE decision events or symbol-table identity
surface. Source semantics and final-home ownership remain unavailable; raw
words, line numbers and web ordinals cannot supply those facts. Stop early
under ADR 0018 with one stage-only source diagnostic: the assigned definition
mechanisms are resolved and provide no further authentic source lever.
Private captures remain under `build/constructor-trace/`. Resume only with an
independently instrumented PRE/rematerialization or stable web/home producer,
followed by exact relocation multiplicity/sites and linked owned bytes.

#### 2026-10-05: repeating the half-buffer product widens the function

Configured full-TU baseline: 45 masked and 45 raw words, target 784 bytes, size delta 0, first mismatch +0x3C. Aligned exact 161, naming 11, immediate 10, really different 18, displacement tax 6. Candidate 196 words, target 196. Owned text sha1 c2f8207c311326cee69bfaaa485e580869db8afd.

Deleting the declared half-buffer size and repeating count times 20 at the allocation and at both buffer offsets scores 195 masked and 195 raw words at size delta +8. The candidate grows to 198 words. Aligned exact falls from 161 to 74, naming rises from 11 to 79, immediate falls from 10 to 9, and really different rises from 18 to 41. Displacement tax rises from 6 to 66. The aligned residual rises from 39 to 129. The absolute size delta grows from 0 to 8. The first mismatch moves to +0x0. The body is not kept. The restored source re-scores 45 at delta 0, and the 784-byte function text matches sha1 c2f8207c311326cee69bfaaa485e580869db8afd. Do not repeat this product. The pointer-addressed size, the self-assignment, and the unsigned size stay closed.

#### 2026-10-06: the half-buffer product after the allocation grows the function

The unmodified body scores 784 bytes, 45 raw and 45 masked words, size delta 0, first mismatch +0x3C. The ROM shifts the count in a temporary. This body shifts it in a2.

Inlining count times 20 only in the allocation call, and assigning halfBufferBytes after that call returns, scores 192 masked and 192 raw words at size delta +8. The candidate grows by 8 bytes. The first mismatch moves to +0x18. Not kept. The 45-word body stays. Do not repeat this late product. The three-site repeat stays closed.

#### 2026-10-06: an or-zero on the half-buffer product is inert

The unmodified body scores 784 bytes, 45 raw and 45 masked words, size delta 0, first mismatch +0x3C. The ROM shifts the scaled count in t7. This body shifts it into a2.

Folding an or-zero onto count times 20 scores the same 45 masked and 45 raw words at size delta 0. The mismatch list is unchanged. The identity is folded. Not kept. The 45-word body stays. Do not repeat this or-zero. The late product and the three-site repeat stay closed.

#### 2026-10-06: one leading frame home moves the size slot

The unmodified body scores 784 bytes, 45 raw and 45 masked words, size delta 0, first mismatch +0x3C. The half-buffer home sits four bytes above the ROM slot. Declaring the loop index before that size leaves the 45-word mismatch list unchanged.

Declaring one unused s32 before the template pointer scores 36 masked and 36 raw words at size delta 0. The function stays 784 bytes. The first mismatch stays +0x3C. Aligned immediate rows fall from 10 to 0, and naming and structural rows stay at 11 and 10. The body is kept. It is not a match. Do not drop this slot: the size home moves back without it.

<!-- plateau-handoff:overlay17CreateChain:end -->
