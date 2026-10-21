# Landing campaign plan — the last two functions (drafted 2026-10-21)

Status at drafting: resolved 99.08% (934,928 / 943,640), 8,712 bytes in two `NON_MATCHING` functions in
`src/main/anim.c`. Every remaining difference is now traced to a named compiler decision, and each decision has a
counterfactual that reproduces target bytes. This campaign turns those counterfactuals into source.

## 1. What each blocker needs (proven by counterfactual)

### func_800517E0, case 0x60 (the one length defect)
- Target compiler output (reproduced by listing surgery on the adoptable tree: 841 -> 113 masked, size 0, residual
  103 -> 96, nothing else moves): the halfword's s16 sign-extension pair is emitted but dead, two ring draws before the
  shift, the shift straight into a1, the mask into a ring temp, and a move into a1. The assembler deletes the dead pair
  (consumers demand <= 16 bits) and keeps the copy (producer-read refusal, law L178).
- Why every spelling so far fails: ugen hands the a1 destination to the first operation on the expression's left spine,
  so a live sign-extension pair takes a1 and the assembler merges the copy; spellings without the pair draw two fewer
  ring temps, which moves the loop-continuation block's load from t9 to t7 and — because a `break` reads every register
  in as1's liveness (L178) — flips copy deletions in 0x4B, 0x79 and 0x7B.
- Open question (a uopt question): what natural source keeps an s16 conversion of the halfword that uopt does not fold
  but the assembler finds dead, or keeps a1 busy while the pair is evaluated. ~25 cast/ternary/comma/carrier spellings
  all canonicalise to the same tree.

### func_800517E0, case 0x79 (20 of the size-0 words)
- Target: the store to D_8007D6BC goes through the materialised address in v1 (an occurrence of the address web);
  the read-back is forwarded; no separate `frame` definition. Listing surgery reproduces it (116 -> 87 on the s8 diag).
- uopt forces show colouring the address piece colours the wrong occurrence: in our stream the store is by symbol, not
  an occurrence of the address web. Open question: what decides by-symbol vs through-register for a global store.
  Rule found (ucode level, 41 cells): uopt keeps global accesses by symbol; an address (lda) web for `&sym` exists only
  from LOADS, and when it is coloured uopt rewrites a STORE through the register only if (a) the web has a load
  reference in that block and (b) the store's RHS tree contains no float multiply. The target's store is the web's
  first occurrence (two occurrences -> net 10 -> v1 naturally) and the read-back is as1 store->load forwarding.
  Reproduced store form: `value = value * scale * factor; frame = D_8007D6BC = (s32) value;` gives the shipped
  lui/addiu v1 ... sw t7,0(v1); move v0,t7 — but rotates FP colours (811/+4) because `value` is a function-wide web;
  `DKWB_SUBST_KEEP` on a dedicated float carrier gives the same form. Lever: a float carrier uopt keeps whose single use
  is the cvt in the store, plus no separate `frame` copy (the target tests blez t7 and divides from v0). Next cells: a
  kept float carrier with a natural second use (shared_expr), `D_8007D6BC = (s32) value; frame = (s32) value;`.

### func_800517E0, colour-only rows
- Combined listing-surgery floor with both blockers fixed: 84 masked on the s8 diag (naming 20, imm 62, different 2).
  Rows in 0x4C (two swapped shift/store pairs), 0x4D, 0x6D (`value` f16 vs f0), 0x7E, 0x40, 0x45, the 0xB group.
  Re-measure on the adoptable tree once 0x60 is closed; ring phase (ring-phase-diff) and L178 liveness may explain
  several.

### func_80054B3C (17 stack words)
- The original reused the dead coordinate locals z1/z2/x1/twoZ1 for the four values live across the first sqrtf.
  D1 (keep the late definitions) is SOLVED as source: early-snapshot carriers (`stepX1 = quadB;` before the `if`,
  `z2 = stepX1 * stepX1; stepX1 = 0.0f;`) are byte-identical to forcing the keeps.
- D2 is open: the kept webs must be sent to memory at their first decision; adding blocks makes them split but the
  remainder piece is still coloured (caller-saved option cost 2) — an order/pressure property. The full 7,473-cell
  split-oracle sweep reproduced the forced state in 0 cells.
- The early doubles twoZ2/twoX2 are arena temporaries in the target; the current `addr = &twoZ2/&twoX2` oracle is the
  wrong mechanism and must be replaced.

