# Synthesizing the overlay relocation surface

Implemented. Tool: `tools/reloc_surface.py`. Generated artifact:
`overlay_undefined_syms.us.txt`. Gates: `gmake overlay-syms` writes it,
`gmake check-overlay-syms` fails on drift.

## Function-sized relocation comparison

After the last guarded function in a shared TU is promoted, the atlas replaces
its mixed exact ranges with one fully matched C container. Preflight can still
authenticate an inner function without inventing a function-sized atlas row:
the generated overlay/ROM identity must agree with a unique exact-C owner and
the linked executable section's defined function geometry. Aliases must agree,
the complete function must remain inside that owner, and its end must meet the
next defined function boundary or the owner end without a gap or overlap.
The entire containing executable C range must also equal the baserom. This
authentication is repeated after a build before consuming the resolution;
unchanged function value and size cannot preserve a pre-build container proof.
With `--no-build`, existing freshness checks still fail closed. This
route does not grant matching credit or relax ordinary-source, freshness,
function-byte, frame, or relocation count/type/offset/identity gates.

Matching lanes can compare a full-TU candidate object's static relocations
with the target function's authenticated relocation surface without rebuilding
a second relocation decoder:

```sh
tools/reloc_surface.py compare overlay7DispatchSelection \
  --candidate-object build_non_matching/src/overlays/o007/overlay_007_tail.c.o
```

The default target context is `build/mickey.us.elf`; `--target-elf`,
`--candidate-symbol`, and `--target-symbol` cover scratch builds and the case
where extracted assembly keeps a generated name while C uses a friendly one.
Use `--json` for a compact machine-readable record and `--check` when a caller
needs non-exactness to return status 1.

The report gives the target count, candidate static count, exact
function-relative offset/type alignments, and exact stable-identity alignments.
Stable identities are runtime `(overlay, byte offset)` addresses, not source
names or the shared `0xF0000000` synthetic VMA. Overlay targets continue to
come from their shipped module runtime tables. Resident targets instead come
from the ordinary canonical object's static `.rel.text`: the resident runtime
patch table is sparse and is not a complete static-link surface. Before those
resident tuples are trusted, the tool requires one canonical `build/src`
object, exact linked/object symbol size, and byte identity across the linked
range after masking only that object's relocation words. Every target tuple
must then resolve to one stable runtime identity, and the masked words are
independently checked against their resolved static-link values. The JSON field
`target_surface_source` states which path supplied the target; the older
`target_runtime_record_count` key remains as a compatibility alias for
`target_record_count`.

The command refuses duplicate/missing overlay ownership, an overlay assertion
that disagrees with the atlas, a target span outside its owner, incomplete or
inconsistent runtime HI16/LO16 pairs, conflicting resident source identities,
a missing canonical resident object, object/link range disagreement outside
relocation words, and unresolved or duplicate resident tuples. A copied
scratch object whose path has lost `build*/src/...` context must pass its
canonical source key with `--source`; `--overlay` is only an assertion and
never selects between ambiguous owners.

### Shared identity normalization

`tools/reloc_identity.py` is the common identity layer used by both
`reloc_surface.py` and `function_preflight.py`. It parses GNU objdump relocation
rows into `(section, offset, type, symbol, addend)` tuples, parses successive
`objcopy --redefine-sym` operations, collapses transitive rename chains, and
applies relocation addends to stable `(overlay, byte offset)` identities.
Linker-script identifier aliases and postprocessed object names therefore use
one canonicalization path in both reports. Resident static relocations that
name an ELF absolute assignment use the separate `absolute:0x...` namespace;
their complete link value is preserved instead of being mistaken for a
resident-VRAM offset.

The layer fails closed: a rename cycle, conflicting original sources for one
destination, conflicting linker identities, malformed relocation rows, or
conflicting numeric assignments never receives a guessed identity. Exact
duplicate rename pairs are idempotent. The public comparison fields and human
report remain unchanged; `stable_identity_*` continues to describe identities
proved statically, while `effective_identity_*` may additionally include an
exact linked-ROM/runtime-table proof after canonical promotion.

### Canonical same-overlay generated-call boundaries

A guarded candidate can call another function in its own overlay by that
function's generated name even when no linker-alias row exists for the name.
The shared synthetic VMA still does not authenticate that call. The comparison
layer resolves this narrower case only for `R_MIPS_26`, and only when all of
the following canonical evidence agrees:

- the caller and encoded generated identity name the same overlay;
- one atlas module and one non-overlapping C `text_ownership` row exist, and
  the generated offset is exactly the row's start rather than merely somewhere
  inside a section or broad translation unit;
- the row's offsets and size agree with the module's exact text/ROM ownership;
- the tracked source has a fresh canonical object whose physical `.text`
  extent equals that owner and contains exactly one function symbol at object
  offset zero; and
- the linked ELF has one function symbol with the same generated name, encoded
  overlay offset, overlay section, and canonical-object symbol metadata.

Physical `.text` extent is the boundary authority. Metadata-only trimming may
leave the function symbol's pre-trim `st_size` larger than that extent; this is
accepted only when the canonical object and linked symbol retain the same size
and the physical section still equals the atlas owner. A smaller symbol,
another function in the object, or disagreement in value/section/size is a
conflict and is refused.

Cross-overlay names, non-call relocations, duplicate owners, missing or broad
boundaries, stale/missing source objects, unsafe source paths, and conflicting
object/linked symbols receive no inferred identity (or stop on contradiction).
Objcopy rename provenance is still propagated through the shared identity
layer, so a many-source destination remains ambiguous.

This closes evidence collection, not matching policy. It does not use runtime
row position to guess a callee, does not turn an unresolved identity into an
exact one, and does not relax promotion's independent offset/type, identity,
linked-range, overlay, and full-ROM requirements. On the current Overlay 22
initializer, it authenticates the generated call at function `+0x274` as the
uniquely owned Overlay 22 `+0xD30` boundary. Candidate identity resolution
therefore moves from 20/21 to 21/21 without changing the object or code bytes;
only 11/21 identities currently align with the target, so the function remains
non-exact.

### Repeated same-overlay call proxies

An undefined overlay-local `R_MIPS_26` proxy may lack both a generated callee
name and an exact matched-sibling witness. One target-aligned runtime tuple is
not evidence for its identity: copying that tuple would be circular. The
comparison layer admits the narrower repeated-use case only when all of these
independent candidate and shipped facts agree:

- the target function and candidate TU already have one authenticated overlay
  owner and canonical synthetic-VMA policy;
- one undefined candidate symbol is referenced exclusively by `R_MIPS_26` at
  two or more distinct function-relative sites;
- every candidate site aligns by offset and type with exactly one shipped
  runtime tuple owned by the same overlay; and
- subtracting each candidate instruction's REL addend independently derives
  one identical nonnegative same-overlay base.

A single use, a missing target tuple, a repeated offset, a defined or
mixed-relocation symbol, or any resident, reserved, or cross-overlay selector
remains unresolved. A duplicate target tuple, out-of-range instruction, or
conflicting derived same-overlay bases is ambiguous and fails closed. The proxy name and the
overlays' shared synthetic VMA are never identity evidence. Existing generated
boundary and exact-sibling routes remain unchanged, and disagreement between
an existing route and the repeated-use proof is ambiguous rather than ranked.

The proof binds only the candidate proxy's base. It never moves a call site or
normalizes an offset/type mismatch, and it does not replace exact relocation,
linked-overlay, or full-ROM gates. Overlay 101's border builder is the proving
case: two calls at function `+0x3C` and `+0x6C` independently derive the same
Overlay 101 `+0x2CE4` base, while either call alone remains unresolved.

### Canonical same-overlay LOCAL/data identities

Overlay data placeholders are not identities merely because a linker-script
assignment gives them a small numeric value. The comparison tool first admits
a same-overlay LOCAL/data base from candidate-side canonical evidence. It also
has one narrower runtime-correlated route for undefined proxy externs: a
complete candidate HI16/LO16 pair may use the owned target runtime tuple at the
same function-relative offset/type only when that tuple is unique and every use
of the proxy derives one consistent stable base after removing the candidate's
REL addend. Missing or conflicting pairs remain unresolved. This route never
uses or equates the overlays' shared synthetic VMA; the runtime table's overlay
number or reserved selector is part of the identity.

The ordinary canonical link can retain two symbols with the same name: one ABS
symbol for the generated numeric assignment and one overlay-specific data or
BSS definition. That pair authenticates a candidate name only when the caller's
overlay is known, the assignment is unique, one current canonical object has a
compatible data/rodata/BSS definition at exactly that object offset, and one
linked definition agrees in name, size, section, and synthetic address. When a
friendly candidate name deliberately has no linked hand alias, a narrower
fallback is allowed only if one fresh canonical object owns the linked
overlay's entire BSS section; the assignment is then an exact byte offset in
that sole object. A partial or shared section does not qualify.

An already matched, function-sized function in the same overlay may also act
as a relocation-name witness. This route requires an atlas row explicitly
marked matched and not `NON_MATCHING`, a fresh canonical object whose physical
text is exactly that row, one agreeing linked function symbol, and byte
identity between the linked row and the retail ROM. Only then may an exact
static relocation offset/type pair in that sibling be associated with the
shipped runtime tuple at the same sibling-relative site. This includes an
anonymous `R_MIPS_26` call proxy when the exact sibling's call record uniquely
names its shipped runtime target; the object-side REL addend is removed before
the base identity is reused. Conflicting witnesses make the name ambiguous;
stale, broad-TU, nonmatched, unpaired, or linked-ROM disagreements supply no
identity.

The witness proves a symbol name, not a candidate offset. The candidate's own
relocation offsets remain unchanged and are still compared independently. For
example, Overlay 71's matched `func_overlay_071_F0000278_18C9D98` now proves
the shared initial-resource proxy used by the guarded `+0x870` renderer, while
the renderer's displaced `D_80000008` HI16 remains a real offset/type mismatch
and receives no normalization or inferred pairing.

The runtime-correlated route likewise binds a proxy name without changing any
site. It does not repair a shifted relocation, accept a defined candidate data
symbol without canonical section ownership, or choose between two runtime
identities. Promotion retains the separate linked-module and full-ROM byte
proof, so resolving a proxy cannot by itself turn a candidate into a match.

Overlay 31 demonstrates the ambiguity gate. Its exact siblings use the static
name `overlay31AllocateReloc` for three distinct shipped resident destinations,
so `overlay31CreatePool` receives no call identity from them. One individually
matching sibling tuple is insufficient when another admissible sibling
conflicts; the tool rejects the name instead of choosing the closest target or
the tuple at the candidate's offset.

The function under proof is never its own witness. Re-proving an already
promoted function used to find that function's own canonical object among the
matched rows, and its runtime tuple at a site is by definition the target's
tuple at that site: a one-site copy of the kind the repeated-proxy route
refuses as circular. The witness pass now skips any row overlapping the
target's module extent. `overlay96DrawObject` exposed it. Its C calls the
resident `func_800349A4` (the shipped record at `+0x640` is a `SYMBOL` to
`resident:+0x34554`) under the generated name `func_overlay_096_F0000000_*`,
because a resident call stores a zero jump field and so does a call to the
overlay's own offset-0 function; splat names both after the latter. The
self-witness bound that name to the resident target while the name's own
shape (through its `overlay96Register` alias) said `(96, 0)`, and the proof
refused. Overlay 86 (`overlay86ScaledVectorPosition`) and overlay 20
(`overlay20ReleaseTree`) failed the same way, with a real sibling witness
standing in for the self-witness. That disagreement between the name-shape
pass and the call pass is now an ambiguity, not an abort: the site stays
unresolved, and only the linked-ROM route can account for it. The offset-0
call-proxy spelling is widespread (overlay 20 uses one such name for four
different resident callees), so treat it as a proxy, never as a statement
about the callee. Measured over the 589 promoted overlay functions on
2026-09-23, 156 had a static-exact surface only through the self-witness;
every one still passes `promotion-proof`, now reported as
`static-plus-runtime-table-and-linked-rom` instead of `static`.

A cross-module call must never be spelled with the callee's own name.
`promotion-proof` refused five overlay 65 and 66 functions with "linked
symbol conflicts with canonical overlay ownership", naming a sibling
(`overlay65Initialize`, `overlay66Select`) rather than the function under
proof. Overlay 49 called `overlay65Initialize` and overlay 11 called
`overlay66Select` by those names (overlay 11's POSTPROCESS rename still named
the pre-promotion `func_overlay_066_F0000000`), so the generator wrote a value
line for each, and a linker-script assignment overrides the definition in
every module: both functions linked as `*ABS*`, and the sibling-witness pass
over their overlays refused them. Both sit at module offset 0, so the stored
addend equalled their real address and the ROM still matched; four more
(`overlay1GetEntry`, `overlay27CanUse`, `overlay3RunCachedModeAction`,
`overlay56SplitTime`) do not sit at offset 0 and matched only because no
word in the ROM depended on their clobbered linked address. All six callers now call
through a `*Reloc` placeholder, like a resident call, and
`reloc_surface.py generate` refuses to value any name another module defines
in its own `.text`, reporting it as `UNRESOLVED` instead. Those that
linked as `*ABS*` were invisible to the promotion-proof census as
well, because an `*ABS*` symbol belongs to no overlay section.

A generated name in C is an identity claim, and a POSTPROCESS rename does not
hide a wrong one from the proof. `overlay11InitializeFour` called
`func_overlay_045_F000000C_188B438`, whose ROM suffix disagrees with overlay
45's atlas start plus `0xC` (the real name ends `188C464`); the object renamed
it to `overlay11CreateReloc`, so the link and the ROM were right, but the
proof maps the placeholder back to the C spelling and refused it with
"encoded ROM address conflicts with atlas ownership". The C now calls
`overlay11CreateReloc` directly, as its siblings do, and
`reloc_surface.py generate --check` (and so `gmake check-overlay-syms`) fails
on any generated name under `src/`, `include/`, `mk/` or
`config/normalizations/` whose suffix disagrees with the atlas. Fixing it
exposed a second disagreement behind it: `D_800D31BC` reads statically as a
resident address, while the shipped table reaches the same byte through the
reserved selector `0xFFD`. Reserved selectors stay distinct identities, so a
symbol pass and a data pass that disagree only as resident against reserved
now leave the name ambiguous, for the linked-ROM route to account for; every
other disagreement still aborts.

Declared relocation filters are accounted for by what proves the removed
site, not by its symbol. The accounting accepted only `.bss` HI/LO pairs,
whose identity comes from the linked BSS base, and refused
`overlay86ScaledVectorPosition`, `overlay20ConfigureEntry` and
`overlay46UpdateTransition` with "unsupported filtered runtime identity",
because each filters a *named* symbol (`gOverlay86Vectors` in overlay 86).
The fresh raw object resolves a named site through the same static identity
routes as any unfiltered site, and the raw comparison already requires every
resolved raw record to align with its shipped tuple, so a named filtered site
the raw surface resolved is proved (receipt route `raw-static`); a `.bss` site
still goes through the linked BSS base (`linked-bss-base`); a named site the
raw surface left unresolved is still refused. All three now prove as
`raw-static-with-declared-metadata-filters`.

