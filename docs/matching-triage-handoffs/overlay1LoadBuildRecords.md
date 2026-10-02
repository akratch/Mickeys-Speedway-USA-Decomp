<!-- plateau-handoff:overlay1LoadBuildRecords:start -->
### `overlay1LoadBuildRecords` plateau handoff

- source: `src/overlays/o001/overlay_001_head.c`
- score: 6/572 words
- frame: 0xD8
- relocations: 110
- first mismatch: +0x1F0
- summary: Overlay idiom rewrite: 469 at -52 to 6 at size 0, frame exact. Open: the two running-max stores rematerialise their address (lui at) in the target.

Summary before this remeasure: Rewritten in the overlay idiom with relocation identities: 469 at -52 to 20 at size 0, frame exact. Open: max-store address rematerialised, FP ring, arg order.

Summary before this remeasure: Exact frame; 13 words short. Per-field group clear adopted. One BSS owner and a null-base 0x94 length do not close the rest.

### 2026-10-02 (lane x-o058): rewrite, 469 at -52 to 20 at size delta 0

The inherited body was rewritten from the listing in overlay 1's own idiom
(the matched F0002B4C/F0003258 shape), after decoding the runtime relocation
records. Measured in order with tools/fast_score.py on the head TU:

- Identities. The "rank delta" and "mode constant" stores are G_o1_83e0 and
  G_o1_83e4 (SYMBOL records into overlay 1 BSS +0x0/+0x4); `D_0` is resident
  gO1Finishers (selector 0xFFF +0x4C1FC, the counter F0001D78 increments);
  the operands of the delta are resident +0x3198 and +0x319C; the 0xFF clear
  is gO1RankOrder[0..5]; the closing clear is D_1DC8[0..4] (the shipped
  address is D_1DC8 + 4 walking down, not D_1DCC upward); the report's first
  argument is the word at overlay 1 .data +0; D_BC..D_D4 are LOCAL records
  against rodata, i.e. the literals 0.8, 0.98, 1.15, 0.65, 0.9, 1.1 and
  FLT_MAX. The 0xC9 records carry a byte at +0xC, the 0xCA records a halfword.
- Shape: a switch on the config mode; every counted loop in the overlay's
  while (i--) idiom; every record walk a plain for with no guard copy: 450 at
  -16.
- Layout: the group-less path is reached by goto (the shipped code jumps
  over the group code from a block holding only the records reload): 298 at
  -12. The missing-large test is an if/else whose else reports and returns:
  181 at -4.
- The closing average reads D_1D58 once through a pointer local, so the
  score load hoists out of its loop: 144 at size delta 0.
- One variable per loop role (the shipped registers say which loops share a
  web): k for the group clear and the point-base loop, j for the point index
  and the finalize count, entry for the two count/point walks, node for the
  link walk. 49, and the frame is 0xD8.
- Declaration order putting records, size and the two spilled metric sources
  on their shipped homes: 20.

Aligner at 20: byte-exact 552, naming 12, immediate 0, really different 8.

Open, measured:

- The two running-maximum stores (D_1D80 in the first walk, D_1D8C in the
  large walk) rematerialise their address with lui in the target; ours store
  through the held address register, so as1 turns the skip into a
  branch-likely. Spelling the update through a value local (two forms) is 42;
  the loads keep the held register in both builds, so this reads as a split
  of the address web at the store block, not a spelling.
- The rank-adjust and average blocks use the float ring one position off.
- The FLT_MAX load and the large-pointer argument trade places around the
  first metric-source call; four statement orders of large/maximum/minimum
  measured 20 to 22.
- The average loop's pointer takes a caller-saved register in the target;
  reusing sourceC for it is 29.

