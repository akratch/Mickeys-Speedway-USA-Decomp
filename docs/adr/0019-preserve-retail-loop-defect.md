# 0019. Preserve the authenticated overlay 45 retail loop defect

Status: Accepted
Date: 2026-10-11

## Context

Overlay 45's rotated text renderer, `func_overlay_045_F0001158_188D5B0`,
reads a local vertical loop limit without initialization. Independent review
of every preceding stack store, the calling convention and address escapes
established that the retail read is neither an incoming argument nor an
object initialized by a helper. The reconstructed `loopY` expresses that
original defect. Initializing it would change the shipped behavior and bytes.

ADR 0017 prohibits uninitialized reads in source-shaping diagnostics. The
owner explicitly approved preserving this authenticated original defect:
"if the game was written with the bug, we keep the bug (for now) until we do
the port work later."

## Decision

Permit this one original uninitialized `loopY` read in the matching renderer.
This narrowly supersedes ADR 0017 for that authenticated retail defect only.
Document it at its declaration and preserve its stock compiler behavior.
This is not a claim that the C has defined portable-language behavior.

No new undefined behavior, dummy padding, invented accesses, or post-compile
instruction edits are authorized. All other source-shaping expressions still
require their ordinary independent semantic review. In particular, local
color and triangle-index carriers must be initialized and actually consumed.

Require the unchanged matching standard: stock compiler output, exact owned
function bytes, boundary and relocation identities, linked overlay proof,
full-ROM identity, provenance disclosure and all normal publication gates.
Private stack and compiler evidence remains untracked.

## Consequences

The matching build retains the original bug with a visible source comment.
A future port should initialize the vertical limit according to its intended
rendering behavior as a separately reviewed behavior change. That fix must
not be silently substituted into the matching build. This exception supplies
no blanket authority to introduce or assume uninitialized reads elsewhere.
