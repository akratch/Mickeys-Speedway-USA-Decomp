# Lessons ledger

One entry per lesson that changed how the campaign is run. Newest first.
Each entry: what was observed, what it cost, what changed because of it
(tool, gate, rule, or prompt). Numbers are the values at the time; recompute
before reusing them. See `docs/epoch14-plan.md` for the plan these feed.

## 2026-09-10

- **A same-module call can still be a SYMBOL relocation record, and then the C
  must not name the in-module definition.** `func_overlay_002_F0001A94_185888C`
  scored 0 masked words and still failed `gmake verify` by exactly two words:
  its two `overlay2ContainsPoint` calls are `SYMBOL` records naming overlay 2
  offset `0x123C`, so the shipped word is the `0xF0000000` addend, while the
  neighbouring `overlay2QueryNode` and `overlay2AdjacentIndices` calls in the
  same function are `JUMP` records that do store `offset >> 2`. An ordinary
  `jal` to the in-module definition therefore links to `0xF000123C` and is
  wrong. `tools/reloc_surface.py` skips any name defined in the module's own
  `.text`, so it neither values nor complains about such a site; the tell is
  `tools/overlay_tables.py`'s record `op_name` at that offset. Fix: call a
  `...Reloc` placeholder extern (or rename the symbol in the object) so the
  reference stays undefined and takes the stored addend. The relocation-masked
  score cannot see this class at all -- it masks exactly the bits that carry it.
- **A relocation-masked score of zero does not name the symbols.** The same
  function first came out with `gOverlay2QueryBestReloc` and
  `gOverlay2QueryResultReloc` swapped: masked score 0, `gmake overlay-syms`
  reporting `UNRESOLVED ... 2 distinct values: ['0x6c', '0x90']`. Writing the
  comparison so the stored-to global is the operand that reaches the second
  load resolved both. Read the `overlay-syms` UNRESOLVED line as a source
  finding, not as tooling noise.
- **L67 is narrower than "comparison operand order is never a lever".** The
  law's receipt is a *copy-propagated variable*, which does print first
  whatever the C says. Where neither operand is a propagated local -- an `s16`
  struct member against a freshly computed `(s16)` cast chain -- the swap
  changes which subtree cfe emits first, and so which ugen ring temps each
  takes. Swapping the three such tests in the function above moved it 41 -> 13
  -> 0 masked words, on top of dropping one declared local. Verified by
  `gmake verify`.

## 2026-09-09

- **A LUNA round on the hardest residue returned zero matches.** Four codex
  lanes at high reasoning, twenty targets, all `size_delta 0` and none over 21
  differing words, 9,472 bytes. All four exited cleanly and banked **ten
  plateau records and no bytes**. The work was sound; the routing was not.
  Those twenty were the *residue* — functions that had already survived several
  passes, whose handoffs say things like "132 declaration permutations, ten
  dead-store colour reservations and six selection-block restructurings are all
  flat". Brute-force iteration cannot move a residual whose whole source
  neighbourhood is already proved flat; it needs a **mechanism**, which is what
  the instrumented-compiler work has been producing. Route brute force at
  breadth (a TU nobody has swept), not at depth (a target everyone has).

  **Correction, measured the same evening.** Re-routing at breadth did not fix
  it. A second round on the unswept one-instruction reconstruction band — a
  size question with a known cause list, exactly the shape brute force should
  suit — returned **two more plateau records and no matches**. Meanwhile three
  Opus lanes on the *same band, the same brief and the same cause list*
  returned **five matches and carried six functions from `size-mismatch` to
  `delta 0`**. Same task shape, same evidence, different model, and the outcome
  was not close. So the fact is about the model on this work, not about depth
  versus breadth as first concluded. Spend the codex budget elsewhere until
  something changes.
