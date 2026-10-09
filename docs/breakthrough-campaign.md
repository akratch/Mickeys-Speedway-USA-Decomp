# Breakthrough campaign: compiler-trace-guided matching of the wall functions

Scoped 2026-08-26, at 984/1464 functions (67.21%). This document is the plan for
getting past the plateau where near-miss source guessing stops yielding. It uses
the decomp workbench's compiler-instrumentation / trace subsystem
(`~/Desktop/dev/n64-decomp-workbench`, README + `docs/`), which no lane has used
yet. No ROM-derived content is tracked here.

## 1. The wall, precisely — it is two different populations

The 208 remaining `#ifdef NON_MATCHING` functions are **not one problem**. By the
workbench/objdiff ranking (`config/nonmatching-ranking.us.json`):

| Population | Count | What it is | The right tool |
|---|---:|---|---|
| **A — allocator-walled** | ~6 (of 16 that are ≤10 words) | Instruction multiset, frame, and schedule already exact; residual is IDO's register/stack-slot assignment (e.g. a temp spills to `sp+0x40` vs target `sp+0x48`; a `t3↔a3` web). Opus + luna-max both confirmed these are **not steerable by guessing at C**. | **This campaign** (traces) |
| **B — structurally incomplete** | ~192 (122 `size-mismatch`, 152 at 40w+) | The candidate C is the wrong size/shape — it was never fully reverse-engineered. Not an allocator problem at all. | Ordinary structural decomp (separate track, §5) |

The near-miss fleet has been grinding Population A and yielding ~0 because guessing
cannot move an allocator decision. Population B has barely been touched because the
closest-first ranking never surfaced it. **The workbench trace subsystem is the
answer for A; B needs decomp effort, not tracing.**

## 2. The technique (proven, not speculative)

The workbench's own final-function campaigns (Hartley, Titania, Aquas, and
`cef4c`/`lbParticleUpdateStruct` — 1,868 instructions, 99.91% → `words=0`) reached
exact matches on register-allocation walls identical to ours. The loop
(`docs/final-function-campaigns.md`):

1. Project build is the final oracle (we have this: `gmake verify`).
2. Object comparison classifies the residual **before** any C change (`diagnose`).
3. Hold instruction shape steady while investigating allocation-only residuals.
4. **Use compiler traces and force probes to prove which pass and which decision
   owns the residual**, then return to a source-level explanation.
5. Keep every source/object pair and its metrics; a scalar score is not a diagnosis.

The insight that breaks the "uncontrollable from C" verdict: when the residual is
one allocator decision, you stop guessing and instead ask *which pass owns this
word* — answerable at the pass boundaries (ugen → as0 → as1), not in C. Once the
owning decision is isolated and force-proven causal, `trace-origin-probe` + the
field guide name the specific source edit that reproduces it.

## 3. What we have vs. what we need (tiered prerequisites)

- **Have:** `tools/ido/cc` is an `ido-static-recomp` build (contains `libc_impl.c`,
  `recomp.cpp`) with the full phase set (`cfe copt uopt ugen as0 as1`); the build
  oracle; the workbench installed (`.venv/bin/decomp-workbench`); each target's
  `asm/nonmatchings/**/<fn>.s`.
- **Tier 1 — available now, no new prerequisite.** `capture make` wraps our
  existing binaries so a normal build retains every pass boundary on disk. Enables
  `probe-lines`, `trace-fifo`, `replay-as1`, and Ucode stream-surgery + replay.
  Covers: `schedule-mismatch`, register FIFO free-list reconstruction, and
  pass-ownership isolation.
- **Tier 2 — bounded prerequisite.** The deep `uopt` register-coloring profiles
  (`instrument-uopt-globalcolor`, `instrument-uopt-alias`, `CDX_FORCE`,
  `trace-webs`, `trace-origin-probe`) patch the *generated* `uopt.c`/`ugen.c` from
  `ido-static-recomp` (github.com/decompals/ido-static-recomp) for our pinned IDO
  5.3 revision — which we do **not** currently have on disk (only the binaries).
  `ido-support.md` confirms IDO 5.3 deep profiles are supported "pinned revision
  only". Prereq = check out ido-static-recomp, regenerate the 5.3 `uopt.c`/`ugen.c`,
  and match the profile's expected source hash (or use the generic `instrument-ugen`
  anchors, "conditional" for 5.3).

