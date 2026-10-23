# Mickey's Speedway USA decomp: agent operating policy

This repository is an active matching-decomp campaign. Decisions governing
how it's worked are recorded in `docs/adr/` (start at `docs/adr/README.md`);
this file is the operating summary. Where this file and an ADR disagree, the
ADR is authoritative: file a new ADR to change policy, don't just edit this
file out from under it.

Read `docs/CONTRIBUTING.md` and `docs/CLEANROOM.md` before changing
canonical source. `docs/LOCAL_DECOMP_AGENT.md` is historical: the local
`n64dllm-v1` lane was evaluated and retired (0 improvements across 5
targets; see `docs/adr/0009-model-routing-and-agent-operation.md`). Don't
route matching work to it.

## Private and public repositories

This checkout is the canonical repository. Its `origin` is
`akratch/Mickeys-Speedway-USA`, and it is **public**. `campaign/unchain` is the
integration branch; `master` is what ships. There is no mirror -- an earlier
one existed and has been retired and deleted -- so there is no "publish later"
step: a push here publishes immediately, and the content rules below are the
whole of the protection.

- Never force-push `origin`. Never push to `upstream`, which is Rare's public
  Jet Force Gemini repository and not ours.
- Nothing ROM-derived is ever committed: no asm, no instruction text, no
  hexdumps, no machine-word arrays, no base64 of ROM bytes, no baseroms, no
  extracted assets, no compiler binaries. The clean-room gates enforce this,
  and a commit that reaches the remote cannot be unpublished by a later
  deletion.
- No absolute workstation paths in tracked files. Write them relative to the
  repository root, or as a placeholder. They leak a home directory and break
  for every other clone.
- A matching change lands only after its function-sized commit is on
  `campaign/unchain` and the tree passes `gmake verify`, `gmake cleanroom`,
  `gmake check-docs` and `gmake check-scoreboard`. Do not land candidates,
  plateaus, or unintegrated lane state on `master`.
- Use `tools/land.sh` rather than pushing by hand. It merges into `master`,
  regenerates the overlay alias list, re-verifies the ROM from the merge
  result, and pushes both branches.
- Treat pull requests as input. Reproduce an accepted contribution in a lane,
  integrate it through `campaign/unchain`, and prove it with the gates before
  it becomes canonical.

Because the repository is public, the instruction files in it are public too.
`AGENTS.md` and `CLAUDE.md` describe how the project is run and are readable by
anyone; write them accordingly, and keep genuinely machine-local material out
of the tree rather than assuming a private side exists to hold it.

## The lane model

As many workers as there is independent work; there is no fixed slot count
to saturate. Each worker operates in its own isolated worktree, created with
`tools/new_lane.sh <name>`, on branch `lane/<name>`. Lanes give every worker
its own `build/` and `asm/`, so workers never contend for the same objects
and never need a job ceiling to avoid stepping on each other
(`docs/adr/0004-build-parallelism.md`).