- **A 4-bit `u16` bitfield has a readable signature.** The target sequence
  `lw / sll 5 / srl 28 / sll 7 / andi 0x780 / lhu / andi 0xF87F / or / sh` is
  IDO's code for a 4-bit bitfield at bits 26..23 of a `u16` container. Writing
  `struct { u16 pad:5; u16 channelMask:4; u16 rest:7; }` and assigning through
  the member lets cfe order the final OR, which a hand expansion fixes wrongly.
  `u32`/`s32` containers cost 12 words; `u8` changes the container. That closed
  `overlay74Update` (400 B) and is now in `docs/ido-learnings.md`.
- **A trailing blank line blocks integration after every gate has passed.**
  The merge transaction runs `git diff --cached --check`, which rejects a new
  blank line at EOF, so a promotion ending `}\n\n` fails *after* verify,
  cleanroom, check-docs and the scoreboard have all gone green. Two lanes hit
  it in one day. End source files with exactly one newline after the final `}`.
- **When a whole TU stops being non-matching, `gmake overlay-atlas-write` must
  run before `gmake extract`**, or extract dies on a stale
  `config/overlays.us.json`. The reverse order is right for a single-function
  promotion, which is what CLAUDE.md's recipe describes.
- **A pull request's diffstat is not evidence of donor value, in either
  direction.** JFG PR #37 changed eight C files and delivered a whole matched
  translation unit; PR #15 changed 276 and delivered one function, because the
  branch was old and most of its diff was main's later work appearing as
  deletions. Count bodies per file.



- **Every object-level score in this project has a blind spot the size of an
  unpaired `%hi`/`%lo`, and it has already cost at least one function three
  work packets.** splat writes a high/low address pair as raw literals
  whenever it cannot see the two halves together, which is exactly what
  happens when IDO hoists the `lui` above a branch. The assembled fallback
  then has no relocation at that site while a correct candidate has two, so
  the workbench, the permuter and `nm_ranking`'s masked-word score all report
  a permanent difference in the immediate fields, dressed up as a register
  choice. `func_800056A4` carried that as "2 differing words, register-only"
  through a forced-color diagnosis, a flat permuter sweep and a donor
  re-screen, then matched on the first attempt once the address was written as
  the symbol it is. The address sat inside a neighbouring symbol's extent, so
  the fix was `&D_800C9460[1]` rather than a new name. Changed: the lane
  scorer now resolves every relocation on both sides against the canonical
  linked ELF before comparing words, so the two spellings compare equal
  exactly when the linked bytes do; the rule and its detector are recorded in
  `docs/ido-learnings.md` under "Search fidelity and false floors". Any
  fallback showing a bare `lui` of a plausible RAM address whose paired load
  is across a branch is a candidate for the same treatment.
- **A direct `tools/ido/cc` compile of a whole resident TU is byte-identical
  in `.text` to the Makefile's `NON_MATCHING` object, and it is ~70 ms.** With
  a link-resolved word scorer on top, one candidate costs about 0.1 s
  end-to-end and twelve run in parallel, which turns "sweep the spelling
  lattice" from sampling into enumeration: 132 declaration permutations, all
  24 orders of four independent stores, and a 72-way product of loop shapes
  were each exhausted in seconds this session. Two of the three results this
  lane produced came out of an exhaustive sweep that would not have been
  affordable at the workbench's 6.6 s per measurement. Verify the
  byte-identity per TU before relying on it.

## 2026-09-02

