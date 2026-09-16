# Working in this repository

Mickey's Speedway USA, clean-room matching decompilation. The US ROM rebuilds
byte-identically; every commit must keep it that way.

## Release posture

There is one repository. `origin` is
`git@github.com:akratch/Mickeys-Speedway-USA-Decomp.git`, it is **public**, and
it is canonical. It was renamed from `Mickeys-Speedway-USA` on 2026-09-15;
GitHub still redirects the old name, so a stale remote keeps working and hides
the rename until the redirect stops. Work lands through lanes on
`campaign/unchain`, is merged into `master`, and both are pushed by
`tools/land.sh` after each integration batch.
Run it rather than pushing by hand: it regenerates the overlay alias list,
re-verifies the ROM *from the merge result* (a merge can combine two lanes'
generated artifacts in ways neither produced alone, which `verify` on
`campaign/unchain` does not cover), and pushes both branches. Never force-push
`origin`.

The `upstream` remote is `https://github.com/Ryan-Myers/Mickeys-Speedway-USA.git`
-- the Mickey's Speedway USA decomp this fork came from, not ours. Never push
there. (This paragraph named Rare's Jet Force Gemini repository until
2026-09-16; that was wrong. JFG is a *permitted source* under
`docs/CLEANROOM.md`, read from `~/Desktop/dev/decomp-refs/`, and is not a
remote of this repository at all.)

Because the repository is public, everything committed is published the moment
it is pushed. The content rules in the next section are therefore the whole of
the protection, and they are about *content*, not about which branch or remote
a thing lands on. Two further hygiene rules follow from being public:

- **No absolute workstation paths in tracked files.** Write paths relative to
  the repository root, or as a placeholder. They leak a home directory, they
  break for every other clone, and they have been committed by accident more
  than once.
- **Nothing that only makes sense to one machine.** Local agent scratch,
  campaign orchestration state, workbench caches and `.codex/` are gitignored
  for this reason. The workbench campaign manifests were once tracked as a
  deliberate exception; they are machine-written and carried absolute home
  paths, so they are gitignored too.

A history note, because it changes what "remove it" means: this repository was
previously described here as private, with a separate public mirror at
`mickeys-speedway-usa-decomp`. That mirror is retired and the description was
wrong. Anything already committed is already public, so deleting a file in a
new commit does not unpublish it -- removing it for real needs a history
rewrite.

## Dispatching a lane

`docs/LANE_BRIEF.md` is the standing brief every lane reads first. It carries
what is true for all of them -- the measurement discipline, the call test, the
law index, the instruments, the traps and the commit rules -- so a dispatch
message only has to carry the targets, their measured numbers, and whatever is
specific to them.

Keep it that way. Seven bespoke briefs written in one session ran about 170
lines each with roughly 120 of those identical, and hand-copying drifted: the
L106 premise was stated wrongly to two lanes, both of which spent a sweep on an
axis their functions do not have. One tracked source of that rule is worth more
than the tokens it saves.

## Nothing ROM-derived is ever tracked in git

Not asm, not instruction text, not hexdumps, not extracted assets, not
workbench ledgers. Not temporarily, not to show a diff, not inside a JSON blob.
Two workbench `ledger.jsonl` files were once committed with the ROM's own
disassembly inside them, and undoing it took a history rewrite.

### Never track

| | |
|---|---|
| `asm/`, `assets/`, `expected/`, `baseroms/` | splat output and ROM images |
| `*.z64` `*.n64` `*.v64` `*.bin` | ROM images and extracted binaries |
| `.decomp-workbench/**`, `.codex/` | workbench state and local agent config. The campaign manifests were once tracked as an exception; they carried absolute home paths, so they are not tracked any more |
| instruction text | mnemonics + operands, in any file, in any format |
| hexdumps, byte arrays, base64 of ROM bytes | same content, different clothes |
| `tools/ido/`, `tools/binutils/` | proprietary toolchain binaries, gitignored. If one is ever staged the `binary-blob`/`oversize` rules catch it, not a path rule |

