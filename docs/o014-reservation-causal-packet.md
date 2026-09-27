# Overlay 14 retained register-home reservation

Candidate source is retained at lane commit
`e3fdcd9f9a6f3a17c4874c91df1c57ec994a392b`. This report's integration does not
integrate that source: the canonical guarded body still has the 29-word
residual and 48-byte frame recorded in the baseline below.

The retained candidate improves from 29 to 27 masked differing words at the
same 480-byte size. Its frame now equals the target's 40 bytes. This is nonexact:
there are still 32 raw differences, four opcode differences and unresolved
runtime relocation bindings. There is no new matching credit.

## Assignment and baseline

The coordinator assigned the specific reservation-producer hypothesis after the
entry-condition investigation. The new isolated lane started at
`7d931fd64342990eb399b675de268eadd72aa1d4`. Before source changes,
`tools/lane_status.py --base HEAD --no-cache --symbol
func_overlay_014_F000013C_186FA14 --json` exited zero with `base-only/ready`,
no active owners and no structured prior plateau. No reopen override was used.
The source wrapper was pinned to `5978c746d766c4b2f70e9e4f7f40e2bfe9833978`
and the included candidate to `308dbdd683bbf5d769ca50e1593a465eb9dfe83a`.
Earlier negative receipts remain in `docs/o014-entry-condition-audit.md`.

The authenticated full-TU stock baseline and actual preprocessed self-context
from that packet were retained unchanged. The owned range remains 120 words;
the baseline frame was 48 bytes, with 29 masked and 34 raw differences. Its
observed stack traffic and saved-register offsets already agreed with target.

## Measured cause and one controlled source probe

A private trace-only UOPT prototype distinguishes two independent quantities.
The allocation reserve starts at 32 bytes and grows to 36 for one four-byte
spill slot. That slot does not set the final emitted local-area definition.
Instead, the register-home emitter's local highwater grows from zero to 16
when emitting a `Urstr` home annotation at local displacement -16, paired with
`Urlod`. These are pseudo operations; they do not prove extra machine stack
traffic. Normal-g0 CFE tracing identifies that source home as `left`.

The prediction was recorded before editing: move only `left` from the fourth
local declaration to the second, leaving `cursor` first. This should change its
retained home from -16 to -8 and emitted local-area demand from 16 to 8, while
leaving allocation reserve and expression behavior unchanged.

The stock full-TU result confirms all three changes: home -8, local-area size
8, and frame 40. The allocator still reserves 36 bytes and selects the same
four-byte spill displacement. Only the two frame-adjustment words improve;
size and remaining executable geometry are unchanged. There are no initializers
in the moved declaration and no changes to calls, expression evaluation,
control flow, widths, volatile accesses or object lifetimes. Complementary entry
returns remain intact. This is a single causal declaration-home control, not a
declaration sweep or evidence that every source local has a stable cross-pass ID.

The current first positional executable/register mismatch is +0x98. Earlier
relocation layout/identity differences remain and must not be hidden by that
masked location. The candidate retains 20 text relocations versus 10 in the
extracted target; 20 identity and 10 metadata differences still need independent
runtime binding proof. The frame correction does not satisfy those obligations.

## Fidelity and stopping boundary

Both the exploratory and narrowed producer traces pass configured full-TU
stock fidelity for text, data, rodata, relocations and symbols. The narrowed
trace additionally passes disabled and enabled controls. A separate CFE-only
product substitution also passes stock/OFF/ON fidelity and joins 73 named
frontend events to exact input Ucode intervals. Its six `left` operations share
the new declaration identity and map to retained input records. This proves
frontend interval provenance; the optimized-home association is the measured
single-declaration control, not a generic automatic lineage claim.

The candidate's actual preprocessed input passes context comparison with itself.
Private baseline/candidate objects, source variants, phase captures, comparison
reports and fidelity receipts remain under `build/uopt-reservation/`.
No source permutation follows: the remaining register/loop geometry lacks a
new supported source lever in this packet. Further work needs a concrete
producer cause for that residual, plus complete runtime relocation identities.
The retained guarded C remains nonmatching and the canonical ROM uses fallback.

Validation includes all 76 project tooling test files and canonical full-ROM
verification; the rebuilt ROM retains the expected US hash. The initial fresh
lane verification exposed missing derived relocation aliases after guarded
objects rebuilt. Regenerating the overlay alias surface and repeating verify
resolved that build-state failure without tracked alias changes. Documentation
and clean-room gates are required again on the final receipt before commit.
The reusable producer tool lives in a separate workbench commit, not in this
function-sized source change.

Timing was sampled at packet start and assignment, but preparation, first-useful
and proof durations were not fully instrumented. No effort estimate is inferred
from commit or artifact timestamps. Tool build retries and game compilation are
separate activities; a complete compile count was not recorded.
