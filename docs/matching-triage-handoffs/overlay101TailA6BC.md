<!-- plateau-handoff:overlay101TailA6BC:start -->
### `overlay101TailA6BC` plateau handoff

- source: `src/overlays/o101/overlay101TailA6BC.c`
- score: 53 differing words
- frame: 0x40
- relocations: 44
- first mismatch: +0x0
- summary: 53 words at size delta 0. Frame 0x40 vs 0x38 with the same slots. Second node-24 row and an unused 8-byte frame remain.

Summary before this remeasure: 119 flags and ten source forms are nonexact; retained +1-word shape has one extra saved index web and 0x20 excess non-save frame.

#### 2026-09-11, coordinator: four-bucket split, no source attempt

Measured with tools/align_symbol.py on the integration branch and reproduced independently of the lane that first ran it, whose numbers were identical but whose worktree persisted nothing.

  - size delta: +4
  - positional masked: 267
  - byte-exact aligned rows: 83
  - register-naming rows: 111
  - immediate-only rows: 9
  - really-different rows: 104
  - displacement tax: 43

Frame is 0x58 against the target's 0x38, a 32-byte excess, and by L119 that is a block or temps question before it is a colouring one. Frame experiments by the measuring pass regressed, 267 to 286 and 285.

This procedure issues calls, so by the call test it is p1-only and L100's save ratio is the axis for colours the web is actually offered. CORRECTED 2026-09-12: the call test retires L106 and nothing else. Statement order still reaches as1's scheduling and ugen's emission order, neither of which the census touches, and it was decisive on every one of seven functions closed in one lane. Do not read this shard as saying statement order is the wrong axis.

#### 2026-09-24, lane w4-o101: size closed, 53 words

insertion_pairs on the inherited body: size +4, frame +32, label spill/reload, aligned residual 224. The candidate-only stack stores were the index and pointer locals. Deleting those (L145) and inlining the opacity multiplier took the aligned residual to 165 at size +16. The two remaining moves were calls 2 and 3 copying the post-bump node index into a2; the target leaves the pre-bump index in a2. Capturing that index in `length` (reused later for the text rows) and passing it removes the moves.

The dead second parameter is homed unless an empty test uses it. The test has to sit at the top of the function: at the tail, or absent, the home store returns and the score goes to 231. `register` on the parameters was inert.

Root-header store order on one line is load-bearing (as1 reverses the first three, so the source lists kind, then width, then height). Second header order is the target's: x42, y46, height48, value4A, value4C, width44, mode, colours, child, data. That path reached 59 words.

A direct store of the creator result, with no handle local, matches the frame at 0x38 but parks the base+0xA2 spill at 0x30 instead of 0x34 (54 words). `register void *handle` puts the spill back at 0x34 and scores 53, at the cost of an unused 8 bytes of frame (0x40 vs 0x38). The seven accessed slots match either way.

Best body, tools/align_symbol.py: 262 byte-exact, 18 naming, 3 immediate, 13 really different, displacement tax 19. First mismatch +0x0 is the frame addiu. The one-sided words are the second node-24 row, +0x3E4 through +0x460. Reordering that row on one line scored 60. Splitting it onto separate lines in the target's byte-store order scored 61. Both reverted.

Stall: those two spellings of the second row, plus moving the empty test off the first line, produced no better residual. Do not colour-sweep; size is zero but the frame cell count is still one alignment step high.

<!-- plateau-handoff:overlay101TailA6BC:end -->
