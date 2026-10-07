<!-- plateau-handoff:wakeAllocate:start -->
### `wakeAllocate` plateau handoff

- source: `src/main/fx.c`
- score: 345 differing words
- frame: 0x98
- relocations: 3
- first mismatch: 0xc
- summary: JFG efd5abb remains assembly-only; zero source attempts. Need new initialization homes and buffer-loop topology evidence.


Reopening audit (2026-09-08), evidence D: PROVENANCE inspection of Jet Force
Gemini public decomp `src/fx.c` and `src/fx.h` at
`efd5abb1c79636e297b831f7c2d5bf47eac39c0c` found no new target C body.
No donor source, names or values were adopted. Mickey remains authoritative.

Configured full-TU measurement: candidate 1372 bytes / 343 words,
target 1404 bytes / 351 words; size delta -32 bytes.
Raw and relocation-masked differences are 345 and 345, respectively,
first mismatch +0xC; candidate/target frames are
0x98/0x90. Candidate/target static relocation counts are
3/3; 0 tuples agree in function-relative offset, type and symbol.
These are fallback-object comparisons, not linked-C promotion evidence.

Workbench comparison: `structure-mismatch`. Diagnosis: `mixed(constant:25, structural:33, register:116)`;
playbook `constant-audit`, lever `stack-home`. The named guides were read.
This heuristic routing supplies no new donor evidence to reopen exhausted forms.
Next concrete lever requires new wakeAllocate donor C exposing initialization order, early stack homes and buffer-loop topology,
then a fresh gate and configured full-TU comparison.

Stopping evidence: zero source attempts; the authorized donor mechanism supplies
no new implementation. This is the assignment's early-exhaustion stop, not a
five-attempt stall. The prior TU lattice is not repeated: since its recorded
`func_8004ACC4` audit at `4be95a3d`, this TU differs only in EOF handoff comments.
See [the anchor donor audit](func_80049E4C.md) for the full delta and batch disposition.

Ignored evidence is retained in `build/tu-fx-audit/` and `build/wb/`: source and
full-TU object, gate receipts, summaries, diagnoses and relocation tuples.
Commands: `lane_status.py --symbol`, `wb_compare.sh --summary-json`, workbench
`diagnose` and `guide`, and `finalize_plateau.py`. The unchanged guarded C stays
NON_MATCHING and receives zero new exact bytes.

Width-only causal audit (2026-10-04), Mickey evidence B: the retail allocator
forwards its fifth incoming stack argument as a full word. The independently
matched sole evidenced caller sign-extends its signed-short source field before
storing the outgoing word, and the exact resource loader accepts a signed word
and masks it to its low sixteen bits before resource behavior. Direct resident
and overlay calls, runtime call records, source references and stored absolute
function pointers were checked; arbitrary computed indirect calls remain
outside this exhaustive claim. No shared declaration or header was changed.

One isolated contrast changed only the fifth formal and its same-TU forward
declaration from signed short to signed word. The configured output changed
only that argument import from a halfword load to a word load. The candidate
remained 1372 bytes / 343 words, delta -32 bytes, frame 0x98, 345 raw and masked
differences, first mismatch +0xC, with the same three relocation sites. All
thirty-eight other configured TU functions, including the exact caller,
retained identical owned bytes and function-relative relocation identities.
Actual compiler-input capture and stock preprocessing accepted each baseline
and contrast self-context; replay fidelity passed text, data, rodata, symbols
and relocations. Whole-file debug metadata differences were disclosed. The
approved formal/declaration change correctly reports changed cross-context.

The historical full-word fifth formal belonged to an abbreviated draft with a
different allocation topology and extra resource-loader arguments; it was not
a prior isolated control on the restored body. This fresh control eliminates
argument import width as the cause of the current structural deficit. The
unchanged baseline remains tracked; the contrast and raw receipts are private
ignored evidence in `build/wake-word/`. No candidate or exact bytes are adopted.
Next work requires independently authenticated topology or producer evidence;
no declaration, buffer, home or allocator grid follows this negative result.