The promotion-proof census (`gmake check-promotion-proofs`) found two more
proof-input gaps behind those. Eight overlay 1 and overlay 8 functions failed
with "expected one tracked exact atlas range": the proof accepts a
mixed-TU exact range only when its label is the function's own C definition,
and two overlay 1 ranges each covered two functions under an invented label,
while four overlay 8 labels were friendly names `overlay_008.c` never
defines. `MIXED_TU_EXACT_C_RANGES` now carries one row per function, labelled
with its definition; the matched byte totals are unchanged. And a declared
filter may name its sites through `@SPEC_FILE`, which the metadata proof
refused as an unsupported specification; it now expands the file exactly as
`filter_elf_relocations.py` does and pins the file's digest among the
capture's inputs.

The census then ended at 23 failures; 20 were overlay functions whose
recipes the declared-metadata proof did not read. That proof now accepts every
metadata form the build ships -- `objcopy --redefine-sym`, `--add-symbol` and
`--remove-section` (never of `.text`, its relocations or the symbol tables),
`rebind_elf_relocations.py`, `externalize_elf_section.py`, the filters and
the trim, `@SPEC_FILE` arguments included -- and proves the filtered sites
against a *replay*: the Make-expanded recipe run, step by step and in order,
on the fresh raw compile with only the filters left out, so the replay
carries every rename and rebind the configured object does. The accounting
then requires the configured object to differ from the replay by exactly the
declared filter records. Two orderings are restated rather than replayed: a
rename that follows a filter carries the filter's symbol with it, and a
section a filter frees for a later `--remove-section` (overlay 91 filters its
`.rodata` HI/LO, then removes `.rodata`) is removed by the accounting after
the filters. A retained site is the configured object's own record and is
proved as in any promoted function, statically or by the linked-ROM route; a
filtered site is proved by the replay's static surface (receipt route
`replay-static`), the linked BSS base (`linked-bss-base`), or, for a section
the recipe removes, by content: every such site must agree on where the
section starts, that offset must lie in the module's initialized data, and the
ROM must hold the section's exact bytes there, jump-table words relocated
against the TU's text (`removed-section-content`).

Reading those recipes exposed tree defects the filters had hidden, each a C
spelling whose identity disagreed with the site it names:

- Overlay 14's `Ref` idiom (`extern O14ValueC0Ref gOverlay14ValueC0;` with
  padding up to the field) makes the name the module's data base, while
  other overlay 14 TUs declare the same name as the `s32` at data `+0xC0`.
  The static surface saw both meanings and left the name ambiguous. The
  `Ref`-idiom declarations in `overlay14ApplyValues`,
  `overlay14AdvanceCommand`, `overlay14StepCommand` and
  `func_overlay_014_F00013F4_1870CCC` now spell the base as
  `gOverlay14<Field>Base`, following `gOverlay14CommandHeaderBase`.
- `overlay11UpdateModeSix` and `overlay11UpdateTwoOptionMenu` read one C
  macro, `D_INPUT + 0x1C4`, for two retail words: the argument past the BSS
  base and the menu input in initialized data (the `D_menuBase + 0x1C4` of
  their siblings). The data reads are now `OVERLAY11_MENU_INPUT` through
  `D_menuInputBase`, and the counter's base `D_cfgA`, a scalar at a reserved
  selector in every other TU, is `D_menuCounterBase`.
- `overlay28InitializeWork` stored `overlay28ResetBuffer` (module `+0`) in
  its `reset` field; the shipped relocation names the start of overlay 28's
  initialized data (`+0x7F0`). The C now stores `gOverlay28DataBase`.
- `overlay91UpdateTimeline` called one `overlay91CallProxy` for eleven
  retail callees in four identities. Each call now names its callee
  (`func_8002A8C0`, `func_8002A8BC`, `func_80000F94`, `func_80006EA0`,
  `func_overlay_007_F0000DBC_185CC44`).

Every one of those relocations stays filtered, so no name is linked and the
ROM, the alias surface and every word are unchanged. Two proof defects fell
out as well: `classify_source_selection` paired an *unconditional* friendly
definition in multi-function `overlay_007_tail.c` with another function's
guarded `GLOBAL_ASM` (only a guarded definition can be a fallback's
alternative), and a resident function's own `.rodata` jump table
(`func_8005BA40`) resolved through a program-wide `.rodata`: an
externalized overlay section survives in the linked ELF as an `*ABS*` zero
section symbol. Section symbols are never name identities now; a resident
object's own section symbol is placed by the link map
(`build/mickey.us.map`), which must place that object's section once, at its
own size.

Linked BSS follows the shipped relocation blobs, while runtime BSS follows only
text plus data/rodata. The tool therefore proves the linked definition first,
then translates its BSS offset from `ROM-size + object offset` to
`text-size + data/rodata-size + object offset`. Initialized data keeps its
linked module offset. Atlas `data_rodata_ownership` must agree when present.

Every HI16 must have its same-symbol LO16 under MIPS REL pairing semantics;
multiple references are allowed, but an unpaired member supplies no identity.
Conflicting assignments, duplicate definitions or ownership, stale source,
object, or linked ELF, cross-overlay definitions, unsafe source paths, and a
shared synthetic VMA all fail closed. Generated-call authentication remains a
separate `R_MIPS_26` route.

The first five-target remeasurement deliberately reports the previous
target-assisted counts beside the new canonical-only counts:

| target | before resolved / exact-aligned | canonical-only resolved / exact-aligned |
|---|---:|---:|
| `overlay15DrawScreenStars` | 0 / 0 | 0 / 0 |
| `overlay33InitializeBuffers` | 0 / 0 | 0 / 0 |
| `overlay1AllocateRecord` | 0 / 0 | 0 / 0 |
| `overlay7AcquireEntry` | 9 / 9 | 11 / 9 |
| `overlay57HandleModeInput` | 17 / 17 | 7 / 7 |

Overlay 7 gains proof for both schedule-displaced `gOverlay7ActiveTail`
records without using target position. Overlay 57's old 17 were not all
canonical evidence: ten depended on the removed target-row shortcut. Its seven
surviving identities comprise six whole-BSS records plus one independently
authenticated call. The other names are assignments without a unique canonical
linked data owner, reserved runtime identities, or unresolved calls and remain
unknown. All five reports therefore remain partial; this route makes their
evidence honest but does not by itself make the stale plateaus finalizable.

Sections 1-4 are the model and the feasibility evidence (lane
`lane/reloc-synth`); section 5 is what the full implementation turned out to
need and what it measured (lane `lane/reloc-synth2`).

**Question.** Every matched overlay function today carries a hand-derived
`POSTPROCESS` rule and/or a hand-written line in `overlay_undefined_syms.us.txt`.
That bespoke work, not the C, is what gates the 279-candidate overlay pool. Can
the relocation surface for a promoted candidate be derived *mechanically* from
the candidate's object plus the shipped tables, with no per-function hand work?

**Verdict: yes.** `overlay_undefined_syms.us.txt` is now generated in full --
every value line and every alias line -- from `config/overlays.us.json` and the
compiled overlay objects, and the ROM stays byte-identical. 2,928
hand-maintained lines became 1,596 values plus 669 aliases over 630 objects,
with none of the hand file's 8 duplicate names and none of its 168 shadowed
assignments. The measurable overlay candidate pool went from 110/279 to
150/279, and a further 44 that used to be an opaque build failure now report an
exact `.text` size delta.

## 1. The model

`docs/overlays.md` §5.1-5.4 establishes that a module ships **unrelocated**:
`runlinkDownloadCode` patches each site named by the module's own `reloc1` /
`reloc2` tables after the DMA. What the ROM image therefore stores at a
relocation site is not an address but the record's **stored addend** - the value
the runtime adds its base to.

A C translation unit cannot express that. IDO emits an ordinary `R_MIPS_26` or
`R_MIPS_HI16` + `R_MIPS_LO16` reference to a symbol that has no address in this
build. The project's existing answer is a placeholder extern whose *value* is
supplied as a linker-script assignment, so the linked instruction word carries
exactly the addend the retail image carries. `overlay_undefined_syms.us.txt`
says so in its own header: "Raw overlay relocation addends used by adopted C."

The consequence this spike tests is that **the value is not a judgement call**.
For a candidate whose schedule agrees with the target at the site, it is
readable from the ROM:

| relocation | required symbol value |
|---|---|
| `R_MIPS_26` | `SYNTHETIC_VMA \| (stored_imm26 << 2)` |
| `R_MIPS_HI16` + `R_MIPS_LO16` | `(stored_hi << 16) + sign_extend16(stored_lo)` |
| `R_MIPS_32` | `stored_word` |

minus whatever addend the object's own instruction already carries. Subtracting
the object's addend is what lets one base symbol serve many struct-field
references: IDO puts the field offset in the instruction, so only the base
belongs in the linker script.

Three measured facts anchor the model, taken from the decoded tables at run
time (`tools/overlay_tables.py`):

- Every `SYMBOL` `R_MIPS_26` site in every module stores immediate zero. Those
  are the cross-module and resident calls the runtime resolves; the placeholder
  therefore has to link to the module base, which is what the existing
  `--redefine-sym <sym>=func_overlay_NNN_F0000000_...` rules do by hand.
- Every `LOCAL` and `SYMBOL` `R_MIPS_HI16` site stores immediate zero, and its
  `R_MIPS_LO16` partner stores the target's byte offset. That is why the
  hand-written aliases are named `D_<offset>` and are assigned that offset.
- `JUMP` `R_MIPS_26` sites store the target's module offset shifted right two,
  i.e. an ordinary intra-module `jal` at the synthetic VMA already produces the
  shipped word with no fixup at all.

## 2. Mapping an object offset to a module offset

The one thing the addend formula needs is where in the module the object's
`.text` sits. `config/overlays.us.json` answers it directly: it records one
contiguous `text_ownership` row per source file, so

    module_offset = row(source_stem).offset + object_text_offset

with no name convention, no heuristic, and - importantly - no linked ELF. The
link is exactly what is missing when a promotion fails to resolve, so an earlier
draft that read `build/mickey.us.elf` was wrong in the only case that matters.

A site the module's relocation table does not name is not a relocation site in
the shipped image; reading an addend there reads an instruction word. When any
site for a symbol *is* corroborated by the table, the tool ignores the ones that
are not. That is the ROM's own statement, not a heuristic, and it is what makes
the procedure tolerant of a candidate whose schedule diverges away from the
sites in question.

## 3. Evidence

### 3.1 Replaying the hand-derived surface

`tools/reloc_surface.py --audit` runs the procedure over every overlay object
the Makefile links and scores the synthesized values against the tracked file:

    tracked-value replay: 1773/1773 agree (100.0%), 0 disagree,
                          982 not tracked, 0 unresolved, 27 stale objects skipped

That is the honest test the spike was asked for, at far greater scale than three
hand-picked functions: every symbol value in `overlay_undefined_syms.us.txt`
that the current build actually exercises - including the heaviest bespoke
rules, `overlay_008.c.o` (32 values behind 46 hand-written `--redefine-sym`
arguments), `overlay_001_tail.c.o` (62), `overlay50Initialize.c.o` (17), and
`overlay_009.c.o` (6 plus a 40-line filter spec) - is reproduced exactly from
the baserom and the atlas, with zero refusals.

### 3.2 Replaying the link itself

The 982 symbols the tool values that are *not* in the tracked file are ones the
link defines by other means: other overlays' functions, resident functions, and
absolute aliases from `undefined_syms_auto.us.txt`. Compared against the linked
ELF's own symbol values:

    linked-ELF cross-check: 979 agree, 3 disagree, 0 undefined

The three are `D_80000039`/`D_8000003A`/`D_8000003B` in
`overlay41UpdateColorRecords.c.o`, referenced only by an `R_MIPS_LO16` with no
`R_MIPS_HI16` partner. A lone `LO16` site does not observe the upper half, so
the tool reports the low half. The emitted instruction word is identical either
way; the value differs, the ROM does not.

Total independent agreement: **2752 relocation-symbol values reproduced, three
partial in an unobservable half, none wrong.**

### 3.3 Unblocking candidates that could not link

`tools/promotion_trial.py` classifies all 279 overlay candidates:

| class | count |
|---|---:|
| `text-differs` (already links) | 110 |
| `build-error`: undefined reference to a placeholder | 100 |
| `build-error`: `cannot grow .text` | 31 |
| `build-error`: `refusing to trim nonzero bytes` | 22 |
| `build-error`: digest / truncated relocation | 5 |
| `build-error`: other | 11 |

The undefined-reference class is the one this procedure addresses. Nineteen were
tried end to end - promote, compile, synthesize, rename the undefined symbols to
per-object aliases, emit the assignments, relink, diff the ROM:

| result | count | functions |
|---|---:|---|
| links, ROM produced | 14 | `overlay1ActivateObject`, `overlay1AdvanceGauge`, `overlay2QueryNode`, `overlay5InitializeAudio`, `overlay13DrawActive`, `overlay13UpdateRecord`, `overlay17CalculateEndpoints`, `overlay20UpdateObjectResource`, `func_overlay_002_F0000C90_1857A88`, `func_overlay_014_F00009F4_18702CC`, `func_overlay_022_F0000A7C_1878B84`, `func_overlay_022_F0000D30_1878E38`, `func_overlay_027_F0000624_187BFFC`, `func_overlay_029_F00010C4_187E374` |
| still fails | 5 | `func_overlay_007_F0000324_185C1AC`, `overlay15InitStars`, `overlay1InterpolatePath`, `overlay1MeasureCurves`, `overlay27UpdateCoordinates` |

Every one of the 14 produced **zero out-of-range differing bytes**: the
synthesized surface disturbs nothing outside the promoted function. The
remaining in-range word counts range from 7 to 196 and are genuine codegen
differences - which is the point. Those candidates now have a linked-ROM oracle
and an exact in-range word count where before they had a link error.

For `overlay20UpdateObjectResource`, fresh configured full-TU C proves exact
`R_MIPS_26` offsets/types for all four calls, but no fresh linked-C trial exists;
current linked equality therefore proves the assembly fallback only. Its four
runtime-table SYMBOL calls are authenticated at function
`+0x14/+0x30/+0x134/+0x168`: resident ORT 202 (`trackGetTrack`), resident ORT
203 (`func_8000FEEC`), Overlay 20 ORT 1690 (`+0xA8`,
`overlay20ConfigureResource`), and resident ORT 101 (`sqrtf`). Fresh C must
preserve those identities; fallback placeholder names do not prove them.

Overlay 20 `+0x1018` (`overlay20RemoveEntry`) owns ten runtime-authenticated
HI16/LO16 sites: entry count at `+0x04/+0x08`, entries at `+0x18/+0x24`, shift
entries at `+0x5C/+0x60`, marker end at `+0x88/+0x8C`, and active bits at
`+0x98/+0x9C`. Runtime metadata resolves those pairs respectively to Overlay
20 `+0x1490`, `+0x14E0`, `+0x14E0`, `+0x19BC`, and `+0x1494`; metadata-only
evidence aliases make all 10 candidate offset/type/identity tuples mechanically
exact without changing the generated canonical fallback symbol surface.
Retained genuine C expresses all ten, while fallback assembly retains only the
last four. ORT 1174 exports it, and resident runtime record 16 at
`func_80007118+0x424` is the sole authenticated inbound. The owned
`+0x1018..+0x10EC` range has no target padding; linked equality proves fallback
only.

