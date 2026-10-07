# 2026-10-07 noon campaign handoff

The owner set a noon Europe/Berlin stopping time. Resume from
`campaign/unchain`, preserving the owner's unrelated `docs/NEXT_CAMPAIGN.md`
edit. Do not publish the entire campaign branch: its candidate and historical
report commits are not the curated public release.

## Measured outcome

The verified scoreboard is 837,852 of 943,640 executable bytes (88.79%) and
1,323 of 1,460 functions (90.62%). The requested 90% overall threshold was
not reached. Another 11,428 executable bytes are needed to exceed it strictly.
Function percentage and whole-program byte percentage are different measures.

The sprint's newly proved font match is `func_8004B1DC`, 2,224 bytes,
campaign commit `87ff82628`. The previously proved overlay 27 renderer,
1,016 bytes, was also captured publicly. These are separate provenance claims;
the latter is not a newly solved function from this sprint. No subsequent
candidate described below is an exact match.

## Preserved source improvements

| Target | Integrated commit | Before to retained | Remaining evidence gap |
|---|---|---|---|
| `func_8005AF14` | `7ed9da4c3` | 377 to 339 masked words; size excess 12 to 8 bytes; normalized distance 361 to 208; aligned relocation identities 9 to 15 of 27 | Frame exceeds target by 24 bytes; two surplus instructions; matrix/camera pointer lifetimes; exact relocation and linked candidate proof |
| `func_overlay_045_F0001158_188D5B0` | `6ed5080e5` | 402 to 395 masked words at unchanged 2,696 bytes; normalized distance 342 to 335 | Stack homes and call-spanning screen values; 24 relocation metadata differences; linked candidate proof |

The model packet corrected trig argument widths, float matrix arguments and
the target's multiply-hazard scheduling. Updating the existing short angle
in place reproduces its converted value's lifetime across calls. The flag is
restricted to this one TU; all ten neighboring functions retain their bytes
and relative relocations. The target's stock configured object reproduces the
preserved candidate. The dedicated flag-impact tool refused its missing sized
resident symbol row; a complete independent TU census supplies the collateral
proof, not a claimed successful tool report.

The overlay 45 packet transfers two byte-pointer arena sums from the newly
matched font renderer. Each site improves independently; the pair is best.
No command, offset width, declaration, flag or inert expression changes.
The local address-region improvement exposes no further supported lever.

Both packets passed actual-input context checks, untouched compiler fidelity,
full fallback ROM verification, documentation, cleanroom and all 99 tooling
test files. Fallback ROM identity proves preservation of the canonical game;
it does not prove either candidate C matches the ROM. Each shard contains
the attempts, limitations and stop condition.

## Tooling and validation

The public tooling batch ending at `8618d6d40` includes guarded-include mode
handling (`b4e2bf71a`), authenticated friendly overlay callees (`a3bc43e83`),
unused token-pasting preprocessing (`daadcb9f9`) and configured recipe
propagation for TU flag-impact checks (`8618d6d40`). None changes matching credit.

The final scanner fix is integrated as `5c2b6cd0a` and curated separately as
`13f374626`. It removes only ROM-authenticated, reviewed nonexecutable ranges
from target opcode sequences. The two affected resident counts change from
36 to 34 and 121 to 118 words; executed and unreviewed nops stay present,
and overlays are distinguished by ROM offsets rather than synthetic VMA.
Only the scanner and its tests change; this is not a new matching result.

Canonical validation after both source integrations:

```sh
tools/with_verify_lock.sh gmake verify
gmake check-scoreboard
```

Both pass. The rebuilt ROM has the expected US hash and the scoreboard agrees
with the built ELF. Each owning lane also passed `gmake check-docs cleanroom`
and `gmake check-tooling`. The public release uses `tools/land.sh --release-ref`
so its own merged tree is ROM-verified and checked before publication.

The final committed-lane pending-claim scan returned no pending claims. The
Grok branches inspected are older October 3 results, not fresh submissions;
the last checked open-PR list was empty. Treat any later PR as input requiring
normal independent reproduction and promotion proof.

## Private evidence and ownership

- Model lane `codex-model-angle-width-20261007`, frozen commit `89935d7e4`:
  ignored `build/angle-width/` preserves baseline, width controls, exact-size
  intermediate, retained producer form, inferior reflection control and proofs.
- Overlay 45 lane `codex-o045-arena-domain-20261007`, frozen commit `ab70ce5cd`:
  ignored `build/o045-arena/` preserves baseline, combined and independent-site
  controls, captures, comparisons, fidelity and fallback verification.
- Root wake lane `codex-wake-conversion-20261007` retains ignored
  `build/wake-conversion/`; its unsigned conversion and induction-bound controls
  are closed and canonical source is restored.
- Overlay 56 signed-initial-conversion and duplicate-correction controls are
  preserved in their owner's frozen lane; the report is integrated as
  `248efd10b`. No source was adopted.

Frozen snapshots are handoffs, not authorization to edit another worker's
worktree. Create a new lane and request any additional private artifacts from
their owner. Committed refs are available for read-only duplicate detection.

## Resume efficiently

1. Read the target's current shard and run `tools/lane_status.py --symbol`.
   Source/report commits consume the old reopen pins. Do not replay a closed
   control or treat an exhausted verdict as permission to start a sweep.
2. For the model target, start with the retained producer-faithful candidate,
   not the older exact-size intermediate. Authenticate compiler-created homes
   and matrix-pointer lifetimes from its actual compiler capture before any
   new declaration experiment. The old home/pointer bundle lacks enough exact
   source detail for a faithful replay; do not invent its missing loop spelling.
3. For overlay 45, preserve the two typed arena sums. Further work needs a
   proved source lifetime for the displaced display-list and screen-value homes,
   not another operand reversal or pointer-domain repetition.
4. The fresh sibling scan is saved privately as `build/noon-siblings.json`.
   Its useful new font-to-overlay-45 lead has been consumed. Strong frontend
   and collision siblings were already covered by their committed reports.
5. The model width fix does not transfer to shadows: those remaining trig
   calls already pass the target's direct signed-halfword loads. The model's
   third matrix call and raw/scaled angle normalization also already match
   their target forms. Overlay 20's terminal return/carrier audit found no
   new source discrepancy. These were read-only exclusions, not failed builds.

Normal promotion still requires untouched stock C output, exact owned bytes
and relocation identities, real linked placement, full ROM and the repository
gates. Publish through `tools/land.sh --release-ref` using a reviewed curated
ref integrated into the campaign. Keep all ROM-derived evidence ignored.