Quoting a couple of instructions in a comment to explain why a function is
named what it is: fine, and `docs/modules.md` does it. Pasting a function's
disassembly: not fine. The detectors draw that line at 40 mnemonic tokens and
1.0 per KiB. The heaviest legitimate file in this tree carries 17
(`symbol_addrs.us.txt`; `include/game/runlink.h` is next at 16), 2.35× under.

## Before every commit

```sh
tools/gates.sh              # verify, cleanroom, check-docs -- true exit status
tools/gates.sh --staged     # same, with cleanroom scanning the index
tools/gates.sh --promotion  # adds check-scoreboard/-overlay-syms/-nonmatching-builds
```

or the three individually:

```sh
gmake verify        # must print the expected SHA1
gmake cleanroom     # or: gmake cleanroom CLEANROOM_ARGS=--staged
gmake check-docs    # derived numbers in the docs still match the tree
```

**Prefer `tools/gates.sh`, and never pipe a gate's output.** Writing

```sh
gmake check-scoreboard 2>&1 | tail -3; echo "exit=$?"
```

reports the exit status of `tail`, which is always 0, so a gate that failed
three lines up reads as a pass. That has happened three times here and twice it
put a commit on top of a red gate. `gates.sh` keeps status and output separate,
names every failing gate in a final verdict, and exits nonzero.

If matching progress changed (a function moved from asm to C, a name was
adopted), also run `gmake scoreboard` and commit the README diff it produces;
`gmake check-scoreboard` fails otherwise.

The clean-room sweep runs automatically once `gmake setup` (or `gmake hooks`)
has pointed git at `.githooks/`:

- **pre-commit** scans the index, what the commit would record.
- **pre-push** scans every commit tree leaving the machine, so a commit made
  before the hooks existed, or in another clone, still cannot ship.
- **CI** re-runs both on push and PR, plus `check-scoreboard --check-partial`
  (`scoreboard.yml`).

Everything else is manual; nothing but the clean-room sweep is wired into a
hook or CI:

| Command | What it checks | Needs a build |
|---|---|---|
| `gmake verify` | ROM rebuilds byte-identically | yes |
| `gmake cleanroom` | no ROM-derived content (hook + CI enforced) | no |
| `gmake check-docs` | derived numbers in the docs match the tree | no |
| `gmake check-scoreboard` | README's Progress block matches the tree right now. Needs the ELF, so CI can only run `--check-partial`; see `docs/CONTRIBUTING.md#checks` | yes |
| `gmake check-overlay-syms` | `overlay_undefined_syms.us.txt` is still what the tree generates. That file is generated by `gmake overlay-syms` from `config/overlays.us.json` and the compiled overlay objects, never hand-edited; a hand edit or a stale promotion shows up here. Run after any overlay promotion | yes (compile-only) |
| `gmake check-nonmatching-builds` | every candidate-bearing TU still compiles with `-DNON_MATCHING`; a TU that does not silently drops its candidates out of the permuter sweep. Run after editing any `#ifdef NON_MATCHING` block | yes (compile-only) |
| `gmake audit-decoders` | the clean-room detectors aren't inventing words. Run after touching `tools/cleanroom_detectors.py` | no |
| `gmake check-fixtures` | the detectors still *catch* real ROM in every encoding and wrap width, the direction `audit-decoders` cannot see. Fixtures are generated from the baserom at run time and never written to disk, so this cannot run in CI | no (needs a baserom) |
| `gmake progress` | prints matched functions/bytes/symbols | yes |
| `gmake scoreboard` | regenerates README's Progress block from the tree | yes |
| `gmake clean` | removes `build/` | no |
| `gmake distclean` | `clean` plus extracted state (`asm/`, `assets/`, linker script, auto-generated `undefined_*`); recovering needs `gmake extract` and a baserom | no |

