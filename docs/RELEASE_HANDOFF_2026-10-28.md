# Proof capture cleanup and JFG handoff

28 October 2026. Code checkpoint:
[`9f3063ff0435ed1e6bb7554c48504d8db930bfe6`](https://github.com/akratch/Mickeys-Speedway-USA-Decomp/commit/9f3063ff0435ed1e6bb7554c48504d8db930bfe6).

This release fixes temporary-file retention in the proof tools. Repeated runs
previously left complete linked images behind. Each capture now has an owner:
its files stay available through the final freshness checks and publication,
then the operation removes them. Failed captures are removed after their owned
subprocesses stop. Game source and matching credit are unchanged.

## Validation boundary

The following checks passed on the code checkpoint:

| Check | Result |
|---|---|
| `gmake verify` | US ROM SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548` |
| `gmake check-tooling` | All 151 discovered test files passed under the configured runner |
| `gmake cleanroom` and staged scan | Passed |
| `gmake check-docs` | Passed |
| `gmake check-scoreboard check-overlay-syms check-overlay-graph` | Passed |

The 20 capture/process tests cover successful and failed lifetimes, held-file
mutation refusals, cyclic witnesses, repeated image captures, nested processes,
Ctrl-C and TERM, queued work, and silent descendants of failed launchers.
All six proof-capture directories were empty after verification.
Astra's review found no remaining blocker for this tooling change.

These results establish the repair's validation boundary. The exact ROM hash
does not establish that all overlay storage and identities are source-owned.

Direct SIGKILL, interpreter crashes and shutdown can still leave private files.
Confirm that their writers have stopped before removing them. Cleanup never
scans other runs or deletes directories based on age or a guessed owner PID.
See [the capture lifetime contract](tools.md) for library use and receipt paths.

## Remaining evidence and source work

The fixed R8 storage-binding adapter requires an existing local reference record
that is currently unavailable. Production validation refuses without that record.
The unit tests use a labelled synthetic fixture to test the validator; that
fixture supplies no authority for a live game proof. Restore the authentic
record, or independently establish and review replacement evidence before using
that adapter. Its check is in
[`resident_storage_bindings.py`](../tools/resident_storage_bindings.py).

The full promotion census is not a release result here. Overlay data ownership,
the general build graph and remaining source-quality work continue under the
existing matching standard. The resident-array proof and O62 cursor changes
remain outside this release.

## What to share with JFG

The revised [JFG handover](runlink-handover.md) leads with gentarkin's goal:
lookup tables and ROM positions derived from current build inputs, with modules
added or removed through declarations. It links to JFG's existing loader,
explains where `reloc_addrs` helps extraction, and separates that recovery step
from generating runtime records from current ELF relocations.

The module-79 example previously reproduced the retail baseline and a live DATA edit. Its unchanged
modules, existing identity registry and outside layout inputs are still
compatibility inputs. It cannot yet add or remove a module or export, or handle
all section and relocation cases. It is useful as a migration example; the
complete JFG builder remains unfinished. The handover identifies the next
bounded BSS experiment and the subsequent acceptance tests.

Share that introduction and the example first. The longer format report and
roadmap are supporting material. The document retains its AI-assistance
disclosure and JFG's contribution preference. No JFG submission or message is
part of this release.
