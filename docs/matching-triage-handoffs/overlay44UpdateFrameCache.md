<!-- plateau-handoff:overlay44UpdateFrameCache:start -->
### `overlay44UpdateFrameCache` plateau handoff

- source: `src/overlays/o044/overlay44UpdateFrameCache.c`
- score: 0/187 words
- frame: 0x48
- relocations: 4
- first mismatch: none
- summary: Exact 748-byte stock C; offset-first addition and ordinary resource-offset access close temporary naming and scheduling, with a used sourceOffset local replacing frame padding.

#### 2026-09-17, lane w8-o044: size closed; colour floor is 30

Arrival was ranking 748 B, delta -4, masked 133, first +0x0, candidate frame 0x40 versus target 0x48, one word short. Aligned buckets were 139 exact, 11 naming, 12 immediate, 29 structural, with displacement tax 81. Candidate-only words sat at +0x68 +0x208 +0x214 +0x290; target-only at +0x198 +0x1FC +0x200 +0x284 +0x288. Frame census: candidate 7 slots, target 6.

The missing word and the eight-byte frame were the same two source-shape facts:

- An unused pointer declared first (L99) grows the frame from 0x40 to 0x48. Declared last it is eliminated and the frame falls back to 0x40. An unused s32 array of length 2 is byte-inert on the 30-word score and does not fix the extra spill slot.
- `nextSlot = -1` belongs after the wrap of `nextFrame`, not hoisted into the 0x80-flag delay. That swap was the +0x68 versus +0x198 one-word pair.
- A block-local `src = (frame * source->frameSize) + source->data` before the cached-frame store is what puts the multiply in the delay-slot shape and closes the jal nop. Inlining `src` (L160) reopens delta -4 and scores 57.

Retained form: 187 words, delta 0, frame 0x48, masked 30, displacement tax 0, aligned 157 exact / 18 naming / 8 immediate / 4 structural. First naming +0x1F4, first immediate +0x218, first structural +0x2A0. Four candidate relocations (HI16/LO16 of the source table and two upload jals); both jals now sit at the target offsets.

Identity gate: instrumented IDO `.text` is byte-identical to the stock object, both score 30 at delta 0. Procedure 0, 20 p1 decisions. Exhaustive `--every-colour` landscape: 157 probes over 20 webs, out directory untracked. No same-kind force beats 30 at delta 0. Nine forces are inert at 30 (web 80 at c3-c6, web 121 at c1/c7, web 142 at c1/c8/c9). L159 packing therefore has nothing to add; L160 does not apply because no force scores 0.

Eliminated on this body, all at the 30-word shape unless noted:

- named `size` local before `src`: 33 masked, naming 21
- `source->frameSize * frame` versus `frame * source->frameSize`: byte-identical
- typed unused (`Overlay44FrameSource *`): byte-identical with `void *`
- named `data` pointer before `src`: 68 masked, delta +4

The remaining 30 is the first-upload register assignment plus stack homes. Candidate still has 7 slots against the target's 6: unique candidate homes at +0x28 +0x2C +0x40, unique target homes at +0x34 +0x38. The 8 immediate rows are the first-call spill offsets. The 4 structural rows are the second-call pair, which spills frameSlot then slot; the target spills slot then frameSlot and reuses two first-call homes. Colour does not move those offsets. The next lever is a source-authentic home packing that drops the extra spill slot, not another colour probe.

#### 2026-09-18, lane w21-o044: extra spill slot packed; 18 naming remain

Arrival reproduced 748 B, delta 0, masked 30, frame 0x48 both, 7 slots vs 6. Identity gate PASS, CDX_PROC=0, 20 p1. First-call spilled t4 t3 t1 v1 to the extra homes; second-call spilled frameSlot then slot onto a new slot. Colour landscape on that shape is void for packing.

L99 packing that closed the extra slot, 30 to 18, homes identical to the target (ladder +0x38 +0x34 +0x30 +0x24 +0x1C +0x18, 6 of 6):

- `source` declared last (t4 stays at +0x24).
- `frame` parked below the spilled cluster (`nextFrame`, `frameSlot`, `nextSlot`) so those three sit at +0x38 +0x34 +0x30.
- `frameSlot = slot` and `nextSlot = slot` moved before each upload so both values are live across the first call and the second call reuses those homes, spilling slot then frameSlot.

Unused pointer remains first (frame 0x48). Coalescing `limit`/`delta` onto the slot names dropped words (delta -4 or -8). L112 pads either matched the unused pointer or grew the frame. Function-scope `src` did not drop the extra slot.

