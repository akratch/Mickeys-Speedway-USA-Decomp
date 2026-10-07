<!-- plateau-handoff:func_overlay_056_F00001A0_18A2F18:start -->
### `func_overlay_056_F00001A0_18A2F18` plateau handoff

- source: `src/overlays/o056/overlay_056.c`
- score: 24 differing words
- frame: 0x1F8
- relocations: 75
- first mismatch: +0x160
- summary: shade test through a declared s32 (a former pad) retires the int ring cycle: 127 to 24 at 0; open: dl v0/v1, D_84 home, product order.

Summary before this remeasure: x/z locals, in-place mapY rebase, frame refit, block temp for cos: 542 to 127 at 0; open: product emission order, int ring.

Summary before this remeasure: x/z locals, in-place mapY rebase, frame refit, block temp for cos: 542 to 129 at 0; open: product emission order, int ring.

Summary before this remeasure: x/z locals, in-place mapY rebase (CSE temp in f20), x-term-first mapY, frame refit: 542 to 131 at 0; open: cos/sin, int ring phase.

Summary before this remeasure: y sum merged into ghost x (mapY f20), frame ladder, ternary fade shift: 542 to 152 at 0; open: sum/mapX, cos/sin, scale/x colours.

Summary before this remeasure: mapY in f20 via the y sum merged into the ghost x web, frame ladder for the f24 save: 542 to 163 at 0; open: sum/mapX, cos/sin, scale/x colour pairs.

Summary before this remeasure: r4300_mul, u8 colour params, ghost racer local, frame ladder: 577 at -8 to 542 at 0; open: mapY caller cost 20 vs toll 21.

Summary before this remeasure: Natural rewrite: aligned 316/121/61/90, delta -8; open: mapY web 197 takes caller c29 (20.0) under the 20.75 callee toll, no f24.

Summary before this remeasure: Colour table reconstructed as a field at overlay-data +0x50 (gOverlay56Data.colors[index]) so both marker sites emit lui plus scaled addu plus lw 0x50 off the data-section base, the live pointer the target already holds. Indexing gOverlay56Colors by name still costs an extra addiu per site. Minimap sprite identity is gOverlay56Resource, not a NULL-page load. Aligned split 132 naming 244 immediate 28 structural 216 versus the prior 130/237/28/223; size is 591 versus 581 (delta +40) because the colour loads add the four missing base words plus leftover surplus elsewhere. The eight-word hole at target +0x6D8 remains unpack schedule: the target shifts RGB immediately after the load (green, blue, then red in the mode-branch delay slot) while this candidate still unpacks in the call. Identity-gate passed with CDX_PROC=6. Do not colour-landscape until size delta is 0. Ghost slots at D_800D1494 (alpha 255) and D_800D1498 (alpha 85) are identified but naming them did not move the aligned residual. Next: force the AI-site unpack before the mode branch without extra copies, then the unsigned-float surplus around +0x498.

