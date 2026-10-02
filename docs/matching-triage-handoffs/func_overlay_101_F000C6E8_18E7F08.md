<!-- plateau-handoff:func_overlay_101_F000C6E8_18E7F08:start -->
### `func_overlay_101_F000C6E8_18E7F08` plateau handoff

- source: `src/overlays/o101/overlay101TailC6E8.c`
- score: 0/317 words, promoted
- frame: 0x18
- relocations: 91
- first mismatch: none
- summary: Matched. Rewritten in the matched siblings' shape after decoding the relocation table: one queue symbol, a plain indexed shift loop, one-name counter subscripts and two-argument creators.
- assignment base: `23dd98ede1da869df2df0948c80efeeb403caa6e`
- owned range: overlay 101 `+0xC6E8..+0xCBDC`, 1,268 bytes / 317 words, with no target-size delta
- baseline: 131 raw and 125 relocation-aware positional differences; normalized distance 117; first raw mismatch `+0x8` and first relocation-masked mismatch `+0x34`
- retained result: 130 raw and 124 relocation-aware positional differences; normalized distance 116; exact 317-word size and exact `0x18` frame
- relocation proof: target and candidate each emit 91 runtime records; 86/91 offsets and types align, one identity is statically stable, and 90 candidate identities remain unresolved, so preflight correctly fails closed
- classification: workbench reports `mixed(constant:7, structural:48, schedule:33, register:81)` with 31 opcode differences; the retained form removes the sole commutative-order difference
- attempts: five bounded source hypotheses—explicit root constants, selector declaration order, pointer-add operand order, selector carrier width, and a persistent root pointer. Three were byte-flat, the root pointer regressed to 327 words, and only the pointer-add ordering improved. The historical flag lattice and root-order work were not repeated; no generic permuter or new flag sweep ran
- retained improvement: spell the queue end as count plus cursor, matching the target's commutative operand order and closing one positional/register word without changing semantics
- next action: reopen only with new source evidence for the root-initialization constant-preload schedule and selector/temp allocation web; do not repeat these five forms or the prior flag lattice
- JFG utility: the overlay donor scan reports no JFG candidate for overlay 101. JFG `src/overlays/o19/overlay_19.c::pauseUpdate` is the nearest skeleton at only 0.0691 and remains assembly-only, so it is context rather than a credible source or insertion point; there is no JFG ledger entry to publish from this plateau
#### 2026-09-11, lane f10-mid: p1-only, and the a1 holder is web 101

Still 119. Frame 0x18 and every stack home agree on both sides, so the
residual is allocation and ring phase only. The instrumented uopt (text
confirmed identical) records 31 p1 decisions and no p2 records: the function
contains calls, so "selector declaration order" measured an axis it does not
have. The closure says the target holds the selector in a1; in the candidate
a1 goes to web 101 (save 5.0, totalsave 10, nocs 2, cost 0) and later to
web 4 (save 0.667, nocs 3). The decision variable is therefore the ratio
that orders the selector's web against web 101 ([L100]); a symbol-boundary
edit or an L109 probe on the selector is the lever, and neither has been
measured. The web numbers are from this lane's own compilation and must be
re-derived before forcing.


#### 2026-09-12, lane p19-reopen: three interfering locals priced at 19 words

Assignment was base-only. Retained before/after buckets are 220 exact /
75 naming / 0 immediate / 26 really different. Positional score is 113 raw,
106 masked at 317 words, delta zero, frame 0x18, first +0x4C, displacement
tax five. The address
map and register census require eleven incoherent windows; this is not one
whole-function ring cycle. No candidate body change is adopted.

The configured full-TU instrument passes byte identity. Ordinal 0 is p1
only. Detailed web records identify nodeIndex as web 60 (total 17 over two
components, c7/t0), previousType as web 80 (6 over two, c8/t1), and previous
as web 84 (6 over two, c9/t2). These three are decided before the address
and constant webs that need the target's t0-t2. Web 138 is an address constant
at total 5 over two components, and web 136 is the 255 literal at 3 over two.
The queue-base high-address web 126 is separate; selector already agrees in
a1. The older selector/save-ratio closure is not the current decision.