Aligned after packing: 169 exact, 18 naming, 0 immediate, 0 structural. First +0x1F4. The 18 is t5/t9/t7/t8/t2/t3/t6, 7 incoherent windows, not one ring phase. p1 colours stop at t4; t5-t9 are ugen ring. Forces of w7/w88/w74 onto t5 (c12) accepted and scored 106/145/113 at delta -4. nonvolatile source, `handles[frameSlot]`, line-folds, L109 nested OR-zero, and function-scope `src` did not beat 18. Next is a ring-draw / emission-order lever at +0x1F4, not another home permutation or same-kind colour force.


#### 2026-09-27, lane wave-framecache: operand lifetime and access qualification

The configured full-TU baseline reproduced 748 bytes, 18/187 masked words,
169 exact, 18 naming, zero immediate and structural differences, and first
mismatch +0x1F4. Stock and instrumented sections, relocations and symbols
passed workbench fidelity. The configured IDO-preprocessed input passed
self-context comparison. The four relocation records matched runtime offset,
type and stable identity, with no unresolved identities.

The authenticated ugen trace had 42 draws and 333 emission records. Its first
upload evaluated size/product before the data operand, whereas the target's
register roles suggested data/size/product. This was an operand-order
hypothesis, not a missing-draw claim. The matched create-state sibling uses
resource data before size in its second-frame expression.

Reversing the addition operands removed all naming differences, but the
blanket volatile source view constrained load scheduling: the candidate grew
one word and had 170 aligned exact, zero naming, seven immediate and eleven
structural differences (58 positional). Draw count stayed 42; the assigned
register sequence changed. Reading only the resource offset through the
ordinary struct view then reached 187/187 exact words, unchanged frame 0x48
and all six homes. Draws stayed 42 and emission records fell to 329.

The inherited unused pointer was replaced by one used function-scope source
offset shared by the two upload blocks, retaining the exact frame without
padding. ABI review then replaced the draft pointer arithmetic with signed
32-bit resource offsets and the upload declaration with the actual
piRomLoadSection offset, size and return types, retaining a pointer-typed
destination with the same 32-bit calling convention. These cleanups preserved every
owned instruction byte; no inert diagnostic or forced compiler output remains.

The coordinator additionally authorized coherent offset-field typing in the
create-state and draw-state siblings. Both now use the same signed field and
name. The create-state's local offsets and upload declaration follow that
representation. Whole-section, relocation and symbol fidelity passed against
its pre-edit exact object; the guarded draw-state candidate passed the same
comparison. An intermediate integer destination cast added a copy in the
create-state and failed ROM verification; retaining its ordinary pointer
argument removed that copy. No sibling matching credit is claimed.

Storage evidence: the source-table relocation resolves to the resident BSS
identity for D_800D76D0. Mickey's matched fmvInit initializes that table using
piRomLoad resource 0x41; the upload relocation resolves to piRomLoadSection.
The table's offset field is ordinary resource metadata, not a device register.
All scalar metadata accesses retain their prior volatile view; each offset
is read once in its upload block, with no motion across either call. The
ordinary-view cast removes a draft-local qualification, not a shared-header
or external volatile contract. This reconstruction uses Mickey evidence only.

Private candidates, traces, fidelity and relocation receipts remain under
ignored build/wave-framecache. Prior colour and home permutations were not
repeated. The two causal edits and three source/ABI cleanups reached the
stock exact candidate without a sweep.

The canonical promotion proves overlay 44 text +0x294..+0x580, 748 owned
bytes and no padding credit. promotion-proof reports 183 non-relocated words
plus four exact offset/type/identity relocations, static identity proof and
frame 0x48. gmake verify rebuilt the canonical candidates and reproduced the
US ROM SHA1 507341c0a40ca3e9a7cee969b396ee53facfb548. The regenerated scoreboard
credits 748 new bytes. The source contains no GLOBAL_ASM or NON_MATCHING guard.

Commands: configured full-TU compile; align_symbol.py; draw_census.py with
retained traces; candidate_context.py on IDO-preprocessed input; workbench
fidelity; reloc_surface.py compare with overlay 44 and its exact source owner;
overlay-atlas-write; refresh_atlas_digest.py; extract; overlay-syms; parallel
build; verify; promotion-proof; ranking prune, refresh-stale and write-doc; scoreboard; check-docs;
check-overlay-syms; check-nonmatching-builds; cleanroom; check-tooling; and
check-scoreboard. No emulator, generated-game execution or external donor
source was used.

<!-- plateau-handoff:overlay44UpdateFrameCache:end -->
