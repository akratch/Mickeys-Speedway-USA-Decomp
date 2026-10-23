# 0020. Maintenance of already-exact source

Status: Accepted
Date: 2026-10-23

## Context

The US executable reconstruction is complete, while the overlay structure
sprint and source-quality queue remain open. Those tasks maintain already-exact
functions: they recover storage ownership, remove build scaffolding, consolidate
translation units and seek natural C with the same compiler output.

ADR 0011's assignment gate serves the unmatched reconstruction queue.
`lane_status.py` correctly reports an unguarded exact function as
`already-integrated/exhausted`, with reason `matched-or-promoted`. Sending
maintenance through that queue would require misrepresenting completed work as
unmatched. The owner authorized completion of the source structure and quality
work on 2026-10-23, with matching acceptance unchanged.

## Decision

- Keep the ADR 0011 gate for assigning unmatched reconstruction and reopening
  plateaus. Its `active`, stale-ledger and exhausted-attempt findings still
  fail closed. Do not add a fallback, change a ledger or bypass a tool guard
  to make an exact function appear assignable.
- Assign already-exact structural or source-quality maintenance by its module,
  paths, symbols and committed baseline. Record the task and its stop condition
  in the existing assignment/handoff workflow. A module or function owned by
  another active or frozen lane remains unavailable until released. A
  `matched-or-promoted` result identifies completed matching work; it gives no
  new matching credit and does not itself authorize a maintenance edit.
- Prove the existing configured baseline before changing a function. Preserve
  ABI, storage identities, observable behavior, stock compiler output and exact
  linked bytes under ADRs 0001 and 0002. A proposed natural form must have a
  semantic explanation as well as an exact compilation. Do not trade a working
  match for a nonexact cleanup or a source-shaping diagnostic.
- Use the existing object, relocation, linked-range and ROM checks appropriate
  to the change, plus the ordinary commit and integration gates. No new test
  framework or duplicate proof ledger is introduced. Report storage recovered,
  scaffolding removed and quality improvements separately from new matches.

## Consequences

The ready queue remains a queue of unmatched work. Completed functions can be
maintained without reopening matching credit or weakening the acceptance
standard. ADR 0006's structural work and the cleanup queue use this maintenance
workflow; exploratory changes still follow ADRs 0017 and 0018.