- ownership: Overlay 56 text `+0x1A0..+0xAB4`, ROM `0x18A2F18..0x18A382C`, exactly 2,324 bytes. `overlay56UnpackColor` starts at `+0xAB4`; the separate `+0xAF4..+0xB00` alignment tail is not owned.
- ABI: the resident inbound at ROM `0x27F18` passes the display-list pointer address, vertex-cursor address, and the current update-rate word. It is the sole direct resident relocation to export table index 1361 at `+0x1A0`.
- configured baseline: the prior retained Mickey-only form emitted 585 instructions, exact `0x1F8` frame, 577 differing words, and first mismatch `+0x4`. The current base is `229a63af7761df4c9b0a620150c4a9c6fd9b8946`.
- early CFG: runtime identities prove three distinct gates. Fade state advances only while `D_800C3A3C == 0`; rendering returns when `gOverlay56Resource == NULL`; and a nonzero `D_800D3450` rejects game states `0/2/1/3/4` from `func_80028F54`. The retained nested spelling preserves the target branch topology and exact frame.
- call ABI: all 21 calls now have authenticated identities. The sequence is `func_80028F54`, `viGetCurrentSize`, `camStandardOrtho`, `func_80005750`, `levelGetLevel`, `frontGet2PlayerSplit`, `func_8002FB34`, `func_8002F618`, a second `viGetCurrentSize`, sine/cosine, `func_800349A4`, `func_8002A82C`, `matrixTranslate`, two `func_8002A604`, `func_80024978`, `mtxf_mul`, `mtxf_to_mtx`, and two final `func_8002F618` calls. Correcting the FP and shifted argument slots is the main strict gain.
- retained result: configured IDO emits 591 instructions versus 581 with the exact `0x1F8` frame, 538 differing words, and first mismatch `+0x50`. The colour-field spelling plus `gOverlay56Resource` as the minimap sprite improve the aligned structural bucket 223 to 216 and byte-exact 130 to 132. Size grew from 583 to 591; positional 538 is insertion shadow of that +40 delta (L155).
- relocation surface: the shipped owner has 75 runtime records. All target runtime identities resolve; the extracted target object exposes 31 text records and the retained C function emits 61 static records. Relocation count, offset, and identity are not exact, so no linked-match claim is made.
- bounded attempts: the prior 119-row flag lattice and five identity forms remain exhausted. This reopen compiled explicit-return and nested gate spellings, early-only identities, typed call ABIs, combined downstream identities, and the `+0x58` base identity. Explicit and nested returns were byte-identical; the typed/combined forms supplied strict gains. No generic permutation was run.
- donor evidence: pinned DKR v77 `src/game_ui.c::hud_render_general` supplies the permitted semantic minimap phase order—fade/gates, orthographic setup, viewport offsets, map transform, reverse racer iteration, marker rendering, and state restore. Exact DKR v77/v80 and JFG object/source scans remain negative; the donor is structural evidence, not a promotable body.
- integration blocker: a guarded definition in this consolidated TU routes 5,024 previously credited C bytes through the nonmatching object, so the global atlas correctly fails stale. Session B did not rewrite it. Agent A can preserve exact C accounting with reviewed mixed ranges `+0..+0x1A0` and `+0xAB4..+0xAF4`, retain fallback `+0x1A0..+0xAB4`, and exclude padding `+0xAF4..+0xB00`; do not physically split at unaligned `+0xAB4`.
- next mechanism: hoist the AI-site RGB unpack (green and blue, red in the mode-branch delay slot) immediately after the colour load without introducing extra copies. Then the unsigned-to-float surplus around candidate +0x498. Ghost identities `D_800D1494` and `D_800D1498` are known but inert as named loads. Identity-gate is `CDX_PROC=6`. Do not colour-landscape until size delta is 0. Do not rerun flags or generic source permutation.
#### 2026-10-04: used RGB lifetime and sibling-backed signedness controls

The committed assignment base `eb90496fa674057eeccf3ba67aea8cbb68f8aedf`
passed the local zero-exit `base-only` gate with source and ledger both pinned
at `4d685542da42fa2b9be7883d065ba43fa0bbf245`. The fresh configured full-TU
baseline reproduces 538 raw/relocation-masked words, 591 versus 581
instructions, size delta +40 bytes, frame `0x1F8`, and first mismatch `+0x50`.
This packet tests the previously recorded RGB-before-mode-branch question;
it does not restart flags, identities, declaration ordering or permutation.

The first natural control names three genuinely used `u32` components
immediately after the existing packed-colour read and before the existing
mode-dependent x branch. Original shifts, masks, load identity, call order,
widths and stores remain unchanged. All values are bounded to 0..255; the
pure extractions do not add observable memory accesses. It produces 541
raw/masked words at the same +40-byte extent, frame `0x200`, first mismatch
`+0x0`. Every one of its 84 changed owned instructions retains its opcode
and registers and changes only a stack-relative displacement or the frame
adjustment. The unpack/branch instruction sequence does not change.

The exact same-TU `overlay56UnpackColor` independently supplies a second
source-type witness: `u32` red and `s32` green/blue. Its existing linked
64-byte range at module `+0xAB4..+0xAF4` compares byte-identically with the
same physical ROM range. One separately authorized contrast changes only
the two named green/blue declarations to `s32`. Both masked values fit that
signed domain. It is text, symbol and ordered-relocation identical to the
all-unsigned control, with the same score, frame and first mismatch. No
casts, extra references, padding, reordered expressions or helper call
were added.

