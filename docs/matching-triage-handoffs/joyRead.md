<!-- plateau-handoff:joyRead:start -->
### `joyRead` plateau handoff

- source: `src/main/joy.c`
- score: 0/159 words, promoted
- frame: 0x38
- relocations: 55
- first mismatch: none
- summary: Matched. A void prototype for osContGetReadData, a zero-argument call to func_8002CD6C, and a second definition of i inside the receive block.

#### 2026-10-02 (lane `w2-front`): matched; both earlier closures were about the wrong variable

47 -> 0 at size delta 0, `gmake verify` OK. Three source facts, measured in
this order:

1. **Call arity (checklist item 9).** `func_8002CD6C` is `void (void)` in
   `saves.c`. The inherited candidate passed `(&count, 1, &count)` to
   reproduce the registers the target leaves at that call. With no arguments
   the function went 4 bytes short (135 masked): the copies were standing in
   for a loop-test word.
2. **The second loop's exit test.** uopt rewrites a counted loop's `i < 4`
   exit test into `!=` (L153), unless the counter is defined again later in the
   same region. The first loop keeps `<` because the second loop's `i = 0`
   follows it. The second loop keeps `<` in the target, so `i` is defined again
   inside the receive block. Measured on a mini TU: a later definition dominated
   by the loop exit keeps `sltu`, and a definition after the region's join point
   does not. A dead `i = 0` after the loop, one before `rumbleTick`, and
   `i = saveDataFlags & 0x1F` feeding `func_8002C94C` all score the same (38,
   delta 0). The last one is the form kept. This resolves the "endpoint
   identity" question below. The endpoint is not shared with the call
   arguments. The call has none, and the registers are the loop's leftovers.
3. **A void prototype for `osContGetReadData`.** The TU had none, so cfe
   treated it as returning int. The allocator records (procedure 2) showed the
   controller-array cursor web, which spans all three loops, with v0 missing
   from its `p1cost` list (forbidden mask `0x40000000`). It was therefore
   coloured v1, and the remaining 38 words were one v0/v1 cycle across the
   three loops. Declaring `extern void osContGetReadData(JoyPad *)` gave 38 -> 0.