The same qualification applies to `overlay5InitializeAudio`: retained genuine
C is current-layout but diagnostic 211/233. It owns 67 SYMBOL and four LOCAL
records: 29 calls and 21 HI16/LO16 pairs. ORTs 1559/1561 and 1565/1568 were
collapsed behind two friendly names and now have distinct source identities;
all 16 resident callees also require per-overlay zero-carrier aliases. ORT 1558
exports the function and Overlay 18 table-1 record 3 at `overlay18Load+0x14` is
its sole inbound. Exact linked function/module/ROM evidence uses the fallback.

For completeness, at the time of the spike the five candidates named in the brief
(`overlay7DispatchSelection`, `overlay8ScaleOutputs`, `overlay18Load`,
`overlay20BuildTileCommands`, `overlay1CloneRecord`) already link on the
existing surface: promoted, they build first time and reproduce the trial's
2/2/2/4/3 in-range words with no synthesis at all. Their residual is codegen,
not relocation surface. This is historical oracle evidence, not current queue
status; later exact promotions such as `overlay8ScaleOutputs` supersede their
listed residual. The blocked pool was the 100 undefined-reference candidates,
not these.

## 4. Limits

The five failures are three distinct classes, and none of them contradicts the
addend model.

1. **Schedule divergence at the site** (`func_overlay_007_F0000324_185C1AC`,
   `overlay15InitStars`). The candidate's instructions differ *at* the
   placeholder's own sites, so no consistent addend exists and the tool refuses
   rather than inventing one. It reports the conflicting values per symbol,
   which localizes the divergence. This is the model's stated precondition: the
   addend is only readable where the schedule agrees. §4.1 narrows that
   precondition from "the same offset" to "the same position in the shipped
   order", which is what the table actually asserts.
2. **Alias-block coupling** (`overlay1InterpolatePath`, `overlay1MeasureCurves`).
   `overlay_undefined_syms.us.txt` also carries 624 hand-written *alias* lines
   of the form `func_overlay_001_F0000CA8_184D088 = overlay1InterpolatePath;`
   so that other TUs' generated assembly can reach a friendly name. Promoting
   one function in a shared TU changes which symbols that TU defines and can
   strand such a line. A synthesizer that owns only the value lines cannot fix
   this; it has to own the alias block too, which is mechanical from
   `text_ownership`. It does now: §5.3.
3. **Relocation sites outside `.text`** (`overlay27UpdateCoordinates`). The
   prototype scans `.rel.text` only. A jump table or initialized pointer in
   `.data` carries the same kind of site and needs the same treatment against
   the `data_rodata` range. In the full run no candidate failed this way -- the
   class the trial reports for `overlay27UpdateCoordinates` is now
   `schedule-divergence-at-site` -- but the generator still scans `.rel.text`
   only, so the limit stands.

### 4.1 Order, not offset: aligning a shifted site

The refusal above was stricter than the evidence requires. A site was
corroborated only when the module's relocation table named its exact
function-relative offset, so a candidate that is a handful of words off, but
moves one `lui`/`addiu` pair by a slot, had no readable addend *anywhere* --
even at the sites it did not move. The trial then reported `build-error`, and
the routing read an unmeasurable row as structural.

What the table asserts is stronger than a set of offsets. `runlinkDownloadCode`
walks `reloc1`/`reloc2` in offset order and patches each site in turn (§1), so
the table records which relocations a function performs *and in what order*.
Instruction scheduling moves the sites; it cannot reorder the references
themselves, because each one belongs to the expression that produced it.

`align_sites` therefore matches an object's relocation sites onto the module's
records over the same text range by relocation type, both sequences in offset
order, and accepts the result only when it is the **only** order-preserving
match -- decided by computing the leftmost and the rightmost embedding and
requiring them to agree. Three things are refusals, not fallbacks:

- a candidate type the retail order cannot supply, in that order;
- more candidate sites than the range has records (a differing multiset);
- two or more placements that both preserve order, where choosing one would be
  inventing an addend.

Two properties keep this from weakening any proof. An object whose sites all
corroborate where they stand is not realigned at all, so the tracked surface is
untouched: `gmake check-overlay-syms` reports it up to date, and `gmake verify`
still prints `507341c0a40ca3e9a7cee969b396ee53facfb548`. And an accepted
alignment is *reported*, never absorbed: the generator prints
`/* ALIGNED <object>: <aligned>/<sites> site(s), <n> shifted */` and the trial
row carries `aligned_sites`/`shifted_sites`, while `in_range_words` counts the
words a shifted site moved exactly as the differences they are. The linked-ROM
byte comparison remains the only proof that a candidate is right; the alignment
only decides whether that comparison can be run at all.

The acceptance test in the trial's docstring still holds under the aligner: a
matched function re-wrapped as a candidate (`overlay63Initialize`) reports
`exact in=0 out=0`, and its object needs no alignment.

Two further limits are worth recording because they are invisible until they
bite:

- A lone `R_MIPS_LO16` determines only the low half of its symbol's value
  (§3.2). Byte-identical output, non-canonical symbol value.
- `overlay_undefined_syms.us.txt` currently assigns eight symbol names twice
  (`gOverlay100Entries`, `gOverlay1ModeObject`, `gOverlay1SubmitArg5`,
  `gOverlay4Groups`, `gOverlay59Entries`, `gOverlay77CallbackArgument`,
  `gOverlay77Handle`, `gOverlay77Selection`). `ld` takes the last, and so does
  the tool, so the audit is consistent - but a generated file would not have
  shadowed lines at all.

## 5. The full implementation

The spike proved the addend model. Building the generator on top of it turned
up three corrections to that model, all of them cases the spike's sample had
not exercised.

### 5.1 The object list comes from the linker script

The spike filtered build artifacts by whether the Makefile mentioned the
object's name, to skip stale ones. That silently dropped the **21 overlay
objects that reach the link through a pattern rule** and are never named
literally -- overlay 34's, 35's, 94's, 104's and 105's among them, which is
exactly why a handful of tracked values looked unreproducible. `mickey.us.ld`
names every input object explicitly, so it is the authoritative list: 630
objects, not 609. Replaying the historic hand-maintained file against the
complete list scores **1,840/1,840 values agree, 0 disagree**, up from the
spike's 1,773 on the incomplete one.

Completeness is now enforced rather than assumed. Generation and audit stop
with the missing linker paths if any of those 630 objects is absent; a partial
build can no longer silently produce a plausible but incomplete tracked
surface.

### 5.2 A value line is also needed for symbols this build *defines*

The spike only valued **undefined** symbols. The hand-maintained file also
assigned symbols the C defines -- `gOverlay77PositiveDivisor`,
`gOverlay57Countdown`, `gOverlay79RaceFlags` -- and dropping those changed ten
instruction words in overlay 77.

