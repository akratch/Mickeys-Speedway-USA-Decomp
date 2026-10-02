<!-- plateau-handoff:overlay17AdvanceChain:start -->
### `overlay17AdvanceChain` plateau handoff

- source: `src/overlays/o017/overlay17AdvanceChain.c`
- score: 0/147 words, promoted
- frame: 0x70
- relocations: 1
- first mismatch: none
- summary: Matched. No local count; unsigned indices share one constant register, the signed copy count of the same unsigned (count - 1) expands to shifts.

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

#### 2026-10-02, coordinator: matched, 47 to 0

Three measured rules, each from a one-function micro-experiment:

- IDO multiplies by a constant with shifts when that constant has one use
  in the function, and loads it into a register and multiplies when two or
  more multiplies share it. The constant's identity includes its type, so
  an unsigned index scaling and a signed multiply by ten do not share.
- `(x - 1) * 10` is distributed to `x * 10 - 10` unless the subtraction is
  unsigned and has a second use.
- `n = expr; while (n--)` computes straight into the counter and keeps the
  pre-decrement copy only when `expr` reads memory; from a local it is
  forwarded into a temporary and the copy disappears.

So: no local count (every read is `chain->count`), both indices unsigned
(`(chain->count - 1U) << 1`, `(u32)chain->count << 1`), and the copy count
`(s32)(chain->count - 1U) * 10`. That left only the frame, 8 bytes large
from the unused `count` declaration; removing it and declaring `n` first
is exact.

#### 2026-10-02, lane g-ovl5: no change, 49

- All 68 dependency-valid orders of the eight setup statements (count, old
  buffer, source cursor, new buffer, store, write cursor, destination cursor,
  count scaling): every one scores 49.
- Target reading: count lives in a3 and count minus one in t0 as separate webs;
  the old buffer is a2 and chain stays in a0, where this build puts the old
  buffer in a0 over the chain parameter. The scaling constant 10 is held in a
  register for two multiplies (source and destination offsets) while the copy
  count is shift-and-add.

#### 2026-10-02, lane x-ovlb: natural indexing, 49 -> 47 unforced

The candidate now indexes the buffers as arrays of a 10-byte vertex,
`&buffers[old][(count - 1U) << 1]` and `&buffers[new][count << 1]`. It
copies with `while (n--) *--dst = *--src;` and uses the same `n` for the
fade loop, so both loops get the target's v0 counter and v1 copy. Aligned
buckets went from 110/21/1/18 to 125/8/0/16 (exact/naming/immediate/
structural). 47 is the number the September exhaustive landscape reached
only by forcing web 52; it now needs no force.

The decision variable is the copy count. The target keeps `count - 1` as a
coloured web (t0), uses it for the source pair index, and expands
`* 10` into the counter register (v0) with shifts, while the two buffer
offsets share the constant 10 as a register (t2). What was measured, at
size delta 0 unless noted:

- `(count - 1) * 10`, with the subtraction in signed, unsigned or cast
  forms, through a separate local, through `count--`, or placed before or
  after the pointer setup. uopt either distributes it to `count * 10 - 10`
  (135 at -4) or multiplies it with the shared t2 register (47 to 123).
- `* 5 << 1`, `* 5 * 2`, `(x << 2) + x` written out, and sizeof-derived
  spellings. The *5 forms are expanded but distributed (80, 129). The
  explicit shifts land in ring temps, not the counter (64 to 117).
- Byte-offset arithmetic for the two pointers, as in the old candidate.
  It is no better (80 to 135).
- A 128-cell type product over the source index, destination index and
  copy-count spellings. Floor 47; the six best cells all keep the source
  index unsigned.

Next: get the decision records for the copy-count web. The question is
whether any IR shape leaves a `mpy` by an immediate 10 for ugen to expand,
rather than a use of the constant web.
<!-- plateau-handoff:overlay17AdvanceChain:end -->