## 2. Instruments now available
(all landed; usage in docs/LANE_BRIEF.md and the workbench docs)
- Listing surgery as the decisive oracle: `tools/stream_surgery.py` (edit the `cc -S` listing, reassemble with stock
  as0/as1) — price any hypothesised compiler output before searching source.
- Shared instrumented farm (the compiler `fast_score.INSTRUMENTED_CC` names) = reproducible uopt-v3 / ugen-v4 / as1-v3 (hash-pinned
  chains in the workbench: `decomp_workbench.uopt_chain`, `compilers/ido-5.3-mickey/{ugen,as1}`), object- and
  trace-identical to the previous binaries under every old variable. New: `CDX_FORCE p1:wN[.K]=y|n`, `CDX_WEIGHT`,
  `CDX_GROW`, folded `DKWB_UOPT_SLOT_TRACE`, `DKWB_UGEN_RING`, `DKWB_CVTL_TRACE`, `DKWB_DEST_TRACE`,
  `DKWB_AS1_TRACE`.
- Readers: `decomp-workbench webmap` (target-side value webs diffed against a candidate), `ring-phase-diff`,
  `trace-as1-peep`, `web_report.py`, `subst_report.py`.
- Search: `lever_sweep.py --source-levers` (literal type, carrier rename with cross-iteration safety, reorder,
  mask/shift placement, read order; --case, --pairs, --diag, --onehot/--greedy, --emit; v3 force oracles). Replay
  rediscovers the landed 0x22/0x25 and 0x6F levers. Forward sweep on the current tree found one component:
  0x79 `frame -> a dead s32 local` removes the frame copy the target also lacks (adoptable size 0 / residual 97 only
  by cancelling 0x60's extra word — adopt together with the 0x79 store fix, not alone); `keep_search.py`
  (forward-substitution keeps); `block_probe.py` (N/4 threshold); `shape_product.py` + `shape_freeze.py`;
  `lane_combine.py`; faithful permuter (`permute.sh` fidelity gate; the interpreter scratch is now identical).
- Semantics gate: `behaviour_check.py --auto-stubs --big-endian` with drivers
  `tools/behaviour_drivers/anim_interpreter.c` (every opcode, byte/halfword edge values, func_8005AD64's clamp
  modelled; 139/150 mutants caught, the rest true no-ops) and `anim_collision.c` (exact float bits, ulp boundaries;
  85/100 mutants caught, rest no-ops); it confirms the 2026-10-21 interpreter edits preserve behaviour and rejects the
  s8 diagnostic. ~2.5 s / 0.6 s per candidate; cached reference.
- Workflow: `microlab.py`, `finalize_plateau.py --record-only --allow-dirty`, gates hygiene.

## 3. Lanes
Each lane: one blocker, a priced target state (listing surgery or force), a decision-level objective checked with the
instruments, the behaviour gate on every adoptable cell, measured stopping rule.

| Lane | Target | Method | Stop when |
|---|---|---|---|
| L60 | 0x60 at 113 masked / size 0 on the adoptable tree | uopt traces of the s16 conversion's life (subst/web_report) to find what keeps a dead-to-as1 conversion; dest/ring traces; microlab for spellings; then whole-TU confirmation | match, or the uopt decision is shown unreachable with the predicate list |
| L79 | 0x79 exact (D2 stream) | rule for by-symbol vs address-register stores; forces to price; webmap to check | as above |
| LCOL | colour-only rows after L60/L79 | lever_sweep --source-levers --greedy on the new base; ring-phase-diff; as1 trace for remote rows | exact or flat over the full catalogue |
| L54 | 17 -> 0 | from the D1-solved cell: price D2 with `CDX_FORCE p1:wN.K=n` on the remainder pieces; search order/pressure levers with lever_sweep oracle mode; replace the early-double oracle with arena-temporary forms (webmap) | match, or remainder refusal shown unreachable |
| PROMO | promotion | when a function reaches 0 at size 0 in the full TU: real link, `gmake verify`, promotion gates, docs, scoreboard | — |

Sequencing: L60 and L79 in parallel (both on the interpreter: work on scratch copies; combine with lane_combine);
LCOL after either lands; L54 in parallel throughout. Arm each lane with `tools/authorize_reopen.py --symbols ...` at
dispatch (the current authorizations are inert because the tree moved).

## 4. Risks
- Both open questions may need source constructs outside the explored vocabulary (e.g. macros, inline helpers that
  IDO does not inline, different struct typing). The instruments make each candidate a seconds-long measurement.
- The size-0 s8 diagnostic is never adoptable (func_8005AD64 clamps negative frames); the behaviour gate catches it.
