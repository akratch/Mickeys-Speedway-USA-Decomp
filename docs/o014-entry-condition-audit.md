# Overlay 14 entry-condition audit

- source: `src/overlays/o014/func_overlay_014_F000013C_186FA14.c`
- score: 29 differing words
- frame: 0x30
- relocations: 20
- first mismatch: +0x0
- summary: Entry-condition repair premise disproved; source unchanged.
## 2026-09-27 entry-condition audit

The assigned hypothesis was that complementary source returns incorrectly made
this loader unreachable. The authenticated target disproves that premise:
both signed entry tests use the unchanged saved group value and exit on
complementary conditions. Their delay slots do not change that value. Neither
branch is covered by a runtime relocation. The only exported entry in this
480-byte range is the function start. This establishes the normal-entry path;
it is not a universal proof against arbitrary computed interior entry or
unobserved runtime code modification.

All 120 extracted instruction words were checked against the authenticated US
ROM. Both runtime relocation tables were decoded; the range contains 20 runtime
records. The extracted target object contains 10 text relocations, while the
candidate contains 20. Literal address fields in the extracted assembly account
for the differing static surface. These counts do not establish identity: the
comparison reports 20 relocation-target differences and 10 metadata differences.

The unchanged configured full TU compiles with O2/mips2 to exactly 480 bytes:
29 masked and 34 raw differences, first mismatch +0x0, frame 0x30 versus target
0x28. Both complementary returns survive compilation, including the remaining
body. There is no justification for making that body reachable. Aligned metrics
are 18 register, 7 constant, 5 schedule and 15 structural differences, plus four
insertions and four deletions; they are distinct from the positional score.

The actual stock preprocessor output passed self-context comparison unchanged.
A repeated configured build with copied stock compiler binaries passes text,
data, rodata, relocation and symbol fidelity. Recompiling the preprocessed C
also preserves sections and relocations, but its symbol surface lacks the
asm-processor private prelude marker; that replay is not claimed fully faithful.
No instrumented compiler-phase investigation was needed to reject the assigned
entry-condition premise.

Zero semantic-repair attempts and no game-source edits. No matching credit.
The next source packet requires a new supported identity, frame-home or operand
lifetime mechanism; close positional geometry alone is insufficient. Entry
condition deletion or inversion is specifically unsupported by this evidence.

Assignment passed zero-exit base-only after the separately committed bounded
include-ownership fix in `8ac3118dd`. The owner wrapper remains pinned to
`5978c746d766c4b2f70e9e4f7f40e2bfe9833978`, and the included candidate to
`308dbdd683bbf5d769ca50e1593a465eb9dfe83a`. No reopen authorization was required
before this receipt; it does not authorize a future packet.

Validation: configured full-TU build, stock repetition fidelity, preprocessed
self-context, object comparison, authenticated ROM extent and runtime-table
checks. The accompanying tooling commit passed all 76 tooling test files and
full ROM verification after the fresh lane's overlay-alias bootstrap. This
report-only follow-up uses documentation and clean-room gates under ADR 0017.
Private objects, compiler input, comparison details and tables remain ignored
under `build/o014-evidence/` and `build/o014-runtime-relocations.json`.

Four C compiler invocations were used for the unchanged baseline and replay
checks; one separate preprocessing invocation. No source variants were compiled.
Preparation, first-useful and total proof elapsed times were not instrumented.
The initial acpp capture wrapper produced no phase captures and supplies no
phase evidence. Full canonical setup/verification compile cost is separate and
was not completely counted.

## Follow-on frame-reservation investigation

The configured baseline remains 480 bytes, 29 masked differences and a 48-byte
frame against the target's 40-byte frame. Both objects have the same observed
stack-access offsets and 12 bytes of saved registers. The extra eight bytes do
not represent additional observed stack traffic; naming them as a particular
local or spill would exceed the evidence.

Native UOPT/UGEN capture and lineage runs preserve the stock full-TU text, data,
rodata, relocations and symbols. Input Ucode reserves 32 bytes of local storage;
optimized Ucode reserves 16 bytes. The traced UGEN dynamic temporary-area path
returns zero. This narrows the unexplained reservation without identifying its
source-level owner. The optimizer's traced descriptor splits do not report the
chosen temporary slot, so descriptor identity is not a final-home witness.

A separately supplied CFE source-name tracer, substituted as the only changed
compiler stage under normal g0 flags, also passes full-TU section, relocation
and symbol fidelity. It binds the seven declared scalar/pointer locals and an
anonymous switch carrier to CFE storage identities. The named variant carrier
and anonymous switch carrier are distinct. Raw declaration fields are not
claimed as final frame homes. No debug flags were used for this proof. A
separate debug-listing attempt aborted in the compiler wrapper and supplies no
object or fidelity evidence.

Two semantic-preserving probes were compiled and retained privately, then
reverted: removing the single-use variant variable in favor of switching on
the call result, and making the operation-one comparison's constant unsigned
(the operation is an unsigned byte). Both produce byte-identical configured
output. Neither eliminates the reservation or improves the residual. These
are causal probes after the entry-condition audit, not semantic repairs.

There is no supported next source lever in this packet. A further reservation
investigation needs a faithful optimizer trace connecting descriptor allocation
to chosen temporary slots and eventual frame usage, or independent source/ABI
evidence. Declaration sweeps and entry-condition changes are unsupported.
Canonical game source is unchanged; no match or executable-byte credit results.
The stock and instrumented objects, traces and both variants remain ignored
under the lane's build directory. Documentation and clean-room validation cover
this evidence-only receipt; the separately integrated discovery tool already
passed its full required gates. Phase/probe elapsed times and a complete total
compiler-invocation count were not instrumented; none are inferred from file or
commit timestamps.