## Promoting an overlay function

An overlay promotion regenerates *before* it builds. Every step below has cost
a lane a failed cycle, and not one of the errors names the step that is
missing:

Two lanes reported this order differently; they were describing constraints
on *different* steps, and both hold. The atlas must be current before the
extract, and the extract must precede `overlay-syms`:

```sh
# 1. Add the promoted range by hand to MIXED_TU_EXACT_C_RANGES in
#    tools/overlay_atlas.py, then write the atlas. `overlay-atlas-write`
#    will NOT invent the range -- it reports "overlay artifacts current" --
#    and editing config/overlays.us.json directly is overwritten. Keep each
#    overlay's entries SORTED BY OFFSET, or `gmake build/.splat-stamp` dies
#    with `ValueError: invalid overlay N mixed-TU exact range` from a
#    traceback naming neither the entry nor the ordering rule.
#
#    This comes first because `gmake extract` refuses to run while
#    config/overlays.us.json is stale.
gmake overlay-atlas-write
.venv/bin/python tools/refresh_atlas_digest.py

#    When the promotion COMPLETES a TU -- the last GLOBAL_ASM in it goes away
#    -- this step is a no-op: the writer drops every mixed-TU range for a TU
#    that has stopped being mixed, so the manifest is byte-identical with and
#    without your entry. Run it anyway; it costs a second and its output tells
#    you which case you are in.

# 2. Re-extract. Removing a GLOBAL_ASM pragma leaves splat's .s behind;
#    running overlay-syms before this fails on build/.splat-stamp, and
#    building before it fails at link on an undefined reference.
gmake extract

# 3. Regenerate the alias list.
gmake overlay-syms

# 4. Build, then regenerate the alias list AGAIN and rebuild.
#    overlay-syms derives its aliases from the compiled overlay objects, so
#    on a first promotion the pre-build run cannot see the new symbol. One
#    pass links with undefined references and R_MIPS_26 overflows.
gmake -j8
gmake overlay-syms
gmake -j8 && gmake verify

# 5. Confirm.
gmake check-overlay-syms
gmake promotion-proof SYMBOL=<symbol>
```

Overlay objects do **not** depend on `mk/overlays.mk`. Editing a POSTPROCESS
rule there triggers no rebuild, and the resulting link failure looks as though
the rule is wrong when it is already correct -- `rm` the object.

**`tools/reloc_surface.py` rewrites resident-call symbol names inside the
compiled overlay objects, and only `gmake overlay-syms` applies it.** So a link
failing with `R_MIPS_26 ... relocation truncated to fit` against resident
symbols is almost always a build-state problem rather than a source one: an
overlay is linked at `0xF0000000` and cannot `jal` a `0x8000xxxx` entry point
directly, so it must go through the generated `*Reloc` surface entry. Run
`gmake overlay-syms`, then build.

This recurs after `gmake extract` **and after editing `symbol_addrs.us.txt`**,
which re-runs splat and rebuilds every object, silently discarding the renames.
`tools/land.sh` regenerates before it verifies for exactly this reason.

**A same-module call can still be a `SYMBOL` relocation record, and then the C
must not name the in-module definition.** `reloc_surface.py` skips any name
defined in the module's own `.text`, so it neither values nor complains about
such a site, and the function can score **0 masked words** and still fail
`gmake verify` by exactly two. `func_overlay_002_F0001A94_185888C` did: its two
`overlay2ContainsPoint` calls are `SYMBOL` records naming overlay 2 offset
`0x123C`, so the shipped word is the `0xF0000000` addend, while its neighbouring
`overlay2QueryNode` and `overlay2AdjacentIndices` calls in the same function are
`JUMP` records that do store `offset >> 2`. "Same module, therefore a direct
`jal`" is false per call site. The tell is `tools/overlay_tables.py`'s record
`op_name` at that offset; the fix is a `*Reloc` placeholder declaration, as with
a resident call. `gmake overlay-syms` reporting `UNRESOLVED ... 2 distinct
values` for a pair of surface names is a source finding, not tool noise.