What this breaks: the 2026-09-12 closure ("no source form inside this function
reaches it; the lever is which translation unit owns the pad arrays") and the
p23 floor of 40 from forces. Both were measured on the 3-argument call shape.
The pad arrays remain `extern`. Defining them in the TU was measured as
byte-inert (38 both ways).

#### 2026-09-12 (lane `lane/p7-res2`): the frame ladder is exact; the residual is two loop endpoint symbols

48 to 47, and the stack is now byte-identical in shape. **Declaring `s32 i;`
before `OSMesg unusedMsg;` moves the message local's home down one slot and
makes `frame_census` report the same four-rung ladder on both sides**
(+0x3C +0x38 +0x30 +0x14, frame 0x38). Before the swap the message local sat
one word higher and the aligner carried two immediate-only words; it now
carries one, and byte-exact went 113 to 114.

Eight declaration forms were measured for that slot and only this one pays:
the message local as a two- or three-element array with the call taking the
array name (64 each), a leading `void *` pad (64), a leading `f32` pad (64), a
trailing pad (64), and a one-element array (48, byte-flat). So it is the
*order* of the two declarations, not an array length: L112's free parameter is
not the lever here and L99's declaration ladder is.

**The remaining 47 is 40 register-naming, 1 immediate, 5 really-different, and
the cause is named.** Both of this function's copy loops are reassociated into
pointer form, and in each the candidate and the target disagree only about
which of the two pool registers the cursor takes against the endpoint. The
decisive observation is in the second loop, over the connected-pad flags:

- the target's endpoint register is *also* both outer arguments of the
  save-action call that follows, so that call sets up **no arguments at all** —
  the endpoint, the loop's TRUE constant and the counter pointer are already in
  a0, a1 and a2 when the call is reached;
- the candidate materialises the endpoint as the flag array's own base plus its
  length, which is a different address constant from the counter's address, so
  the two cannot be one web, and the call pays two register copies. The
  candidate spends those two words and saves one on the loop test (an equality
  test against an exactly-reachable bound where the target tests `<`), which is
  why the sizes agree at 159 while the aligner reports one surplus and one
  missing instruction.

That makes the decision variable **the identity of each loop's endpoint address
constant**, not any statement ordering: the flag array's end and the counter's
address are the same address, and the target's compiler knew it because the two
objects were adjacent *in its own translation unit*. In this tree the pad and
button arrays are `extern`, so IDO must compute base-plus-length. Twenty-five
loop spellings — counted `for`, `do`/`while`, `while`, `!=` against `<`, and an
L97 `if (1)` region around either loop, crossed both ways — are one object at
47, and the `!=` forms are 49. **No source form inside this function reaches
it**; the lever is which translation unit owns
`D_800CF370`/`D_800CF388`/`D_800CF3B8`/`D_800CF3BC`, and moving that ownership
is a tree-level change, not a lane edit.

Axes covered this pass: eight message-local declaration/padding forms; twenty-
five loop-spelling forms including both L97 region openers. Not covered: giving
this TU the definitions of the four pad/flag arrays so IDO can see the
adjacency, which is the named next lever and needs a coordinator decision
because it moves `.bss` ownership.

### Exhaustive landscape and endpoint diagnostics (2026-09-12, p23-lastmile5)

The fresh configured baseline is 159 words (636 bytes), frame `0x38`,
55 candidate and target ELF relocations, 47 masked and 48 raw positional
differences, first masked mismatch `+0x2C`. The aligned split is 114 exact,
40 naming, one immediate and three paired structural rows, with a
candidate-only word at `+0xE0` and a target-only word at `+0xC8`. The older
five-structural count includes those two gaps.

The captured Ucode receipt maps this symbol to procedure two of nineteen.
The exhaustive same-save-kind colour scan sampled 18 coloured webs and
132 rows: 104 accepted forces at size delta zero, twenty four bytes short,
two eight bytes long, and six refused forces. The complete delta-zero
winners list is `p1:w9=c3` at 44, `p1:w81=c4` at 46, and `p1:w82=c5` at 45.
All seven nonempty winner subsets were measured. The three-force combination
reaches the diagnostic floor of **40**. The first two forces have a one-word
favourable interaction; the other two pairs are additive. Stock/instrumented
text, data, rodata, symbol and relocation fidelity passes. This measured
winner lattice does not bound arbitrary combinations or source changes.

Four layout-dependent source diagnostics expressed integer-address loop
bounds using adjacent named objects. The first copy-loop endpoint scores 155
with size delta -28; the flag-loop endpoint scores 149 at -32; both endpoints
score 155 at -28; adding the final button-loop endpoint scores 155 at -20.
These are explicitly discarded diagnostics, not portable pointer-bound
replacements or improved candidates. They change geometry substantially
instead of preserving the target endpoint/argument identity. No source body
was adopted. Their source, object, score and rationale remain private under
`build/p23/joyRead/`.

The earlier ownership conclusion also needs narrowing: this current TU
already defines the controller map, enable byte, connected-pad flags and
connected count (`D_800CF3B0`, `D_800CF3B4`, `D_800CF3B8`, `D_800CF3BC`).
The pad and button arrays remain external. Thus moving flag/count ownership
into this TU cannot by itself be the missing change, and the earlier absolute
claim that no function-local spelling can reach the result is not proved.
The unresolved source question is the exact endpoint IR identity that also
serves the following call arguments. Address adjacency and a coincident
numeric value do not establish that identity.

The endpoint diagnostics exhaust this new bounded mechanism without an
improved residual; the previously closed declaration and loop-spelling axes
were not repeated. Resume only with evidence for that shared endpoint web
or a justified TU-boundary experiment. Validation used `residual_map.py`,
Ucode capture plus `allocator_trace_receipt.py`,
`web_footprint.py --every-colour`, all winner subsets in `force_lattice.py`,
configured stock probes, `finalize_plateau.py` and
`tools/gates.sh verify cleanroom check-docs`. The guarded 47-word candidate
and canonical fallback remain; there is no new matching credit.

<!-- plateau-handoff:joyRead:end -->