Authentic retained whole-TU streams distinguish the source effects. The
unsigned control adds three load, three store and three local-location
records at CFE output; only the three local-location records remain in
optimized output. Signed green/blue additionally adds four CFE conversion
records and two optimized conversion records. Neither changes the final
non-stack instruction output. These are measured stage counts and output
comparisons; no source-variable generation, PRE event, allocator-home or
optimizer replacement lineage is inferred from them. The prediction that
these named component lifetimes recover the earlier target unpack emission
is falsified for both supported forms.

All seven configured C siblings retain their bytes and symbol geometry.
All 88 ordered full-TU static relocation rows retain offsets, types and
symbols, and the REL operands at their sites remain unchanged. Fresh target
preflight still refuses promotion: 75 shipped runtime records versus 69
candidate owned static records, 37 candidate identities resolved and 32
unresolved, with ten effective identities aligned. This supersedes the
older retained 61-record candidate count; it does not relax identity proof.
The null-address tail proxies and other incomplete physical bindings remain
unproved and cannot supply linked-match credit.

Untouched stock compiler-input capture/replay passes every allocated-content
section, symbols and relocations for the baseline and both controls.
Whole-file debug metadata differences remain disclosed. Raw prepared-input
self-comparison refuses multiline comments containing macro definitions;
untouched stock preprocessing of the captured actual input passes expanded
self-context, and both outside-body comparisons are unchanged. Ordinary
fresh-lane overlay-alias generation repairs the initial cold-link failure;
the failure and retry receipts are retained rather than treated as a source
change.

The baseline source is restored and recompiled, with full-content/symbol/REL
restoration fidelity passing. All meaningful inputs, objects, scores,
first mismatches, retained phase streams and receipts remain ignored under
`build/o056-rgb/`. Stop on elimination of the two assigned mechanisms,
without a count-based stall claim. Future work needs an independently
supported executable, ABI or physical-storage fact; another component
spelling, type or ordering grid is not a fresh mechanism. No C source or
matching credit is adopted.

#### 2026-10-07: duplicate half-dimension conversion corrections are real but nonexact

The assigned source/handoff pair passed the zero-exit base-only gate at
`a6c6a897e`. The configured full-TU baseline reproduces 591 versus 581
instructions, 538 raw/masked differing words, size delta +40, and frame
504 bytes. Actual configured compiler output contains four unsigned-float
correction branches around the two half-dimensions; the target has two.
Each baseline dimension first converts a u32 to float, then explicitly tests
its signed interpretation and adds the unsigned correction again.

Both inputs are logical right shifts by one, so their values lie in
0..2147483647 for every 32-bit input. The explicit negative predicates are
therefore unreachable. The packet removes only those two explicit fixup
blocks, retaining the native unsigned conversions, the earlier signed-height
conversion/correction, RGB expressions, declarations and other control flow.
This is a defined semantic correction, not an invented operation or padding.

Measured controls, as candidate words / byte delta / raw and masked
mismatches / frame bytes / aligned structural rows:

- Baseline: 591 / +40 / 538 / 504 / 195.
- Width fixup removed: 587 / +24 / 580 / 496 / 321.
- Height fixup removed: 585 / +16 / 578 / 496 / 184.
- Both fixups removed (native-only): 575 / -24 / 553 / 496 / 172.

The combined control retains exactly the target's two native conversion
corrections. Its normalized distance falls from 593 to 573, but its frame is
eight bytes short and its instruction count is six short. The single-site
controls isolate the two removals; neither reaches exact code. The first
positional mismatch is entry offset zero for all controls, versus baseline
+0x50. These are diagnostic full-TU comparisons, not relocation or linked
identity proofs. No candidate is adopted and no matching bytes are credited.

Captured actual compiler-input replay agrees with an independent configured
stock invocation in allocated content, symbols and ordered relocations for
baseline and all controls. Expanded compiler-input self/context comparisons
pass; all seven C siblings retain their bytes, extents and relative relocation
surfaces. Source and configured baseline allocated content/symbols/relocations
are restored exactly. Inputs, objects, dumps and comparison/fidelity receipts
remain ignored under `build/o056-half/`.