## 4. Execution phases

### Phase 0 — prerequisite validation (cheap, ~1 session)
- `decomp-workbench capture make tools/ido .decomp-workbench/capture`, point one TU's
  compiler root at it, build **one** file, confirm captures land and bytes are
  unchanged (fidelity test). This unlocks all of Tier 1.
- `decomp-workbench fingerprint-toolchain` on our binaries → record the exact
  lineage; determine the ido-static-recomp revision for Tier 2.
- Attempt the ido-static-recomp generated-source regen in a throwaway dir
  (`~/Desktop/dev/ido-static-recomp`, **not** tracked); validate against the pinned
  profile hash. Go/no-go for Tier 2.

### Phase 1 — Tier 1 on Population A (immediate, low machine cost)
Per target function (the ~6 allocator/schedule near-exact ones — start with
`func_800508D4` schedule-only, then `func_80055970` `func_8004D40C` `setLanguage`
`frontSet2PlayerSplit`):
1. `tools/wb_compare.sh <fn>` + `decomp-workbench diagnose …` → confirm the class.
2. `schedule-mismatch` → `decomp-workbench probe-lines` (one variant, free): does
   physical line assignment drive the schedule? If yes the fix is a preprocessing
   change, not a respelling — an entire mechanism ruled in/out for free.
3. Register/FIFO residual → capture the ugen stream, `decomp-workbench trace-fifo …
   --registers t0,…,t9 --fail-on-violation` to reconstruct the free list and find
   the source order that yields the target allocation.
4. If a single word remains and no C family moves it → Ucode stream-surgery +
   `replay-as1` to prove downstream sufficiency (which pass owns the word), then
   hunt the source spelling with a *known destination*.
5. Exact → drop the wrapper, `gmake verify` + `wb_compare.sh --rom`, commit.

### Phase 2 — Tier 2 register-coloring (gated on Phase 0 go)
For the register-web residuals that Tier 1 can't reach:
1. `instrument-uopt-globalcolor <generated uopt.c> uopt.traced.c`; build the traced cc.
2. `CDX_LOG=1 CDX_DETAIL_WEB=<n>` to read the coloring decision; `CDX_FORCE=p1:w<n>=c<reg>`
   to test "if this web took the target register, is the object exact?" — isolates cause.
3. `trace-webs` / `trace-origin-probe` to classify how one controlled source edit
   moves the web, converting the forced proof into a legal source route.
4. Verify + commit as in Phase 1.

### Phase 3 — generalize
Write the measured allocator/scheduler laws we discover into a reusable playbook
(mirroring the workbench field guide) so later lanes apply them without re-deriving,
and re-run over Population A as more functions enter it.

## 5. Population B — the separate, larger track (not this campaign)
The ~192 structurally-incomplete functions need genuine reverse-engineering (correct
types, control flow, call structure), guided by `diagnose` but driven by decomp
effort — the luna/Opus model ladder doing *structural* work, not near-miss nudging.
This is the bulk of the remaining 33% and the real path to 100%, but it is a
different, longer campaign; keep it off the trace track. The workbench's role there
is `diagnose` (classify structure-mismatch), `campaign target.o candidates/*.c`
(rank many structural variants with caching/ledger), and `public-match-check`
(does a verified public solution exist — signal only; adoption still bound by the
CLEANROOM provenance policy).

## 6. Machine-safety & ROI
- The trace campaign is **serial and deep** (one function at a time: capture,
  analyze, probe), far gentler on the machine than the parallel fleet — the heavy
  work is analysis, not concurrent `-j` builds. It fits the resource-safe limits
  already in force (`-j4`, load-gated).
- ROI: Tier 1 is cheap and immediate and should unlock the schedule/FIFO functions;
  Tier 2 has a real one-time setup cost (regen generated source) for the
  register-coloring functions. Both are proven to reach `words=0`. The by-product —
  a measured IDO 5.3 allocator playbook — pays forward across the backlog.
- **Recommendation:** execute Phase 0 + Phase 1 first (low risk, low cost, immediate
  signal). Gate Phase 2 on Phase 0's go/no-go and Phase 1's results. Run Population B
  as an independent structural-decomp track when machine and model budget allow.