- **A trial projection that is not scoped to the trial is not a projection of
  the trial.** Every one of the 174 `text-differs` rows in the last
  `--overlays-only` sweep reported about 528,000 out-of-range bytes -- the
  whole overlay region behind overlay 1's data section -- and 168 of them named
  the same first differing offset, `0x1854500`, whatever overlay the candidate
  lived in. Nothing about the candidates was wrong. `overlay_atlas.py
  --trial-projection` carved *every* entry in `FIXED_DATA_RODATA_OWNERSHIP`
  into the yaml on every trial, and a carve is only correct while the owning TU
  emits those bytes, which only that TU's own promotion does. For any other
  trial the carved range was unclaimed and, worse, the raw remainder behind it
  was emitted as a second `bin` row with the *same* asset name, so splat wrote
  both slices to one file and the shorter one won: overlay 1 shipped 0x2c of
  its 0x2C0 data bytes, the module shrank 0x290, and every module behind it
  slid. Changed: the projection carves only the TU named by `--trial-source` /
  `PROMOTION_TRIAL_SOURCE` (`promotion_trial.py` passes the TU it is
  promoting), so a trial of an uncarved TU renders the tracked yaml byte for
  byte and does not even re-split; each raw slice gets its own asset name, and
  the trial deletes the trial-only `*_data_rodata_<offset>.bin` on restore. A
  genuine module size change is now its own class, `rom-size`, carrying the
  signed byte delta from the linked ELF and keeping `in_range_words`, so
  "ownership carve needed" stops being spelled `text-differs`. Acceptance: a
  matched overlay function re-wrapped as its own NON_MATCHING candidate
  (`overlay63Initialize`) must trial `exact in=0 out=0`; it does, and the three
  overlay 1 candidates that read as 528,726/528,700-byte disasters are
  `text-differs` with 3, 4 and 4 in-range words and **zero** out of range.
  Cost: one sweep's entire class mix. The tool's own soundness had never been
  measured against a candidate whose quality was not in question.

## 2026-08-28

- **A relocation surface must not assign a name the resident segment owns.**
  Fifteen overlay candidates reported `resident-symbol-missing` and four
  `relocation-truncated (R_MIPS_26)`. One cause: adopted C spells an
  overlay-to-resident call with the resident's own global name
  (`func_80029FE4`, or an ordinary libultra global like `alHeapDBAlloc`), and a
  value line for that name does not give the overlay an addend -- it moves the
  resident function for every resident caller. `texLoadTexture = 0xf0000000`
  turns `models.c`, `level.c`, `menu.c` and four asm objects into truncated
  relocations. The addend itself was never in doubt: measured at the sites,
  overlay 49's resident calls are `SYMBOL` `R_MIPS_26` records storing
  immediate zero, exactly like a cross-module call, so the value is
  `0xF0000000`. The trampoline word `0C00CCE8` belongs to `mainRelocTable` --
  the resident segment's calls *into* overlays -- and does not appear at these
  sites. Changed: `reloc_surface.py generate` derives the rebind the Makefile
  already does by hand (overlay 49: `func_800254FC` -> `overlay65UpdateReloc`),
  renaming every undefined `R_MIPS_26` against a resident-owned name to
  `<name>_o<NNN>Reloc` and valuing the alias; a refused name gets no value line
  under its global name either. It is a no-op on the matching tree (no overlay
  object carries such a relocation), so `--audit` stays 2446/2446 and
  `check-overlay-syms` reports no drift. `resident-symbol-missing` and
  `relocation-truncated` are now zero across the 56 candidates that name a
  resident target; nine of the fifteen carry an in-range word count, four are
  honestly `schedule-divergence-at-site`, two are `rom-size`.
- **"Resident auto-name" was the wrong discriminator; "the resident side
  defines it" is the right one.** The first cut keyed off the `func_8XXXXXXX`
  shape and fixed overlay 34, then broke `overlay5InitializeAudio` on
  `alHeapDBAlloc` / `osCreateMesgQueue` / `n_alCSPSetMessageQ`. Changed:
  `resident_defined_names()` collects the 4,886 globals defined by every
  non-overlay object the linker script names plus the auto-generated symbol
  scripts -- available before the link, which is when the surface has to decide.
- **Refusing costs measurements that an alias makes safe to take.** The strict
  reading ("no corroborated site, no value") returned `overlay34SortAndDraw`
  from 168 in-range words to a bare build failure. Under a per-module alias an
  addend read from an uncorroborated site can only produce a differing word
  *inside* the promoted function, which is the measurement the trial exists to
  take. Changed: uncorroborated resident calls emit a `/* NOTE */` naming the
  offsets instead of a refusal; only the genuinely ambiguous cases (a symbol
  reached by both a call and a data reference; no `text_ownership` row) refuse.