A separately authorized contrast at `a36b99d4c` retains both manual fixups
and changes only their initial conversions to `(f32)(s32)` of each shifted
value. The same range proof makes both signed casts exact and defined. This
removes the native correction paths while preserving the manual lowering.
This manual-only form emits 579 words, size delta -8, the exact 504-byte
frame, 521 raw/masked
differences and first mismatch +0x40. Aligned constant/register/structural
rows are 30/316/181, versus baseline 30/315/195; normalized distance worsens
from 593 to 635. Actual context, stock executable/symbol/relocation fidelity
and all seven sibling checks pass. This alternate form is also nonexact and
remains private; it does not establish a promotable reconstruction. The two
combined artifacts are retained in `build/o056-half/both/` (native-only) and
`build/o056-half/signed-initial/` (manual-only), alongside the baseline and
single-site controls.

The two-word deficit is not a proved two-nop correction. The target's early
rotated-coordinate multiplies have two hazard separators, while this candidate
defers part of that arithmetic and has one adjacent multiply pair elsewhere.
No further flag sweep, RGB spelling, home or allocation control is justified
by that net size. Both ways of eliminating duplicate conversion lowering are
now measured. Further work needs independent evidence for the remaining
expression-availability differences and incomplete physical bindings.

The restored canonical fallback passes full US ROM verification. A fresh-lane
alias bootstrap initially preceded the guarded canonical-object rebuild and
caused resident relocation overflows; regeneration after that rebuild restored
the committed alias file exactly and verification passed. Documentation,
clean-room and all 99 tooling test files pass. No game source or generated
alias change is committed.

#### 2026-10-07 (lane b-o056): natural-source rewrite from the listing

The inherited m2c body was discarded and the function rewritten from the
target listing and the relocation records (brief items 1, 3, 6, 20). Each
cell below is a whole-TU compile scored by aligned edit distance, measured
as exact / naming / immediate / structural rows and size delta.

- Inherited body: 132 / 244 / 28 / 216, delta +40.
- First natural draft (typed structs, while (i--) loops, three distinct
  resident bytes for the mode tests): 144 / 225 / 59 / 185, delta +24.
- GBI packet macros taking dl++ (pipe sync, scissor, matrix, prim colour,
  DKR vertex and polygon): 156 / 231 / 46 / 171, delta -8.
- Ghost selector as an else-if chain with a final else obj = NULL (the
  shipped redundant branch to the join): structural 91 to 90.
- x/z products written inline, not as locals: mapY is then computed at the
  loop head as shipped instead of being forwarded past the calls.
- mapY rebased in place (mapY += offset) before matrixTranslate: 440 to 421
  aligned disagreement.
- Colour word read from the table at each unpack with no colour local:
  421 to 283. This alone produces the shipped srl/srl/andi/andi/move/move
  unpack before the mirror branch; a colour carrier lets uopt forward all
  three components into the call block. Typing red/green/blue u8, splitting
  the masks into a second statement, and four statement orders were
  byte-identical or worse (measured, 16 cells).
- Retained: 316 / 121 / 61 / 90, 579 against 581 words, frame 0x1F0
  against 0x1F8. Positional 577 is insertion shadow.

Resident mode bytes, from the relocation records: 0x800D3198 is the
fade-rate test (== 3), 0x800D3194 the player count (== 2 split test,
slot = count - 1, loop bound i < count), 0x800D31A8 the mirror flag (every
negation and +520 offset). The header's gOverlay56Mode is a fourth object
(SetMode stores 0x800CD60C). Promotion needs a placeholder name per byte.

Decision variable. The two missing words are the f24 save and restore.
Instrumented uopt (CDX_PROC=6, .text identity-gated against the stock
object) records web 197 (mapY) with caller cost 20.0 at c29 against the
callee toll 20.75 (L56: nBB 83), so it is never offered a callee register.
Forcing p1:w197=c30 is accepted (forced=30) and puts mapY, cos and sin in
f20, f22, f24 exactly as shipped; the forced object scores 386 positional
at delta -8 with only scheduling and ghost-loop rows left structural.
Web 339 (ghostAlpha, v1, caller cost 20.0) is the same decision; the target
holds it in s1. Web 203 (mapX, save 20, caller cost 20.0) is decided first
and must stay caller (the target spills it to its declared home at 0x1B4),
so lowering the toll below 20 alone would hand mapX the bank. The source
form has to raise mapY's caller cost above the toll (another call crossing)
or order mapY ahead of mapX. Not yet found.

Follow-up measurements on the retained body (records, CDX_PROC=6):

- Caller cost is 10 per call crossed. A diagnostic store of mapY after
  matrixTranslate (one more crossing) moves web 197 to cost 30 and it takes
  c30 at the 20.75 toll unforced. Not a source form; it locates the lever.
