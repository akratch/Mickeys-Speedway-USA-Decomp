<!-- plateau-handoff:overlay17AdvanceChain:start -->
### `overlay17AdvanceChain` plateau handoff

- source: `src/overlays/o017/overlay17AdvanceChain.c`
- score: 49/147 words
- frame: 0x70
- relocations: 1
- first mismatch: +0x18
- summary: Exhaustive colour landscape: only web 52 reaches 47 under force; indexed-copy forms regressed structurally, so the source cursor remains guarded.

Fresh configured evidence on 2026-09-04 re-proves the owned overlay-17
`+0x668..+0x8B4` range at 147 instructions, frame `0x70`, and 96/147
positionally exact words. The sole runtime `R_MIPS_26` record at function
offset `+0xF4` resolves to the local overlay entry at `+0x0`; target and
candidate offset, type, and identity all agree.

Workbench 0.8.0 reports a candidate pool lane one web longer than retail and
routes the first divergence through its drop-a-declared-local lever. The one
authorized structural reshape removed `newBuffer` and expressed the stored
and indexed value as the same `(u8)(oldBuffer ^ 1)` computation. IDO commoned
the repeated expression back into the existing web: `.text` SHA-1 remained
`f8d1c89a853e`, with 51 raw/positional differences, 33 opcode differences,
16 alignment gaps, and the same first mismatch at `+0x18`.

The retained source is restored. This closes the unnamed-new-buffer route;
reopen only with producer evidence for the interleaved source/destination
offset schedule, not another declaration removal or repeated xor spelling.

### Re-test and retained candidate (2026-09-12)

The configured full-TU candidate retains 147 words, the `0x70` frame and its
single call relocation. The positional masked residual improves from 51 to
49 words, first `+0x18`. Aligned buckets (exact/register/immediate/structural)
move from 108/21/3/18 to 110/21/1/18. Moving the count and output-pointer
homes reproduces the target's home set. Typed point stores and a final pointer
increment are byte-identical to the retained raw-offset stores and make the
layout explicit. The fallback remains canonical; this is not a matching claim.

The inherited claim that deleting the new-buffer index is byte-flat survives.
Deleting the halfword-copy carrier restores the target's copy register class
but worsens the whole function. Separating the initial point count lets uopt
reassociate the remaining-count addition, so it also fails to reproduce the
setup. A separate copy-buffer carrier is coalesced into the pointer saved
across the call; moving the output load after the loop instead introduces a
read on the empty-copy path. Neither gives the target's pre-call transfer.

A ten-byte vertex view confirms that a shifted pair index preserves the two
extent multiplications, whereas a multiplied pair index is reassociated and
removes them. A sizeof-derived copy count reuses the multiplication constant
and adds a third multiplication; that is a different shape despite its scalar
alignment score. An explicit remaining-count local grows the frame, including
with a register qualifier. Grouping its homes moves displacements but does not
restore the frame. These alternatives are retained as private evidence only.

The closing attempts made no progress: the register qualifier was byte-flat;
unsigned size addition and moving the copy-count definition ahead of buffer
setup tied the existing split-count candidate without eliminating its residual.
This satisfies the ADR 0018 stall rule. No flag or donor closure was retried
without new evidence. The workbench reports `structure-mismatch` and a mixed
structural/register residual, with its next lever: constants first, structure
second, register classes last. Resume on a proved count-web or copy-buffer
identity that produces the required pre-call setup with the existing frame.
The post-call stores and fade do not justify another broad spelling sweep.

Validation: configured stock probes; aligned, register and frame censuses;
`tools/wb_compare.sh --diagnose overlay17AdvanceChain`; full ROM verification
through the lane gates with the guarded fallback retained. No new executable
bytes receive credit.


### Exhaustive landscape and literal identities (2026-09-12, p23-lastmile5)

Fresh configured evidence retains 147 words (588 bytes), frame `0x70`,
one candidate and target ELF relocation, and 49 raw/masked differences,
first `+0x18`. The aligned split is 110 exact, 21 naming, one immediate and
twelve paired structural rows. Candidate-only offsets are `+0x3C`, `+0x6C`,
`+0x84`; target-only offsets are `+0x30`, `+0xD8`, `+0xDC`. The earlier
18-structural count includes these six gaps. Positional density across those
gaps must not be mistaken for a colour-only problem.

The exhaustive procedure-zero same-save-kind scan sampled twelve coloured
webs and 89 accepted alternative forces: 81 retain exact size and eight grow.
The complete delta-zero winners list is `p1:w52=c5` and `p1:w52=c6` at 48,
and `p1:w52=c7`, `p1:w52=c8`, `p1:w52=c9` at 47. Every winner controls the
same web, so they are rivals and cannot be combined. Selecting `p1:w52=c7`
reproduces the measured diagnostic lattice floor of **47**. Stock and
instrumented text, data, rodata, symbols and relocations pass fidelity. The
result is not a bound over other source forms or arbitrary multi-web forces.

Six stock literal-type probes test the remaining count/extent identity.
Unsigned source subtraction scores 145 with size delta +8. Making only the
source stride unsigned scores 141 at +4; making only the destination stride
unsigned does likewise. Making both strides unsigned scores 49 at delta zero;
unsigned buffer-toggle spelling also scores 49 at delta zero; combining
unsigned subtraction with the toggle scores 145 at +8. Separating one
multiplication's literal identity changes geometry, while changing both
preserves the existing residual. No candidate improves and none is adopted.
The last three cells provide the closing stall evidence; their objects,
sources and measurements remain private under
`build/p23/overlay17AdvanceChain/`.

The next question remains a proved count/copy-buffer producer identity that
reproduces the pre-call schedule. A literal signedness change alone does not
supply it. The already exact post-call region and the call identity were
preserved; no broad declaration or pointer sweep was repeated. Commands:
`residual_map.py`, `web_footprint.py --every-colour`, `force_lattice.py`,
configured stock probes, `finalize_plateau.py`, and
`tools/gates.sh verify cleanroom check-docs`. The guarded fallback remains
canonical, with no new matching credit.

#### 2026-10-02, lane g-ovl5: no change, 49

- All 68 dependency-valid orders of the eight setup statements (count, old
  buffer, source cursor, new buffer, store, write cursor, destination cursor,
  count scaling): every one scores 49.
- Target reading: count lives in a3 and count minus one in t0 as separate webs;
  the old buffer is a2 and chain stays in a0, where this build puts the old
  buffer in a0 over the chain parameter. The scaling constant 10 is held in a
  register for two multiplies (source and destination offsets) while the copy
  count is shift-and-add.
<!-- plateau-handoff:overlay17AdvanceChain:end -->