The reason is the same one §1 gives. The module's data is placed by the
*runtime*, not by this link, so a reference to it must carry the stored addend
(its offset within the module's data region) rather than whatever address `ld`
happens to give the definition. A linker-script assignment overrides the
definition, which is what makes that expressible at all.

The one reference that needs no assignment is an **intra-module call**: a JUMP
record stores the target's module offset shifted right two, which is exactly
what the assembler emits for that symbol at the synthetic VMA. So the rule is:

> value every symbol a relocation names, except one defined in this module's
> own `.text`.

That is derivable without a link -- the module's own objects say which names
they define in `.text` -- and it covers resident functions, other modules'
functions, and this module's own data uniformly.

### 5.3 An aliased identity must not also carry a value

`func_overlay_045_F000000C_188C464` is both a cross-module call placeholder
(wants the addend `0xF0000000`) and the generated identity of
`overlay45CreateDescriptor` (wants the real address). The hand file carried
both lines and `ld` silently took the last, which happened to be the alias.
The generated block emits the alias only, so the file has **no duplicate
names**: the 8 the hand file assigned twice, and the 168 values it shadowed
with an alias, are simply not written.

### 5.4 The `.text` extent, as a report instead of a failure

`trim_elf_section.py $@ .text <size>` appears in 588 Makefile rules and the
size is the `text_ownership` row's own extent. A promoted candidate whose
codegen is a different size therefore trips the trim guard at *compile* time,
and the build dies before the link -- which is why 53 of the 279 candidates
used to report nothing but `cannot grow .text` or `refusing to trim nonzero
bytes`.

`tools/postprocess_guard.py` gives every digest-guarded POSTPROCESS pass a
report-and-skip mode, enabled by `PROMOTION_TRIAL` in the environment
(`gmake PROMOTION_TRIAL=1`, or `tools/promotion_trial.py`, which sets it for
its own builds). The guard prints one marker line and skips its pass:

    PROMOTION-TRIAL: text-size-differs (+24 bytes): ... refusing to trim
    nonzero bytes from .text

The resulting ROM is not a valid build -- a skipped normalization leaves the
object un-normalized -- and is never verified; the trial classifies from the
marker. **The normal build never sets the variable**, so `gmake` and
`gmake verify` are untouched, and a usage error is never skipped, so a harness
bug cannot hide behind a green build.

### 5.5 Integration: the window between compile and link

The surface can only be derived once the candidate's object exists and before
the link resolves it -- a window a plain `gmake` does not offer.
`tools/promotion_trial.py` therefore builds, regenerates
`overlay_undefined_syms.us.txt` from the objects on disk, and builds again; the
second pass relinks only. Both the source file and the surface are restored
afterwards.

### 5.6 A resident call is an ordinary addend with a name that is not free

`tools/promotion_trial.py` reported 15 candidates as `resident-symbol-missing`
and 4 as `relocation-truncated (R_MIPS_26)`. Both classes have one cause, and
it is not the addend model.

**What the ROM stores.** Section 5.2's census says a module's `SYMBOL`
`R_MIPS_26` record stores immediate zero, and it makes no distinction between a
target in another module and a target in the resident segment. Measured at the
sites themselves, that holds: overlay 49's three resident calls -- the ones the
Makefile rebinds by hand to `overlay65UpdateReloc` -- are `SYMBOL` `R_MIPS_26`
records at module offsets `0x218`, `0x2c0` and `0x314`, and each stores
immediate zero. The shipped word is `jal 0`; the runtime patches it, exactly as
it patches a cross-module call. The trampoline encoding `0C00CCE8` that 370 of
the 375 *`mainRelocTable`* entries carry (§5.3) belongs to the **resident**
segment's calls *into* overlays, not to a module's calls out of one, and does
not appear at these sites. A resident **data** reference is the other measured
case and needs no patch at all: its `HI16`/`LO16` pair stores the real resident
address, which is why the surface already carries lines like
`D_80000040 = 0x80000040` and why they are correct as they stand.

So a resident call wants the same value every other cross-module call wants,
`0xF0000000`. Nothing about the addend is special.

**What is special is the name.** Adopted C spells the call with splat's
resident auto-name, `func_80029FE4`. That name is global and shared with the
resident segment. A value line for it does not give the overlay an addend, it
*moves the resident function for every resident caller*: assigning
`func_80034448 = 0xf0000000` turns `models.c`, `level.c`, `menu.c`,
`texLoadTextureAddr.c` and four asm objects into `relocation truncated to fit:
R_MIPS_26`. That is the whole of the `relocation-truncated` class, and it is
also why the earlier attempt to rename these placeholders to the *real*
resident symbols failed the other way round: a `jal` from the module's
synthetic `0xF0000000` VMA cannot reach a `0x8…` address either. Neither the
overlay's name nor the resident's name can carry both meanings.

The auto-name shape is not the defining property, either. `overlay5InitializeAudio` calls `alHeapDBAlloc`, `osCreateMesgQueue` and `n_alCSPSetMessageQ` --
ordinary libultra globals -- and valuing those breaks `audio_manager_1050.c`
and the whole libultra link the same way. What the two shapes have in common is
that the **resident side of the link owns the name**, so that is what the
generator measures: `resident_defined_names()` collects the global symbols
defined by every non-overlay object `mickey.us.ld` names, plus the names
`undefined_funcs_auto`, `undefined_syms_auto` and `libultra_undefined_syms`
assign -- 4,886 of them, available before the link and without it. A call to
another *module's* function is deliberately excluded: that is the
generated-identity alias case of §5.3 and must keep its own name.

**The fix is the rebind the tree already does by hand.** Overlay 49's
POSTPROCESS rule rebinds the relocation at `0x218` from `func_800254FC` to
`overlay65UpdateReloc`, a per-module placeholder, and values *that*. The
generator now derives the same thing: `reloc_surface.py generate` renames every
undefined `R_MIPS_26` relocation against a resident-owned name to
`<name>_o<NNN>Reloc` in the object, then values the alias from the site like
any other placeholder. The resident name keeps its real address; the overlay
keeps its stored addend.

Three properties make it safe to run inside `generate --write`:

- **It is a no-op on the matching tree.** No overlay object in the current
  build carries an `R_MIPS_26` against a resident auto-name -- every matched
  resident call is already rebound by a hand-written rule -- so the generated
  block is byte-for-byte what it was, `--audit` stays at 100%, and
  `check-overlay-syms` reports no drift.
- **Checks are read-only.** The alias no longer matches the resident-name
  pattern, so a second write pass renames nothing and values the alias from the
  same site. `generate --check` never invokes `objcopy`; if an object still
  needs a resident-call rebind, the check names the object and required
  `POSTPROCESS` mapping and fails. `--compare` is also read-only.
- **It names what it cannot read, and a refusal is total.** Only `R_MIPS_26`
  sites are aliased. A resident symbol reached by *both* a call and a data
  reference is refused -- one placeholder cannot carry two different addends --
  as is a call in an object with no `text_ownership` row, which has no module
  offset. A refused name must then get **no value line under its global name
  either**, and that is the trap that made the first cut of this change worse
  than the problem it fixed: `synthesize()` still reads an addend for the
  refused symbol from its corroborated sites, and emitting
  `func_8002A8C0 = 0xf0000000` to help one overlay produced thirty-odd
  `relocation truncated to fit` errors in `shadows.c`, `camera.c`,
  `charControl.c` and `track.c`. `generate()` now drops a refused resident name
  from every object's values, the reason is reported once, and the trial
  classes the candidate `resident-call-unreadable`.

  **Corroboration is a note, not a refusal.** A call site the module's table
  does not name is where the candidate's schedule has diverged. Under the
  alias, an addend read there can only produce a differing word *inside* the
  promoted function -- which is the measurement the trial exists to take -- so
  the generator emits the value and a `/* NOTE … */` line naming the offsets
  rather than throwing the symbol away. This was measured, not assumed: the
  strict version cost `overlay34SortAndDraw` its 168-word reading and returned
  it to a bare build failure. Notes are printed under their own marker so a
  caller matching `UNRESOLVED` does not read a measurable candidate as a
  failure.

Overlay 59's entry preparer demonstrates both roles in one body. Its calls at
module `+0xB8/+0x140` are local `JUMP` records to `overlay59Release`, while
`+0xD4/+0x104` are resident `SYMBOL` records to `func_80034448`. All four store
zero and collapse to one symbol in the synthetic target object. The candidate
therefore uses separate per-module aliases valued at `0xF0000000`, with
postprocessing preserving the target object's collapsed surface; assigning
the resident global itself would corrupt every resident caller of
`func_80034448`.
The function's other two records are LOCAL HI16/LO16 at function
`+0x08/+0x10`, resolving through base `+0xA20` plus stored addend `+0x5A4` to
module `+0xFC4` (`gOverlay59DescriptorTables`). Runtime therefore requires six
distinct roles; fallback and the synthetic target expose only the four
collapsed calls, and the current split-carrier C has not yet emitted an
authenticated relocation table. The function is unexported. These are table-2
records 0/1 (descriptor pair), 2/3 (release JUMPs) and table-1 records 1/2
(ORT 135 acquisitions). Four local inbounds are table-2 records 25/28/29/30
from `overlay59Advance+0xAC/+0x224/+0x2A8/+0x300` (module
`+0x418/+0x590/+0x614/+0x66C`). The owned range is `+0x70..+0x168`, ROM
`0x18B87C0..0x18B88B8`, with no target padding; the next function starts at
`+0x168`, while `+0xA1C..+0xA20` is separate overlay padding.

Overlay 59's advancer at `+0x36C..+0x784` has 15 shipped runtime records and
the configured candidate emits 15. Thirteen candidate offsets/types coincide
with runtime and twelve of those retain the required identity: the entry-base
high half, retained switch-table pair, four calls to the entry preparer, the
table-info pair, the release call, and the entries-end pair. The prologue
schedule displaces the factor pair from target `+0x30/+0x54` to candidate
`+0x08/+0x0C`; candidate `+0x54` then carries the entry-base low identity
instead of the target factor identity, while target entry-base low `+0x50` has
no candidate record. This is relocation evidence for the scheduling plateau,
not a promotable surface. ORT 1435 and two resident calls authenticate the
owner and ABI; linked equality remains assembly-fallback-only.


#### What it measured

Re-trialled: the 56 overlay candidates whose `NON_MATCHING` body names a
resident target (splat auto-name or `D_8…`), which is a superset of the 15
`resident-symbol-missing` and every `relocation-truncated` case among them.
**`resident-symbol-missing` and `resident-call-unreadable` are now zero.** The
15 the lane was pointed at:

| candidate | before | after |
|---|---|---|
| `overlay1UpdateAimedTransient` | `resident-symbol-missing` | `text-differs` 59 words (2 out of range) |
| `overlay4UpdateObjectMotion` | `resident-symbol-missing` | `text-differs` 15 |
| `overlay5InitializeAudio` | `resident-symbol-missing` | `text-differs` 22 |
| `func_overlay_011_F0001E4C_186A694` | `resident-symbol-missing` | `schedule-divergence-at-site` |
| `func_overlay_011_F00022E8_186AB30` | `resident-symbol-missing` | `schedule-divergence-at-site` |
| `overlay11UpdateMenu` | `resident-symbol-missing` | subsequently matched: 301/301 words and 102/102 runtime relocation identities |
| `func_overlay_026_F0000D24_187B11C` | `resident-symbol-missing` | `text-differs` 39 |
| `func_overlay_027_F0000064_187BA3C` | `resident-symbol-missing` | `schedule-divergence-at-site` |
| `func_overlay_029_F00005C4_187D874` | `resident-symbol-missing` | `schedule-divergence-at-site` |
| `overlay34CreateRecord` | `resident-symbol-missing` | bounded C: 30 relocation-aware positional differences; exact 12/12 offset/type/identity surface |
| `overlay37RenderEffect` | `resident-symbol-missing` | `text-differs` 138 |
| `func_overlay_041_F0000854_1887B8C` | `resident-symbol-missing` | `rom-size` |
| `func_overlay_046_F0000120_188E518` | `resident-symbol-missing` | `rom-size` |
| `func_overlay_071_F0000278_18C9D98` | `resident-symbol-missing` | subsequently matched: 332/332 words and 6/6 runtime relocation identities |
| `overlay94UpdateController` | `resident-symbol-missing` | `text-differs` 13 |

`overlay11UpdateMenu` owns Overlay 11 `+0x1398..+0x184C` with no padding and
exactly 102 runtime records: table-1 134..193 are 60 SYMBOL records; table-2
172..213 contain 36 LOCAL address records, five LOCAL JUMPs, and one same-
overlay SYMBOL. Its 76 address records cover resident/BSS identities `D_0`,
`D_cfgA/B/C`, `D_flags`, `D_count`, `D_table`, `D_paramA/B/C/D`, `D_modeFlag`
and local `D_1B8/D_1BC/D_1C4/D_1CC/D_204/D_menuBase+0x1C4`; all candidate
offsets/types agree. The 26 calls are `+0x08` to resident `func_80028F54`,
`+0x84/+0xA8/+0xF0/+0x100` to `func_80000F94`, `+0x13C` to Overlay 45
`+0x1BF4`, `+0x160` to `func_8002554C`, `+0x1B8` to Overlay 66 `+0`,
`+0x1C0/+0x1C8/+0x1D8` to `func_800290AC/func_800291D8/func_800006BC`,
`+0x1E0` to Overlay 11 SYMBOL `+0x2BF4`, LOCAL JUMPs
`+0x208/+0x398` to `+0x1058`, `+0x220/+0x3B0` to `+0x1130`, and `+0x3E4`
to `+0x2948`, plus `+0x25C/+0x41C` to `func_80005820`,
`+0x328/+0x370/+0x46C/+0x490` to `func_80028374`, `+0x330/+0x474` to
`func_80028528`, and `+0x3DC` to `func_8003A754`. The function is unexported;
its sole inbound is table-2 LOCAL JUMP record 68 at module `+0x930` from
`func_overlay_011_F0000150_1868998+0x7E0`. The promoted C object is 301
instructions with the exact `0x48` frame and no padding, and mechanical surface
comparison proves all 102 offsets, types, identities, and addends; naming the
Overlay 66 `+0` endpoint as `overlay66Select` closed the last formerly
ambiguous call. The two relocation-masked words that remained at
`+0x138/+0x140`, where the index and handle spill stores were reversed around a
call, are closed: see `docs/ido-learnings.md` for the disambiguation fact and
the scheduler line tie that decide them. `gmake promotion-proof` records
`301 words, frame=0x48, relocations=102/102,
identity=static-plus-runtime-table-and-linked-rom`.

Nine now carry an in-range word count and a linked-ROM oracle. Four are
`schedule-divergence-at-site`, which is the honest answer and a codegen problem:
`func_overlay_029`'s two remaining refusals are `ext_o0_2a4c0` and
`ext_o0_6ec00`, each demanding two and three different addends from sites that
disagree. Two moved to `rom-size`, which is a different scaffolding question.

`relocation-truncated (R_MIPS_26)` also disappears: the one case in this set,
`func_overlay_066_F00004E0_18C6948`, is now `text-differs` at 181 words. The
class is closed by construction -- it existed only because a resident-owned
name was being assigned -- but only the candidates whose source names a
resident target were re-trialled here, so a full `--overlays-only` sweep is what
would confirm the other three.

Two side effects worth recording. `overlay34SortAndDraw`, which is *not* one of
the 15, went from `text-differs` 168 to `resident-call-unreadable` and back to
168 across the two refusal policies -- it is the measurement that settled §5.6's
note-versus-refusal question. And `func_overlay_057_*`'s divergence report now
names `func_800508B4_o057Reloc` rather than a bare placeholder, which is the
aliasing showing its working.

#### A stale log is a wrong class

Six of the candidates above stayed in `resident-symbol-missing` after the
surface had already valued them, because `promotion_trial.py` classified from
`log1 + log2`. The first pass links against the *stale* surface -- that is the
whole point of the two-pass integration (§5.5) -- so its `undefined reference`
lines survive into the concatenation and `UNDEF_RE` reported symbols the second
link resolved perfectly well. Markers still come from both passes, since a
POSTPROCESS marker is printed by the compile; every *link* diagnostic now comes
from the second pass alone.

## 6. What the trial measures now

`tools/promotion_trial.py --overlays-only`, all 279 overlay candidates, lane
`lane/reloc-synth2`:

The counts and per-function scores below are the historical linked,
relocation-masked spike result. They are not the later raw isolated ranking;
for example, both Overlay 1 angle scans show four schedule words here but five
raw positions when their unresolved local-data addend is included.

| class | before | after |
|---|---:|---:|
| `text-differs` -- links, N words differ in range | 110 | **150** |
| `text-size-differs` -- links, with an exact size delta | 0 | **44** |
| `build-error` | 169 | **85** |

Nothing came out `exact` or `text-exact`; every candidate that links has real
codegen work left, which is the point -- what changed is that 194 of 279 now
carry a number instead of a build failure.

The 150 that link, by in-range words:

| words | 1-2 | 3-4 | 5-8 | 9-16 | 17-32 | 33-64 | 65+ |
|---|---:|---:|---:|---:|---:|---:|---:|
| candidates | 7 | 11 | 13 | 30 | 22 | 27 | 40 |

The 44 size reports, by delta: 24 are shorter than the module owns and 19
longer (one guard reports a non-size digest); **18 are within ±16 bytes**, the
range where a codegen nudge is plausible. The extremes (`-472`, `+456`) are
candidates whose shape is wrong, not their scheduling.

The 85 remaining build errors, by cause -- each needs different work, which is
why they are named rather than pooled:

| cause | count | what it means |
|---|---:|---|
| `schedule-divergence-at-site` | 49 | the candidate's instructions differ *at* a placeholder's own sites, so no consistent addend exists. §4.1; the synthesizer reports the conflict rather than inventing a value |
| `resident-symbol-missing` | 15 | an undefined splat auto-name in the resident address space. Not a relocation-surface problem: the tree does not define that function yet |
| `rom-size` | 14 | the image's own size moved and no guard marker explains it |
| `relocation-truncated` | 4 | the reference does not fit its field at the synthesized value |
| `unresolved-placeholder` | 3 | undefined with none of the above |

`schedule-divergence-at-site` is now the dominant class, and it is the honest
one: it says the candidate does not yet agree with the target *where the
relocation table can see it*, which is a codegen problem, not a scaffolding
problem. The spike's alias-coupling and non-`.text` failure classes are gone --
`overlay1InterpolatePath` and `overlay1MeasureCurves` link now, because the
generator owns the alias block (§5.3).

### 6.1 The 31 candidates within 8 in-range words

    1  overlay80InitializeContact          o80    4  overlay1FindNextAngle              o1
    1  overlay97InitScale                  o97    4  overlay1FindPreviousAngle          o1
    2  overlay18Load (historical)          o18    4  overlay20BuildTileCommands (hist.) o20
    2  overlay7DispatchSelection           o7     4  overlay3FindClosestObject          o3
    2  overlay84AdvanceCurrent             o84    4  overlay43FilterImage               o43
    2  overlay8ScaleOutputs                o8     4  overlay62Update                    o62
    2  overlay99RenderSegments (historical) o99   4  overlay84LoadCurrent               o84
    3  overlay101DrawClock (historical)    o101   6  func_overlay_022_F0000000_1878108  o22
    3  overlay1CloneRecord                 o1     6  func_overlay_041_F0001650_1888988  o41
    3  overlay40FadeRecords                o40    6  overlay68PromoteSecondary          o68
    4  func_overlay_014_F0000000_186F8D8   o14    6  overlay74Update                    o74
                                                  6  overlay7CommitSelection            o7
    7  func_overlay_022_F0000A7C_1878B84   o22    8  func_overlay_009_F0000540_1866BB8  o9
    7  func_overlay_038_F0000000_1885D10   o38    8  func_overlay_073_F0000000_18CAAC0  o73
    7  overlay14ResetMode                  o14    8  overlay1AssignRecordIndex          o1
                                                  8  overlay1ResolvePathPoint           o1
                                                  8  overlay20UpdateObjectResource      o20

`overlay84LoadCurrent`'s four-word row is historical. A bounded
annotated-target permutation produced exact 72-word C, promoted in
`306a1c31`. Its five runtime relocation roles, linked owned range, complete
Overlay 84 image, and preserved full ROM are byte-identical. It no longer
belongs in the `NON_MATCHING` queue.

`overlay101DrawClock`'s three-word row is also a historical pre-promotion
snapshot. Canonical source is unguarded and the tracked promotion owns exactly
`+0x332C..+0x36E4` (238 words) with a `0x98` frame; current postprocessing is
only a symbol rename and zero-tail trim. The old candidate row is not current
proof. A future exact reproof must retain the configured object and record each
runtime relocation site's type, operation, addend, and stable target identity;
the export row alone does not authenticate its five-argument ABI or callees.

`overlay99RenderSegments`'s former two-word row is superseded. Configured
`-O2 -mips2 -32 -Wab,-r4300_mul` C is exact across Overlay 99
`+0xBA4..+0xDDC`: 142 words, frame `0xA8`, and no padding. Its 15 records
agree with the runtime table by offset, type, effective identity, and addend.
The calls resolve to `viGetCurrentSize`, `rcpClearZBuffer`, `func_80034920`,
`Arctanf`, `sqrtf`, `func_80009E78`, and local
`overlay99RenderSortedEntries`; three local data pairs resolve to the
segment array and count, and the remaining pair resolves to the texture
carrier. Static fallback evidence retains only one identity, while the linked
ROM plus shipped runtime table proves all 15 without collapsing their roles.
ORT 1925 exports the owner, whose sole inbound is local module call `+0x1324`.
The linked range, complete Overlay 99 image, and full ROM are byte-identical.

`overlay18Load`'s two-word row is the superseded pre-`OSTime` candidate. The
canonical unguarded body owns Overlay 18 `+0x000..+0x1F4` (125 words) and uses
the authenticated O32 `osSetTime(OSTime)` ABI. Retained configured-object and
linked-overlay evidence agrees across all 60 runtime records, but a fresh
current-HEAD whole-ROM reproof remains pending. Do not route further source
search or permutation to this row.

`overlay20BuildTileCommands`'s four-word row is also historical. Canonical
unguarded C owns Overlay 20 `+0x7C4..+0x9DC` (134 words) with no target
padding. The call at function `+0x38` is table-1 record 13, `R_MIPS_26/SYMBOL`,
ORT 206 to resident `func_80034554`; the source auto-name is only a zero-valued
carrier. ORT 1692 exports `+0x7C4`, and Overlay 20 table-2 record 4 from
`overlay20DrawResource+0x54` is the sole inbound. Promotion evidence remains
the byte-identity authority; queue only an unchanged current-HEAD reproof, not
more source search.

`func_overlay_022_F0000000_1878108`'s six-word entry is that historical linked
trial. The surviving isolated object has five stack-home/store-order words but
omits the TU's required `-Wab,-r4300_mul`; fresh configured V0 must reconcile
the count and all 21 runtime relocation sites before either score is reused.

`overlay8ScaleOutputs`'s two-word row is historical; the function was promoted
in `07e04a0f`. Its two raw LO16 fields become exact under the four proved
runtime LOCAL tuples, and it no longer belongs in the NON_MATCHING queue.

`overlay1AssignRecordIndex` owns Overlay 1 `+0x36A0..+0x3750`, 44 words with
no padding. Its runtime contract has seven records: table-1 record 128 is a
SYMBOL call at function `+0x28` through ORT 257 to resident `GetRomlistInfo`;
table-2 LOCAL pairs 458/459, 460/461, and 462/463 at
`+0x44/+0x48`, `+0x70/+0x74`, and `+0x90/+0x94` all resolve through BSS base
`+0x83E0` plus addend `+0x1D8C` to module `+0xA16C` (`D_1D8C`). Retained
diagnostic C emits only five records and is 34/44 literal, 36/44 after runtime
normalization. Clean source now expresses all seven identities but is
uncompiled. ORT 1219 exports `+0x36A0`; resident relocation 61 at ROM `0xB894`
from `func_8000AA38+0x25C` is the sole inbound. Linked equality proves fallback
only.

Overlay 9 `+0x540` now has a fresh unchanged configured full-TU reproof under
canonical `-O2 -mips2 -32 -Wab,-r4300_mul`: exact `0x204` size/frame `0x58`,
120/129 raw words and 121/129 after runtime relocation or synthetic-VMA link
resolution, first `+0x4C`. Its ten candidate records agree with the runtime
contract by offset, type, and identity: `LOCAL` HI16/LO16 pairs at function
`+0x48/+0x4C`, `+0x50/+0x54`, and `+0x58/+0x5C` use module base `+0x18B0`
with addends `+0xC/+0x10/+0x14`; `SYMBOL` calls at `+0x80/+0xDC` resolve
through ORT 707/279 to resident `mathDiffAngle`/`func_8002A8C0`; and the
`LOCAL` pair at `+0x158/+0x168` uses BSS base `+0x1930` with addend zero. The
synthetic target object collapses the zero-field calls to one local placeholder,
so the runtime table remains the callee-identity authority. The eight linked
differences are a four-way saved-FPR color cycle; linked equality remains
fallback-only and the bounded reproof is parked without flags, trace, source
variants, or permutation.

Overlay 1 `+0x7D6C`'s eight-word entry is the addend-normalized result; the
retained isolated ranking has ten raw sites. Only 19/22 candidate runtime
tuples agree: `D_218` LO16 and `D_1D88` HI16 exchange `+0x090/+0x094`, and
`D_1BA4` LO16 is at `+0x14C` rather than target `+0x148`. The first clear pair
resolves to existing local `D_220`; the former `D_220_Clear` name had no
linker/runtime identity.

Overlay 14 `+0x498` (`overlay14ResetMode`) owns 18 table-2 records. Four LOCAL
HI16/LO16 pairs at `+0x20/+0x2C`, `+0x1C/+0x30`, `+0x18/+0x34`, and
`+0x14/+0x38` resolve respectively to module data `+0x1D3C` (`D_FC`),
`+0x1D24` (`D_E4`), BSS `+0x1EF8` (`D_128`), and data `+0x1D2C` (`D_EC`).
The SYMBOL call at `+0x54` resolves through ORT 1652 to Overlay 14 `+0x1B54`
(`overlay14ReleaseOwner`). Four more LOCAL pairs at `+0x84/+0x88`,
`+0x8C/+0x90`, `+0x94/+0x98`, and `+0x9C/+0xA4` resolve to module data
`+0x1D38`, `+0x1D18`, `+0x1D1C`, and `+0x1D20`; the LOCAL JUMP at `+0xB0`
targets `overlay14MoveCommandCursor` at `+0x578`. The synthetic fallback target
object exposes only ten records, omitting four loader-owned pairs and collapsing
the zero-field call. Historical pre-current-alias C was reported to carry 17
identities, but no candidate artifact survives; current C proof of the repaired
SYMBOL alias and all 18 emitted tuples remains pending. The function has no padding
or export and has five local inbound JUMPs: table-2 records 226, 238, 241, 267,
and 270 from `overlay14AdvanceCommand` twice, `overlay14StepCommand`,
`overlay14DispatchCommand`, and `overlay14CallUpdate`.

Overlay 16 `+0x1E0` (`overlay16ApplyGradient`) owns exactly six LOCAL records.
The HI16/LO16 pairs at function `+0x4/+0x8`, `+0x30/+0x34`, and
`+0x40/+0x5C` resolve respectively to `gOverlay16Buffer` at module addend
zero, `gOverlay16Phase` at `+8`, and `gOverlay16Mode` at `+4`. The retained
configured candidate agrees with all six runtime records by offset, type,
stable identity, and addend. ORT 1313 exports module `+0x1E0`; resident
relocation 155 at exact-C `func_8000D978+0xFC` is the sole authenticated
inbound. The function owns `+0x1E0..+0x424`; module `+0x424..+0x430` is
separate twelve-byte assembly padding. Its retained 145-word/frame-`0x20` C
still has 60 register-only differences from first `+0x3C`, so current linked
equality proves only the assembly fallback.

Overlay 7 `+0x894` (`overlay7DispatchModes`) owns 23 text and seven switch-table
records. The text surface comprises LOCAL pairs to mode arrays at module
`+0x1264/+0x14BC`, a LOCAL pair to the switch table at `+0x18F4`, a SYMBOL
pair through ORT 1579 to resident BSS `D_800D3128`, two SYMBOL calls through
ORT 171 to resident `mathRnd`, nine local calls to `overlay7CreateEntry`, and
four local calls to `overlay7AppendEntry`. The seven `R_MIPS_32/LOCAL` data
records at module `+0x18F4..+0x190C` target case labels inside
`+0x894..+0xAA0`. ORT 1471 exports the function; resident relocation records
342, 345, 347, 351, and 355 plus Overlay 25 table-1 record 15 are its six
inbounds. The natural unsigned left-shift/right-shift sign-bit expression
restores the target temporary allocation without changing its branch schedule.
Configured IDO output is exact at all 131 words with frame `0x20`. The
compiler's table-address pair is rebound, by symbol index only, to the shipped
LOCAL addend `+4`; its 32-byte duplicate `.rodata` section is discarded only
after a fixed payload digest, while the seven-entry retained table remains in
initialized data and receives no new ownership credit. All 23 text plus seven
table runtime records agree by offset, type, effective identity, and addend.
The 524-byte linked owner, complete overlay, and full ROM are byte-identical.

Overlay 7 `+0xCCC` (`overlay7DispatchSelection`) owns 13 records: a SYMBOL
HI16/LO16 pair at `+0x00/+0x04` through ORT 1579 to resident `D_800D3128`;
LOCAL pairs at `+0x3C/+0x40`, `+0x64/+0x88`, `+0x68/+0x84`, and
`+0xB8/+0xC0` to module `+0x1BA8`, `+0x100C`, `+0xFCC`, and `+0xFEC`; a
LOCAL JUMP at `+0x90` to `overlay7CreateEntry` at `+0x228`; and SYMBOL calls
at `+0xA4/+0xD8` through ORT 284 to `camGetMode` and ORT 1580 to Overlay 59
`overlay59AppendValue`. Current configured full-TU C emits all 13 offsets and
types, and runtime metadata proves every identity, including the repaired calls
at `+0xA4/+0xD8`. It is 55/60 raw and 58/60 after runtime normalization; the
two remaining words are register allocation, not relocation. The fallback has
only five static records and collapses both calls onto `overlay7ReleaseEntry`.
ORT 1510 exports the function and fourteen calls arrive from Overlays 1, 7, and
8; linked equality is fallback-only.

Overlay 7 `+0xDBC` (`overlay7CommitSelection`) owns 17 records: a SYMBOL pair
at `+0x00/+0x04` through ORT 1579 to resident `D_800D3128+2`; calls at
`+0x58/+0xAC` to `mathRnd`, `+0xE8` to resident `func_800031E8`, and `+0x108`
to `amSndPlay`; LOCAL pairs at `+0x68/+0x80`, `+0x84/+0x88`, `+0xD8/+0xDC`,
`+0xF0/+0xF8`, and `+0x100/+0x104` to module `+0x1714`, `+0x18B4`, `+0xFC0`,
`+0x1BA0`, and `+0xFC4`; and a LOCAL JUMP at `+0xF4` to
`overlay7ReleaseEntry` at `+0x000`. Current configured full-TU C emits all 17
offsets and types, and runtime metadata proves every identity. It is 67/72 raw
and 69/72 after runtime normalization; the three remaining words are one
post-`mathRnd` u16 allocation carrier, not relocation. The fallback exposes
only nine records: five calls collapsed onto the module entry and pairs at
`+0x84/+0x88` and `+0x100/+0x104`; it omits the other eight address records.
ORT 1345 exports the function and seventeen authenticated calls arrive from
resident code and Overlays 1, 7, 36, 86, 90, and 91. Linked equality is
fallback-only.

`func_overlay_014_F0001830_1871108`'s ownership trial proves module growth and
the exact seven-entry switch payload only. Its retained isolated candidate used
one false callee for all eight calls; repaired six-identity, 21-record, linked C
proof is still required before its five normalized schedule sites can be reused.

Overlay 99 `+0x800` (`overlay99RenderSortedEntries`) has ten authoritative
runtime records: table-1 records 20..25 call at
`+0x09C/+0x1E4/+0x220/+0x258/+0x32C/+0x360` to
`camGetProjZ`, `camGetPtr`, `func_80022E80`, `func_8002AA50`,
`mtxf_transform_point`, and `func_80022FD4`, plus LOCAL HI16/LO16 pairs at
`+0x1F4/+0x21C` and `+0x260/+0x264` for module base `+0x1410` with addends
four and eight (table-2 records 49..52, transform-Z and intensity-scale
constants). Its assembled fallback target retains only the six generic calls,
so the runtime table is authoritative. The function is unexported; table-2
record 57 at `overlay99RenderSegments+0x1D4` is its sole inbound. Retained
218/233 C used false frame gaps; clean `CameraSprite + MtxF` source is uncompiled
and linked equality proves fallback only.

Resident `levelGetCounts` owns 37 records: calls to `func_8002B280` at
`+0x24/+0xC8/+0x280/+0x2E0`, `piRomLoad` at `+0x30/+0x290`,
`piRomLoadSection` at `+0x124/+0x310`, `mmFree` at
`+0x26C/+0x274/+0x3E0`, and `align4` at `+0x2D4/+0x2EC`, plus twelve
HI16/LO16 pairs. Retained C has every offset/type but binds the endpoint pair
`+0x44/+0x50` to `D_800CF3E0+0x40` instead of `D_800CF420`, so the historical
pre-endpoint measurement had only 35/37 exact identities. No attributable C
object survives. Source now names the target endpoint but is uncompiled; all 37
current offsets, types, and identities remain pending. ORT 518 has sole inbound
Overlay 18 table-1 record 56 at `overlay18Initialize+0x8`. The owned
`0x263F0..0x267FC` range has no padding. Linked equality is fallback-only.

Resident `levelInit` owns 110 exact static tuples in retained diagnostic C:
70 R_MIPS_26 calls and 20 HI16/LO16 pairs. Eight resident runtime records occur
inside its `0x810`-byte range: `+0x208` to ORT 1350 (Overlay 35 `+0`),
`+0x3B8` to ORT 1351 (`overlay7InitPool`), `+0x3E0/+0x3F0` to ORT 1352
(`overlay34InitStorage`), `+0x648` to ORT 1353
(`overlay33InitializeBuffers`), `+0x660` to ORT 1354 (`overlay42Init`),
`+0x678` to ORT 1355 (`overlay16InitializeBuffer`), and `+0x720` to ORT
1356 (`overlay103CheckSignature`). Typed weak aliases expose those ABIs and a
metadata-only object rename restores the shipped `TrapDanglingJump` identity.
ORT 526 exports `levelInit`; `func_80028564+0x5F8` is its sole direct caller,
with no resident runtime, overlay `SYMBOL`, or aligned-pointer inbound. Current
linked equality proves fallback only.

Resident `func_80028FCC` owns three `R_MIPS_26` records to `func_80028FB8` at
`+0x14/+0x30/+0x4C`, all exact in fresh configured full-TU C with canonical
`-Wo,-Olimit,100`; that retained spelling is 17/27 words with exact `0x6C`
boundary/frame `0x18` and first mismatch `+0x1C`. ORT 663 exports the function,
but exhaustive resident relocation, overlay SYMBOL, direct-JAL, literal-pointer,
object-reference, and source scans authenticate no caller. The shared-result
probe regressed to 25 words and moved the latter two calls; linked equality is
fallback-only.

Overlay 61 `+0x1648` has eleven authoritative records: resident calls at
`+0x14/+0x34/+0x4C/+0x6C/+0x8C/+0x140/+0x154` to `packOpen`,
`packOpenFile`, `packFileSize`, `func_8002B280`, `packReadFile`, `mmFree`, and
`packClose`; two calls at `+0xC0/+0xDC` to Overlay 68 `+0`; and a LOCAL
HI16/LO16 pair at `+0x24/+0x28` with stored addend `+0x164`. Exact C emits all
11 records and names the local pair `gOverlay61SavePathReloc`; the generated
ledger resolves that friendly alias at `+0x164`. The disclosed one-iteration
grouping gives the exact 92 words and 0x38-byte frame. Owned
`+0x1648..+0x17B8` has no padding, one local inbound at `+0x120C`, and no
export. The linked overlay and full ROM are byte-identical.

Overlay 31 `+0x6B0` (`overlay31InitializeBuffers`) is exact C with 54 static
records: 16 calls and 19 HI16/LO16 pairs. The shipped tables authenticate 47
SYMBOL records and seven local JUMPs. Data pairs resolve to
`gOverlay31MaxLine`, `gOverlay31MaxPoint`, both vertex buffers, triangle and
rectangle sources, configs, point pool, line records, dummy assets/count, and
effect records/count. Resident calls resolve to `reset_particles`,
`func_8002B280`, `piRomLoad`, `func_800355A0`, `func_80034448`, `func_8001F520`,
and `mmFree`; seven local calls cover the four config builders, pool, records,
and reset helper. Moving `assetBuffer` after the three integer locals gives the
exact 245 words and 0x48-byte frame while preserving every distinct role. ORT
1386 and resident record 240 authenticate sole inbound `func_80028564+0x5BC`;
owned `+0x6B0..+0xA84` has no padding. The linked overlay and full ROM are
byte-identical without relocation filtering or identity-collapsing rebinds.

Overlay 68 `+0x1250` (`overlay68RebuildSecondaryEntry`) owns 19 runtime
records. Table 1 pairs 52/53, 60/61, and 63/64 at function
`+0x08/+0x1C`, `+0x1A4/+0x1B0`, and `+0x1BC/+0x1C0` resolve through reserved
DATA1 `+0x1498`/ORT 1850 to resident `D_8007A1F8`; 54/55 at `+0x74/+0x90`
resolve to `func_8002B280`; 56/59 at `+0xB8/+0x198` to `piRomLoadSection`;
57 at `+0xC0` to `func_800291C4`; 58 at `+0xCC` to `levelGetBlurEffect`; and
62/65 at `+0x1B4/+0x1CC` to `mmFree`. Table 2 LOCAL pairs 14/15 and 16/17 at
`+0x04/+0x0C` and `+0x34/+0x38` share data base `+0x15B0`, addend `+0x14`,
module identity `+0x15C4`; record 18 at `+0x68` calls local
`overlay68PayloadLimit` (`+0`). Policy-clean configured C represents all 19
runtime roles, but its 120-word/`0x38`-frame structure shifts static sites after
`+0x64` against the 122-word/`0x40` target. ORT 1163's sole inbound is resident
relocation 4, `func_80004FE0+0x4C8`; there are no local or cross-overlay
callers. Current linked equality proves fallback only.

Overlay 41 `+0x000` owns exactly three runtime records: a SYMBOL HI16/LO16
pair at function `+0x14/+0x28` resolving to `D_800D6B58`, and a SYMBOL call at
`+0xD0` resolving to resident `func_8000D16C`. Exact C emits all three at those
offsets and uses distinct stored-zero proxies so the linked image retains the
retail operands without collapsing their runtime identities. The 73-word body,
`0x30` frame, owned `+0x0..+0x124` range, linked module, and full ROM are exact;
there is no target padding. ORT 1452 and resident relocation 314 authenticate
sole inbound `func_80051364+0x2D4`; there is no local or cross-overlay caller.
Production trims only standalone section alignment.

Overlay 41 `+0x2AC` (`func_overlay_041_F00002AC_18875E4`) owns 20 LOCAL
runtime records: ten HI16/LO16 pairs spanning function offsets
`+0x30..+0x4D8`, all based at module initialized-data `+0x1DE0`. Exact C emits
the same 20 offsets and types with unchanged addends. Its compiler-private
jump table and constants reproduce the retained `+0x0..+0x3C` payload and are
discarded only after a digest check; relocation metadata binds the text to the
retained zero-addend base. The unchanged runtime table plus the exact linked
ROM range prove all 20 effective identities. The 340-word body, `0x58` frame,
owned `+0x2AC..+0x7FC` range, complete module, and full ROM are exact.

Overlay 41 `+0x1740` (`overlay41SpawnItems`) owns 11 runtime records. Its
configured 135-word C has the exact `0xA0` frame and relocation shape. A
metadata-only retained-pool contract resolves the two formerly differing
LO16 results through four unchanged relocations, an ABS initialized-data base
at `+0x58`, and a digest-checked discarded duplicate. The final cross-module
call is authenticated by the shipped record as Overlay 12 `+0x1B4` and by the
exact linked range. The complete module and US ROM are byte-identical.

Overlay 41 `+0x1650` (`func_overlay_041_F0001650_1888988`) owns four runtime
records: SYMBOL HI16/LO16 at `+0x08/+0x0C` resolving to resident
`D_800D6C58` (`gOverlay41Slots`), and LOCAL HI16/LO16 at `+0x98/+0xA0`
resolving through initialized-data base `+0x1DE0` plus `+0x54` to module
`+0x1E34` (`D_0[0x15]`). Candidate C places the LOCAL LO16 at `+0xA4`; target
uses `+0xA0`. ORT 1461 exports the function; resident relocations 325 and 326
call it twice from `func_800517E0`. Fallback equality does not prove C.

Overlay 21 `+0x10C` (`overlay21ApplyPriorities`) owns nine runtime records: a
SYMBOL call at `+0x1C` to resident `camGetPtr`; LOCAL pairs at `+0x24/+0x28`
for object count and `+0x30/+0x40` for the object array; and count-reload pairs
at `+0x114/+0x11C` and `+0x19C/+0x1A0`. Exact C emits all nine at those
offsets, including a distinct `overlay21GetReferenceReloc` call carrier that
resolves at runtime to resident `camGetPtr`. The 114-word body, `0x28` frame,
owned `+0x10C..+0x2D4` range, linked module, and full ROM are exact. ORT 1315
and resident runtime record 157 authenticate sole inbound
`func_8000DDE4+0x128`; the following 12 bytes are separate padding and earn no
C credit.

Resident `func_8000DDE4` owns exactly two static R_MIPS_26 records:
`runlinkIsModuleLoaded` at `+0x114` and the metadata trampoline
`TrapDanglingJump` at `+0x128`. Configured full-TU C reproduces all 118/118
words, the `0x28` frame, and both exact offset/type/symbol tuples. Runtime
resident record 157 replaces the second role with ORT 1315, Overlay 21
`+0x10C` (`overlay21ApplyPriorities`). Sole direct inbound is
`func_8000E5EC+0x288`; there is no ORT export, resident-runtime or overlay
inbound, stored pointer, data relocation, or target padding. The owned linked
range and full ROM are exact.

Overlay 45 `+0x764` owns 24 runtime records: 13 external calls, one local call
to `+0x1158`, two SYMBOL HI16/LO16 pairs for the resource head and resident
scissor state, and three LOCAL pairs for retained constants. The historical p4
body collapses six call roles, including two distinct matrix identities, onto
one offset-zero carrier. Its apparent equality also rewrote the three constant
LO16 instruction fields at `+0x160/+0x168/+0x174`, which ADR 0002 prohibits.
Fresh C must preserve all 24 semantic identities and emit those addends
naturally from the already-owned initialized data.

Overlay 34 `+0x0` (`overlay34InitStorage`) owns eight runtime records: SYMBOL
JUMPs at `+0x28/+0x74` to resident ORT 82 (`func_8002B280`), then LOCAL
HI16/LO16 pairs at `+0x34/+0x38`, `+0x80/+0x84`, and `+0xB8/+0xC4` for module
data `+0`, `+4`, and `+8` (`gOverlay34Records`, `gOverlay34Pointers`, and
`gOverlay34Count`). Its standalone target object retains only four static
records and bakes the final `+8` addend; runtime normalization is authoritative.
Historical pre-type-repair C emitted all eight at 45/50 raw and 46/50
normalized words. Pointer aggregate and allocator tag types are now repaired,
so current tuple/score proof awaits clean V0.
ORT 1352 has exactly two authenticated inbounds, resident records 205/206 at
`levelInit+0x3E0/+0x3F0`; a neighboring source-only two-argument unresolved
carrier is not an authenticated initializer caller. The function ends at
`+0xC8` without target padding; genuine C's eight trailing alignment bytes are
outside ownership. Current linked equality proves fallback only.

Overlay 34 `+0x2C8` (`overlay34RemoveRecord`) owns seven runtime records:
LOCAL HI16/LO16 pairs for `gOverlay34ActiveCount` at `+0x0/+0x4` and
`+0x74/+0x78`, a LOCAL pair for `gOverlay34Pointers` at `+0x10/+0x20`, and a
SYMBOL call at `+0x6C` through the stored zero carrier to ORT 140
(`func_800347A0`). The fallback object retains only five static records because
the pointer pair is baked; runtime metadata is authoritative. The prior 32/44
raw C used a false second argument and allocation aids, so it is diagnostic.
Clean pointer-typed, one-argument source is staged but uncompiled. The owned
`+0x2C8..+0x378` range has no padding, and linked equality proves fallback only.

Overlay 34 `+0x40C` (`overlay34UpdateRecords`) owns seven runtime records.
LOCAL HI16/LO16 pairs at function `+0x28/+0x2C`, `+0x40/+0x4C`, and
`+0x44/+0x48` resolve to module data `+0xC`, `+0x4`, and `+0x10`
(`gOverlay34ActiveCount`, `gOverlay34Pointers`, and `gOverlay34Value10`). The
LOCAL JUMP at `+0xF0` resolves to `overlay34RemoveRecord` at module `+0x2C8`;
the exact C object binds that record through the existing zero-field proxy so
the shipped runtime relocation remains the effective-identity authority. The
configured object is exact at 77 words with a `0x30` frame, all seven records
agree by offset/type/effective identity, and the linked owner, complete overlay,
and full ROM are exact.

Overlay 22 `+0xA7C` (`func_overlay_022_F0000A7C_1878B84`) owns three runtime
records. A LOCAL HI16/LO16 pair at function offsets `+0x2C/+0x30` binds the
module constant base (the LO16 carries the `D_0[4]` displacement), and the
SYMBOL `R_MIPS_26` at `+0x110` targets resident `sqrtf`. Exact C emits those
three sites and unchanged addends. Its configured 173-word instruction stream
and `0x88` frame are exact; metadata-only rebinding names the call's shipped
overlay placeholder before link. The unchanged runtime table plus exact linked
range prove all three effective identities. The complete module and full ROM
are byte-identical.

Overlay 22 `+0xD30` (`func_overlay_022_F0000D30_1878E38`) has 12 exact
runtime-backed tuples in the configured candidate. LOCAL HI16/LO16 pairs at
function `+0x14/+0x18` resolve through count `+0xED0`; pairs at
`+0x20/+0x38`, `+0x70/+0x7C`, and `+0x90/+0x94` resolve to the node-array
base at `+0xEA0`. SYMBOL calls at `+0xE8/+0x118/+0x148/+0x150` resolve to
resident `partUpdateTriggers`, `func_80002FE0` twice, and `func_80006EA0`.
The extracted fallback object retains only the count pair and collapses the
four call identities, so the shipped runtime tables are the identity authority.
The configured C is now byte-identical; linked equality proves the C object
and the full ROM.

Overlay 73 `+0x0` (`func_overlay_073_F0000000_18CAAC0`) owns eight LOCAL
records: `+0x1C/+0x3C` resolves through base `+0xEB0`, addend `+0x80`, to
module `+0xF30`; pairs at `+0x140/+0x144`, `+0x14C/+0x160`, and
`+0x164/+0x170` resolve through base `+0xF80` with addends `0/+4/+8`.
The fallback target statically retains only the first pair, so runtime tables
authenticate the other six. ORT 1248 exports `+0`; resident relocation 90 at
`func_8000AA38+0x42C` is the sole inbound. The function owns
`+0x0..+0x190` / ROM `0x18CAAC0..0x18CAC50` with no target padding; the next
assembly owner begins at `+0x190`. Fresh policy-clean V0 comparison reports
target/candidate 8/8 records, offset/type 8/8, identity 8/8, and all eight
resolved.

Overlay 80 `+0x11C` (`overlay80UpdateContact`) owns 20 records. SYMBOL calls
at function `+0x30/+0xFC/+0x240/+0x294` resolve through ORTs 371, 101, 381,
and 967 to resident `func_8005776C`, `sqrtf`, `func_8005AD64`, and
`func_8005ABA8`. Eight LOCAL HI16/LO16 pairs resolve through data base
`+0x3F0` with addends `+4,+8,+0xC,+0x10,+0x14,+0x18,+0x1C,+0x20`. ORT 1290
exports `+0x11C`; resident relocation 132 at `func_8000AEEC+0x3B0` is the sole
inbound. The promoted C is 180/180 words with frame `0x80`; all 20 relocation
records, the owned `+0x11C..+0x3EC` range, linked overlay, and full ROM are
exact. The separate `+0x3EC..+0x3F0` padding word is excluded from C credit.

Resident `func_8005A948` owns 13 static records: HI16/LO16 pairs to
`D_800D7D04` at `+0x00/+0x04`, `D_800D7CF4` at `+0x34/+0x38` and
`+0x134/+0x13C`, and `D_800D7CF8` at `+0xB4/+0xB8` and `+0xD0/+0xD4`, plus
calls to `piRomLoadSection` at `+0xC8/+0x124` and `func_8002B314` at `+0xF8`.
It has no export-table entry or overlay runtime record; `func_8005A7A0+0x104`
is its sole caller, with no stored-pointer inbound and no target padding.
Fresh configured V0 and the retained natural candidate both reproduce all 13
offset/type/identity tuples exactly. The natural candidate is exact-sized/frame
and reaches 85/94 raw and relocation-normalized words (91/94 under the
workbench's stack-home normalization); its remaining three register and six
stack-home sites are unrelated to the relocation surface. Linked equality
remains fallback-only.

Overlay 62 `+0xD4` (`overlay62Update`) owns 71 runtime records: 43 SYMBOL and
28 LOCAL, comprising 21 calls and 25 HI16/LO16 pairs. Fresh configured C emits
all 71 at exact offsets and types. The identity comparator resolves 18 friendly
aliases and leaves 53 proxy names unmapped, so those roles are not yet proved
for promotion; the fallback target's 29 generic records remain incomplete. The
function owns `+0xD4..+0x56C` with no target padding; the next owner begins at
`+0x56C`. ORT 1444 exports `+0xD4`; resident relocation 302 at
`func_80038E1C+0x3A4` is the sole runtime inbound. Runtime tables, not friendly
proxy names or fallback ELF records, are the identity authority. Linked
equality proves fallback only.

Overlay 7 `+0xEDC` (`overlay7FillValues`) is an exact 11-word C island inside a
mixed TU. Its only records are LOCAL HI16/LO16 at `+0x00/+0x04`, resolving
through base `+0x1910` plus addend `+0x2AA` to BSS `gOverlay7ValuesEnd`.
ORT 1517 exports it; Overlay 1 table-1 relocation 167 calls it from
`overlay1UpdateObjectPhysics+0x8C0`. Linked range/module/ROM equality is
C-produced; its two inert source aids are tracked in the cleanup queue.

Overlay 19 `+0xD78` (`overlay19ClassifyEdge`) owns no static or runtime
relocations. It is unexported. Its sole inbound is table-2 JUMP index 6 at
module `+0xCF4`, from `overlay19FindAdjacent+0xD8`; linked equality currently
comes from the fallback. Configured full-TU C is exact-sized and frameless at
110/120 raw/normalized words, first `+0x138`; its candidate object also has zero
records. The complete flag lattice and three natural forms are nonexact, so the
zero-record surface does not promote the C body.

Overlay 1 `+0x63CC` (`overlay1UpdateCountdown`) is exact C after masking seven
records: LOCAL pairs at `+0x00/+0x04`, `+0x10/+0x14`, and `+0x30/+0x38` for
BSS addends `+0x1DA0/+0x1D94/+0x1D9C`, plus a SYMBOL call at `+0x34` to
Overlay 1 ORT 1525 (`+0x5BA4`). ORT 1535 exports it; table-2 index 954 stores
its sole inbound function pointer at module data `+0x821C`. Its legacy inert
condition is cleanup-tracked and does not authorize guards in unmatched C.

Resident `rcpClearZBuffer` owns an exact HI16/LO16 pair at `+0x00/+0x04` to
resident BSS `D_800D2FAC`; fresh configured nonmatching C reproduces both
records' offsets, types, and identities exactly. It is exported as ORT 765.
Three shipped SYMBOL records call it: Overlay 60 module `+0x2194` from
`func_overlay_060_F0000334_18BA10C+0x1E60`, Overlay 91 module `+0x518` from
`overlay91Render+0x5C`, and Overlay 99 module `+0xC8C` from
`overlay99RenderSegments+0xE8`. Its fourth direct caller is resident
`rcpClearScreen+0x48`; no other shipped overlay record targets ORT 765.

Resident `func_8002B040` owns no static or runtime relocations across its exact
ROM `0x2BC40..0x2BCD0` range and is not exported. Its four authenticated
inbounds are Overlay 69 module `+0xC0`, Overlay 88 module `+0x110`, and Overlay
94 module `+0xE0/+0x414`, all `R_MIPS_26` calls. The next resident function,
`mmInit`, starts immediately at `0x2BCD0`, so there is no target padding.
Configured C remains one instruction long; linked equality proves fallback
only.

Resident `func_8003A2C8` owns exact HI16/LO16 pairs at `+0x00/+0x04` to
`D_8007C090` and at `+0x24/+0x28` plus `+0x3C/+0x40` to `D_800D3128`. ORT
606 exports it. Its direct callers are resident `func_80027EC0+0x80`, Overlay
46 table-1 record 60 at module `+0x684` (`overlay46ReleaseState+0x70`), and
Overlay 60 table-1 record 206 at module `+0x156C`
(`func_overlay_060_F0000334_18BA10C+0x1238`); no other shipped overlay record
targets ORT 606. There are no resident runtime sites inside the function, no
resident runtime inbound, no additional direct `jal`, and no aligned literal
function pointer. Linked equality currently proves the fallback only.

Overlay 1 `+0x7B64` (`overlay1FindBestRecord`) owns four LOCAL records:
table-2 884/885 at `+0x00/+0x08` resolve through BSS base `+0x83E0` and addend
`+0x220` to module `+0x8600` (`gOverlay1BestRecords`/`D_220`), while 886/887
at `+0x04/+0x14` use addend `+0x1D88` to resolve to module `+0xA168`
(`gOverlay1SelectedType`/`D_1D88`). The assembled fallback literalizes the
second pair, so the runtime table is authoritative. The function is unexported;
its only callers are `overlay1CreateRecord+0x30` and `+0x144`, table-2 LOCAL
JUMP records 889 and 895 at module `+0x7C0C/+0x7D20`. Current configured C
emits the same four offsets and types under the friendly identities, while the
assembled fallback literalizes the selected-type pair. The bounded 119-flag,
one-trace, three-form reproof is nonexact solely on a 12-site `a1/a3`
allocation exchange. Promotion still requires distinct metadata-only rebinding
for both C identities and one shared caller/callee record type; current linked
equality proves fallback only.

Overlay 1 `+0x378` (`overlay1FindType5ByKey`) owns table-1 SYMBOL record 1 at
function `+0x14`/module `+0x38C`, resolving through ORT 128 to resident
`func_8000572C`. Source now calls that identity directly and metadata rebinds it
to the Overlay 1 zero-addend placeholder. ORT 1485 exports `+0x378`; its sole
inbound is table-2 SYMBOL record 548 at module `+0x4018`, from
`overlay1TransitionState+0x40`. There are no resident, cross-overlay, or local
JUMP inbounds. Fresh configured full-TU V0 is 22/39 positional words with the
outbound site exact by runtime offset/type/identity. The complete 119-row flag
lattice and two natural source forms are nonexact; current linked equality
proves fallback only.

Resident `func_8002C69C` owns no static or runtime records. Five exact inbound
R_MIPS_26 calls come from matched `func_8002C94C` at
`+0x80/+0x90/+0xA0/+0xE0/+0xF0`; no other direct call or stored pointer is
authenticated. ORT 727 exports it, with zero resident-runtime or overlay
`SYMBOL` users. All five sites pass the writer in `a0`, a signed value in `a1`,
and the bit count in `a2`, and consume no return value, authenticating the
adopted `void (SavesBitWriter *, s32, s32)` ABI. Its exact owner is VRAM
`0x8002C69C..0x8002C70C`, ROM
`0x2D29C..0x2D30C`, 28 words with no padding. Fresh configured C preserves that
extent and emits zero relocations, but eleven register-only words differ; linked
equality therefore proves fallback only.

Resident `func_8001D880` owns VRAM `0x8001D880..0x8001D910`, ROM
`0x1E480..0x1E510`, with no padding and no static or runtime relocations.
ORT 405 exports it. Overlay 26 table-1 record 17 at module `+0xA0C` and
Overlay 80 table-1 record 1 at module `+0xD0` are its only runtime inbounds;
there is no resident inbound. Their current friendly call prototypes and
argument setup conflict with the target's observed
`f32 (f32, f32, f32 *, f32)` ABI, so they authenticate identity and call sites
only, not caller semantics. Fresh configured C is exact-sized at 36 words but
has 29 positional differences; linked equality therefore proves fallback only.

Resident `func_80012574` targets one R_MIPS_26 record to `sqrtf` at `+0xA4`.
Historical pre-cleanup configured C was reported to emit that exact tuple, but
no candidate object/hash survives and current clean C output is pending. It owns
VRAM `0x80012574..0x80012658`, ROM `0x13174..0x13258`, with no padding. ORT 308
exports it from entry ROM `0x1849C00`.
Five direct calls exist at `func_80011CDC+0x288/+0x3CC` and
`func_800563B4+0xF8/+0x4E8/+0x65C`. There are zero resident runtime records
inside the function, zero resident or overlay records targeting ORT 308, and
zero aligned absolute function pointers. Current linked equality proves
fallback only.

Resident `func_80020D8C` has no static relocations and is exported as ORT 374.
Eight shipped SYMBOL calls target it: Overlay 57 table-1 record 258 at module
`+0x3288` (`overlay57ApplyValue+0x50`), Overlay 60 record 337 at `+0x2268`
(`func_overlay_060_F0000334_18BA10C+0x1F34`), and Overlay 82 records 12..17
at `+0x310/+0x320/+0x368/+0x38C/+0x3BC/+0x3DC` from
`overlay82Update+0x2D0/+0x2E0/+0x328/+0x34C/+0x37C/+0x39C`. Resident
`func_8001BB10+0x60` is the ninth call; it passes an additional owner/context
argument in `a3` that the callee overwrites without reading. No other shipped
overlay record targets ORT 374; linked equality currently proves fallback only.

Resident `func_8002CF6C` owns 11 exact records: R_MIPS_26 calls at `+0x0C` to
`joyMessageQ`, `+0x18` to `func_80070170`, `+0x28` to `func_8002B280`,
`+0x54` to `func_8002CCE4`, `+0x60` to `packCalculateGameChecksum`, `+0xC4`
to `packCalculateGlobalFlagsChecksum`, `+0x120` to `mainResetPressed`, `+0x13C`
to `func_8002C8B4`, and `+0x144` to `mmFree`, plus a HI16/LO16 pair at
`+0x80/+0x90` to `D_8007A304`. ORT 505 exports it from entry ROM `0x1849F14`;
exactly one export row and one direct call (`joyRead+0x130`) identify it. There
are zero resident runtime records inside the function, zero resident or overlay
records targeting ORT 505 across the shipped tables, and zero aligned absolute
function pointers. The current stack-homed candidate emits all 11 symbol/type identities at
86 instructions, frame `0x48`. Call offsets still drift by the two-word
extent deficit; the `D_8007A304` pair remains at target-relative
`+0x80/+0x90`. Current linked equality therefore proves the assembly
fallback only.

Resident `debug_text_width` owns five exact records: R_MIPS_26 calls at `+0x18`
and `+0x30` to `sprintfSetSpacingCodes`, `+0x28` to `vsprintf`, and a
HI16/LO16 pair at `+0x4C/+0x50` to `D_8007CE98`. ORT 862 exports it, but all
375 resident relocation entries, every shipped overlay SYMBOL record, direct
JAL and literal-pointer scans, and source references are empty. Fresh configured
full-TU C emits all five at 59/66 raw/normalized words under the canonical
flags; all 119 flag rows were attempted and none was exact. There is no target
padding. Ordinary 66/66 and current linked equality prove fallback only.

Resident `func_80045BBC` is exact linked C over ROM `0x467BC..0x468AC`
(240 bytes, no padding). Stock IDO emits 18 static records with exact
relative offsets, types and identities: the three header-source globals,
`D_800D5D40`, `D_800D5D48`, the two file-name objects, three `_bcopy` calls
and `packWriteFile`. The extracted fallback has six additional HI16/LO16
records for the fixed header destinations `0x80705014`, `0x80705018` and
`0x8070501C`; the C encodes those addresses directly. They resolve to the same
six instruction fields. This is a literal-versus-symbol static layout, not a
runtime relocation deficit; no metadata or instruction patch is applied.
Resident runtime records, ORT exports, overlay SYMBOL inbounds and stored
pointer inbounds remain absent. `func_80045CAC+0x64` is the sole direct caller.
Canonical linked owned bytes and the complete rebuilt ROM are byte-identical.

Resident `func_8004BA8C` owns nine target static records. Historical
policy-defective C emitted all nine exactly:
HI16/LO16 pairs to `D_800D60E4` at `+0x04/+0x08`, `D_800D6628` at
`+0x14/+0x28`, and `D_800D6644` at `+0x34/+0x38` and `+0x48/+0x54`, plus
an R_MIPS_26 call to `func_8004D39C` at `+0x40`. ORT 880 exports resident
offset `0x4B63C`; resident callers are `func_8004B1DC+0x1E4/+0x294` and
`func_8004BCC4+0x12C`, while Overlay 41 table-1 record 88 at module `+0x1CD0`
and Overlay 45 record 22 at `+0x3B0` call it from `overlay41DrawItem+0x4C`
and `overlay45ConfigureLayout+0x9C`. No additional direct JAL, resident
runtime-table, overlay SYMBOL, or stored-pointer inbound is authenticated.
Current clean C is uncompiled, so its tuple surface awaits V0 and linked equality
proves fallback only. Overlay 41's existing rename and Overlay 45's proxy already
preserve the runtime carriers; no new resident rebinding is required.

Resident `func_8002B7AC` targets 12 static records: HI16/LO16 pairs to
`D_800D21B0` at `+0x08/+0x0C`, `D_800D21A8` at `+0x44/+0x48`,
`D_800D20A8` at `+0x50/+0x5C` and `+0x60/+0x6C`, and `D_800D1CA8` at
`+0x64/+0x68`, plus R_MIPS_26 calls to `ReleaseUnusedLinkSlots` at `+0x3C`
and `func_8002B8A8` at `+0x8C`. The matched C reproduces all twelve at those
offsets. ORT 593 exports
resident offset `0x2B35C`; resident runtime and overlay SYMBOL inbounds are
zero, and `func_80026FB4+0x5F8` is the sole direct caller. The owned range and
the full ROM are byte-identical.

Resident `func_80047CD8` owns four exact R_MIPS_26 records to
`func_800349A4` at `+0x19C,+0x1F0,+0x244,+0x278`. It has no resident runtime
record or ORT export. Authenticated inbounds are resident
`func_80009414+0x520`, Overlay 69 table-1 record 9 at
`overlay69DrawSortedGeometry+0x530`, and Overlay 88 table-1 record 12 at
`overlay88DrawSortedGeometry+0x530`; the overlay proxy names now disclose the
shared draw-cone identity and await regenerated metadata. The current
configured full-TU candidate emits exactly the same four offsets, types, and
identities and no other record in its owned range. Current linked equality
still proves fallback only.

Resident `func_800498FC` owns five exact static records: a HI16/LO16 pair to
`D_800D5F58` at `+0x2C/+0x30`, calls to `func_80021FB0` at `+0x88/+0xC4`,
and a call to `camGetMode` at `+0x9C`. The retained exact-sized candidate
reproduces every offset, type, and symbol identity. The resident comparison now
authenticates `0x800498FC..0x80049A8C` against the canonical fallback object and
linked ELF, then reports all five target and all five candidate tuples exact;
the sparse resident runtime table is no longer mistaken for the target static
surface. Five `main.c` calls and one `weather.c` call authenticate the ABI;
linked equality proves fallback only.

Resident `func_80041CE4` owns nine exact static records in retained genuine C:
HI16/LO16 pairs to `D_8007C894` at `+0x04/+0x08`, `D_8007C88C` at
`+0x48/+0x4C` and `+0x1E4/+0x1F0`, and `D_7C900` at `+0x9C/+0xA4`, plus an
R_MIPS_26 call to `func_800349A4` at `+0x158`. Resident runtime records, an
ORT export at offset `0x41894`, overlay SYMBOL inbounds, and stored-pointer
inbounds are all zero. `partDraw+0xEC` is the sole direct caller. Current
bounded C remains 126/153 words with candidate SHA prefix `90eeefb220a1`;
all 119 flags and two flat natural forms are nonexact, so linked equality
continues to prove fallback only.

Resident `func_80058250` targets 20 static records across its exact
`0x58E50..0x58EA8` range: eight HI16s and twelve LO16s naming the handle,
doppler-pitch, and racer-object fields of four contiguous sound slots. The
configured named-global V0 represents all twelve identities but emits 24
records because each field receives its own HI16/LO16 pair; its four extra
HI16s account exactly for the `+0x10` text-size delta. Target code instead
uses a handle base and racer-object base per slot, with each racer-object high
half also carrying the preceding float's distinct LO16. The function is
frameless and has no padding; `func_80004FE0+0x54C` is its sole direct caller,
and there are no calls or pointer relocations inside it. Current linked
equality proves fallback only.

Resident `func_8003E8D8` owns ten exact static tuples in bounded 139/140
configured full-TU C:
HI16/LO16 pairs to `D_8007C898` at `+0x00/+0x04`, `D_8007C890` at
`+0x28/+0x2C`, `D_8007CA90` at `+0xC4/+0xF4`, and `D_80082A48` at
`+0x1F8/+0x1FC`, plus R_MIPS_26 calls to `func_80034448` at `+0x174` and
`mathRnd` at `+0x1C0`. Resident runtime records inside the function, an ORT
export, overlay `SYMBOL` inbounds, and aligned stored-pointer inbounds are all
zero. `func_8003E7B8+0xE4` is the sole direct caller. The exact tuple set and
`0x230` boundary survive the declaration-home improvement; current linked
equality proves fallback only.

Resident `func_80019DE8` targets three static records: R_MIPS_26
`mathOneFloatRPY` at `+0xBC` and a `D_800CB290` HI16/LO16 pair at
`+0xC8/+0xE0`. Matched C carries all three at the target offsets. Resident
runtime records inside the function are zero. ORT 358 exports offset
`0x19998`, but all resident runtime and overlay SYMBOL records have zero
inbounds; direct callers are `lightDefaultObjectLight+0x38` and
`func_8001A008+0x74/+0xC4`, with no stored pointer.

Resident `func_8002B524` owns 12 exact tuples in retained diagnostic C:
HI16/LO16 pairs to `D_8007A278` at `+0x04/+0x08`, `D_8007A270` at
`+0x1C/+0x3C`, `D_8007A27C` at `+0x4C/+0x50`, and `D_800D1C60` at
`+0x90/+0x94`; R_MIPS_26 `runlinkGetAddressInfo` at `+0x74`; and
`func_8002BB40` at `+0x134,+0x160,+0x180`. ORT 547 exports offset `0x2B0D4`.
Resident direct callers are `runlinkSuspendCode+0xB4`,
`runlinkResumeCode+0xD8`, and `func_80034448+0x12C/+0x1D0`; five overlay
SYMBOL calls occur at Overlay 2 `+0x11A8`, Overlay 18 `+0x320/+0x334`,
Overlay 19 `+0x1B4`, and Overlay 35 `+0x5C4`. ROM `0x7AE40` carries one
R_MIPS_32 function pointer consumed by `RevealReturnAddresses`; no resident
runtime-table record targets ORT 547. Current linked equality proves fallback
only, and overlay proxy names await identity-explicit metadata regeneration.

Resident `func_8002BB40` owns eight exact tuples in canonical C:
HI16/LO16 pairs to `D_8007A270` at `+0x08/+0x64`, `D_800D21B0` at
`+0x14/+0x18`, and `D_800D1C60` at `+0x28/+0x2C` and `+0x40/+0x44`.
The C and assembled-target count, offset, type, and identity agree. Their
serialized row order differs for the second `D_800D1C60` pair; the linker does
not assign identity by row order, and the linked owned bytes are exact.
Resident runtime records, an ORT export at offset `0x2B6F0`, overlay SYMBOL
inbounds, and stored pointers are zero. Direct callers are
`func_8002B3A8+0xE0` and `func_8002B524+0x134/+0x160/+0x180`. The linked owned
range is exact C.

Overlay 40 `+0xE8` (`overlay40UpdateEntries`) owns four runtime records: one
HI16/LO16 pair for the eight-entry table and one for the object-table pointer.
The bounded direct-shift C emits all four identities; three records align by
offset and type, while the object-table LO16 is at `+0x10` instead of target
`+0x0C` because its address completion swaps with loop-count initialization.
ORT 1451 exports the exact `+0xE8..+0x1A0` no-padding owner, and resident
runtime relocation at `func_80051364+0x78` is its sole inbound. The caller
supplies the amount in `a0`; the source's second carrier is overwritten before
use. Current linked equality proves fallback only.

Overlay 40 `+0x690` (`overlay40FadeRecords`) owns five SYMBOL HI16/LO16 pairs:
timer at `+0x00/+0x04`, current at `+0x0C/+0x10`, target at `+0x38/+0x3C`,
duration at `+0x44/+0x50`, and output at `+0x9C/+0xA0`. They resolve to
`D_800D6C4C`, `D_800D6C52`, `D_800D6C50`, `D_800D6C4E`, and `D_800D6C54`.
The compiled output-origin spelling emits all ten roles and measures 98/101
words, with three register-only differences. One allocator trace isolated a
single `v0`/`v1` globalcolor outcome and the 119-flag lattice found no exact
object. The fallback target retains none statically, making the runtime table
the identity authority. ORT
1314 exports the function, and resident runtime record 156 at
`func_8000D978+0x130` is its sole authenticated inbound/trap site; it is not an
ordinary direct static call. Current linked equality proves fallback only.

Overlay 74 `+0xB8` (`overlay74Update`) owns eight runtime-authenticated records:
SYMBOL calls to resident `func_8005776C` at `+0x70`, `func_800291B4` at
`+0x128`, `amSndPlay` at `+0x134`, and `func_8003A680` at `+0x178`, plus
HI16/LO16 pairs to resident `D_800D3128` at `+0xE8/+0xEC` and
`+0x13C/+0x140`. Policy-clean configured C emits all eight at those exact
offsets and types. It remains nonexact at 39 relocation-masked words because
its frame and integer allocation differ; the fallback target collapses the
call symbols, making runtime metadata the identity authority. ORT 1285 exports
the owned `+0xB8..+0x248` range, and resident
runtime record 127 at `func_8000AEEC+0x34C` is its sole authenticated inbound.
The following `+0x248..+0x250` padding is assembly-owned. Current linked
equality proves fallback only.

Overlay 79 `+0x1290` (`func_overlay_079_F0001290_18CE230`) owns 15
runtime-backed records: paired SYMBOL calls to the random, spawn, emit-at, and
finish roles at `+0x7C/+0x138`, `+0x8C/+0x148`, `+0xB4/+0x1B8`, and
`+0xCC/+0x1D0`; local call `overlay79FindNearby` at `+0xDC`; emit and trigger
roles at `+0x17C/+0x194`; a LOCAL HI16/LO16 counter pair at `+0x154/+0x158`;
and a SYMBOL flags pair at `+0x184/+0x188`. Runtime identity resolution maps
the resident calls to `mathRnd`, `func_8000590C`, `func_80002FE0`,
`func_80006EA0`, `func_80000F94`, and `func_800291B4`, all with zero addends;
the local jump targets Overlay 79 `+0xEFC`, the counter pair resolves to
Overlay 79 `+0x14F4` through addend `+0x14`, and the flags pair is the reserved
loader SYMBOL with zero addend. An identity-canonical comparator run proves
all 15 candidate/runtime offsets, types, identities, and addends. The assembled
target exposes 13 static records because the runtime table patches the already-
zero flags pair. Retained genuine C is 111/123 raw/runtime-normalized words;
linked equality proves fallback only. ORT 1297 and resident relocation 139 at
`func_8000AEEC+0x43C` authenticate the sole inbound. No cross-overlay inbound
or target padding exists; production trims only four non-owned section
alignment bytes.

Overlay 97 `+0x508` (`overlay97InitScale`) owns no static or runtime
relocations across its exact `+0x508..+0x748` range. Historical configured C
under `-O2 -mips2 -32 -Wab,-r4300_mul` was measured frameless and 143/144
raw/normalized words; no candidate artifact survives and current C output is
pending. Its sole historical `+0xD0` difference was an address-equivalent
carrier choice, not a relocation. ORT 1194 and resident runtime record 36 at
`func_8000AA38+0x7C` authenticate the sole inbound. There is no target padding;
Overlay 97's separate `+0xA54..+0xA60` padding follows another function.
Current linked equality proves fallback only.

Overlay 43 `+0x1378` (`overlay43FilterImage`) owns no static or runtime
relocations across its exact `+0x1378..+0x1424` range. Two local JUMP records
at overlay `+0x218/+0x24C`, both from
`func_overlay_043_F0000194_188A164`, are its only authenticated inbounds; it
has no ORT export, resident or cross-overlay inbound. Policy-clean configured
full-TU C is exact-sized and frameless at 8/43 raw/normalized words, first
`+0x4`, and its object confirms an empty relocation table. All 119 flags are
nonexact; one UOPT trace plus bounded natural forms isolate a remaining
temporary-FIFO/web-coalescing source blocker. The following
`+0x1424..+0x1430` three-word assembly padding is separate ownership, while
standalone C's one alignment word is trimmed and earns no credit. Current
linked equality proves fallback only.

Overlay 15 `+0x428` (`overlay15MoveStars`) owns 21 runtime records across its
exact `+0x428..+0x500` range: one SYMBOL `R_MIPS_26` call at function `+0xC0`
to resident `starfieldFastMove`; LOCAL pairs for movement-base `+0xCC0` at
`+0x08/+0x10`, star pointer `+0xC74` at `+0x1C/+0x24`, and count `+0xC70` at
`+0x80/+0x88`; plus five bound high halves and nine bound low halves spanning
runtime identities `+0xCC0..+0xCE0`. Configured C emits 25 records because it
uses nine independent bound symbols and therefore four extra HI16 producers;
only 12 offset/type sites currently align. Pair aggregates reduce records but
form explicit general-register bases, preserving the four-word size excess and
worsening instruction schedule. ORT 1663 exports the function. No resident or
cross-overlay relocation directly targets it, and the next function starts at
`+0x500` with no padding. Linked equality proves fallback only.

Eleven of these were not measurable before this lane. They are the sweep's next
targets: within eight words is the range where the permuter closes candidates.

A later independent reproof of the public Overlay 98 contribution authenticated
`overlay98CollectAccepted` without accepting its instruction-rewriting helper.
`overlay98CollectAccepted` at `+0x144..+0x234` owns six exact runtime tuples:
R_MIPS_26 resident `+0x241DC` at function `+0x2C`; LOCAL HI16/LO16 identities
Overlay 98 `+0xAB4` (stored addend `+0x84`) at `+0x34/+0x38` and `+0xAB8`
(stored addend `+0x88`) at `+0x4C/+0x50`; and the local JUMP to Overlay 98
`+0x848` at `+0x74`. A later fidelity-gated IDO stack-home trace selected the
natural carrier/lifetime form that emits the same six offsets/types and all
60 instruction words with the retail `0x50` frame. Canonical promotion proves
all six effective identities through the runtime table and linked ROM, plus
the linked owner, complete overlay, and full-ROM identity. ORT 1322 and
resident relocation 164 authenticate the inbound from
`func_8000E5EC+0x190` (ROM call site `0xF7CC`).

The same reproof authenticated `overlay98CheckObject` at `+0x848..+0xA04`.
It owns six runtime tuples: R_MIPS_26 resident `+0x1312C` at `+0xA4`; LOCAL
HI16/LO16 identities Overlay 98 `+0xAB0` (stored addend `+0x80`) at
`+0xE4/+0xE8` and `+0xD38` (stored addend `+0x308`) at `+0x108/+0x10C`;
and R_MIPS_26 resident `+0x124CC` at `+0x154`. Canonical `-O2 -mips2`
places all six sites four bytes early; `-Wab,-r4300_mul` restores every
offset/type and the exact 111-word extent, but metadata-only rebinding and
section trimming still leave only 78/111 linked words equal. It has no export;
its sole inbound is the local JUMP from `overlay98CollectAccepted+0x74` at
module offset `+0x1B8`.

Resident `func_8000A830` owns three exact configured records across its
`0x208`-byte range: a PC16 default branch and the compiler-table HI16/LO16
pair. It is frameless with no saved registers or outgoing calls. Its sole
inbound is `func_8000590C+0x3BC`, passing the object and storage cursor and
consuming the returned size. There is no ORT export or runtime inbound.
The direct switch expression removes the named type carrier and reproduces
all 130 instruction words under the existing shared-TU flags. The 119-row
flag lattice attempted every configuration, scored 53 and retained 66 tool
rejections; no row was exact before that source change.

Resident `func_8000AA38` owns 74 exact configured records across its
`0x4B4`-byte range: one PC16 default branch, a compiler-table HI16/LO16
pair, six resident calls and 65 runtime overlay calls. The runtime table
identifies every overlay by module and offset; those calls intentionally
share the resident `TrapDanglingJump` encoding. ORT 169 exports the dispatcher,
and `func_8000590C+0x9D8` is its sole direct inbound, passing object, entry
and zero preserve-state. No resident runtime, overlay SYMBOL or stored-pointer
inbound references that export. The source forwards all three argument
registers; the character-control case supplies its separately proved mode one.

IDO resolves both default branches internally. Same-section symbols at the
branch sites permit asserted PC16 records with unchanged addends; ordinary
linking preserves each displacement and reproduces the respective fallback's
exact destination. Both 92-entry compiler tables now occupy their original
ROM positions in the shared `0x52C` rodata prefix, after the earlier tables
and constants. Only the final zero alignment word is trimmed. This retires
the first dispatcher's temporary retained-table labels, filters and absolute
rebinding. A named symbol at the compiler input rodata section's zero offset
binds both table address pairs without changing their addends; the linked ELF
therefore exposes their actual section base without anonymous-section ambiguity.
Configured and raw TU executable sections remain identical. Independent
comparisons against the archived fallback tuples prove both relocation counts,
types, offsets and effective identities; all 184 table destinations, both
linked owned ranges, the complete shared rodata range and full ROM are exact.

Resident `func_8000AEEC` owns 312 executable words at ROM
`0xBAEC..0xBFCC`, without padding, and uses a 32-byte frame saving the object,
update rate and return address. The two direct calls are
`func_8000784C+0x188` and `+0x364`; there is no ORT export. Its 69 configured
relocations reproduce every archived fallback offset/type and effective
identity: 23 static identities plus 46 authenticated runtime overlay calls.
The latter retain the resident trap encoding at their original patch sites.

A direct signed-halfword-minus-three selector removes all 65 register-only
residuals under the unchanged shared-TU flags. The full 119-row lattice was
attempted, with 53 scored configurations and 66 retained tool rejections; none
was exact. The compiler's default branch already reaches its exact destination;
a site-bound PC16 record preserves its addend, and the existing named rodata
base binds its table address pair. Raw and configured executable sections are
identical. The fifth compiler table extends the shared input section to
`0x690` bytes with no trailing alignment removal. Independent comparison of
all 89 destinations exposed four erroneous draft case labels even though the
linked function's instructions were exact: source case order 78, 79, 80, 77
restores those runtime bindings. The owned linked range, all five tables and
three floats, and the full ROM compare exactly.

### 6.2 What the site alignment unlocks

The 77 `schedule-divergence-at-site` rows re-measured, in lane
`lane/reloc-sched`. Two measurements, because they answer different questions.

**The surface, over all 77.** Each candidate's TU compiled with its
`NON_MATCHING` body promoted, then valued by `synthesize()` (§4.1). No link, so
no load gate:

| outcome | count |
|---|---:|
| alignment accepted, and the object has no refusal left -- the surface values every symbol | 19 |
| alignment accepted, refusals remain (two sites of one symbol demand different addends) | 2 |
| no alignment needed; the refusals are per-symbol addend conflicts at corroborated sites | 2 |
| alignment refused: no unique order-preserving match | 54 |

The 21 accepted alignments move 466 sites off the offset the table names.

**The linked-ROM trial**, `--resume --jobs 1`, the first 39 rows in queue order
before the workstation load gate stalled the run; the remaining 38 are queued:

| class | rows |
|---|---:|
| `build-error / schedule-divergence-at-site` | 22 |
| `text-size-differs` | 9 |
| `build-error / rom-size` | 4 |
| `text-differs` | 3 |
| `rom-size` | 1 |

Nine of the 39 carried an alignment, and eight of those left the divergence
class: `func_overlay_011_F00022E8_186AB30` (51 of 79 sites shifted),
`func_overlay_012_F00003A8_186D628` (33/33), `func_overlay_014_F0001830_1871108`
(13/14), `func_overlay_041_F0001464_188879C` (5/5) and `overlay19BuildPlanes`
(3/4) to a size verdict -- an ownership carve, not code work -- and three to a
word count: `func_overlay_027_F0000064_187BA3C` **62**,
`func_overlay_031_F0000000_187F520` **94**, `func_overlay_029_F00005C4_187D874`
**438**. `overlay19BuildPlanes` carries both, `rom-size` at 512 in-range words.

The ninth, `overlay10Initialize`, aligns all 41 of its sites (26 shifted) and
still refuses: three symbols whose *aligned* sites disagree about the addend.
That is the §4.1 refusal working as intended -- the alignment made the sites
readable and the readings then contradicted each other, which is real codegen
divergence, not a missing offset.

The other nine rows that left the class did so without any alignment; they are
the harness fix in the same lane (a trial projection that moved only
`config/overlays.us.json` never re-split, so `prune-asm`'s deletions stranded
an `overlay_001_head` .s that the whole link needs).

None of the 77 came out `exact` or `text-exact`: an unlocked row buys a number,
not a promotion. What it changes is the routing -- `text-differs` with a word
count goes to the permuter sweep or a lever, `rom-size` to the ownership carve,
and only a row that still refuses is read as structural.

**The 54 refusals.** All are the same shape: the candidate's relocation-type
sequence does not embed uniquely into the module's record sequence over that
TU's text range. Two causes, both honest. Either the candidate emits a
relocation the shipped function does not perform, or omits one it does -- a
differing multiset, which is a structural disagreement and not a scheduling
one; or the sequence embeds two ways, which happens when the surplus records
in the range have the same types as the sites being placed (a run of
HI16/LO16 pairs with an unspelled intra-module reference among them). The
second class is a limit of matching on type alone: the shipped table names a
LOCAL record's section, not the symbol, so two `%hi` sites over the same
section are indistinguishable to it. Narrowing them would need a per-record
identity the table does not carry for LOCAL rows, so the tool refuses instead
of choosing.

## 7. What is still hand-written

- **Section externalization.** `externalize_elf_section.py` takes either the
  expected payload as a hex literal or its SHA-256 digest in the Makefile,
  which is the one part of the machinery not derivable from addresses alone.
  The digest form keeps larger private literal pools fail-closed without
  embedding their bytes in tracked text.
- **The `POSTPROCESS` trim sizes themselves.** They are the ownership row's
  extent and could be emitted from the atlas rather than written out per file;
  this lane made the *failure* derivable, not yet the rule.
- **Relocation filters and instruction normalizations.** Genuinely per-function
  reviewed assertions; nothing here suggests they are mechanical.
- **A lone `R_MIPS_LO16`** still determines only the low half of its symbol's
  value (§3.2). Byte-identical output, non-canonical symbol value.

## 8. Cleanroom note

`tools/reloc_surface.py` reads the baserom and `config/overlays.us.json` at run
time and emits only symbol names and the addresses/values the link already
requires. It writes no extracted data, embeds no ROM bytes, and prints no
instruction text. This document quotes no ROM words.

The generated `overlay_undefined_syms.us.txt` is tracked, and is the same class
of content as the hand-maintained file it replaces: symbol names and the
relocation addends the link requires, in the same two line forms. It is
smaller than what it replaced (2,265 lines against 2,928) and carries no
instruction text, so the clean-room detectors see strictly less than before.