- Dropping the ghost loop (diagnostic) cuts the toll to 17.25 and then
  mapX and mapY both open callee registers, which the target does not do.
- So the shipped allocation needs mapY to cross three calls while mapX
  crosses two (caller 30 against toll 20.75 against caller 20). Every
  natural form measured so far gives both values the same two crossings.
- mapX shape (in-place negation, rotX first or last, negate-then-copy):
  mapY stays at cost 20 in all four; retained form is best.
- Unused s32 cells ahead of count (0, 1, 2, 4, 8) and mapX declared
  directly after count: 272 or 268 aligned disagreement, no allocation
  change. The target places a spilled mapX at a declared home (0x1B4)
  between count and the matrices; ours spills to a temporary slot.

Cycle-21 line: find the third call crossing for mapY (or a form where
mapX loses one) and confirm with web 197 cost in the records before
scoring; then the frame homes (frame_census) for the 61 immediate rows.

#### 2026-10-07 (lane b-o056, resume): call-crossing cells for mapY

Records read per cell (float webs, CDX_PROC=6) before scoring.

- mapY rebased (mapY += offset) before func_800349A4, or between
  func_800349A4 and func_8002A82C: web 197 goes callee c30 unforced at
  20.75 (caller 40), mapX stays caller. Scored 289 and 280 aligned rows at
  delta -8 and -4: the offset loads move ahead of the calls, which the
  target does not do (it loads them after func_8002A82C). Not kept.
- Not computing mapY at the loop head (sum written in both branches, PRE
  hoist) with x/z as locals: mapY cost 20, x and z spill. Not kept.
- The AI-branch marker.y store moved after the posX branch: cost 20. The
  crossing count is per call, not per block; no block-level shortcut.
- switch for the game-state reject: toll 20.50, 394 rows. Not kept.
- Ternaries for the shift, alpha clamp, posX and the mirror negation:
  cfe lowers them to the same blocks; only the clamp ternary moves two
  naming rows (272 to 270) and is kept. Mirror-negation ternary gives
  delta -4 but 317 rows.
- ghostAlpha (web 339, save 10) is decided before racer (web 182, save
  8.57), so s1 is not yet in the save mask and it takes v1 at caller 20.
  The target's s1 needs racer decided first (higher save than ghostAlpha);
  ghostAlpha typed u8/u32/s32, the colour arguments passed directly or
  through red/green/blue, and racer re-read inline in the ghost loop: no
  colour change (12 cells).

Cycle-21 line: two allocator facts remain, both priced in the records.
mapY needs a third call crossing with its rebase still after
func_8002A82C (or mapX a cheaper one); ghostAlpha needs racer's save above
10. Neither is a spelling of the regions already measured.

#### 2026-10-07 (lane d-mid3): flag, callee prototype, ghost racer, frame ladder; 577 at -8 to 542 at 0

Aligned rows (residual_map --object), exact / naming / immediate /
structural, one-sided words in brackets:

- Entry (tree): 318 / 119 / 61 / 74 (7 and 9), 577 at -8.
- Declaration order rebuilt from the target's home ladder (frame_census):
  eight cells above count, seven between count and mapX (target +0x1B4),
  three between mapX and mtxA, one between gameState and dl, six between
  height and ghostAlpha, three after it. Frame 0x1F8 exact, every traffic
  home on the target offset. The six GBI macros each declared a block
  local _g, and block locals take frame cells below every function-scope
  local; the target has room for none, so the packet cursor is one
  function-scope local (it takes one of the top cells). 352 / 142 / 22 / 56.
- func_8002F618's colour parameters u8 (the matched o052 prototype): the
  ghost call's three colour words become one andi of ghostAlpha as shipped.
- The ghost loop reads the racer through its own local (ghostRacer). With
  racer one web over both loops it interferes with ghostAlpha and its save
  (60/7) sits under ghostAlpha's 10; split, ghostAlpha takes s1 with the
  shipped load before and store after the ghost loop (lw/sw 0x8C).