- **Concatenating two build logs makes the first build's failures outlive
  them.** `promotion_trial.py` builds, regenerates the surface, and builds
  again -- the first pass links against the *stale* surface by design -- then
  classified from `log1 + log2`. Six candidates stayed in
  `resident-symbol-missing` after the surface had valued them, because
  `UNDEF_RE` was reading the first link's errors. Changed: markers still come
  from both passes (a POSTPROCESS marker is printed by the compile), every link
  diagnostic from the second pass alone.

- **The overlay scaffolding was hand-maintained because nobody had checked
  whether it had to be.** `overlay_undefined_syms.us.txt` -- 2,928 lines, one
  hand-derived line per adopted overlay symbol -- is entirely derivable: a
  value line is the stored relocation addend read from the baserom at the site
  the module's own table names, an alias line is the generated splat identity
  for a module offset from `text_ownership`. Generated, it is 2,265 lines, the
  ROM is byte-identical, and 8 duplicate names and 168 shadowed assignments
  disappear. Changed: `gmake overlay-syms` writes it, `gmake
  check-overlay-syms` gates it, `tools/promotion_trial.py` regenerates it
  between compiling a candidate and linking it. Overlay candidates carrying a
  measurable number went 110/279 -> 194/279.
- **A guard that aborts the build hides the measurement it was about to
  make.** Every POSTPROCESS normalization asserts the matching object's exact
  layout, so a promoted candidate of the wrong size died at compile time and
  53 of 279 candidates reported only "a guard fired". Changed:
  `tools/postprocess_guard.py` + `PROMOTION_TRIAL=1` make the guards report and
  skip, so the same candidates now report `text-size-differs (+N bytes)` -- 18
  of the 44 are within +/-16 bytes. Never set in the normal build.
- **"build-error" is not a class, it is a refusal to look.** Splitting the
  overlay trial's failures by cause turned 169 undifferentiated errors into 85,
  of which 49 are `schedule-divergence-at-site` (a codegen problem), 15 are
  `resident-symbol-missing` (not an overlay problem at all) and 14 are
  `rom-size`. Two of the spike's named failure classes -- alias coupling and
  non-`.text` sites -- stopped occurring once the generator owned the whole
  block. Changed: `promotion_trial.py` names every cause and `--resume` keys
  results by function instead of appending a second verdict.
- **A filter that silently drops everything looks like success.** The
  synthesizer ignores relocation sites the module's table does not corroborate;
  when *every* site for a symbol was dropped it emitted no value and no
  complaint, and the caller saw only "undefined reference". Changed:
  `synthesize()` reports a symbol whose sites were all filtered. The matching
  tree reports zero, so the complaint only fires on real divergence.
- **The Makefile is not the list of what the link consumes.** Filtering build
  artifacts by whether the Makefile mentions the object's name dropped the 21
  overlay objects that reach the link through a pattern rule, which made a
  chunk of the tracked surface look unreproducible. `mickey.us.ld` names every
  input object explicitly and is the authoritative list.

- **A candidate inside a matched overlay TU un-credits the whole TU.** The
  atlas credits ownership per source file; two far-off "middle function"
  candidates cost 5,024 resolved bytes at merge. Changed: candidates go in a
  per-function file or stay as the pragma; `tools/check_match_regression.py`
  now refuses such a merge; lane prompts carry the rule.
- **A TU that fails to compile with `-DNON_MATCHING` silently drops every
  candidate in it out of the permuter sweep.** fx.c, all of overlay 1 and
  overlay 8 were locked out for days by an undefined type and conflicting
  candidate declarations; the ELF normalizations also ran in the NON_MATCHING
  tree and failed by construction. Changed: `gmake check-nonmatching-builds`
  gate, `RUN_POSTPROCESS` skipped in `build_non_matching`, `--resume` re-runs
  errored rows.
