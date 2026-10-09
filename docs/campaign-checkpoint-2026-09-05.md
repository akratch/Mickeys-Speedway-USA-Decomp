# Safe-stop checkpoint: September 5, 2026

Private operating material. The user requested a pause for stocktaking; do
not treat the previous broad execution objective as an instruction to resume
matching or publication automatically. No new assignments, integrations, or
pushes were started for this checkpoint.

## Landed state

Private canonical `campaign/unchain` is clean at `cebb5d66`. Its latest
integration passed ROM verification, documentation, clean-room and scoreboard
gates. Transaction staging (`5cea9e0f`) and ranking freshness through
`d3786a71` are integrated. The latter is superseded by a pending fidelity fix:
do not use its cached scores as configured full-TU proof.

Workbench `main` is clean at `45581d1`, with geometry-aware candidate ranking
integrated locally and not pushed. Public `master` remains unchanged at
`2e5b6d932`; the newer private exact matches have not been released.

Exact private commits since the earlier `a8b4c3f0` checkpoint are
`21d0325a` (`func_80035E88`), `a6376bae` (`texDPInit`),
`d5247b25` (`texFreeSprite`), `2eb99669` (`texInitTextures`),
`92e29ad5` (`func_80035ADC`), and `4ab997cc` (`func_80002188`).
Recompute progress with `.venv/bin/python tools/progress.py --version us
--csv` against the current linked ELF; do not carry a historical percentage
forward as a fresh measurement.

## Frozen handoffs, not yet integrated

| Owner branch | Commit | Scope and qualification |
| --- | --- | --- |
| `lane/improve-probe-census` | `b2827aca` | Raw assembly ownership census; padding and scaffold surface earn no matching credit |
| `lane/improve-probe-census` | `ac6554dd` | Preserve reviewed generated outputs when incoming changes delete their tracked paths |
| `lane/improve-probe-census` | `e03bf160` | Restore overlay Make rule placement and missing-ledger test isolation |
| `lane/improve-probe-census` | `a95687d5` | Compact diagnostic probe and explicit flag-lattice coverage; generic flag context remains a limitation |
| `lane/diagnose-objects-close` | `fa90a376` | Authenticate local resident PC16 branch relocations; no new match |
| `lane/improve-transaction-sweep` | `71582411` | Durable bounded-search receipts and full configured compiler argument extraction |
| `lane/fix-ranking-fidelity` | `08b1bea2` | Compile each configured full TU once, preserve source/line/header context, and extract symbols without shared scratch races |
| Workbench `wb-pc16-diagnosis` | `028b25c8` | ELF-authenticated local branch normalization for diagnosis only; raw exactness remains unchanged |

The probe/census lane also retains these checkpoint and strategy notes.
Its full tooling suite, documentation and clean-room gates passed; the
unchanged-command Make relocation also passed ROM verification.

The full-TU ranking handoff passed its relevant tests and repository gates.
Its broader suite still encounters the missing-ledger fixture failures fixed
separately in the probe/census lane. Integrate those fixes before treating a
combined suite result as established. The PC16 workbench handoff passed its
full suite and static checks but still needs review and real Mickey replay.

## Explicitly incomplete work

The transaction-sweep lane retains modified `tools/permute_batch.py` and new
`tools/promotion_transaction.py` after its committed receipt unit. These are
unfinished promotion journaling, rollback and deadline changes. Syntax and
existing receipt tests pass, but cancellation propagation and injected
promotion-failure tests are missing. Do not integrate or run this WIP with
`--apply`. Its owner froze the lane and saved a separate ignored checkpoint
under `build/checkpoints/promotion-wip-71582411-20260905/`, containing
`tracked.patch`, the new helper and a `MANIFEST.md` with base and hashes.
Request a handoff before touching its worktree.

Other known gaps:

- `build_recipe_for()` still changes source timestamps with `os.utime`.
  Replace this with a non-mutating Make dry-run forcing mechanism.
- The standalone flag lattice still uses generic define/include arguments.
  Consume the complete configured recipe before claiming TU fidelity there.
- A zero masked score for `func_8000AA38` did not prove relocation identity;
  a guard-lift trial also exposed retained switch-table references to local
  labels. Restore ownership of that surface before retrying promotion.
- The compact probe's already-promoted route has synthetic coverage but its
  attempted real replay failed symbol/preflight resolution. Keep that gap
  explicit; ordinary nonexact and oversized real probes were exercised.

## Recommended restart order

Review and integrate the committed tooling handoffs, then finish promotion
rollback/cancellation tests and flag recipe fidelity. Validate the combined
tree serially within a worktree: build-producing gates must not overlap its
verification build. Refresh ranking only after tool hashes settle, and prove
that an unchanged refresh performs no compilation.

Run a small acceptance batch spanning resident, overlay, near-exact and
structural candidates. Record time, cache reuse, error classification and
linked exact closures. Expand disjoint TU-owned lanes only after that batch
shows that the new tools remove repeated work. Keep manual retries gated by
new mechanism evidence and bounded sweeps resumable by actual search inputs.

Measure ROM-exact bytes per elapsed campaign time, with investigation,
compilation, verification and integration costs reported separately. Lower
diff scores, more carved source, or additional tooling commits are not match
progress. Prioritize recurring failure mechanisms over further dashboards.

Release eligible exact commits only as a separate, reviewed public-safe
batch with the public mirror's own gates and scans, after the user resumes
execution. See `campaign-efficiency.md` for the ongoing acceptance criteria.
