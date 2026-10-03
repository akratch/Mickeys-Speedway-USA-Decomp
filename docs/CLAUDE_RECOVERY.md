# Claude recovery and continuation plan, 2026-10-03

This audit starts at integration commit `d39cc6e60`. The existing uncommitted
`docs/NEXT_CAMPAIGN.md` update in the primary checkout is preserved separately.
Committed lane objects were inspected through Git; other workers' worktrees,
indexes, processes, and uncommitted artifacts were not inspected (ADR 0011).
Recovery of those artifacts requires an ownership handoff, not a fleet-wide
status or file sweep. No public push or landing is part of this recovery.

## Verified starting point

`gmake progress` reports 826,928 / 943,640 resolved executable bytes (87.63%),
1,319 / 1,460 matched functions, and 116,712 unresolved bytes. The integrated
ROM has SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`. Verification,
clean-room, documentation, and scoreboard gates pass on the integration tree.

Resolved includes 17,184 bytes of intentionally retained handwritten assembly.
Matched C alone is 809,744 bytes (85.81%). The campaign's existing endpoint
retains that assembly; a requirement for every executable byte to be C would
need a separate decision about those functions.

The ranking covers 66 identities / 105,368 bytes. Progress reports 106,804
NON_MATCHING bytes and 9,908 GLOBAL_ASM bytes, leaving 11,344 unmatched bytes
outside this ranking. Shared include bodies, aliases and bodies without a C
candidate must be tracked in the ready queue as well.

## Committed recovery inventory

| Lane | Tip | Integration state and retained result |
|---|---|---|
| z-anim | `6c43dc616` | Integrated; func_80051364 retains 7 masked differences, size delta 0 |
| z-fxchar | `b972ee324` | Integrated; func_8001E5C4 retains 157 masked differences, size delta 0 |
| z-o008 | `dcf5c6474` | Integrated; F00042A8 retains 126 masked differences, size delta 0 |
| z-ovl1 | `ba0a836b9` | Integrated; overlay 43 F0000324 promoted, 2,240 bytes |
| z-ovl2 | `3e6e7aaae` | Integrated; overlay99ApplySegment promoted, 920 bytes |
| z-res | `63632dac7` | Integrated; effectboxControl promoted, 772 bytes |
| z-shad | `777490905` | Integrated; func_80016890 retains 315 masked differences, size delta 0 |
| z-track | `67e4752c5` | Two support commits remain outside the integration ancestry |

The two track commits are `6660cd3cf` (TU rodata ownership and normalization)
and `67e4752c5` (scoring listings with migrated rodata). Both were applied in
`lane/codex-claude-recovery-20261003` and reproved against the full ROM.
This recovery earns zero new matching bytes. Fresh configured-TU measurements
retain func_80011980 at 195 masked differences and size delta +4; the original
commit's reported 172-word scratch candidate is not present in its source.
func_800103D4 remains 158 masked differences, size delta -16.

The first fresh-lane link required `gmake overlay-syms` after extraction.
After regenerating that relocation surface, `gmake verify` passes with the
expected hash. No source or compiler-output instruction patch was needed.

## Ordered continuation

1. Finish the recovered support unit's gates and commit, then integrate it
   through campaign/unchain and repeat the integration gates. Reconcile the
   main handoff's stale open-function list against current guards and shards:
   it still lists several now-matched functions. Preserve the old attempts
   as history; they are not current queue entries.
2. Reconcile the 11,344-byte ranking gap by executable ownership identity.
   Keep includes and overlay aliases explicit. Refresh the sibling scan after
   each integration, but authenticate every lead as matched C: the scan
   currently proposes assembly-backed overlay88DrawSortedGeometry as a
   matched sibling, so its 0.91 lead cannot serve as matching evidence.
3. Prioritize func_800115E4 in track.c using newly matched func_8001EC44 as
   a structural sibling (mnemonic similarity 0.92). Compare ABI, field types,
   relocations and loop semantics before adopting any shape. Its assignment
   gate currently returns stale-ledger: reconcile its source/handoff pins,
   record the new sibling mechanism, then require a base-only verdict.
4. Next consider overlay17CreateChain against overlay17AdvanceChain's now
   matched source, and overlay 8 F00034A0 (3,592 bytes, 17 masked differences).
   The constructor's current 65-word plateau already tested a natural rewrite;
   repeating that rewrite is not a new mechanism. Its reopen authorization
   is stale. Pin current evidence and state a new specific hypothesis before
   assigning it. An uncommitted 45-word scratch result in the old handoff is
   not a proved baseline.
5. Work the allocation/scheduling residuals only with a causal explanation:
   anim func_80051364 (7), overlay 27 F0000624 (12), track func_8000E5EC (12),
   and overlay 44 F0000580 (13). Diagnostic forced registers do not establish
   a match. Read committed shards and obtain base-only assignment verdicts;
   reopen exhausted targets only with current pins and a genuinely new lever.
6. Give large, less-explored functions their own bounded reconstruction
   packets after checking existing attempts and matched siblings. Overlay 47's
   8,672-byte function deserves attention, but its presumed second extern
   identity must first be authenticated from Mickey relocation evidence.
   Leave explicitly exhausted matrix/allocator targets closed until a new
   mechanism exists.

Each writable matching packet owns a disjoint lane and TU, records an exact
base, evidence, soft deadline and handoff grace period, and stops after three
consecutive attempts without new information (ADR 0018). Preserve source,
object, size, masked/raw score, first mismatch and rationale privately. Commit
one exact function or coherent plateau unit at a time. Promotion requires
untouched compiler output, owned boundaries, exact relocations, linked bytes,
full-ROM identity and all normal provenance/accounting gates.

The next measurable milestone is 90% resolved executable bytes: 849,276 bytes,
22,348 beyond this baseline. It is a checkpoint on the way to all currently
unresolved 116,712 bytes, not a replacement for the 100% campaign objective.