- **The closeness ranking goes stale within hours.** A "2 words" row had a
  permuter base score of 700; scratch fidelity was verified sound, so the
  ranking was the fault. Changed: regenerate `tools/nm_ranking.py` before
  every sweep relaunch; the workbench stamps rankings with the tree hash.
- **Two-thirds of the NON_MATCHING queue is structurally wrong** (246 of 360
  candidates over 32 words; 34 within 8). The permuter cannot fix shape.
  Changed: sweep spends its time on the near third; shape/reshape lanes own
  the rest; the "permuter-stuck flat" list decides whether deeper compiler
  instrumentation is funded.
- **Flat-at-six-minutes was flat-at-twenty on every measured run.** Changed:
  `--flat-minutes 6` early stop (flat = no output strictly better than base),
  ~3x more functions per hour; score-trend extension covers descending runs.
- **Parent-only kills leave permuter worker pools orphaned.** Three batches
  of idle workers after one morning. Changed: process-group kill at the cap
  (host runner and workbench `run_owned`).
- **Overlay promotions need the atlas regenerated or the bytes are never
  credited.** Changed: the runner runs `overlay-atlas-write` + digest and
  stages them on overlay promotions.
- **Static compiler-flag tables are wrong for TUs that look default.**
  `lights.c` carries `-Wab,-r4300_mul`; `gmake -n` (with continuation lines
  joined) is the only authoritative source. Changed: `build_recipe_for()`;
  the earlier 0/38 permuter farm result was discarded as unsound.
- **Lane target lists must come from `#ifdef NON_MATCHING` ownership, not
  from names.** Ranking rows use friendly names, `.s` files use splat
  auto-names; two lanes were assigned already-owned functions. Changed:
  bare = pragma outside any NON_MATCHING block, computed by
  `permute_batch.discover_queue`.
- **Codex conversion by function class.** Tiny resident functions with
  donors match directly (5 gzip/boot in 50 min); mid-size resident bodies
  yield shape candidates only, which the permuter then closes when they are
  within ~10 words (`func_80003760`: 8 words → exact in 74 s); reshaping
  far-off display-list overlay candidates mostly did not move. Changed: the
  pipeline is shape lane → sweep; lanes get a per-function time budget, not
  the three-plateau stop, and a load threshold of 14.
- **The permuter's process load starves anything gating at load 10.** Sweep
  alone sits at 8–11 on 14 cores. Changed: lanes gate at 14; sweep runs two
  4-thread searches.
- **Shared checkouts are not safe for two agents.** Two workbench agents
  collided on one branch switch. Changed: every agent works in its own
  `git worktree`; merges are fast-forwarded from a throwaway worktree.
- **Public mirroring needs its own scrub list.** Agent passes left `CREW-`
  task ids in source comments and integration-branch names in scripts.
  Changed: scan for `/Users/`, `lane/`, Codex/luna/crew/Claude/Opus/Fable,
  `CREW-`, `campaign/unchain`, scratchpad, `.codex` before every public push.
- **"Proven pass-owned" verdicts fell to a sound permuter twice more today**
  (`func_8004D40C`, `overlay31CreateConfig`). Standing rule: no wall verdict
  before a sound sweep of that function; the workbench verdict text now
  routes allocation ties to the permuter.