- Build with `gmake -j$(sysctl -n hw.ncpu)` (or the machine's real core count) inside
  your lane. There is no campaign-mandated compiler-job ceiling.
- Never read or touch another lane's worktree, index, process state, or
  in-flight files. If you need something another lane owns, ask for a handoff
  rather than editing it directly. The coordinator and read-only tools may
  inspect committed `refs/heads/lane/*` objects through Git's object database
  for duplicate detection and sibling discovery (ADR 0011); a commit-message
  match claim is scheduling information, never match proof.
- The primary/coordinating agent's job is to keep the ready queue populated
  and assign disjoint targets to free lanes, not to police a slot count. Run
  `tools/lane_status.py --symbol <name>` before assigning a target and proceed
  only on its zero-exit `base-only` verdict. `active`,
  `already-integrated/exhausted`, and `stale-ledger` fail closed.
  This is the unmatched reconstruction queue. Already-exact structural and
  source-quality maintenance follows ADR 0020 with an exact baseline and
  disjoint ownership; it does not reopen matching credit or another lane's work.
- Every non-interactive lane has an explicit soft deadline and short hard-stop
  grace period. Reserve the end for an exact commit or plateau handoff; do not
  start a bounded tool call that cannot fit in the remaining budget. The
  budget follows work class and may grow as the queue hardens (ADR 0011).
- Full two-phase `gmake verify` run against a SHARED worktree (not a lane) goes through `tools/with_verify_lock.sh`, now in `campaign/unchain`. A lane's own `gmake verify` needs no lock: each lane already has its own `build/`.

Raw compiler-output `asm` blocks and standing permuter sweeps follow ADR 0016.
Carve and ROM-prove the TU before decompiling a contained function.
An explicitly recorded causal exploration packet follows ADR 0017: one owned
family anchor, 60–90 minutes, falsifiable hypotheses and a proved baseline
feedback loop, stopping on the ADR 0018 stall rule rather than a count. Its source/handoff pins must
still pass the ADR 0011 assignment gate. This packet can investigate structural
or wider allocation deficits; it does not waive any matching acceptance proof.

### Three-session interactive crew

For a persistent interactive crew, use one leader in `campaign/unchain` and
the two permanent slots `worker-1` / `lane/crew-worker-1` and `worker-2` /
`lane/crew-worker-2` (ADR 0013). Initialize or inspect their shared mailbox
with `tools/crew.py`; its state lives under Git's common directory and is never
tracked.

- The leader alone owns the ready queue, assignments, and integration branch.
  Each actor alone replaces its own status file; inbox messages are immutable.
- Every assignment states exact symbol/path ownership, base commit, evidence,
  deadline, and stop condition. Follow `READY -> ASSIGNED -> ACKED -> WORKING
  -> HANDOFF -> INTEGRATED -> RELEASED -> READY`; report `BLOCKED` or `PLATEAU`
  explicitly. During review, ADR 0014 permits one additional `PIPELINE` packet
  and the `PIPELINED` state so the worker can continue disjoint work.
- A handed-off permanent lane stays frozen until integration. The worker may
  continue one leader-assigned follow-on task in its own isolated pipeline
  worktree, using the same compile permit; it transfers committed work onto the
  permanent lane only after that lane fast-forwards to the integration commit.
  The coordinator never touches either worker-owned worktree.
- Subagents are read-only unless given a disjoint child worktree. The parent
  worker owns child integration and removes only its own clean, released child
  worktrees. A worker and all its subagents share one compile-heavy-process
  allowance.
- Goal mode provides persistence inside each session; the mailbox is the
  cross-session control plane. An idle worker remains `READY` and keeps its
  crew goal active until the user ends the crew. Empty-mailbox and integration
  waits are expected monitoring states, never a durable-goal blocker; while a
  handoff is reviewed, the leader supplies the ADR 0014 pipeline task rather
  than leaving the worker without writable matching work.

## Commit discipline

Commit on your own lane branch as work lands: small, function-sized
commits, not an end-of-session dump (`docs/adr/0010-commit-discipline.md`).
A commit is one exact function plus its symbol-table line and atlas row, or
an equivalent coherent unit.

- Hooks are always on. **Never pass `--no-verify`.** If a hook fails, fix
  the underlying issue and make a new commit; don't bypass it.
- Run `gmake check-docs` and `gmake cleanroom` before every commit (no build
  needed); run `gmake verify` before anything you believe changes matching
  status. See `CLAUDE.md`'s "Before every commit" section for the full list.
- Lane branches integrate through the existing campaign/unchain flow.
  Nothing should exist only in a working directory between integration
  points, which is how a month of work once went unswept by the clean-room
  gates and nearly got lost to a bad `checkout`.

## Validation is part of the job

- Source reconstruction is the start of a decomp task, not its completion.
  A function isn't done until it's compiled, byte-compared, and given a
  clear verdict: matched (ADR 0001), `NON_MATCHING`, `NON_EQUIVALENT`, or a
  recorded plateau once the ADR 0018 stall rule is met.
- Normal in-scope validation doesn't need repeated permission prompts:
  compilation, assembly, linking, ELF inspection, section extraction,
  object/byte comparison, hashing, donor/provenance scans, atlas/progress
  checks, and ROM rebuilds are all ordinary parts of owning a function.
- Never execute a generated game binary or ROM, launch an emulator, or use
  a GUI/device workflow as decomp validation.
- Matched means: `tools/ido/cc`'s untouched output for the C in the tree,
  linked at the real offset, is byte-identical to the ROM
  (`docs/adr/0001-matching-standard.md`). No instruction word may be edited
  after compilation to reach a match. See
  `docs/adr/0002-no-post-compile-instruction-editing.md` for exactly what
  post-compile step is and isn't permitted. A function only reachable via a
  prohibited step is written as `#ifdef NON_MATCHING` C over
  `#pragma GLOBAL_ASM`, not silently counted as matched.
- Require exact owned bytes and exact relocation count/type/offset/identity.
  Exact size, a high objdiff score, or semantic agreement alone is
  non-exact.
- Separate executable bytes from target padding, compiler section alignment,
  and already-matched islands and data. Only unmatched executable bytes ever
  become new credit; source-only surface and padding never do.
- Synthetic-address and zero-valued proxy links are diagnostic only. Every
  promotion extracts the owned range from the linked output and byte-compares
  it against the same offsets in the baserom.
- Preserve every meaningful attempt: source, object, score, first mismatch,
  and rationale. Don't overwrite the best artifact without retaining it.

## Clean-room rules

- Use Mickey's own headers, symbols, source, assembly, objects, relocation
  metadata, and ROM bytes as authority. Follow the `PROVENANCE` disclosure
  rules in `docs/CLEANROOM.md` for any permitted published-decomp material;
  if provenance is uncertain, quarantine the candidate and continue with
  Mickey-only evidence.
- Never add ROM images, extracted assets, disassembly, instruction
  listings, hexdumps, machine-word arrays, or workbench ledgers to tracked
  files. `CLAUDE.md`'s "Nothing ROM-derived is ever tracked in git" section
  is the full rule; the pre-commit hook enforces it.
- Identify overlay functions by `(overlay, section, offset)` and exact
  ROM/text ownership; a shared synthetic VMA is not a unique identity
  across overlays.
- dp64 and `sfadebug` names/code/comments remain prohibited until the
  project owner rewrites `docs/CLEANROOM.md` (`docs/adr/0008-provenance.md`).

## Matching workflow

1. Confirm exact symbol boundaries, TU ownership, ABI/callers, current
   pragma/body, per-file flags, padding, and relocation surface.
2. Check the near-match oracle (coddog / `skeleton_scan.py`,
   `docs/adr/0007-matching-tools.md`) for the closest donor or already-matched
   sibling and put it in context before writing a candidate. `mips_to_c.sh`
   is a draft aid, never authority. If the evidence points to SDK or standard
   library code, start from the permitted official source and exhaust plausible
   versions, flag groups, conditional paths, and TU boundaries before writing
   a custom body (ADR 0012).
3. Compile the unmodified baseline immediately. Record size, exact words,
   first mismatch, frame/register shape, and relocations.
4. Classify the mismatch before editing: boundary/alignment, ABI/type,
   CFG/branch-likely, expression association, register allocation, loop
   spelling, instruction-set/flag mode, or relocation binding. Let the
   decomp workbench do the classifying on anything close: `tools/wb_compare.sh
   <symbol>` builds `build/wb/<symbol>.target.o` and compares it against the
   full-TU candidate object, and `.venv/bin/decomp-workbench diagnose
   build/wb/<symbol>.target.o build/src/<tu>.c.o --function <symbol>
   --objdump tools/binutils/mips64-elf-objdump` names the mechanism and, in
   its `next:` footer, the field-guide lever to apply in the C
   (`decomp-workbench guide` lists them; `campaign --stop-on-exact` compiles
   and ranks several source variants at once; the tool's own docs are in
   `~/Desktop/dev/n64-decomp-workbench/docs/`). Its ledgers and caches under
   `.decomp-workbench/` and `build/wb/` are ROM-derived and never committed;
   a plateau note quotes the verdict and the lever, never the aligned rows.
   Try the flag lattice before hand permutation; run the permuter only as a bounded batch
   job, never inside your own reasoning loop. Global toolchain flags, shared
   headers, symbol policy, yaml layout, and data placement are not changed
   merely to improve one candidate; a flag change that survives the sweep
   becomes canonical only with target evidence and an impact review across
   every consumer of that flag group.
5. Iterate with coherent hypotheses, preserving semantics, widths,
   signedness, and call order. Don't improve a score by inventing guards,
   merging unproved identities, or patching object bytes.
   ADR 0017 separately permits labelled, defined and semantically inert
   source-shaping diagnostics inside an authorized causal packet. Nonexact
   diagnostics stay ignored; an exact inert spelling needs semantic review,
   disclosure and the cleanup queue before normal promotion proofs.
6. Stop on absence of new information, not on a count
   (`docs/adr/0018-progress-based-stopping.md`). Keep going while attempts
   still produce a better residual, a newly proved identity, or an eliminated
   hypothesis; stop after three consecutive attempts that produce none of
   those, and record the plateau: best candidate, score, first mismatch,
   what's blocking it, and the stall evidence. A monotonically improving
   series is a reason to continue within the wall-clock budget. Stop early,
   with zero attempts if warranted, when committed evidence already rules out
   the mechanism available to you.
7. Promote an exact result into the canonical split as its own commit.
   Rebuild the configured object and re-prove the function, boundary,
   relocation table, linked overlay, and full ROM.
8. Run donor/provenance, atlas/YAML, derived-number, progress, and
   clean-room gates before banking bytes or claiming a closure.

When an exact result or controlled toolchain experiment proves reusable IDO
behavior, add a generic symptom/mechanism/lever/limits entry to
`docs/ido-learnings.md` in a coherent documentation commit. Per-function
addresses, scores, register assignments, and attempt histories stay in the
resident/overlay/triage ledgers (ADR 0012).

## Evidence and reporting

- Every claim in `docs/modules.md`/`symbol_addrs.us.txt` states its
  evidence tier (A byte-identity, B call graph, C string correspondence, D
  structural inference, `docs/modules.md` §1). A claim with no stated
  method is a bug in the document.
- Derived numbers (counts, percentages, sizes) are recomputed from the
  tree, never carried forward from an earlier message or file version.
- Lead reports with measurable outcomes: exact words/bytes, boundaries,
  relocations, linked status, remaining deficit. Distinguish
  source-reviewed, compiled, object-exact, canonical-staged, and ROM-exact
  states; never call a partial score a match.
- Every handoff states files changed, commits (with hashes), commands run,
  best score and first mismatch, unresolved blockers, and the next concrete
  action.