- -Wab,-r4300_mul on the TU: the two missing words were the R4300 mul.s
  hazard nops in the rotated-coordinate products (target +0x400, +0x408,
  +0x830). Size delta -12 to 0. gmake verify passes with the flag (the
  TU's other functions are unchanged). 356 / 134 / 31 / 55 (5 and 5),
  542 masked at delta 0.

Measured and not kept: u8 ghostAlpha (s1 but a byte home, the target's is
a word), u8 red/green/blue locals (main-loop unpack regresses, 577),
do-while wraps around the ghost colour statement (toll 21.5, no colour
change), mapY as a repeated expression instead of a local (+32 bytes,
587), the mapY offset add inlined into the matrixTranslate argument
(578).

Decision variable, priced on this body (instrumented uopt, CDX_PROC=6,
.text identity-gated; web numbers are this compile's): mapY is web 194,
save 12.5, caller cost 20 at c29 against the callee toll 21.0 (nBB 84;
the target's branch and call census is identical, so its toll is too).
Forcing p1:w194=c30 is accepted and scores 278 positional at -4 (frame
then needs two cells fewer for the f24 save). Placing the rebase between
func_800349A4 and func_8002A82C prices the web at caller 30 and it opens
f20 unforced (355 at delta 0, but the offset loads then precede
func_8002A82C, which the target does not do; aligned 153 / 62 / 51). The
same blocks, the only change being which call block carries the
reference, so the caller price is per call block, 1 unit when the web
is transparent through it and 2 when it is referenced there.

Also measured on the committed body: mapY assigned per branch (before
func_800349A4 and at the head of the dot branch) opens f20 unforced but
uopt does not hoist it back to the loop head (400 at +4); assigned after
func_800349A4 instead, 586 at +36.

Cycle-21 line: mapY needs three caller units with its definition at the
loop head and its offset add after func_8002A82C. Since a transparent
call block prices 1 unit and a call block holding a reference prices 2,
look for a reference to mapY in the func_800349A4 or func_8002A82C block
that emits no word; then drop two of the top cells (the f24 save adds 8
bytes) and run frame_census.

#### 2026-10-07 (lane e-big): mapY in f20 from a merged sum web; 542 to 163 at delta 0

Aligned rows (residual_map --object), exact / naming / immediate /
structural: entry 356 / 134 / 31 / 65 (542 at 0); retained
420 / 154 / 2 / 4 (163 at 0), one one-sided word each way.

- Records (CDX_PROC=6, configured command with only the compiler swapped).
  A use of mapY between func_800349A4 and func_8002A82C prices web 194 at
  caller 30 and it takes c30 unforced; a use before func_800349A4 or after
  func_8002A82C does not. Measured: marker.y = mapY there (30, but a
  store), -(-mapY) and mapY / 1.0f (30, but they emit), and
  func_8002A82C(&mtxA + (s32)mapY * 0) (30, emits nothing; ugen folds the
  product). Inert (cost 20): bare, void, self-assign, += 0.0f, * 1.0f,
  (f32) cast, empty if, if (0), do-while-zero, & 0, dead local copy.
- The shipped form needs no probe. The target computes the y sum into f18,
  a register mapY does not hold, so the sum is not an in-place rebase. A
  fresh local for it takes c24 (f0) and leaves mapY caller (291 at -4).
  Assigning it to the ghost loop's x (or z) merges it into a web that is
  offered no argument colour and is decided ahead of mapX; with c28 and c29
  both held by interfering webs mapY is offered only the callee bank and
  takes f20 with no probe (probe on or off: identical text).
- Frame: the f24 save raises every sp-relative home by 8 (homes ascend
  from the save area in reverse declaration order, so top pads only resize
  the frame; 0 to 8 pads swept). Dropping the unused colour local and
  placing two of red/green/blue/posX below ghostAlpha restores the ladder;
  41 placements measured, 233 to 228 for the best, then the sum form.
- Product (20 cells: x/z as main-loop locals, sum target in mapY/x/z/rotX/
  inline, probe on/off): x/z locals in the main loop change size (+4 to
  +24); inline sum 239 to 555; rotX 164; x or z 163.

Open, all naming at delta 0: the sum takes f16 and mapX f18 (target f18 and
f16: the merged x web, save 25, is decided ahead of mapX at 20); cos and sin
are f24 and f22 (target f22 and f24: sin's save 41/12 outranks cos's 41/13
because cos spans one more block); level->scale and the x product are f0
and f2 (target f2 and f0); the D_84 pointer temporary sits at 0x74 against
0x7C; the fade-shift head swaps a0 and v1.

Cycle-21 line: give the y sum a web saved below 20 that still crosses a
call elsewhere (so mapX is decided first and takes f16), then cos's save
above sin's (one more weighted reference or one fewer spanned block), and
read the ladder after each in the records before scoring.

#### 2026-10-07 (lane e-big, second commit): fade shift as a conditional expression, 163 to 152

The fade head swapped a0 and v1: the mode byte (web 9, CSE temp, save 3/2)
and the shift local (web 12, save 3/2) tie and the byte, numbered first,
takes v1. Product over the head (6 shapes by 2 orders of the clamp test,
12 cells): writing the shift as `updateRate << ((D_800D3198 == 3) ? 2 : 1)`
inside the add removes the shift local and gives the shipped a0/v1 (152 at
delta 0, aligned 431 / 143 / 2 / 6). The if/else, a default-then-override,
a ternary into the local, a negated test and a long-hand add stay at 163;
shifting updateRate in place changes size; testing the state before the
mode byte in the clamp costs 8 bytes. Also measured on this body: the y
sum in mapY (548), a fresh local (546), x or z (152), inline (555); the
ghost loop with inline products instead of x/z locals is +24 bytes in
every sum form.

#### 2026-10-07 (lane e-big, after the 152 commit): x and z as main-loop locals, the shipped colour split at +4

Records on the 152 body: the merged x web (218, blocks 49 and 76) is
decided at save 25 ahead of mapX (20) and takes c28, so the ghost loop's x
is f16 against the shipped f0; level->scale is the web spanning both loops
(c24) where the target's scale is f2.

Writing the main loop with the ghost loop's own locals (x = obj->x *
scale; z = obj->z * scale; rotX and mapY from them) and the y sum back as
an in-place mapY rebase gives every float colour the target has: x c24
(both loops, f0), scale c25 (f2), z c26 (f12), rotX c27, mapX c28, the
rebased mapY symbol web c29 (f18, the shipped sum register), and uopt's
CSE temp for the loop-head mapY value live across the calls takes c30
(f20) with no probe. Aligned naming falls 143 to 93, but the cell is 373
masked at +4: uopt evaluates the mapY temp's products before rotX's, so
one extra R4300 mul.s hazard nop appears and the add leaves the branch
delay slot; frame 0x200 (homes +8).

Measured on that shape (product, about 70 distinct cells): mapY statement
before or after rotX, rotX carried or written as in-place negation,
if/else and ternary mirror forms; two or one top pads; five placements of
ghostAlpha among the bottom cells. Floor 373 at +4 (mapY first), 376 at +8,
377 at +16, 579 at -12 when mapY is written after the mirror block. Bottom
placement and top pads do not move the masked count on this shape.

Cycle-21 line: on the x/z-locals shape, make uopt emit rotX's products
before the mapY CSE temp (the decision variable is expression numbering of
the type-4 mapY temp against the rotX symbol web; brief items 21 and 28,
a dead read or a first-occurrence edit ahead of the loop head), then
re-fit the frame with frame_census. If that order is unreachable, the 152
body's open pairs are the fallback: sum web save below 20 crossing a call
elsewhere, and cos ahead of sin (cos spans one more block, 41/13 against
41/12).

#### 2026-10-07 (lane e-big, resumed): x/z-locals shape at size 0, 152 to 131

On the x/z-locals shape (373 at +4), the order of the two terms inside
mapY's own expression decides which products uopt emits first: mapY =
x * sinA + z * cosA puts rotX's products first and removes the extra
mul.s hazard nop (182 at 0). Splitting either statement into two, moving
x's or z's definition, and rotX after mapY all stay at 373 at +4 (8 forms).
Frame refit: homes were 8 high above ghostAlpha and the D_84 temporary
4 low; dropping the unused cell that replaced shift, ghostAlpha last in
declaration order, and two of the four float locals (x and z) declared
above the pads put every home on the target's ladder except one top cell
(0x1D0 against 0x1D4): 131 at 0, aligned 453 / 120 / 2 / 5. 18 placements
of the moved pair by 3 top-pad counts measured; any pair containing x at
one top pad reaches 131.

Open: cos and sin still f24 and f22 (cos spans one more block), which also
puts the mapY add ahead of the rotX subtract; the int ring is a closed
seven-cycle from the prim-colour packet (+0x764) to the end.

#### 2026-10-07 (lane e-big, resumed): cos ahead of sin, 131 to 129

Records: cos (first angle call) 41/13 against sin 41/12, so sin took f22.
Routing the first result through a block-scoped temporary (f32 c =
func_8002A8BC(...); sinA = func_8002A8C0(...); cosA = c;) gives the temp
save 5.46 and cos and sin take f22 and f24 as shipped: 129 at 0, aligned
455 / 114 / 4 / 7. An empty if (cosA) {} after the calls does the same at
127 (457 / 116 / 2 / 5) but is a diagnostic construct and not adopted.
Measured negatives: the temp at function scope in three declaration slots
(131 and 178), rotX or mapX as the carrier (361 at +4). With the colours
right, mapY's products are emitted before rotX's in every term order: 16
rotX by mapY spellings by 2 statement orders on both bases, flat at 127
(empty-if base) and 129; z * cosA + x * sinA is 340 at +4.

Cycle-21 line: the loop-head emission order (mapY's CSE temp numbered
ahead of rotX) is the remaining float residual; then the one high home
(0x1D0) and the int ring seven-cycle from the prim-colour packet.

#### 2026-10-07 (lane e-big, second resume): count home, 129 to 127

- Dead reads for first-occurrence numbering, assigned to existing locals
  (mapX or mapY or rotX = x * cosA, or the whole rotX expression, before or
  after the rotX statement; 18 cells over both mapY term orders): every
  cell byte-identical to its base (129, or 341 at +4 with z * cosA first).
  Emission order of the loop-head products follows only the term order
  inside mapY's expression, not first occurrence. Closed for this shape.
- The high home was count (0x1D0 against 0x1D4): one s32 pad between slot
  and mapX (where the shift local stood) with one top pad fewer, 127 at 0,
  aligned 457 / 114 / 2 / 7. Seven positions by two top-pad counts
  measured; any position between count and mapX works.
- Integer ring from +0x764: the target draws the 0xFA000000 prim-colour
  constant before mtx++'s add; we draw it after the alpha byte load. Flat
  (16 cells, 127): test operand order, >= 1, an unsigned != 0 test, prim
  operand order, the packet written long-hand in either store order.
  mtx++ in the matrix macro, as += 1, as &mtx[1], or folded onto the macro
  line: 127. mtx++ after the if-block: 123 masked but structurally wrong
  (the mtx store moves into the branch), not adopted.

Cycle-21 line: the prim-colour constant's draw ahead of the mtx++ add
(draw_census lines 319 to 322 against the target's order lw mtx, lw dl,
lui 0xFA00, addiu mtx+64), then the loop-head product order.

#### 2026-10-07 (lane g-2): the shade test is a variable; 127 to 24 at delta 0

Entry 127 masked at delta 0, aligned exact 457, naming 114, immediate 2,
structural 7; the int ring was a closed seven-cycle from +0x5B4 to the end.

- The prim-colour packet written as `_g = dl; _g->w0 = ...; _g->w1 = ...;
  dl++;` (or `dl = _g + 1`): 119. It moves the 0xFA000000 draw ahead of the
  test as shipped but adds a dl reload; not kept. Eleven other packet forms
  (store orders, increment placement, explicit local w0, dl[0] indexing):
  119 to 292.
- The test through a declared int local: `shade = (racer->alpha * alpha) >> 8;
  if (shade > 0)`, packet unchanged (it re-reads racer->alpha and recomputes
  the product, as shipped): 26 with posX as the local, 24 with any of the
  five unused s32 pads (so one pad was this variable; first pad kept, renamed
  shade). red, green or blue as the carrier: 292 (they merge with the ghost
  loop's webs). A fresh local: 93 (frame). The conditional assignment inside
  the test, and a cast on the product: 119 to 292.
- Kept: 24 masked at delta 0, aligned exact 557, naming 15, immediate 4,
  structural 5. The seven-cycle is gone.

Open, all small: the dl cursor in v0 where the target has v1 (+0x160 and
+0x620 packet sites); D_80000004's address in v1 against a0 at +0x608; the
D_84 temporary at 0x78 against 0x7C (four sites); the loop-head product
order (mapY's products before rotX's, +0x3F0 to +0x41C); one lw/andi pair
swapped in the ghost loop at +0x7F0.

<!-- plateau-handoff:func_overlay_056_F00001A0_18A2F18:end -->