- **Candidates in one TU drift into conflicting declarations.** Two lanes
  declared the same function with different parameter types (an earlier
  caller candidate's extern vs a later definition), so the merged TU failed
  the NON_MATCHING build; the matched callers' top-level prototype is the
  ABI truth. Changed: candidates adapt to the existing prototype with casts
  inside the body; `merge_lane.sh`/`finish_merge.sh` run
  `check-nonmatching-builds` before committing a merge; lane prompts carry
  the rule.
- **Two lanes editing the same TU always conflict at merge, even on disjoint
  functions.** Neighbouring candidate blocks collide line-for-line and each
  merge cost an agent pass to resolve. Changed: assign lanes by translation
  unit, not by function list; when a TU must be shared, the second lane
  waits for the first to integrate.
- **The permuter is the wrong oracle for overlay functions.** The splat
  target spells every overlay call relocation with a placeholder symbol and
  the real link resolves calls through the module's relocation table, so a
  candidate with identical instruction words still scores hundreds (`overlay18Load`:
  2 real words, permuter base 700). Changed: `tools/promotion_trial.py`
  promotes each candidate in place and classes it by the LINKED ROM
  (exact / text-exact / text-differs / build-error); the sweep runs
  `--resident-only`; overlay routing uses the trial's numbers.
- **Overlay promotions are gated by relocation scaffolding, not C.** Most
  overlay candidates fail to link (undefined `*Reloc` placeholders, digest-
  guarded ELF passes) because every matched overlay function carries a
  bespoke POSTPROCESS rule hand-derived from the target relocation table.
  A generic relocation-surface synthesizer (from the atlas census) is the
  lever for the whole 299 KB pool; a feasibility spike is running.
- **The overlay relocation surface is a pure function of the baserom and
  the atlas.** Modules ship unrelocated, so each placeholder symbol's value
  is the stored addend at its sites (`R_MIPS_26`: `0xF0000000 | imm26<<2`;
  `HI16/LO16`: `(hi<<16)+sext(lo)`; `R_MIPS_32`: the word) minus the
  object's own addend. `tools/reloc_surface.py --audit` reproduced
  1,773/1,773 hand-derived values and 979/982 link-defined ones; 14 of 19
  blocked candidates linked with zero collateral. Changed: the surface is
  generated, not hand-written per function (implementation in progress);
  the bespoke POSTPROCESS ritual stops being the gate on 299 KB.
- **Six Codex lanes plus two permuter passes crashed the workstation** (load
  15–17 sustained; the earlier freeze threshold was ~20 but sustained ~15
  with build bursts was enough). Changed: hard cap of four Codex lanes at a
  time, one permuter pass, total load target ≤ 12; launch lanes one at a
  time, 60 s apart, after `splat extract` of the previous one has finished.
- **An object-level "instruction words identical" verdict is not an overlay
  oracle either.** A lane reported 15 overlay candidates at 0 words by
  diffing objects with relocation-bearing words masked; the linked ROM still
  showed the original 1–4 words, and the 1-word case was plain codegen
  (`addiu a2,a3,2` vs `addiu a2,a1,0x3e`, no relocation). Changed:
  `tools/promotion_trial.py` now records, per in-range word, the target word,
  the built word and the relocation at that site; overlay lanes measure with
  the trial only and commit "trial-exact" when it reports zero.
- **Why the permuter scored overlay functions in the hundreds.** The splat
  target assembles with no relocations at all (`jal <self>`, `lui/addiu …,0`),
  the candidate carries symbols, and decomp-permuter ignores symbol-name
  differences only when both sides carry one. `overlay18Load`: score 700,
  two real words. Changed: `tools/reloc_surface.py`'s `permuter_annotation()`
  rewrites the scratch target at exactly the sites the module's own
  `reloc1`/`reloc2` tables name, and renames the candidate's placeholders to
  the same ROM-derived identities, so both sides render identically there.
  `overlay18Load` 700 -> 400 (62 differing rows -> 2, its two real words);
  `overlay7DispatchSelection` 75 -> 10; `overlay40FadeRecords` 75 -> 25;
  `overlay84AdvanceCurrent` 41 -> 16. `tools/permute_batch.py --overlays-only`
  replaces `--resident-only` as the routing flag; promotion still goes
  through the linked build.
- **Adapting a candidate to a prototype with casts can destroy its shape.**
  `func_8000DDE4` went from 24 words (permuter-ready) to 116 after
  casts/locals were added to satisfy a `u8` prototype. Restoring the
  callee-derived types recovered the retained 24-word candidate, but a green
  default `gmake verify` exercised the guarded assembly fallback for both this
  function and its reconstructed caller; it did not validate the C ABI.
  Changed: establish caller/callee ABI evidence before changing a prototype,
  then prove the candidate in a configured `NON_MATCHING` object before using
  the default linked build as collateral evidence.
- **Overlay candidates must keep resident-target placeholders.** Renaming
  `func_80029FE4`-style names in candidates to the real resident symbols
  looked like a stale-name cleanup; it produced `relocation-truncated
  (R_MIPS_26)` (a direct `jal` from the module's 0xF… VMA cannot reach
  0x8…) and, where matched code shared the name, changed bytes. Overlay code
  reaches resident targets through the module relocation table; the
  placeholder's value must come from the stored site bytes, like every other
  placeholder. Reverted (4c81938c); the generator now derives a per-module
  alias and values it -- see the 2026-08-28 entry, which also corrects the
  trampoline half of this note: the module's own table stores `jal 0`, and
  `0C00CCE8` belongs to the resident `mainRelocTable`.
- **Overlay promotion-trial classes after the generator (2026-08-28 19:40):**
  209/279 candidates measurable (165 text-differs — 21 ≤4 words, 13 at 5–8,
  32 at 9–16; 44 text-size-differs), 70 build errors: 54
  `schedule-divergence-at-site` (real codegen at a relocation site) and 16
  `rom-size`, which is the rodata-ownership class (the promoted TU emits its
  own `.rodata` — jump table or float pool — where retail's module holds it
  at a fixed offset; the yaml `.rodata` ownership carve is the lever, as on
  the six resident functions in August). Route: ≤8-word → permuter with the
  annotated target / hand levers; rom-size → ownership carve; divergence →
  reshape.
- **A "the score improved" claim is not the check that matters; "an exact
  candidate scores 0" is.** The first annotation scheme named a `SYMBOL`
  HI16/LO16 site by the record's `overlayRomTable` entry, which is *not* the
  value stored at the site, so one symbol read through a `SYMBOL` record and
  written through a `LOCAL` one got two names and neither side matched. Every
  before/after number still improved; only re-wrapping an already-matched
  overlay function (`overlay62Initialize`, exact C) as a `NON_MATCHING`
  candidate exposed it -- 150, not 0. Changed: a HI16/LO16 pair is named by
  the link *value* `synthesize()` derives at the site, a symbol whose sites
  disagree is left unannotated rather than half-annotated, and the
  matched-function round trip (score must be 0) is the acceptance test for
  any future change to the annotator.
- **A promoted overlay candidate failed the build before it compiled anything.**
  Splicing a candidate flips that TU's mechanically-derived `nonmatching` flag,
  so `config/overlays.us.json` goes STALE and `overlay_atlas.py --check` --
  a prerequisite of `build/.splat-stamp`, and therefore of everything -- kills
  the build. `commit_match()` regenerated the atlas *after* a successful
  promotion, which a promotion could not reach. `overlay101DrawClock` scored 0
  and was reported as a failed promotion for that reason alone; with the atlas
  regenerated before the build it promotes, verifies and commits. Changed:
  `_promote_locked()` regenerates the atlas after the splice, and restores both
  it and `overlay_undefined_syms.us.txt` on a revert so a rejected candidate
  leaves no dirty generated artifact behind.
- **`objcopy` refuses two `--redefine-sym` arguments sharing a target name,
  and a shell script without a trailing newline swallows the next command.**
  Both failed silently: the renames simply did not happen and the score stayed
  high with no error anywhere. Changed: colliding renames go in successive
  `objcopy` invocations, and the scratch's `compile.sh` is newline-terminated
  before anything is appended (`replicate_objcopy` only terminates its last
  line when it wrote one, so a TU with an unreplicable POSTPROCESS ends
  mid-line).
- **A candidate that already scores zero was reported "no improvement".**
  `__scSchedule` (sched, 488 B) scored base 0 — its words are exact and only
  the jump table's ownership (`jtbl_800823F4`, wrong TU) blocks verify — and
  the runner skipped promotion because no output dir existed. Changed:
  base 0 promotes the base source directly; the ownership carve for sched
  rodata is queued as its own lane.