## Phase 0 results (2026-08-26) — validated

- `decomp-workbench capture make tools/ido <dest> --link` wraps ugen/as0/as1; a
  build with `CC=<dest>/toolchain/cc` routes the phases through the wrappers and
  leaves `captures/<ts>-ugen` and `captures/<ts>-as1` run directories.
- **Fidelity proven:** a capture build of `src/main/anim.c` is byte-identical to
  the stock build in `.text`, `.data`, and `.rodata`; only `.mdebug` differs
  (embedded compile paths, which never reach the linked ROM). The capture is
  codegen-faithful → Tier 1 is trustworthy.
- Target objects already exist under `build/wb/<fn>.target.o` (via
  `tools/wb_compare.sh <fn>`), e.g. `func_800508D4`, `func_80055970`,
  `func_8004D40C`.

### Two wrinkles the next operator must know
1. **`wb_compare.sh <fn>` (non-rom) is meaningless on a still-wrapped function.**
   In a normal build the object holds the `#else GLOBAL_ASM` *assembly* (the
   target's own `.s`), so the compare is the target against itself → a false
   `words=0`. To analyze the *candidate*, the `#ifdef NON_MATCHING` C body must
   be the compiled definition.
2. **`gmake <obj> NON_MATCHING=1` changes the object path** ("No rule to make
   target …"). Do **not** try to capture via `NON_MATCHING=1` on a single object.

### Phase 1 per-function loop (the working recipe)
In the `mickey-lane-opus-hard` worktree, per target function:
1. Temporarily **unwrap** the function: delete its `#ifdef NON_MATCHING` /
   `#else` / `#pragma GLOBAL_ASM(...)` / `#endif` lines so the C body is the sole
   definition. (Revert with `git checkout -- <file>` when done.)
2. `rm build/src/<tu>.c.o; gmake build/src/<tu>.c.o CC=<cap>/toolchain/cc` →
   builds the candidate through the wrapped phases, capturing its ugen/as1 streams.
3. Read the streams: `decomp-workbench trace-summary <cap>/captures/<ts>-ugen/…`,
   `trace-fifo … --registers t0,…,t9 --fail-on-violation` (register free list),
   and for `schedule-mismatch`, `probe-lines` (compile wrapper = the same
   capture cc; target = `build/wb/<fn>.target.o`).
4. The trace explains the candidate's allocation; compare against the target's
   register/stack usage (`build/wb/<fn>.target.objdump`) to find the source
   change that flips it. Apply, rebuild, re-diff.
5. Exact → re-wrap-remove permanently, `gmake verify` + `wb_compare.sh --rom`,
   commit on `lane/opus-hard`, integrate via the runner.

**Status:** foundation validated and Phase 1 recipe established; the actual
per-function allocator investigation (multi-stage, per the cef4c precedent) is
the labor that remains.

## Phase 1 first results (2026-08-26)

**`func_800508D4` — precisely diagnosed via the capture+diagnose loop.**
`verdict=schedule-mismatch words=4 signature=prefix-exact@15`, register lanes
**identical 28/28** — a *pure schedule* difference: the candidate materialises
`unsignedScale = D_80083FA8` (the only global-load among four float scales) at
instruction 15; the target places it at instruction 19 (four later). Reordering
the declaration to last did **not** move it (same object hash) — confirming the
doc's point that this class is not respellable and needs the line-assignment /
trace route (`playbook=g0-schedule-probe`). Build flags for the TU:
`-O2 -mips2 -Wo,-loopunroll,0`.

**Bug found: the permuter was compiling at the wrong ISA.**
`tools/permute.sh` imports via decomp-permuter's `import.py`, which inferred a
default `-O2 -mips1 -32` and dropped the per-file `-Wo,-loopunroll,0`. The real
build is `-mips2`. IDO emits materially different code at `-mips1` vs `-mips2`,
so **every prior permuter run (including the "0 hits" farm and the
`func_80055970` experiment) searched the wrong instruction space and its results
are invalid.** Fix: after `import.py` builds the scratch, rewrite
`scratch/compile.sh` with the real per-file flags (extract from
`gmake -n build/src/<tu>.c.o`). A corrected-flags re-run is under way; if it
converges, the cheap permuter lever is revived for the whole near-exact set and
Tier 1/2 tracing is only needed for what it still can't reach.

**Next actions (priority order):**
1. Land the `permute.sh` per-file-flag fix (high ROI, cheap) and re-baseline the
   near-exact set with a valid permuter.
2. Stand up `probe-lines` on an isolated single-function compile (the permuter
   scratch's `compile.sh`, flag-corrected) for the schedule class.
3. Tier 2 prerequisite (ido-static-recomp generated `uopt.c`) only for residuals
   neither the permuter nor Tier 1 reaches.

## Phase 1 execution results (2026-08-26) — DECISIVE: these need Tier 2

A luna/max trace lane worked all five Population-A functions through the Tier 1
loop (`capture make` streams + `replay-ugen`/as1 stream surgery). Outcome: **0
matches, but a proof for each that the residual is ugen-owned and unreachable
from C, and a hard limit on Tier 1.**

| Function | Class | Proven pass owner (via stream surgery + replay) |
|---|---|---|
| `func_800508D4` | schedule (4w) | moving the one global-scale Binasm record after the two immediate-scale records makes stock `as1` exact → **ugen record scheduling**. probe-lines negative (thr 400, 60), acpp negative. |
| `func_80055970` | operand/stack (4w) | changing the four stack operands `sp+0x40`→`sp+0x48` in Binasm makes `as1` exact → **ugen stack-home selection**. All type/qual/lifetime/order/pad probes inert. |
| `func_8004D40C` | register web (5w) | target as0 has an explicit `or t2,a3,zero` after the row-24 branch that the candidate ugen never emits → **ugen copy/register-web + branch scheduling**. |
| `setLanguage` | uopt pool (5w) | base/value `a1`/`a0` crossing; pool diverges at slot 18 → **uopt pool-position** (plus a separate `jtbl_80082734` rodata-ownership blocker). |
| `frontSet2PlayerSplit` | ugen FIFO (3w) | temp lane diverges at slot 0; target regs are ring-only → **ugen web-existence/FIFO** decision. |

**The Tier-1 limit, confirmed empirically:** `capture make` retains *raw binary*
Ucode/Binasm streams. `trace-summary`/`trace-fifo`/`trace-webs` reject them
(UTF-8 decode errors) — those parsers need the **textual** `CODEX-*`/`DKWB-*`
traces that only the **instrumented** ugen/uopt (Tier 2) emits. So Tier 1 can
*prove which pass owns a residual* (via replay/surgery) but cannot *find the
source lever*. **Every remaining Population-A function is ugen/uopt-pass-owned
and requires Tier 2 instrumentation.** Binasm surgery is disposable proof only —
never a match.

**This makes Tier 2 the required path, not an option.** Prerequisite (unchanged):
the `ido-static-recomp` generated `ugen.c`/`uopt.c` for our pinned IDO 5.3, then
`instrument-ugen` / `instrument-uopt-globalcolor` to emit textual traces +
`CDX_FORCE` to prove and then source-route each decision. Without it, these ~6
functions are permanent NON_MATCHING.

## Tier 2 SET UP (2026-08-26) — operational
ido-static-recomp cloned @9c242adc; generated build/5.3/uopt.c SHA matches the pin b0058f15; instrument-uopt (globalcolor+alias) + instrument-ugen applied and rebuilt; instrumented toolchain drop-in at ~/Desktop/dev/ido-instrumented (Mickey phases + instrumented uopt/ugen), fidelity proven (.text identical with traces off). Traces on emit 1312 [CDX] coloring records + ~1M DKWB ugen events. CDX_FORCE proves causal webs. A dedicated Fable agent now works Population A with this; luna works Population B.

## Durable allocator receipts

Use [`allocator-trace-receipts.md`](allocator-trace-receipts.md) for new Tier-2
captures. `tools/allocator_trace_receipt.py --map-only` maps a symbol to its
run-local optimizer procedure from the named `Uent` records in uopt's exact
captured Ucode input and the complete `procindex` table. The full command then
gates the traced object against stock output and emits a compact hash/count
receipt. It fails closed on ambiguous mapping, incomplete detail, fidelity
drift, or missing attempt accounting. The current ugen
producer still lacks per-compiled-procedure markers, so mixed-TU temporary/FP
rows remain outside the automatic receipt rather than being misattributed.