The trap it hides: a promotion's `gmake verify` can pass in a worktree that
holds renames the *commit* does not carry, so the tree stops linking for
everyone else. The overlay 60 promotion renamed 8 of its 51 resident callees
that way, and no lane worktree created afterwards could build at all until the
remaining 57 rules were added. If a promotion adds resident calls, check that
`mk/overlays.mk` names every one of them.

**The same hazard runs the other way, and `check-overlay-syms` passes on it.**
A lane whose overlay objects were rebuilt after the last `gmake overlay-syms`
regenerates an alias file that has silently *lost* renames, and because the
check compares that file against what the tree currently generates, it reports
"up to date" on the wrong content. The tell is a one-line diff turning
`<sym>_oNNReloc` back into a bare `<sym>`. Never commit that diff: run
`gmake overlay-syms` and rebuild, and the tracked file regenerates byte-identical.
Restoring the tracked file *without* rebuilding leaves the link failing, because
the objects still reference the bare name.

**A generically-named `D_` symbol can collide across overlays, and the link
does not complain.** `D_EE0` exists in overlay 22 and overlay 29 with *different
stored addends* (`0xEE0` against `0`), so promoting an overlay-22 function that
referenced it silently rewrote two overlay-29 words — the ROM still built, and
only `verify` caught it. Give the promoted overlay its own placeholder rather
than sharing the generic name, and check any `D_`-named reference a promotion
introduces against the other overlays before believing a clean link.

`check-overlay-syms` is a *drift* check on already-regenerated output, so it
cannot catch a promotion that never regenerated; reading its table entry below
as "run this after promoting" is what leaves the build broken. Likewise
`promotion-proof` refuses with `expected one tracked exact atlas range for
<sym>, found 0` until step 2 is done.

## Three traps when integrating a branch

Both of these produced a failed integration on 2026-09-10, on a branch whose
own work was correct.

**A compiled object does not depend on the `Makefile`.** Change a per-TU flag
and `gmake -j8` rebuilds *nothing*: the object is newer than its source, and
Make never learns the recipe changed. So a bad flag is invisible until
something else evicts that object, and then it surfaces far from the commit
that introduced it, usually as `ERROR: n64crc rewrote the CRC words`. When a
merge touches a `CFLAGS +=` line, force the affected objects to rebuild before
believing `gmake verify`. This is the same hazard as the `mk/overlays.mk`
note above, and it is why a stale `build/` can hide a broken promotion.

The concrete case: a donor commit's `Makefile` hunk carried

```make
$(BUILD_DIR)/$(SRC_DIR)/main/menu_3B1A0.c.o: CFLAGS += -Wo,-loopunroll,0
```

as leading context. That line was inert when the donor wrote it, but had since
been *deleted* to match `func_8003A7D0` in a different TU. Resolving the hunk
toward the donor silently reinstated a flag that unmatches a function the
donor never touched. The C body was byte-exact all along.

**A clean auto-merge of a generated file bypasses the resolver.**
`tools/resolve_lane_conflicts.py` sends generated JSON to `--theirs` because a
keep-both hunk cannot produce valid JSON -- but it only runs on files git
reports as *conflicted*. Where git merges the file's rows cleanly, the result
can parse and still be internally inconsistent: one merge dropped the row
another lane had retired while keeping the incoming header count, giving a
`resolved` one higher than the number of rows. `nm_ranking.py` refused it, which
is the invariant doing its job, but do not rely on that -- **regenerate every
generated file after a merge rather than trusting the merge**, and prefer a
full regeneration to a stale refresh, because a refresh reads the same broken
header.

