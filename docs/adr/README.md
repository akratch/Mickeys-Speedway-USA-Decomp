# Architecture Decision Records

This directory records the decisions the project owner made on 2026-08-24,
reviewing `docs/acceleration-survey.md`, particularly §13's survey of what
the published N64 decomps (DKR, JFG, BK, PD, dp64) actually do. These
files are the durable record of *what was decided and why*; they are not
re-opened to relitigate a decision, only superseded by a new ADR when the
decision changes.

## Format

Each ADR is one file, `NNNN-short-title.md`, numbered sequentially and never
renumbered or reused. MADR-style, short (the target is 40-90 lines):

```
# NNNN. Title

Status: Accepted | Superseded by NNNN | Deprecated
Date: YYYY-MM-DD

## Context

What situation forced a decision, and what was actually measured (a
citation to the survey section, a repo, a number) rather than asserted.

## Decision

What was decided, stated as an instruction someone can follow without
having read the context.

## Consequences

What this changes in the tree, what it does not change, and any follow-up
work the decision creates.
```

A decision is written down once it is made; it is not deferred pending
implementation. Where a decision requires code or doc changes elsewhere in
the tree, the ADR says so under Consequences and the follow-up is tracked
the ordinary way (a commit, a campaign entry), not by leaving the ADR
"Proposed."

## Index

| ADR | Title |
|---|---|
| [0001](0001-matching-standard.md) | Matching standard |
| [0002](0002-no-post-compile-instruction-editing.md) | No post-compile instruction editing |
| [0003](0003-scoreboard.md) | Scoreboard |
| [0004](0004-build-parallelism.md) | Build parallelism |
| [0005](0005-work-prioritisation.md) | Work prioritisation |
| [0006](0006-overlay-source-layout.md) | Overlay source layout |
| [0007](0007-matching-tools.md) | Matching tools |
| [0008](0008-provenance.md) | Provenance |
| [0009](0009-model-routing-and-agent-operation.md) | Model routing and agent operation |
| [0010](0010-commit-discipline.md) | Commit discipline |
| [0011](0011-cross-lane-knowledge-and-task-budgets.md) | Cross-lane knowledge and task budgets |
| [0012](0012-known-sources-and-reusable-knowledge.md) | Known sources and reusable compiler knowledge |
| [0013](0013-three-session-crew-control.md) | Three-session crew control |
| [0014](0014-pipelined-handoff-work.md) | Pipelined handoff work |
| [0015](0015-verified-lane-bootstrap-cache.md) | Verified lane bootstrap cache |
| [0016](0016-raw-asm-tu-pipeline-and-standing-sweep.md) | Raw-assembly TU pipeline and standing sweep |
| [0017](0017-causal-exploration-and-risk-proportional-validation.md) | Causal exploration and risk-proportional validation |
| [0018](0018-progress-based-stopping.md) | Progress-based stopping replaces the fixed attempt cap |
| [0019](0019-preserve-retail-loop-defect.md) | Preserve the authenticated overlay 45 retail loop defect |
| [0020](0020-maintaining-exact-source.md) | Maintenance of already-exact source |
| [0021](0021-source-module-linking.md) | Source modules linked with runtime section-relative addresses |
