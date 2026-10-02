<!-- plateau-handoff:overlay1LoadBuildRecords:start -->
### `overlay1LoadBuildRecords` plateau handoff

- source: `src/overlays/o001/overlay_001_head.c`
- score: 469 differing words
- frame: 0xD8
- relocations: 114
- first mismatch: +0x34
- summary: Exact frame; 13 words short. Per-field group clear adopted. One BSS owner and a null-base 0x94 length do not close the rest.

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
<!-- plateau-handoff:overlay1LoadBuildRecords:end -->