**Moving a carve boundary orphans the previous extract.** After a
`0x833E0` -> `0x83430` move, the old `asm/data/833E0.rodata.s` survives
unreferenced by `mickey.us.ld`, yet still feeds `check_derived_numbers`, which
then reports a jump-table count one too high and makes a correct recomputed
number look wrong. Delete the orphan after any boundary move.

## What the gates cover

The hooks are client-side, opt-in per clone (`core.hooksPath`), and skippable
with `--no-verify`. CI catches what reaches the remote, but by then the objects
are published and the remedy is a history rewrite. They are depth against
mistakes, not a barrier against a determined bypass.

One server-side layer exists: the GitHub ruleset `protect-master`
(id `20111399`, active on `master`) blocks force-push and branch deletion. It
does not restrict *content*: GitHub refused a push ruleset scoped by file
path/extension/size because push rules require an org-owned repo and this is a
personal fork. A required status check on `master` is still the only way to
block bad content before it lands, and would mean pull requests instead of
direct pushes; that change has not been made. Blocking force-push also means a
future history purge of `master` requires disabling `protect-master` first.

The content rules catch mistakes (an asm dump, a hexdump, a ledger, a base64
blob, a leak spread across files), measured against this repository's whole
history on one side and real ROM fixtures on the other. They are not
adversary-proof: detecting arbitrarily-encoded data is undecidable, a padded
file carrying up to 191 machine words (~764 bytes of ROM; the per-file limit is
192) slips under every threshold, and `--no-verify` skips the lot.
`docs/CLEANROOM.md` lists the measured limits. The load-bearing guarantees are
structural: the `.decomp-workbench` path whitelist and manifest schema check,
the ROM path and binary rules, the tool-level ledger redaction, this policy,
and `protect-master`.

**Never pass `--no-verify`.** Never lower a threshold to make a file pass. If a
file is genuinely a false positive, restructure it, or add it to
`CONTENT_EXEMPTIONS` in `tools/cleanroom_detectors.py` with a written reason.
If something ROM-derived is already committed, it has to be rewritten out of
history (`git rebase -i`, `git filter-repo`); a follow-up commit that deletes
the file still ships the bytes.

## Permitted sources, and disclosing them

`docs/CLEANROOM.md` is the policy; read it before adopting anything from
elsewhere. In short: the five named published retail-derived decomps (Diddy
Kong Racing, Jet Force Gemini, Perfect Dark, Banjo-Kazooie, Conker's Bad Fur
Day) may supply names and adapted function bodies **with a `PROVENANCE` note at
the point of use**. Anything leaked is forbidden outright. Reference repos,
their baseroms and their build outputs live in `~/Desktop/dev/decomp-refs/`,
outside this repo, never committed.

## Evidence discipline

`docs/modules.md` §1 defines four evidence tiers (A byte-identity, B call
graph, C string correspondence, D structural inference) and the Tier A
adoption threshold.

- **Declare the tier inline, per symbol.** A claim with no stated method is a
  bug in the document.
- **A sub-threshold adoption is argued individually**, in the table in §1.2.

## Derived numbers are recomputed, never remembered

Every count in the docs (function totals, matched bytes, segment sizes, the
percentages in `README.md`) is derived from the build and the tree. Recompute
from the underlying lists; do not carry a number forward from an earlier
message or an earlier version of the file. `gmake check-docs` re-derives what
is mechanically checkable and fails on drift; `tools/progress.py --verbose`
shows its working.

## Build quickstart

```sh
gmake setup     # venv, deps, toolchain, baserom SHA1 check, splat, git hooks
gmake -j8       # build/mickey.us.z64
gmake verify    # byte-compare against 507341c0a40ca3e9a7cee969b396ee53facfb548
gmake progress  # matched functions/bytes/symbols
gmake check-docs
```

`gmake`, not `make`: macOS's built-in make is too old. Bring your own legally
dumped ROM at `baseroms/mickey.us.z64`.
