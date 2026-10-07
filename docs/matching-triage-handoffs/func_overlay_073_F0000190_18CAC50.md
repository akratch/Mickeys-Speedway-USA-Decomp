<!-- plateau-handoff:func_overlay_073_F0000190_18CAC50:start -->
### `func_overlay_073_F0000190_18CAC50` plateau handoff

- source: `src/overlays/o073/func_overlay_073_F0000190_18CAC50.c`
- score: 741 differing words
- frame: 0x98
- relocations: 46
- first mismatch: +0x1C
- summary: Configured stock 760/762 result, 740 relocation-masked differences and exact frame; observed declared homes and the multiply-hazard schedule are reconstructed. Entry narrowing, one compiler scratch home and exact relocation/linked proof remain unresolved.

Header regenerated from the ranking on 2026-09-23 (check_shard_metrics --write); it read first mismatch +0x0.

#### 2026-10-02, lane x-sib2: first notes after the sibling Draw matched

`func_overlay_073_F0000D70_18CB830` matched as a sibling copy of the overlay
71 renderer (packet macros, `vertices[vertexBank * 6]` over ten-byte
vertices); this updater writes the same vertex banks.

- `D_20` through `D_54` are LOCAL records against the module's rodata (data
  +0xD0); they are float literals (0.004, 0.1, 0.064, 22500.0, 1.2, 1.6).
  Writing them as literals is inert (755 at +24) but is the shape the
  promotion needs.
- Entry: the target re-sign-extends the 16-bit field at state+0x94 before
  the multiply by `updateRate` (a cast the candidate lacks), keeps the
  state pointer in a temporary spilled at +0x58, and in case 0 stores the
  zero timer and passes the zero float argument from two separate
  materialisations (the candidate shares one).  The hits buffer is at
  +0x38 in the target (+0x90 in the candidate), so the local set and order
  differ from the first block on.

#### 2026-10-03, owned output-storage and lifetime packet

Independent exact resident `func_8005776C` writes `HitCopyState *` elements and
returns an `s32` count. All four shipped calls bind to this callee. Replace the
false scalar output with a compatible opaque tagged pointer array of eight
entries and an exact pointer-to-pointer formal; preserve the indexed fourth
selection and the first three element-zero selections. The supported query
and RNG domain is count 0..8, index 1..8 and decremented index 0..7. Neither the
producer nor getter universally clamps the registry count to eight. Keep the
callee's `s32` ABI; narrow returned values only at independently observed
signed-halfword transports. These changes disclose an intentional local
prototype-context correction, not unchanged context against the old source.

The exact matched drawer independently proves twelve ten-byte, alignment-two
vertices, resource at state+0x78, bank byte at +0x7C, state size 0x9C and
alignment four. Reconstruct this actual producer storage and bank*6 indexing;
keep the eight x/y stores, all z/color bytes and opaque resource unchanged.
Supported banks are zero and one; the preserved-state path does not clamp the
bank. Share the mutually exclusive vector and angle/delta roles only after
proving their dominated assignments and nonoverlapping lifetimes.

The two case4 paths join the shipped existing timer tail. Both execute the
same load/add/wrap exactly once after their original side effects; the early
path sets mode two and skips countdown/query/target/RNG updates. This removes
the duplicate float-pool pair and closes text relocation cardinality from
48 to 46. It does not establish exact relocation offsets or linked identity.

In case3 and case4, refresh target data after the independently authenticated
angle/RNG calls instead of retaining a named data pointer across them. Retail
reloads and the eliminated compiler spills support the lifetime reconstruction.
Arctanf has no stores or calls; the RNG helper writes only its canonical seed.
Intervening angle stores cannot overlap state.target or target.data on valid
live allocated objects. The constructor places own state beyond the header
for nonnegative signed resource counts; allocation selector 84 means raw
header kind 85. No equivalence is claimed for malformed, interior, freed or
asynchronously mutated pointers, or negative resource counts.

Preserved stock controls: typed array alone 757/+24/frame B8; common carriers
744/+28/frame98; precise return-width transports 762/+80/frame98; typed vertex
storage 765/+92/frame98. A used short-copy/compound multiply is output-inert.
Shared timer tail yields 750/+32/frame98/46 sites. Widening only the angle
snapshot reproduces the missing premultiply conversion but regresses to
758/+40/frameA0, so remains a private diagnostic. Case3 pointer refresh keeps
750/+32/frame98 while reducing opcode distance 103 to 96; case4 refresh yields
749/+32/frame98 and reduces it to 90. The final aligned residual has 110
structural sites; alignment changes make these counts incomparable with a
positional score.

Actual configured compiler input was captured synchronously under untouched
stock IDO arguments. Self-context is unchanged; all allocated TU bytes,
section geometry and effective relocation rows match raw stock output.
Only this guarded body and its local declaration/type context changed;
canonical fallback ROM remains the acceptance baseline. The narrower fidelity
reader does not account for the compiler-owned switch section; full-TU byte
and relocation equality supplies the stronger capture proof, not runtime
binding proof. Exact linked-byte and runtime relocation geometry remain
unproved. No match credit is claimed.

Remaining concrete blockers are eight excess words, stack-home relationships
(buffer/state versus the retail homes), and the entry signed-width conversion.
No independently supported next source lever was available after the two
informative pointer-lifetime controls; no declaration-order or padding grid
was run. Preserve all meaningful source/object/capture/score artifacts privately.
#### 2026-10-07: committed handoff recovery