Next lever: the instrumented uopt decision records for the D_1D80 address
web (does globalcolor split it at the store block in the target's shape),
then the DKWB freelist trace for the two float blocks.

Later the same sitting, 20 to 6 at size delta 0:

- The rank adjust written `rank += (maximum - score) * scale` puts both float
  blocks (the adjust and the closing average) on the shipped ring: 20 to 10.
  The other four spellings of that expression (operand orders, an explicit
  (f32) on the read) are 20.
- FLT_MAX assigned on the same source line as the first metric-source call:
  as1 then fills that call's delay slot with the literal load and leaves the
  argument copy before it, as shipped (L59): 10 to 8. Five statement orders of
  large/maximum/minimum on separate lines are 10 to 12.
- The closing average through its own pointer local (`base`, which the
  allocator gives a caller-saved register, as shipped): 8 to 6. Reusing the
  metric loop's `large` keeps it in s6; reusing sourceA also measures 6.

Aligner at 6: byte-exact 566, naming 0, immediate 0, really different 6.
All six are the two running-maximum stores. Alias names for the stores (a
second extern for the same address) are 460 at +16; storing through
`*(s32 *)&global` is byte-identical to the plain store.

- identity: Overlay 1 text `+0x10C8..+0x19B8`, ROM `0x184D4A8..0x184DD98`, 2,288 executable bytes with no credited padding
- ABI and flags: `void overlay1LoadBuildRecords(void)`, configured `-O2 -mips2 -32`; candidate and target frames are both `0xD8`
- V0: 559 candidate versus 572 target words, 469 relocation-masked and 490 raw differences, 114 candidate versus 32 target relocations
- donor: the nearest permitted masked-shape row is JFG `squadsInitAIArrays` at 0.0845; all other returned rows are below 0.044, so no credible donor body exists
- retained evidence: target offset `+0x6C8` loads packed offset `+0xC` unsigned-halfword, proving `link` is `u16`; representing metric iteration as a 0x10-byte sliding cursor with fields at `+0x14` reproduces the target's `+0x70` cursor bases and reduces aligned differences from 471 to 462
- attempts: four coherent source mechanisms were compiled; declaration order and reverse clear-pointer forms regressed and were rejected, while duplicate-symbol spelling was byte-flat and reverted
- blocker: target local-data accesses largely carry no runtime relocation, while the split candidate uses many `Reloc` calls and separately relocated extern aliases; workbench reports 145 relocation-metadata and 153 relocation-target mismatches
- next lever: the remaining 13 words. The 0x94 length still multiplies by a hoisted 148 at the allocation and at the large-record end pointer. Do not colour-sweep while size_delta is nonzero.

### 2026-10-02 remeasure

Flag check: overlay_001_head.c.o has only -Wab,-r4300_mul. No -Wo,-loopunroll,0, -Olimit, or -O2 -g3 on this object.

Baseline reconfirmed before edits: 2288 target bytes, 491 raw, 470 masked, size delta -92, frame 0xD8, first mismatch +0x34. insertion_pairs on that body: one open pair from +0x220 to the end, labelled missing-CSE, 30 target-only words and 7 candidate-only words. The first target-only cluster is the stack reload, branch, and delay nop beside the first packed-record scan. Later target-only words sit on the group-clear address (two alus) and on five per-field clear reloads, then on the 0x94 length (two consts and one alu).

Attempt 1, data owner. One Overlay1BuildOwner extern at D_1BA0 covering 0x1BA0 through 0x1DCC, with the rank delta and mode constant stored through the existing G_o1_83e0 and G_o1_83e4 symbols. Rodata floats stayed separate. Score: 549 masked words, size delta -156. Reverted. Eliminated: that single owner shortens the function and raises the masked residual. The 114 candidate relocations match the overlay runtime table (82 LOCAL, 31 SYMBOL, 1 JUMP). The assembled listing's 32 are its named sites. Masking already ignores relocation immediates, so the size deficit is codegen, not a reloc count a struct can delete.

Attempt 2, group clear. The loop writes each field as D_1BA0[index].field instead of taking one pointer and walking it backward. Adopted. Score: 469 masked words, 490 raw, size delta -52, frame 0xD8, first mismatch +0x34, 114 relocations. The initial element address is now a shift sequence rather than a multiply by the live 28.

Attempt 3, 0x94 length. shape_product.py --jobs 2, one axis. Cell 0 is D_1D8C * 0x94. Cell 1 is the byte length of a null-base Overlay1LargeRecord subscript. 2 cells, 0 compile errors, floor 469 masked at delta -52. Both cells are that floor, first mismatch +0x34. Eliminated: the null-base difference is byte-identical to the integer multiply, so it does not defeat the hoisted multiply by 148.

Stop. Three attempts, and the dispatch cap of one owner consolidation plus two insertion-pair spellings is spent. Attempt 2 is the only better residual. No exact promotion. The kept object still multiplies by 148 for the allocation size and for the large-record end pointer; those sites were not given a second spelling after the address difference proved flat.

Tool gaps: insertion_pairs was not re-run on the adopted body, so the one-sided map above is the pre-edit body. shape_product reports the floor and not which multiply survived. fast_score does not print frame or relocation count; those were taken from the kept object. No ROM verify, because size_delta is not 0.

### 2026-10-02 (lane x-o058), wrap-up: the six words that remain

Exactly two sites of three words each, nothing else differs:

- +0x1F0, +0x1F4, +0x1F8: the D_1D80 running-maximum store in the first
  record walk. The target branches with a plain beqz, fills its delay slot
  with the high half of D_1D80 and stores through that; ours stores through
  the held address register (s6), so as1 makes the branch likely and copies
  the size load into the slot.
- +0x55C, +0x560, +0x564: the same shape for the D_1D8C store in the
  large-record walk (held register a1).

In both builds the loads of those globals use the held register; only the
conditional store differs. Measured flat at 6 (the store still goes through
the register): store through `*(s32 *)&g`, `*(u32 *)&g`, `*(void **)&g`,
`(&g)[0]`, a volatile-qualified lvalue on either side, a do/while(0) or
if (1) region round the store, compare operand orders, `continue` forms of
both walks, a pointer local taken from `&g` for the compare (three
declaration positions), chained or same-line zeroing of the three counters,
and `+= 1` for the count. Worse: a value local (9 to 28), ternary or
self-assign forms (450 at +8), a second extern name or array or struct alias
for the store (233 to 460, uopt then folds the loads), `p1:w378=s` and
`p1:w379=s` forces on the two address webs (both accepted; every reference
rematerialises, 552 at +16 and 554 at +4).

Records on our build (instrumented uopt, proc 5, identity gate passed): the
address webs are 378 (D_1D80, s6, save 2.19, nocs 16, totalsave 35) and 379
(D_1D8C, a1, save 2.63, nocs 8); both are decision=color and both list the
store block as a referenced block.

Next thing to try: matched `overlay1AssignRecordIndex` in overlay_001_tail.c
ships this exact D_1D8C update with a direct store (`recordIndex = &D_1D8C;
newIndex = record->index + 1; next = newIndex; if (*recordIndex < next)
D_1D8C = next;`). Port that statement group verbatim, its two value locals
included, into both walks (re-solving the frame, which gains cells), and read
webs 378/379 afterwards: the pointer read alone did not change the store
here, so the two distinct value locals are the untested part.
<!-- plateau-handoff:overlay1LoadBuildRecords:end -->