Flags-CFG causal audit (2026-10-04), Mickey evidence B: the retail guarded
flags initialization branches to separate byte stores of one or zero, whereas
the configured baseline computes a Boolean and emits one byte store. One
isolated explicit if/else contrast at that same nonnull-loader point, on the
proved full-word fifth-formal reconstruction, retained this fork in stock
output. Both paths still execute exactly one store, with the same full signed
word condition, guard, byte value, call order and other expressions.

The contrast measured 1384 bytes / 346 words versus 1404 bytes / 351 words,
delta -20 bytes, 342 raw and masked differences, first mismatch +0xC and
frame 0x98 versus 0x90. It has the same three call identities; relocation
positions remain nonexact. All thirty-eight other configured TU functions,
including the exact caller, retained identical bytes and function-relative
relocation identities. Actual compiler-input self-context and stock replay
fidelity passed text, data, rodata, symbols and relocations; cross-context outside the owned body
remained unchanged. The fixed full-word baseline reproduced the
previous width-only result rather than constituting another width hypothesis.

This proves a source CFG discrepancy and closes three words of the size
shortfall, but leaves five words and the frame deficit. No supported additional
source lever was established within this packet. The canonical diagnostic
baseline is restored unchanged; the defined branch contrast, configured
objects, capture receipts and scores remain ignored in `build/wake-flags/`.
Neither a private candidate improvement nor a mask score receives match credit.
Further work needs fresh producer or structural evidence, not old allocator,
loop, home or declaration grids.

#### 2026-10-07: divisor conversion audit closes without a source change

The configured full-TU baseline reproduces 343/351 words, 345 raw and masked
differences, size delta -32 bytes, first mismatch +0xC, and frame 0x98 against
0x90. Captured compiler-input self-context passes; replay agrees with the
configured object in every allocated section, symbols, and relocation tuples.

The proposed missing unsigned-to-float correction was a mistaken initial
reading: the baseline already emits that correction for its byte divisor.
Explicitly widening the byte cast to u32 is executable-byte-inert. Replacing
the divisor with an unsigned low-byte mask adds one instruction and measures
344 differing words at -28 bytes, while opcode distance worsens from 30 to 36
and aligned structural differences from 33 to 39. This is not a structural
improvement and is not adopted. Both forms preserve the byte-valued divisor.

Source is restored unchanged. The conversion hypothesis is closed; no new
matching bytes. Ignored source, actual inputs, objects, context and fidelity
receipts remain under build/wake-conversion. Existing word-argument and flags
CFG findings remain separate; this packet does not repeat or promote them.

#### 2026-10-07: induction-bound identity controls

Read-only target comparison identifies three count webs: one original count
for guards/remainders and separate copies for the ten-byte and sixteen-byte
unrolled termination extents. The configured baseline instead shares one
copy between those extents. Neither missing allocation space nor reuse of a
saved stride explains this discrepancy.

Three configured full-TU controls, each with accepted compiler-input context:

- Unsigned conversion of only the second loop's address index leaves the
  entire executable text byte-identical to the 343-word baseline.
- A separate signed second-loop index retains the same single bound copy,
  343 words and opcode distance 30; its frame grows from 0x98 to 0xA0.
- An unsigned second-loop index with a signed loop test retains one shared
  bound copy but prevents that loop's remainder/four-way unrolling. It emits
  324/351 words, opcode distance 53 and 97 aligned structural differences,
  versus 30 and 33 on the baseline. The original signed trip domain is retained;
  the smaller output is not progress toward the target.

Independent inspection of frozen configured objects confirms that none creates
the target's third count web. These typed-address and induction-index identity
controls are closed; no further causal lever emerged. Source is restored,
with unchanged allocated sections, symbols and relocation tuples. No matching
credit. Private source/object/context captures remain in build/wake-conversion.

<!-- plateau-handoff:wakeAllocate:end -->
