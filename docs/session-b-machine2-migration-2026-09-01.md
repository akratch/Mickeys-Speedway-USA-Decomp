# Session B machine migration handoff — 2026-09-01

This private-only handoff transfers the remaining Machine 2 evidence before
the local Mickey workspace is removed. It is based on private canonical
`70f6adc5c81ffc2b0773a791836b46c589ae6ac8` and must never be mirrored to the
public repository.

## Resume contract

1. Fetch private `origin`, fast-forward `campaign/unchain`, and prefer a newer
   canonical commit if one exists.
2. Session B owns even overlays, `src/libultra/**`, and new branches named
   `lane/burn-b-night-*`. It never writes or pushes `campaign/unchain`.
3. Run `tools/lane_status.py --symbol <symbol>` immediately before assignment
   and proceed only on a zero-exit `base-only` result.
4. Do not repush old `lane/burn-b-*` branches. Their remote refs were removed
   after canonical ancestry, patch-equivalence, or supersession was proved.
5. Build only in isolated lanes. Exact owned bytes, exact relocation identity,
   linked-owner proof, and full-ROM proof remain mandatory.

## Canonical queue state at transfer

The filtered Session B queue is empty at the base above:

- high-confidence: `ready=0`, 18 rows already integrated/exhausted;
- expected-yield: `ready=0`, 394 integrated/exhausted and three stale-ledger
  rows, all under `src/main/**`;
- broad-structural: `ready=0`, 376 integrated/exhausted and the same three
  resident/main stale-ledger rows.

The three stale rows are `func_800573C8`, `func_8001FC50`, and
`modLoadModel`; they are Session A work. A temporary empty Session B queue is
a monitoring state. Regenerate all three queues after every canonical fetch.

The following previously active Session B fronts are closed at this base:

- `overlay14ResetMode`, `overlay62Update`,
  `func_overlay_056_F00001A0_18A2F18`,
  `func_overlay_008_F0001294_185EFEC`, `overlay20RemoveEntry`,
  `func_overlay_022_F0000D30_1878E38`, and `overlay14CreateValue` have current
  canonical source/handoff dispositions.
- `func_overlay_014_F0000000_186F8D8` now has a committed definition without
  its fallback and must be treated as integrated, not reopened.
- `overlay34InitStorage`, `func_overlay_036_F0000818_1883CD0`,
  `func_overlay_026_F0000B18_187AF10`,
  `func_overlay_022_F0000000_1878108`, `overlay84InitializeAndUpdate`, and
  `overlay94UpdateController` have stale reopen pins. Do not mutate them
  without a new canonical authorization grounded in the current source and
  structured handoff.

## Proven results already consumed by canonical

### Overlay 14 reset

`overlay14ResetMode` reached 56/56 normalized words with frame `0x30`. A
diagnostic full link reproduced the 32 MiB target ROM exactly. The required
identity closure was four natural data aliases (`D_F8`, `D_D8`, `D_DC`, and
`D_E0`) while preserving `overlay14ResetReleaseOwnerReloc` as the zero-addend
per-module call placeholder. The runtime relocation table separately
authenticated `overlay14ReleaseOwner` at overlay-local `+0x1B54`. The current
canonical disposition supersedes the local promotion packet.

### Overlay 14 initializer

`func_overlay_014_F0000000_186F8D8` was remeasured at 75/79 words, frame
`0x28`, with four saved-register differences and 48 exact relocation sites.
Runtime identity proved the slot/count/end address-web birth order. The only
new source form was a nonvolatile count pointer with that order. Canonical now
contains a definition without the fallback, so this work is finished.

### Overlay 62 identity topology

`overlay62Update` reproduced 287/294 normalized words, frame `0x88`, and all
71 relocation sites. The retained broad aliases conflated overlay-local
`+0x5B4` data with `+0x5F4/+0x5FC/+0x600/+0x604/+0x608` BSS identities. That
explained the sole literal low-addend mismatch and supplied a genuine
web-existence mechanism for the seven register residuals. Canonical has a
current plateau; do not repeat scheduler, comma, flag, or permutation sweeps.

## Reusable bounded mechanisms

These are evidence records, not active authorizations. Reopen only after
`lane_status` becomes base-only on a fresh canonical pin.

- Overlay 56 direct reconstruction: preserve the shipped runtime identities
  instead of m2c proxy aliases. Two loop tails are naturally post-decrement
  conditions; the retained pre-decrement spelling skips index zero and is
  invalid for count one. Test identity-only and combined forms separately,
  retain strict gains only, and do not run a generic flag/permutation sweep.
- Overlay 8 main update: target 1,259 words/frame `0xB0`; retained candidate
  1,256 words/frame `0x180`. One target-authenticated post-decrement repair is
  valid. Separately, the excessive frame is concentrated in early declaration
  order: the two call-crossing float homes are adjacent and reversed in retail,
  while they are separated by seven declarations in the candidate. A future
  authorization should permit one declaration-only topology before at most
  three refinements.
- Overlay 20 removal: the permitted DKR
  `lensflare_override_remove` donor proves an indexed global-array compaction
  lowering that creates the retail base/end/cursor temp-ring family. Current
  V0 was 39/53 words, frameless, with ten authenticated identities. Only one
  indexed-loop form is justified.