Forcing w60 to c10, w80 to c11 and w84 to c12 records accepted forced values
10, 11 and 12. Scored directly, that object gives 94 raw / 87 masked at delta
zero, buckets 242/53/0/26. This prices 19 positional masked words and 22
aligned naming rows without claiming source equivalence or matching credit.
L142 alone cannot free these three coloured occupants; the issue is their
interference and ordering, not an unoffered argument colour.

Removing only the node pointer locals is text-identical at 106: L131 keeps
the repeated indexed expression as one name. Removing the index carrier as
well gives 166 masked and adds 12 bytes; combining that with direct previous
links gives 162 and adds 12. Direct previous-link stores alone give 109 at
delta zero, buckets 212/91/4/11. A fresh dependency-guarded order climb on
that shape evaluates 224 forms in two passes and reaches 108, still worse
than the retained baseline. The prior shape's order closure was not assumed
to cover it, and no speculative store reordering was adopted.

An index-based queue loop gives 295 and adds four bytes. Regions around the
previous-link or root-link groups give 155-166 and add four or eight bytes.
A region after the first node-index definition is text-identical; applying
it to all such definitions gives 111 at delta zero. Separating the node20
index into the existing orderIndex local gives 107 at delta zero. The final
index/region and new-shape order controls do not improve the retained result
or expose another causal decision. This is a bounded plateau, not a proof
that no source spelling can match.

The next concrete source question is how to move the three named locals out
of the address/constant webs' interference without widening the instruction
stream; accepted forced objects supply a target for that question. All
source/object/record and climb receipts remain in private external scratch.
Commands: alignment, residual map, register/frame censuses, configured
stock/instrumented compilation, direct forced-object scoring, guarded
blockclimb, `tools/finalize_plateau.py`, `tools/gates.sh`. The 91 runtime
relocations still lack a complete identity proof; no promotion was attempted.
The unchanged assembly fallback passes full-ROM verification. Only the source
handoff comment and this shard are refreshed.

#### 2026-10-02, lane x-o101: matched, 106 -> 0 at delta 0, promoted

Every earlier pass varied spellings and forces inside the inherited m2c
shape (node pointer and index carriers, captured prior indices, four-argument
creators, a walking queue cursor, two extern names for the queue). The
relocation table says otherwise, and the rewrite was priced on the fast
direct compile (base copy measured 106, equal to score_symbol.py):

- The selector and the queue base are one SYMBOL target (resident data
  +0x31DC), so the selector is the queue's first byte; the queue count is
  +0x31E4.
- The two sprite creators (0x8F, 0x90) are one resident routine, the same
  one overlay101TailAB4C and overlay101TailA6BC call with two arguments; the
  a2/a3 the shipped calls carry are the root and order-count addresses left
  in those registers. The compact creator also takes two (key, node).
- Cases 14 to 19 are overlay101TailA6BC with 0..5, 22/23 are
  overlay101TailBA34 with 0/1, the special and default calls are
  overlay101Reset(NULL), and the 0x5F1/0x5F2/0x58 acquires are one routine.
- Written in the siblings' form (one-name counter subscripts, creator result
  stored straight into the node, the queue shift as
  `for (i = 0; i < count; i++) queue[i] = queue[i + 1];`) the first
  candidate scored 274 at +8. The extra words were a second count test: the
  for loop's own guard is the target's second `blez`, so the inner
  `if (count > 0)` the inherited shape carried is redundant. Without it: 0
  masked at delta 0.

Promotion: every call is a placeholder (all are SYMBOL records, including
case 10's overlay101BuildPresentationA, which the old candidate called
directly). The 25-entry switch table duplicates the retained rodata at
data_rodata +0x5CFC (rodata-relative +0xEAC): overlay 58's metadata-only
form, the two text references rebound to gO101TailC6E8SwitchTableReloc, the
pool externalized by digest, an externalized ownership row in the atlas.
gmake verify OK; check-overlay-syms and promotion-proof PASS (317 words,
91/91 relocations).

<!-- plateau-handoff:func_overlay_101_F000C6E8_18E7F08:end -->