Recovered the source and handoff from committed lane result
`e90f31aff2eca30cf491406feec42285f09fcf2f`, without accessing that lane's
working state or making a new matching attempt. Fresh configured stock IDO
reproduces 768 versus 760 words, 749 raw and relocation-masked differences,
first difference +0x1C and the exact 0x98 frame. Static and target runtime
relocation cardinality is 46 on both sides, but only two offsets/types align;
16 candidate identities resolve and 30 literal/switch-table address records remain
unresolved. This remains NON_MATCHING, with zero new matched bytes.

The actual compiler input was captured and stock-preprocessed synchronously;
self-context comparison is unchanged. Every allocated section's geometry and
bytes and every normalized relocation identity agree between raw compiler
output and the configured object. The only symbol-table addition is the
assembler prelude guard. The query producer's pointer-array ABI and matched
drawer's vertex layout remain the independently authenticated storage basis
from the earlier handoff; no declarations or semantics were changed here.

`gmake verify` reproduces the expected US ROM SHA1 with the assembly fallback
selected. `tools/wb_compare.sh --summary-json`, targeted ranking refresh and
`function_preflight.py --analysis-only --json` provide the fresh measurements.
Actual compiler capture, self-context, fidelity, preflight and verify evidence
remain private under `build/recovery-o073/`. This recovery does not authorize
new attempts: integrate it and establish current reopen pins before a separate
complete stack-home and signed-angle reconstruction packet.

#### 2026-10-07: joint automatic-home and angle packet

The authorized packet followed recovery integration and a fresh zero-exit
`base-only` assignment gate. The configured candidate is now 762 versus 760
words, 741 raw and 740 relocation-masked differences, first difference +0x1C,
with the target 0x98 frame. It remains NON_MATCHING: zero matched bytes.
The shape-tolerant opcode distance fell from 90 to 6; this diagnostic is not a
percentage or a substitute for exact bytes and relocation identities.

Real function-scope roles reconstruct every observed declared home: coordinate
components, the signed step, vertex and state pointers, and the eight-entry hit
buffer. Shared roles have dominated assignments and disjoint lifetimes. The
remaining frame difference is one compiler-created float-rate spill home,
+0x30 instead of +0x34, with the same three saves and three reloads. Target
storage widths, vertex-bank bounds and the earlier query/RNG domain remain
unchanged. The signed angle snapshot is word-sized; the persistent step stays
a signed halfword. Both steering comparisons now negate the promoted signed
step without an incorrect narrowing cast, preserving the target's behavior
when that step is the signed-halfword minimum.

The target retains a named scaled-radius value across each distance expression.
Reusing the existing, nonoverlapping float `limit` role reconstructs that
transport. Integer zero in the case1 velocity/query lifetime and the case3
sign comparisons reproduces the target's shared positive floating zero. These
integer-to-float conversions preserve the value and floating comparison
semantics; the distinct float zeros after the reset calls remain distinct.
First-hit reads precede independent state-field writes. The fourth query uses
a signed-halfword count and a word-sized selected index, with its actual
nonzero arm first. The case3 nonnull arm and final velocity-limit arms follow
the target's control flow. Case4 uses a complete natural if/else with one
shared timer tail, preventing an incorrect early conversion merge while
preserving the skipped side effects and exactly one timer update.

The object-local `-Wab,-r4300_mul` correction is supported by the target's
independent floating-multiply hazard sequence, not by a positional score.
On identical captured compiler input it restores four missing scheduler nops
and the radius-test branch-delay form. The full TU owns only this guarded
function; no other object receives the flag. Without the flag the same source
is 758 words with opcode distance 12. The configured flagged source has two
excess entry normalization words, followed by a query-result scheduling
residual and allocation differences. The actual stock compiler input and raw
output are preserved; no emitted instruction is edited.

Closed entry controls include direct/result casts, ternary and assignment
predicates, a register hint, widened carriers, phase/short-phi splits, unsigned
views, low-product masks, unsigned negation, multiply-by-negative-one, external `abs`,
and scalar/one-field-struct/one-element-array storage. None closes the signed
step's entry normalization and persistent halfword home together. The array
introduces real entry memory traffic; the widened forms spill a word; standard
`abs` emits an unwanted call. Named float-rate controls either hoist the wrong
store lifetime or grow the frame. Chained query assignment, assignment inside
the predicate and count-versus-index predicates are output-inert; stop that
schedule axis after these three controls. Destructive coordinate squaring and
nonzero integer-literal spelling also failed to explain the residual.

Fresh configured preflight still finds 46 static and 46 runtime relocation
sites, but only one offset/type pair aligns. Sixteen candidate identities are
resolved and thirty literal/switch-table identities remain unresolved. The
candidate exceeds the owned executable range by eight bytes. Actual compiler
capture agrees with the configured object's allocated sections, geometry and
all normalized relocation identities. The dedicated TU flag-impact report
confirms one consumer and 46 static sites under either flag choice; its raw
positional preference for the unflagged build does not override the observed
hazard mechanism. Fresh extraction, overlay-symbol regeneration and
`gmake verify` reproduce the expected US ROM SHA1 with the assembly fallback. No
candidate linked-byte or runtime-identity proof is claimed.

All meaningful sources, actual preprocessed inputs, raw/configured objects,
contexts, frame censuses, scores and stock phase streams remain private under
`build/o073-home/`. A reproducible next packet needs new evidence for the CFE
short-assignment/predicate web split or the compiler spill-pool ownership;
repeating the closed width and spelling controls is not a new hypothesis.
The float-pool/switch address bindings and final linked owned-byte proof also
remain necessary before any promotion.

<!-- plateau-handoff:func_overlay_073_F0000190_18CAC50:end -->