- Overlay 22 removal: the same permitted donor proves an indexed compaction
  form for `func_overlay_022_F0000D30_1878E38`. Current V0 was 48/91 words,
  frame `0x28`, and all 12 relocation tuples authenticated. Only one form
  reusing the existing index is justified.
- `overlay14CreateValue`: runtime identity proves its selector call is resident
  `frontGetLanguage`. Substituting that identity preserves geometry and closes
  all 15 stable relocation identities. Use it only to refresh diagnosis; prior
  flag, line-tie, permutation, pointer, ordering, and carrier families remain
  closed.

An exhaustive scan of 94 live even-overlay candidates against the locally
permitted DKR and Banjo-Kazooie builds found no overlooked whole-function
donor. Only the Overlay 20 donor and the now-consumed Overlay 14 initializer
relationship exceeded the useful threshold. The earlier Overlay 96 lens-flare
proposal was falsified by runtime identities and is explicitly withdrawn.

## Shared compiler-evidence blocker

The remaining allocator/frame authorizations asked for fields the current
workbench producer does not emit:

- final caller-save-slot or stack-home assignment for overlays 22, 26, 34, 36,
  and 84;
- direct producer-to-final-home identity;
- direct source-semantic attribution for `overlay94UpdateController`.

Stock global-color traces have no calibrated `virtual_offset` or
`final_offset`; UGEN free-list events do not carry cross-pass operand identity;
and detailed allocator records do not carry a trustworthy source semantic.
Line number, table index, raw word, physical register, or ordinal must not be
used as a substitute.

Concrete baselines preserved in canonical handoffs include:

- `overlay34InitStorage`: exact 50-word/frame-`0x30` geometry; four stack
  operands use `sp+0x18` instead of retail `sp+0x1C`.
- `overlay94UpdateController`: 268/275 normalized, frame `0x70`, 36/36 runtime
  identities exact; fidelity-gated proc 0 captured 22 integer and 10 FP
  decisions, but all 32 webs were run-local and source-unattributed.

The next machine should wait for either a hash-pinned, trace-off-fidelity-proven
producer for these fields or a new mechanism measurable by current tools. It
must not repeat declaration, carrier, flag, or permutation sweeps merely to
keep a worker busy.

## Queue tooling note

An earlier `lane_status.guarded_fallback()` regex incorrectly terminated an
outer `NON_MATCHING` block at a nested `#else`. The nesting-aware implementation
already exists in `finalize_plateau.guarded_candidates()`. The bug affected
three even-overlay targets during this session. Canonical later promoted the
Overlay 14 initializer, but a future recurrence should be fixed by reusing the
nesting-aware parser and adding a nested-guard regression test—not by bypassing
the fail-closed verdict.

## Local-only state deliberately represented here

No uncommitted game source remained at migration time. Two clean local lane
tips for Overlay 34 were patch-equivalent to canonical, and every other old
Agent B lane tip was a canonical ancestor or superseded. They must not be
repushed.

The primary `tools/asm-processor` checkout had one unexpected text
modification, duplicated byte-for-byte in several old worktrees. It removed
the 31-line definitions of `endlabel`, `alabel`, `dlabel`, `enddlabel`, and
`jlabel` from `prelude.inc`; the resulting file SHA-256 was
`6d28824d14d60b66b2e3a3e7dc32274c710c5e3b285d8d47cbe9314f70f8b94f`.
The exact patch is:

```diff
@@ -8,37 +8,6 @@
-# The end of a function symbol.
-.macro endlabel label
-    .size \label, . - \label
-.endm
-
-# An alternative entry to a function.
-.macro alabel label, visibility=global
-    .\visibility \label
-    .type \label, @function
-    \label:
-.endm
-
-
-# A data symbol.
-.macro dlabel label, visibility=global
-    .\visibility \label
-    \label:
-.endm
-
-# End of a data symbol.
-.macro enddlabel label
-    .size \label, . - \label
-.endm
-
-
-# A label referenced by a jumptable.
-.macro jlabel label
-    \label:
-.endm
-
-
 # Label to signal the symbol haven't been matched yet.
```

This was preserved as evidence only. No submodule commit or parent gitlink was
created because the canonical pins must not be changed. Several old lanes also
had `libc.so.1` or `libC.so.1` copied over its sibling; in each affected
checkout both files had the same hash. Those proprietary compiler binaries are
not source work and were intentionally not committed.

One old Overlay 40 lane had an untracked `.decomp-workbench.toml` containing
only the normal object-mode configuration for `overlay40UpdateEntries` and a
five-minute, one-job permuter budget. It contained no unique source or result.

## Validation and artifact policy

The handoff commits prose only. ROMs, target/candidate objects, disassembly,
trace logs, workbench caches, compiler binaries, and extracted assets remain
untracked and are intentionally excluded under the clean-room policy. The
metrics above are sufficient to reproduce the work from canonical source,
structured handoffs, permitted donor builds, and a legally supplied baserom.

Before taking new work, run `gmake check-docs`, `gmake cleanroom`, the relevant
queue tests, and the normal configured baseline/preflight. Any exact promotion
still requires the full private canonical gate set and Session A integration.
