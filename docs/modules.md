# The module map

What lives where in Mickey's Speedway USA (USA), and how each claim was
established. This file is the naming convention later phases follow, plus the
address map those names hang on.

Two rules govern everything below.

1. **Every claim carries its evidence.** A range with no stated method is a bug
   in this document, not a fact about the ROM.
2. **Evidence tiers are never mixed.** A byte-identical match and a plausible
   reading of a disassembly are labelled separately here and in
   `symbol_addrs.us.txt`, and the labels are load-bearing.

All ROM offsets are byte offsets into `baseroms/mickey.us.z64`
(SHA1 `507341c0a40ca3e9a7cee969b396ee53facfb548`). For the resident segment,
`VRAM = ROM + 0x7FFFF400`.

---

## 1. Naming convention

### 1.1 The four tiers

| Tier | What it is | What may be adopted | Where it is used |
|---|---|---|---|
| **A — byte-identity** | The ROM's bytes are identical, under relocation masking, to a named symbol in a permitted decomp's *built* objects | The reference build's name, verbatim | the libultra corridor, DKR game code |
| **B — call graph** | The function's role is pinned by who calls it, with what, and what the caller does with the result | An invented descriptive name, or a public decomp's name where the role matches exactly | `piacs`/`siacs`, the linker cluster |
| **C — string correspondence** | The function builds the address of a distinctive string literal, and a public decomp of the same engine uses that literal in a function whose job matches | That project's name | `diRcp*`, `diCpu*`, `gsSndp*`, `osScGetTaskType` |
| **D — structural inference** | Read off the disassembly and nothing else | An invented descriptive name | `SetLinkSlot`, `ReleaseUnusedLinkSlots` |

Tier A is the only tier that is a *measurement*. B, C and D are arguments, and
an argument can be wrong; the tiers are kept apart so a reader can tell, at the
point of use, how much a given name is worth.

### 1.2 Tier A: the adoption threshold

A byte-identical match is adopted as a name only when **all three** hold:

- **at least 6 unmasked instruction words** compared. Masked words are the ones
  the reference object's own relocation records say the linker patches; they
  prove nothing.
- **unique across the whole 32MB image.** Not unique within the window that was
  scanned, a distinction that has already cost one wrong claim. `occ` in
  `tools/find_known_objects.py` is window-scoped; **`--rom-occ` computes the
  figure this bullet asks for**, and every name adopted since it existed
  carries a measured `romocc`. Where a comparison has no run of two consecutive
  unmasked words to anchor a whole-image search on, the tool prints `romocc=?`
  and the row is not adoptable on uniqueness grounds.
- **exactly one candidate name** for those bytes. Several different functions
  in one reference build routinely compile to identical instructions; when they
  do, byte comparison cannot say which one this is, and the row is noise.

**Six adopted symbols fall below the 6-word bar, resting on four distinct
arguments.** Every one is disclosed at its point of use in
`symbol_addrs.us.txt`, and each rests on something a word count does not
measure:

| Symbol | Words | What carries it instead |
|---|---|---|
| `osScGetCmdQ` | 2 | The **pair**: two *different* immediates (`addiu v0,a0,0x78`, `addiu v0,a0,0x40`), 8 bytes apart, in the reference's order, each ROM-wide unique |
| `osScGetInterruptQ` | 2 | as above |
| `stack_pointer` | 2 real | Semantically unambiguous on its own: the whole body is `jr ra` / `move v0,sp` |
| `__osGetCause` | 2 real of 4 | Same argument as `stack_pointer`: the body is `mfc0 v0, Cause` / `jr ra`, and **those two words occur exactly once in the 32MB image**, unlike the rejected `leointerrupt`, whose body alone occurs 39 times, so the padding is doing none of the work. Three reference builds name those bytes, all identically |
| `__osExceptionPreamble` | 2 unmasked of 4 | Shape: `lui k0,0x8005` / `addiu k0,-4032` / `jr k0` jumps to the very next instruction |
| `__osPopThread` | 4 unmasked | Island context: three other matches already establish that this is `exceptasm.s` |

Six rows, four arguments: `osScGetCmdQ` and `osScGetInterruptQ` share one, and
`__osGetCause` reuses `stack_pointer`'s. A pass that adds a fifth argument
should be treated as suspect; exceptions are meant to stay rare enough to
enumerate.

Passing the threshold is not the same as the name being *right about Mickey*.
Tier A establishes that the routine is the same routine; the reference build's
name may still describe that project's use of it. `weather_clip_planes` and
`audspat_jingle_off` are carried because renaming shared code would hide the
sharing, and are flagged where they appear.

**A whole-`.text` match outranks a standalone function match.** When both are
available for one address, the whole-object match wins and the standalone one
is recorded as noise; see the `__osPfsGetInitData` and `__osPiGetAccess`
collisions in `symbol_addrs.us.txt`.

### 1.3 When a public decomp's name is adopted, and how it is disclosed

`docs/CLEANROOM.md` permits public retail-derived decompilations (DKR, JFG, PD,
BK, Conker) as sources. This project reads **all five**, and
`docs/references.md` records each one's repo URL, pinned commit, verified
baserom checksum, build outcome and match yield. The two that supply most of
the tree are:

- **Diddy Kong Racing**, for tier A, its *built objects only*: compiled bytes
  plus symbol and relocation tables. That restriction is what makes tier A
  evidence rather than transcription, and it is a claim about the method, not
  about what a human was allowed to read: both projects' sources are permitted
  reading under `docs/CLEANROOM.md`, and DKR's was grepped for string literals
  during the tier-C work. It produced **no adopted name**; every DKR name in
  the tree came out of a built object.
- **Jet Force Gemini**: two uses, not to be conflated. Its *published source
  text* was read to answer "what did that project call the function that does
  this", and (in `src/main/runlink.c`) for adapted function bodies, disclosed
  at the point of use. Separately, its *built objects* are the largest single
  source of tier-A names in the tree: **84 of the 87 translation units that
  pass adopted are JFG's, carrying 187 of its 190 names**, on the same
  built-objects-only basis as DKR's.
- **Perfect Dark, Banjo-Kazooie and Conker**: built objects only. PD
  contributed three names (the Transfer Pak driver, which no other reference
  build contains). BK and Conker contributed **none**, and that documented zero
  is in `docs/references.md` alongside what they did contribute:
  independent corroboration of **2** and **8** of the adopted translation units,
  and re-confirmation of **73** and **65** subsegments Phase 1 had already
  named. Those are two different measurements and they are not summed; see
  `docs/references.md` §"Three different things, counted separately", and note
  that only the corroboration figures are recomputable from this tree. None of
  the three had its source read for any purpose.

Nothing in this tree was independently derived from Mickey's bytes and then
found to agree with a reference project. **The names are borrowed.** What the
byte comparison establishes is that the routine is the same routine; the name
comes from the other project, and that is what tier A means here.

Every borrowed body in this tree carries an explicit `PROVENANCE` disclosure at
the point of use, and so does every block of borrowed names.
`src/main/runlink.c` opens with one, because the bodies in it are adapted from
JFG's `runLink.c`; the tier-C block in `symbol_addrs.us.txt` carries another;
each `src/libultra/*.c` carries a one-line note that its body is SDK libultra
source as published in public decomp trees. The rule is not scoped to JFG:
**the provenance line goes in before the body.** Byte-identity against Mickey's
ROM is what makes the borrowing sound, not a reason to leave it unmentioned.

Where Mickey's ROM and a public decomp disagree, **Mickey wins and the
divergence is written down**. Current divergences: `MipsInstruction`'s field
order, `runlinkCallResumeFunction`'s do/while-15 scan, and `RelocTableEntry`,
whose layout is not JFG's at all (see §5.2).

### 1.4 Casing

Follow the surrounding code; do not impose a house style on borrowed names.

| Kind | Style | Examples |
|---|---|---|
| libultra | whatever the SDK calls it | `osRecvMesg`, `__osViSwapContext` |
| Engine code shared with DKR/JFG | that project's spelling, unchanged | `mtxf_transform_point`, `diRcpPrintDL`, `gzip_inflate_codes` |
| The runtime linker | JFG's `PascalCase` / `runlinkCamelCase` mix, unchanged | `ResolveRelocAddress`, `runlinkDownloadCode` |
| Mickey-specific inventions | `PascalCase` for functions, `lowerCamel` for data | `SetLinkSlot`, `overlayCount` |
| Structs / types | `PascalCase` | `OverlayHeader`, `RelocTableEntry` |
| Macros | `SCREAMING_SNAKE` with a family prefix | `RELOC_TYPE_*`, `SC_TASK_*` |

The mix is deliberate and is not worth normalising. A name spelled exactly as
DKR or JFG spells it can be grepped against that project; a name that has been
tidied cannot.

### 1.5 What must not happen

- No name without a stated tier and stated evidence.
- No name for a function whose C is parked non-matching; it would put a second
  evidence tier into the symbol file. Such names live in the source file's
  comments only.
- No name inherited from a reference build's *address placeholder*. Importing
  one would assert an address that does not exist in this game. Two such cases,
  different in kind: `func_80070058` (DKR's `math_util.s`) matched at ROM
  `0x2A90C` and is left unnamed with the reason recorded in
  `symbol_addrs.us.txt`; `func_800676F8` (JFG's `diCpu.c`) is what the string
  evidence for Mickey's `0x80045BBC` pointed at, so that address keeps its own
  `func_` name. Beyond those, **37 placeholder addresses fall inside the
  translation units the cross-title pass matched** (mostly inside JFG's large
  `n_audio` and `gsSnd` objects, which that project has matched but not yet
  named), and none was imported. The rule is applied by the generator, not by
  judgement, which is why the number can be this large without being a risk.

---

## 2. Top-level ROM map

| Range | Size | What | Evidence |
|---|---|---|---|
| `0x000000`–`0x000040` | 0x40 | header | splat `header` segment |
| `0x000040`–`0x001000` | 0xFC0 | IPL3 boot | standard N64 layout |
| `0x001000`–`0x086640` | 0x85640 | **resident segment** (code + data + rodata) | §3 |
| `0x086640`–`0x087000` | 0x9C0 | table of ROM offsets (unidentified) | entropy transition; still `bin` |
| `0x087000`–`0x16B0000` | 22.16 MiB | compressed assets | entropy 7.1–8.0 across the band |
| `0x16B0000`–`0x1848B70` | 1.60 MiB | unclassified | Nothing reaches it: the resident segment builds four address literals in `0x1600000`–`0x18FFFFF` and they are the four block bases below. Entropy 5.5–6.4 with a 0.35 MiB flat run |
| `0x1848B70`–`0x184C3E0` | 0x3870 | **the three overlay tables**, flat and uncompressed | §5.3 |
| `0x184C3E0`–`0x18F1FE0` | 0.65 MiB | **107 overlay module images** | §5.3 |
| `0x18F1FE0`–`0x2000000` | 7.06 MiB | `0xFF` fill | verified byte by byte |

The resident segment's end is derived, not guessed: the entrypoint zeroes BSS
from VRAM `0x80085A40`, and `0x80085A40 - 0x80000400 + 0x1000 = 0x86640`, so
the ROM only needs to supply bytes up to there.

### 2.1 The build stamp

ROM `0x7AD00` holds three consecutive string pointers: to `"1.1153"`,
`"18/08/00 13:08"` and `"pmountain"` at `0x80081A80`/`0x80081A88`/`0x80081A98`.
A version, a build timestamp and a build-host or branch tag. 18 August 2000 sits
about three months before the game's release, which is consistent with a
release-candidate build. Nothing in the resident segment's disassembly builds
the address of this triple, so whatever prints it is in an overlay.

---

## 3. The resident segment (`main`)

The resident ledger (per-translation-unit census, evidence, plateaus and
PROVENANCE) moved to [`docs/resident.md`](resident.md) on 2026-08-25 when
this file crossed the 256 KB tracked-file limit again. Section numbering
there continues as 3.x.

## 4. libultra

### 4.1 The corridor: ROM `0x6F420`–`0x76D10`

VRAM `0x8006E820`–`0x80076110`, `0x78F0` bytes. **101 named subsegments,
including 100 measured whole-`.text` file boundaries plus one exact
function-tail carve, and 130 named functions**, all tier A. The yaml carries
the boundary argument at both ends and
`symbol_addrs.us.txt` carries the per-function names.

**Where the drift went.** The first sweep, against DKR's built libultra alone,
named 80 subsegments and 107 functions and left `0x1AE0` (22.2% of the
corridor, in ten runs) unnamed: libultra-shaped code not byte-identical to
DKR's build, with a best-alignment fuzzy scan at 35% tolerance returning zero
candidates. Those runs are not drifted copies of DKR's libultra; they are a
*different build*. Run the finder over Jet Force Gemini's libultra and **eight
of the ten runs fall**, in fifteen whole-`.text` matches.

The remaining unnamed code is `0x0`, **0.0% of the corridor**, in zero runs.
Every executable byte in the corridor now has an authenticated owner; the
last gap closed with Mickey-specific reconstructions of the adjacent EEPROM
read and write translation units.

`__osPiGetAccess` ("the same 17 instructions as DKR's, scheduled differently")
is named from JFG's whole `io/piacs.c` TU at `0x80071B80`. `libultra/piacs`
was the one corridor subsegment named without a measured boundary; JFG measures
it, which is why all 100 whole-TU subsegments now carry measured boundaries.
The exact function-tail carve is explicitly narrower:
`__osEepStatus` is byte-exact to DKR's `io/conteepwrite.c`, while the preceding
functions in that donor TU are not, so no larger file-boundary claim is made.

### 4.2 libultra outside the corridor

**The corridor is where libultra is contiguous, not where libultra exists.** A
whole-image sweep against four more reference builds found **68 translation
units of libultra below the corridor**, in three blocks.

| ROM | VRAM | What | TUs | Names | From |
|---|---|---|---|---|---|
| `0x4FC30`–`0x505E0` | `0x8004F030` | `os/exceptasm.s`, the exception handler and thread dispatcher | 1 | 9 | JFG |
| `0x5E6B0`–`0x6AF90` | `0x8005DAB0` | the `n_audio` synthesis library (45 TUs) with two JFG maths TUs interleaved | 47 | 106 | JFG |
| `0x6B3D0`–`0x6F3E0` | `0x8006A7D0` | Transfer Pak, Rumble Pak, Controller Pak filesystem | 18 | 34 | JFG + PD |

Plus the two scheduler accessors at `0x30F10`/`0x30F18` (`osScGetCmdQ`,
`osScGetInterruptQ`, `sc/sched.c`), which remain the only libultra named below
the corridor that is *not* part of a measured TU.

**The exception island is stock libultra.** The whole `0x9B0` `.text` of JFG's
built `os/exceptasm.s.o` matches ROM `0x4FC30` in one piece, 76 of 620 words
masked, ROM-wide unique, including `__osException`, `__osEnqueueThread`,
`__osDispatchThread` and `__osEnqueueAndYield`, which match nothing in DKR's
build at all. The TU ends at `0x505E0`, not `0x506D0`; the `0xF0` bytes between
are a separate, still-unidentified run, split off in the yaml. The code stays
`asm`: it is hand-written assembly, and identifying it did not require
decompiling it.

**The corridor's lower boundary.** The yaml once justified `0x6F420` partly on
"the function ending at `0x6F3DC` is game-shaped (saves s5/s6)". That function
is `corrupted`, the last routine of libultra's own `io/pfschecker.c`: the
observation was accurate, the inference from it was not. libultra is contiguous
from `0x6B3D0` to `0x76D10` apart from `0xEF0` of unnamed code, of which
`0x920` is the corridor drift above, `0x590` is between the Transfer Pak
routines, and `0x40` is the hole at `0x6F3E0`–`0x6F420`.

### 4.3 What was rejected, and why

Ten candidates below the corridor have been rejected or reconsidered; all are
listed, because a rejection is a result and because two of them proved
revisable once more reference builds existed.

**Reversed**: the reason held against the evidence then available and does not
hold against the evidence now:

| ROM | Was | Now |
|---|---|---|
| `0x61990` | `alCSPGetState`, rejected: a one-line accessor whose ROM-wide uniqueness is an accident of padding | `n_alCSPGetState`, adopted: the address falls inside a whole-`.text` match (`0x20`, 0 masked, `romocc=1`), so what places it is the translation unit, not the accessor's own bytes. The `n_` prefix is also new information: this is the n_audio variant |
| `0x620E0` | `alCSeqGetTicks`, same objection | `n_alCSeqGetTicks`, adopted, inside `n_cseq.c`'s whole `0x9D0` TU |

**Standing:**

- `io/leointerrupt` (`0x4FC20`): whole `.text` matched, ROM-wide unique, and
  the entire body is `jr ra` / `move v0,zero`. The uniqueness is an accident of
  8 bytes of padding, and that is measured rather than asserted: the section
  occurs once in the image, the two-instruction body alone occurs **39 times**.
  Banjo-Kazooie also matches here and offers only the placeholder
  `func_8038AAB0`.
- `__osDisableInt`/`__osRestoreInt` (`0x2A25C`/`0x2A288`): `romocc=2`, and the
  bytes are the tail of the larger, ROM-wide-unique
  `interrupts_disable`/`interrupts_enable` in DKR's `math_util.s`, which is why
  they matched at an offset of `0xC`. Not, as once read, a second copy of
  `os/interrupt.s`.

**From the cross-title pass:**

| ROM | Candidate | Ground |
|---|---|---|
| `0x20008` | Conker `src/init_3920.c.o` | Places only `func_10003920`, a reference-build address placeholder (§1.5) |
| `0x2C8AC` | PD `game/stubs/game_11eff0.o` | `stub0f11eff0`, an 8-byte stub. Semantically vacuous, same ground as `leointerrupt` |
| `0x58E40` | ten objects across four titles | A `0x10` `return x->field`. **Ten candidate names**, `romocc=?`. Fails §1.2's one-candidate rule outright, and the `?` is the tool declining to answer rather than a `1` |
| `0x63B40`, `0x68110`, `0x76C80` | Conker `n_synaddplayer`, PD `n_save`, Conker `ldiv` | **Subsumed.** Each is a fragment of a TU already matched whole at a lower address. §1.2's "a whole-`.text` match outranks a standalone function match" |
| `0x2A2B0` | BK `src/mgu/mtxxfmf.o` → `guMtxXFMF` | A clean whole-object match (`0xA0`, 0 masked, `romocc=1`), and its **boundary is still not declared**. The surrounding evidence is stronger and says something different: `0x2A250`–`0x2AE44` is thirteen routines matched against DKR's *one* hand-written `math_util.s` object, in DKR's order. Mickey inlines the SDK's `guMtxXFMF` into a hand-written maths file, exactly as DKR does, so declaring a TU boundary here would assert a file that is not there. The identification is recorded in `symbol_addrs.us.txt` §6; the address keeps `mtxf_transform_point` |

### 4.4 n_audio: matching progress

Per docs/acceleration-survey.md §13.3's ruling, `n_audio` is unblocked:
JFG's fully-matched naudio tree (`libultra/src/naudio/`, commit
`c75c270`, "Finish matching libultra naudio files") supplies matched C
bodies with `PROVENANCE` notes, using JFG's headers (imported under
`include/n_audio/**` and `include/n_libaudio.h`) and the bare-`OPT_FLAGS`
(`-g`, no `-O`) + `-mips2 -32` flag group already measured on
`n_cspsetvol`.

All 45 `n_audio` TUs are matched (`n_cspsetvol` and `cents2ratio` were
adopted before this pass; the other 43 use disclosed JFG bodies). This includes
every masked=0/1/2 TU (the
thin `N_ALEvent` posters and one-line accessors), `n_sl` (which places
the driver singletons `n_alGlobals`/`n_syn` — VRAM `0x80080160`/
`0x80080164`, measured directly off a built candidate diffed against the
ROM with `tools/wb_compare.sh --rom n_alInit --show-diff` and carved out
of the resident `.data` band in `mickey.us.yaml`, since almost everything
else in the library reads `n_syn`), the seven `ALParam`-update setters
that funnel through it, `n_synallocfx`, `n_alcspchan` (needs
`-DRAREDIFFS` for Rare's added MIDI control-change codes), and
`n_syngetfxref`. The final TU, `n_synfreevoice`, contributes `0xD4` executable
bytes plus `0xC` bytes of compiler section alignment. Its five relocations
retain the exact count, types, offsets, and symbol identities, and the full
`0xE0`-byte `.text` section is byte-identical to the whole-object JFG donor.
The `0x3220`-byte `n_csplayer` text and its owned
`0x200`-byte `.data`, `0x300`-byte `.rodata`, and `0x40`-byte `.bss`
also match exactly from the JFG source; its multiply sequences require
`-Wab,-r4300_mul` in addition to `-DRAREDIFFS`. JFG's main compressed-sequence
source (`n_csq.c`) also supplies the exact `0x9D0`-byte `n_cseq` text with the
bare flag group and no owned data or rodata. The JFG `n_reverb.c`/`n_save.c`
pair supplies Mickey's combined `0x16B0`-byte reverb TU plus its `0x30` bytes
of rodata; it requires `-DN_MICRO -Wab,-r4300_mul`. The 15-function,
`0x1160`-byte `n_seqplayer` TU matches the JFG source under the bare flag group.
JFG's `N_MICRO` resampler path supplies the exact `0x2A0`-byte `n_resample`
text and its `0x10` bytes of rodata under the same bare flag group.
The same JFG flags reproduce all `0x100` text bytes and the six relocations of
`n_synsetfxparam`; assigning its `0x10`-byte literal section to ROM `0x853B0`
also resolves the recorded `0.1f` rodata-offset plateau.

`alsurround` also owns a `0x10`-byte `.bss` section at Mickey VRAM
`0x800D7DC0`: the two linked functions' HI16/LO16 references place its
four donor-named globals there in declaration order, independently of the
JFG addresses embedded in their imported identifiers.

`n_event` owns the following `0x10`-byte `.bss` section at VRAM
`0x800D7DD0`; ten linked HI16/LO16 references independently place its
external-event clock at the section start.

`n_synsetfxparam` additionally owns the `0x10`-byte `.rodata` island at
ROM `0x853B0`; separating it from the formerly undifferentiated pool
places `n_alSynSetOutputLPParam`'s `0.1f` literal at its linked address.

`n_load` is the SDK's n_audio microcode branch (`-DN_MICRO`) and contributes
three exact functions, `0xB4C` executable bytes, four relocations, and the
translation unit's final four-byte alignment word. The internal decoder is
named `_decodeChunk` from the permitted donor source; its three callers retain
that exact relocation identity.

`n_alLPFilter` owns the `0x10`-byte `.rodata` island at ROM `0x85470`.
Its two functions contribute `0x41C` executable bytes and 13 relocations; the
translation unit ends with one separate four-byte alignment word.

`n_drvrNew` uses Rare's per-bus configuration layout (`-DRAREDIFFS`) and
the R4300 multiply scheduler. Its six functions contribute `0xD0C`
executable bytes and 34 relocations, followed by one alignment word. The TU
also owns `0x40` bytes of initialized effect parameters at ROM `0x80D80` and
the `0x20`-byte constant island at ROM `0x85390`; both sections compare
directly against their retail ranges.

`n_synthesizer` uses Rare's per-bus effect layout and the n_audio microcode
ABI (`-DRAREDIFFS -DN_MICRO`). Its nine functions contribute `0xAC8`
executable bytes and 173 relocations, followed by IDO's eight-byte return-stub
alignment at `0x800623CC`. The TU also owns `0x10` bytes of initialized
parameter counters at ROM `0x80D70` and the `0x10`-byte floating-point constant
island at ROM `0x85370`; both sections compare directly against retail. No
post-compile insertion is involved.

`n_env` is the SDK's n_audio microcode path (`-DN_MICRO`). Its five functions
occupy `0xFEC` executable bytes with 59 relocations, followed by one compiler
alignment word in the `0xFF0`-byte text section. The TU also owns the
`0x100`-byte equal-power table at ROM `0x80DC0` and a `0x50`-byte switch table
and constant island at ROM `0x853F0`; the initialized table compares directly
against retail and the linked rodata is covered by the full-ROM proof.

`n_resample` uses the SDK n_audio microcode branch (`-DN_MICRO`); the earlier
tail mismatch came from compiling the non-n_audio command path. Its two
functions occupy the full `0x2A0`-byte text section with eight relocations,
and its `0x10`-byte constant island at ROM `0x85490` compares directly against
retail.

---

## 5. The overlay system

The overlay ledger (module layout, per-overlay ownership and evidence, the
donor scan, and the retired normalization notes) moved to
[`docs/overlays.md`](overlays.md) on 2026-08-24 when this file crossed the
256 KB tracked-file limit. Section numbering there continues as 5.x.

## 6. Compiler flags

### 6.1 What is measured

| Scope | Flags | How established |
|---|---|---|
| Project default | `-O2 -mips1 -32` | splat/IDO preset; not measured |
| `src/main/` (game code) | `-O2 -mips2 -32` | **Measured.** `ResolveRelocAddress` at `-mips1` emits five load-delay `nop`s the ROM does not have |
| overlay game code, except overlay 5 | `-O2 -mips2 -32` | **Measured** across tranche-A leaves and structural pilots |
| selected overlay TUs | `-O2 -mips2 -32 -Wab,-r4300_mul` | **Measured per object** where the R4300 hazard schedule changes emitted words; kept as narrow `mk/overlays.mk` overrides |
| six overlay 5 audio-bank patcher TUs | `-O3 -mips2 -32` | **Measured.** DKR supplies the source/flag crosswalk; Mickey's own text fixes six separate boundaries. The game-code `overlay_005.c` TU remains `-O2` |
| `src/libultra/string.c` | `-O2 -mips2 -32` | **Measured.** Branch-likely instructions |
| 49 libultra io/os TUs | `-O1 -mips2 -32` | **Measured**, one variant at a time. At `-O2` IDO folds away a stack frame the ROM has. Locals need `register` or `-O1` spills them |
| 23 libultra PI/EPI/PFS TUs | `-O2 -g3 -mips2 -32` | **Measured**, one TU at a time. See below |
| 1 libultra TU (`setglobalintmask`) | `-O1 -mips2 -32`, `uopt` forced on at `-O1` | **Measured.** The only group the `cc` driver cannot express; see below |
| 2 libultra libc TUs (`ldiv`, `xlitob`) | all four IDO phases at `-O3 -mips2 -32` | **Measured.** The phase wrapper bypasses the driver's failing `uld` handoff while preserving the phase options that reproduce the ROM |
| `ll` | `-O1 -mips3 -32` | **Measured.** MIPS III compiler-runtime helpers; the generated ELF flags are normalized to O32 after compilation |
| `n_cspsetvol`, `cents2ratio` | `-g -mips2 -32` | **Measured.** Old debug-codegen objects |
| `sinf` | `-O2 -mips2 -32 -Wab,-r4300_mul` | **Measured.** The R4300 multiply-hazard assembler option is required |

The `-mips2` finding now holds for both measured resident game code and the
adopted overlay tranche. It remains expressed as directory-scoped rules, not a
project default: libultra has independently measured per-file flag groups and
future overlays still require at least one discriminating function before a
new module-wide optimisation assumption is made.

`-g3` is IDO's "optimise, but keep full debug information" level, and it
changes code generation. All twelve combinations of
`{-O0,-O1,-O2}` × `{with, without -g3}` × `{-mips1,-mips2}` were compiled and
compared against ROM `0x730A0`/`0x730F0`:

| Flags | Function size | vs the ROM's `0x48` |
|---|---|---|
| `-O0`, any of the four | `0x60` | no |
| `-O1` / `-O2`, `-mips1` or `-mips2`, no `-g3` | `0x44` | no |
| `-O1 -g3 -mips1` | `0x50` | no |
| `-O2 -g3 -mips1` | `0x4C` | no |
| **`-O1 -g3 -mips2`** | `0x48` | **byte-identical** |
| **`-O2 -g3 -mips2`** | `0x48` | **byte-identical** |

The phase wrapper makes `ldiv` and `xlitob` reproducible by asking the driver
for an `-O2` four-stage pipeline, then promoting `cfe`, `uopt`, `ugen`, and
`as1` individually to `-O3`. The earlier ordinary-driver experiment stopped in
`uld` and left `xprintf`/`xldtob` deferred. The 2026-08-27 final JFG pass closed
that gap: invoking `tools/ido/cc` directly at `-O3 -mips2` supplies the required
interprocedural path and reproduces both complete text sections, their
data/rodata, and every relocation. Their Makefile recipes bypass
asm-processor only because its CLI intentionally has no `-O3` mode; no object
instruction is rewritten after compilation.

Three consequences:

- **`-O2` was not established by `epiread`/`epiwrite`, and is now.** On those
  two files `-O1 -g3 -mips2` and `-O2 -g3 -mips2` are byte-identical, so their
  bytes do not discriminate the optimisation level at all, and `-O2` was
  borrowed from JFG's published Makefile. The nine TUs added to the group since
  do discriminate it: `pidma` (ROM `0x6FB90`) is 48 instructions at
  `-O2 -g3 -mips2` and matches word for word, against 68 at `-O1 -g3 -mips2`
  and 78 at `-O0`. So `-O2 -g3 -mips2` is now measured on this group rather
  than assumed for it. The *structural* claim epiread supported still holds:
  `-g3` stops IDO hoisting the third argument's spill into the first `jal`'s
  delay slot and reverses the epilogue's `lw ra` / `lw v0` order, and the ROM
  agrees with `-g3` on both.
- **The ten `-O1 -mips2` TUs above should be re-examined.** They were measured
  before `-g3` was known to be in play. They match byte-for-byte, so nothing is
  wrong; but "the flags that reproduce these bytes" and "the flags this file
  was built with" are different claims, and only the first is established, as
  the `-O1`/`-O2` tie above demonstrates for the new group.
- **`-g3` came from a reference build's configuration, not from deduction.**
  Reading a permitted decompilation's build configuration is the same
  permission as reading its source (§1.3); what makes `-g3 -mips2` a fact about
  Mickey is that it was then measured against Mickey's bytes. `-O2` did not
  survive that test.

A practical side effect: `-g3` makes IDO write a `.u` sidecar into the working
directory for every such compile. `.gitignore` covers it.

`-Wab,-r4300_mul` is now settled for `sinf`: without it the object does not
match, and with it the text is byte-identical. It remains per-file evidence,
not a project default.

**The fourth group: `uopt` at `-O1`, which the driver cannot produce.**
`tools/ido/cc` runs `uopt` only at `-O2` and above — `cc -v` lists no `uopt`
stage at `-O1` × `{-g0,-g1,-g2,-g3,none}` — so "optimised by `uopt`, at `-O1`"
is unreachable through it, and `-Wo,-O1` does not fake it because
`-W<pass>,-O<n>` is inserted *before* the driver's own `-O` and the last `-O`
wins. `tools/ido-phases.py` drives `cfe`/`uopt`/`ugen`/`as1` directly,
taking each phase's command line from `cc -v` rather than reimplementing the
driver, and accepts `-Xphase,<phase>,<option>` overrides (`-Xphase,uopt,+`
forces the stage to exist). With no override it is byte-identical to `cc`,
verified on `string.c`, `epidma.c`, `si.c`, `pfsreadwritefile.c` and on a
`GLOBAL_ASM` TU under asm-processor.

What it buys: `__osSetGlobalIntMask` (ROM `0x75080`) is byte-identical with
`uopt` at `-O1` — 19 of 19 words, only the six the linker fills in differing.
Through the driver the same source gives 20 instructions with the first `jal`'s
delay slot empty, and none of twenty-two `{-O0,-O1,-O2}` × `{-g,-g1,-g2,-g3,
none}` × `{-mips1,-mips2}` combinations closes it. `-g3` is irrelevant here;
`-mips2` is required; `-O2` destroys the frame size.

This is also the strongest evidence in the tree that the vendored IDO *is*
Mickey's compiler for libultra: a whole ROM function, delay slots included,
reproduced exactly once the driver stops hiding a phase. `uopt`'s `-O` is the
only per-phase `-O` with teeth — on `string.c` at `-O2 -mips2`,
`-Xphase,uopt,-O1` changes `.text` from `0xA0` to `0xE0` bytes while
`-Xphase,ugen,-O1/-O3` and `-Xphase,as1,-O1/-O3` are bit-identical to the
baseline.

### 6.2 Odd floating-point registers: settled — those files stay assembly

ROM-wide there are 1727 odd FP register operands across 9 static-segment asm
files, and GNU as warns once per occurrence. **No SGI IDO build can produce
them.** That is a mechanism, not an exhausted sweep, and the question is closed.

`ugen` does implement SGI's `-fp32regs`, which frees the odd single-precision
registers, and the `cc` driver can route it — `-Wc,` is the code generator,
`-Wo,` is `uopt`, which prints "unrecognized option" and drops it, which is why
earlier sweeps found nothing. `cc … -mips2 -32 -Wc,-mips3 -Wc,-fp32regs` is
byte-identical to driving the phases by hand, and it does emit odd registers.
It still cannot produce Mickey's:

- **The mechanism.** In the function-matched decompilation of real IDO 7.1's
  `ugen`, `-fp32regs` is one unconditional loop that adds all 16 odd registers
  `$f1,$f3,…,$f31` to the free list, with no reference to which are argument,
  return or callee-saved halves. The reservation logic that protects live
  registers walks the register file by even steps only — it predates
  `-fp32regs` and was never taught about odd halves. The two paths are
  structurally disconnected, so `-fp32regs` can never confine itself to
  caller-saved temps and can never save what it does clobber, in any
  invocation. The `-ufsm`/`-ufsa` callee-save tracking is a separate feature
  the `-fp32regs` free-list path does not consult.
- **Every version agrees.** Six builds — IDO 4.1, 5.2, 5.3, 6.0, 7.1 and
  MIPSpro 7.4.4 — choose the identical all-32 register set on the same test
  function; 5.2, 5.3, 6.0 and 7.4.4 are byte-identical for it. 4.1 differs only
  in schedule length, not in allocation policy. There is no version boundary to
  find.
- **What our build actually gets.** With `-Wc,-mips3 -Wc,-fp32regs`,
  `MatrixMultiplyVec4` comes out at the ROM's exact 53 instructions, the exact
  instruction kinds, and the odd-register phenomenon — but allocated across all
  32 registers including `$f0/$f2` (return), `$f12/$f14` (argument) and the odd
  halves of callee-saved `$f20`–`$f30`, written in a leaf function with no
  save/restore. The ROM confines itself to `$f4`–`$f11` and `$f16`–`$f18`: the
  caller-saved even temps plus their odd partners, and nothing else.

**Consequence for this project: the nine files stay as assembly. Do not
re-sweep compiler flags or IDO versions for them** — the mechanism above says
in advance that every such sweep fails. Hand-written assembly is the standing
explanation, consistent with the density spread (61.3% of FP operands in
`asm/59DB0.s` down to 2.7% in `asm/16140.s`, the matrix TU at 40.1%) and with
sibling decomps, where genuinely hand-written float assembly (JFG's
`asm/hasm/math_matrix.s`, PD's `src/lib/mtxasm.s`) sits in the same odd-density
range and is equally ABI-indifferent, while real IDO-compiled float code in
those trees contains no single-precision odd-register arithmetic at all.

Two things would reopen it, and only these two: a code generator whose
odd-register free list is gated by the same reservation logic the even path
already uses (no such build is known — it would have to be a hand patch), or a
matched, non-assembly function in some other decomp that uses odd single
registers, with a working build recipe attached.

No TU is built with `-Wc,-mips3 -Wc,-fp32regs` and none is expected to be. If
one ever is, it must be a per-file group and never a default: all seven
already-matching TUs tested change bytes under it, and the damage is entirely
`-mips3` reaching `ugen` (which `-fp32regs` requires) — on FP-free code
`ugen -mips3` and `ugen -mips3 -fp32regs` are bit-identical, so `-fp32regs`
itself is inert there, while `ugen -mips3` stops folding small negative
immediates and grows `libultra/string.c`'s `.text` by 30%. `as1` also asserts
outright on some `-fp32regs` ucode, so any member must be built and checked,
never assumed to compile.

It is not a blocker for the naming work: nothing named so far is float-heavy,
and the tier-A matches that *are* in float-adjacent code (`mtxf_*` in
`math_util.s`) are against DKR's **hand-written assembly** objects, which
sidesteps the question rather than answering it.

Relevant to any future resolution: `mtxf_transform_point`, `mtxf_mul`,
`mtxf_to_mtx`, `vec3s_reflect` and `mtxf_from_inverse_transform` at ROM
`0x2A250` are byte-identical to DKR's `.s` sources. If Mickey's own
`main/matrix` TU at `0x2B650` turns out to be hand-written too, that is the
same story one file over.

**Names are already sitting there, waiting on the C.** A masked-skeleton scan
against Jet Force Gemini's built `asm/hasm/math_matrix.s.o`, independently
re-verified byte-for-byte with `tools/find_known_objects.py` (real
relocation-masking, not the coarse scan), identifies all four of
`src/main/matrix.c`'s parked functions:

| VRAM | ROM | JFG name | masked/words | romocc |
|---|---|---|---|---|
| `0x8002AB78` | `0x2B778` | `matrix_RPY_XYZ` | 6/67 | 1 |
| `0x8002AC84` | `0x2B884` | `matrix_XYZ_YPR_SCL` | 6/99 | 1 |
| `0x8002AE10` | `0x2BA10` | `matrix_XYZ_YPR` | 6/87 | 1 |
| `0x8002AF6C` | `0x2BB6C` | `matrixTransposeVectorMultiply` | 0/53 | 1 |

**Resolved 2026-10-07 as hand-written assembly, not as C.** The run
`0x2B778`-`0x2BC40` holding these four, in JFG's order, is now the
verified-assembly subsegment `main/math_matrix` (`verified_asm.us.txt`): 306
of 306 words agree with JFG's routines two to five once the R_MIPS_26 fields
are masked, and every masked word is a `jal` whose JFG relocation names
`Cosf` or `Sinf`, landing here on `0x8002A8BC` or `0x8002A8C0` (JFG also
places `Cosf` four bytes before `Sinf`). The odd-register question is
answered for them: they were never compiler output. The other two functions
of the old TU stay guarded C in working splits: `0x8002AA50` (`main/matrix`)
is JFG's first routine `matrix_SCL_RPY_XYZ` in a shorter revision, `0x128`
bytes against `0x148`, identical for its first 18 words and different after;
`0x8002B040` (`main/matrix_2BC40`) has no JFG counterpart, and JFG's object
is followed directly by `memory.c` exactly as `main/memory` follows it here.
The four JFG names now clear section 1.5 (the bytes are in the ROM as
assembly) but were not adopted in the ownership change, to keep it a pure
reclassification; adopting them is a rename across their callers.

### 6.3 Data ownership: SDK tails carved without guessing the bulk boundary

ROM `0x76E60`–`0x81590` is currently treated as `data` and
`0x81590`–`0x86640` as `rodata`. Most remains anonymous. This epoch assigned
only sections whose size and ordering are fixed by a matching reference object
and whose first symbol is reached by Mickey's own relocation: initialized data
for `aisetnextbuf`, `initialize`, `controller`, `vimgr`, `thread`, `siacs`, `vi`, `timerintr`, and `xlitob`;
rodata for `cents2ratio`, `sinf`, and `devmgr`; and BSS slices for `seteventmesg`, `vimgr`,
`sptask`, `siacs`, and `timerintr`. Anonymous gaps remain raw and explicit.

**The BSS recipe: name a fixed address inside an anonymous gap without
carving the gap.** `.data`/`.bss` are still one unsplit whole-program
subsegment (the anonymous `bss_gap_*` entries in `mickey.us.yaml`), so
giving a TU's global its own `.bss` in that TU is not available yet -- that
route makes the linker place the object by link order, which is not the ROM's
order, and shifts every fixed BSS symbol after it (measured on
`lane/libultra-rest`, `docs/CLEANROOM.md`'s deleted-history incident aside,
the actual failure there was two unmatched Transfer Pak functions'
register-save order and callee addresses drifting by 0x30, the signature of a
BSS symbol moving under a live reference). The working recipe, already in use
for `__osPfsPifRam`/`__osContLastCmd`/`__osMaxControllers` and the PI-access
queue (`__osPiAccessQueue`, `piAccessBuf`) since Phase 2:

1. Give the object a plain, untyped `NAME = 0xVRAM;` line in
   `symbol_addrs.us.txt`, inside the existing gap comment block, with a
   comment deriving the address from a masked %hi/%lo pair in an
   already-boundary-matched whole-`.text` function (Tier A/B evidence) --
   never guessed from struct layout alone. A typed `size:` annotation is
   rejected by splat for anything but its fixed vocabulary
   (`func`/`u8`/`u32`/... or a custom type starting with a capital letter);
   for a struct like `OSPifRam` the plain untyped form is what the existing
   entries already use, so follow that, not `type:object`.
2. Declare it `extern` in the shared header (`PRinternal/controller.h`
   already does, for every object in this gap) and never `#define` it as a
   real global in any `.c` file. The C body reads exactly like the SDK
   source; only the storage-class keyword differs, and that line carries a
   `DEVIATION FROM THE REFERENCE` comment saying so (see `piacs.c`,
   `controller.c`). Because nothing defines it, no object file's `.bss`
   claims space for it, so its final address depends on nothing but the
   linker-script assignment splat emits from `symbol_addrs.us.txt` --
   link order is irrelevant. This was verified directly: adding
   `__osContPifRam = 0x800D80F0;` alone (no source change) rebuilds the ROM
   byte-identically, and adding the two functions that reference it on top
   does too once they are.
3. The anonymous `bss` gap subsegment in `mickey.us.yaml` is untouched --
   BSS has no ROM bytes backing it, so the yaml's gap boundaries only affect
   generated disassembly labels, never the linked address of a
   `symbol_addrs`-fixed name. No yaml edit is needed to add a name inside an
   existing gap.
4. **Function order inside the `.c` file must match ROM order, not
   convenience.** A `#pragma GLOBAL_ASM` TU with multiple functions links
   its object's contents in file order; reordering `osContInit`'s
   `#pragma GLOBAL_ASM` after two newly-decompiled functions in the same
   file (tried and reverted while landing `__osContGetInitData`/
   `__osPackRequestData`) makes the linker place all three at addresses
   that no longer match `symbol_addrs.us.txt`, producing a corridor-wide
   byte mismatch that looks exactly like a BSS-layout bug (every relocation
   into the reordered range moves) but is a `.text` file-order bug instead.
   Keep declaration order == ROM address order in every partially-scaffolded
   TU.

Worked example: `__osContPifRam` (`OSPifRam`, 0x40 bytes) sits at
`0x800D80F0` inside `bss_gap_D800D80F0` (`0x800D80F0`-`0x800D8180`), reached
by `__osContGetInitData`/`__osPackRequestData`'s masked %hi/%lo pairs
(ROM `0x6F8E8`/`0x6F9B8`). `0x800D80F0 + 0x40 == 0x800D8130`, the already-fixed
address of `__osContLastCmd`, with no slack -- corroborating the size from a
second, independent direction. The remaining `0x800D8132`-`0x800D8180` stays
an anonymous, unnamed part of the same gap; nothing currently reaches it by
relocation, so nothing is claimed there.

**`gmake verify` is not the gate for a rodata boundary.** Re-slicing never
creates, drops or reorders a byte, so a wrong boundary rebuilds the ROM
byte-identically and every gate stays green. Cutting four bytes into
`jtbl_80080D2C` was tried deliberately: the build passes, and the half of the
table on the far side of the cut comes out as a fresh `dlabel D_80080D30`
holding a plain `.word 0x80007774` where the whole table holds
`.word .L80007774`. Same bytes, silently stripped of their typing. What checks
a boundary is reading the generated `.s` on both sides of it.

Where the boundary comes from:

- **`0x81510` is where read-only content demonstrably starts, although the
  current conservative split is `0x81590`.** A 32-space pad
  string, a 32-zero pad string and `"0123456789ABCDEF"` -- libultra `xprintf`'s
  field-padding tables -- then the audio subsystem's `printf` format strings,
  unbroken to `0x81920`. The symbol before it ends exactly at `0x81510`.
- **It is an upper bound, not the boundary.** Everything below it in
  `0x76E60`–`0x81510` is undifferentiated; some of it is certainly rodata too.
- **`0x80080D24` (ROM `0x81924`)** is a weaker bound of a different kind: the
  `FLT_MAX` constant loaded from ROM `0x50B0`, two words below the first jump
  table, and the lowest address any resident instruction reads as read-only
  data. The strings above it are read by nothing resident at all -- the same
  pattern as the model/sprite strings in §7 -- so a reference-derived bound
  cannot see them.
- **rodata order follows text order exactly.** The pre-carve inventory held
  forty-four jump tables. The 17 tables still emitted in `asm/`, from
  `jtbl_80080D2C` through `jtbl_800841F4`, remain monotonic in text and rodata
  order with **zero inversions**. The other 27 are now owned by C, including
  five in `n_csplayer`, one in `n_reverb`, one in `main/font`, and the table
  promoted with `xprintf`. This leaves the per-TU carve strategy intact.

The charControl scalar pool at ROM `0x82440`–`0x824A0` is now owned by
`src/main/charControl.c` (Tier A: the complete `0x60`-byte readonly input
section equals that ROM range). Its 23 single-precision cells follow their
consuming functions in text order; the last four bytes are section alignment.
Data-only assembly pragmas preserve their separate identities and existing
external references. The two unmatched functions retain their original cells
in separate companion guards selected with their assembly fallbacks. A
single-function C activation must also deselect its companion data group;
its literal pool and ordering require fresh proof before promotion. The carve
preserves all 14,480 TU text bytes, 350 relocation rows and function boundaries,
and the full ROM remains exact.
It adds no executable matching credit.

Two toolchain facts govern the per-TU split:

- Typing the tail as rodata labels every jump *target* in the text it lands in.
  Those labels are emitted with `asm_jtbl_label_macro`, and inside a
  `#pragma GLOBAL_ASM` file a `jlabel` line makes asm-processor miscount the
  section. The yaml uses `glabel`, which is the same global label and parses.
- Attaching rodata to a `c` subsegment migrates it into the function's own
  `.s` as `.late_rodata`. The vendored asm-processor now recognizes splat's
  zero-size `enddlabel` directive, so the symbol-size metadata is preserved
  and this is no longer a tooling blocker. The remaining blocker is evidence:
  do not assign another slice until its object size and ROM references agree.

---

## 7. What this map does not cover

- **The original source ownership inside overlay modules.** All 107 modules
  are represented and all 106 non-empty text ranges are buildable code
  segments (§5.4), but the generated assembly does not establish original TU
  boundaries. Exact donor matches establish a few functions; most overlay
  functions still carry generated `(overlay, offset)` identities.
- **`0x16B0000`–`0x1848B70`** (1.60 MiB, the `unclassified` segment). It is not
  overlay data and its entropy is not the asset band's. Nothing in the resident
  segment or the overlay tables points into it.
- **The asset region `0x87000`–`0x16B0000`** is one `bin`. Entropy suggests at
  least two classes (a near-random band to ~`0xAC0000` and a more structured
  one beyond), but nothing is decoded. `gzip_inflate_*` at `0x4EA60` is the
  decompressor that reads it, which is the obvious way in.
- **`0x86640`–`0x87000`** looks like a table of ROM offsets and is unidentified.
- **The model/sprite code**, on the same evidence as the build stamp (§2.1).
  `"CREATE LOD MODEL :: null model pointer!"` (`0x80081904`) and `"Cam do 2D
  sprite called with NULL pointer!"` (`0x800819F1`) are in the resident
  segment's rodata, but **no resident instruction builds either address**: the
  nearest references anywhere in `asm/` are `D_80081898` below and
  `D_80081A1C` above, and the two strings sit in the gap between them. So the
  asserts that print them are in an overlay, and the resident segment holds
  only their text. In particular `0x1FC9C` is *not* a model/sprite anchor:
  `func_8001F09C` is a float routine that steps a value at `+0x50` toward a
  bound at `+0x54` and clamps it, with no connection to either string.
- **4.1% of the libultra corridor** (§4.1): `0x70C30`–`0x70E20` and
  `0x74090`–`0x743A0`, `0x500` between them, matching nothing in any of the
- **2.4% of the libultra corridor** (§4.1): `0x743A0`–`0x74680`, `0x2E0`,
  matching nothing in any of the
  five reference builds. **It may not be libultra at all**: the label is
  inherited from a map made when the corridor was 78% identified against one
  build, and five builds' worth of silence is now the more informative fact.
  Disassemble it rather than mine it further; §4.1.
- **What is still `asm` that need not be.** 175 subsegments are named; 172 of
  them have a measured whole-`.text` boundary (the exceptions are `main/matrix`
  and `main/runlink`, which are decompiled rather than matched-as-a-unit, and
  the exact `__osEepStatus` function-tail carve described above).
  **95** are now `c`: 84 fully C and the other 11 carrying only
  `#pragma GLOBAL_ASM`. A scaffolded subsegment is worth having on its own:
  it proves the file boundary against the link before any C is written, and it
  turns every function in the unit into a separate work item. The remaining
  80 named subsegments include verified original assembly as well as compiler
  output; `verified_asm.us.txt` is the explicit evidence ledger that keeps
  those two completion states separate.
- **`libultra/contramread` and `libultra/contramwrite` are blocked on one
  scheduling decision each.** Both reproduce every word of the ROM but five,
  inside one four-times-unrolled byte fill: the ROM puts `addiu s0, s0, 4` in
  the loop branch's delay slot, this IDO bumps the pointer early and puts
  `sb zero, -4(s0)` there. The instruction multisets agree. `setglobalintmask`
  was in this list and is no longer: its empty delay slot was the `cc` driver
  refusing to run `uopt` at all below `-O2`, and driving the phases directly
  closes it (§6.1's fourth flag group). That removes the "the scheduler is not
  the one that built Mickey" reading of the third case; what remains for these
  two is a real disagreement, not a phase the driver skipped.
- **The `0xF0` bytes at `0x505E0`–`0x506D0`**, between the end of
  `os/exceptasm.s` and the next subsegment. Unidentified.
- **`0x6B860`–`0x6BDF0`** (`0x590`), between Perfect Dark's three Transfer Pak
  routines. Almost certainly more of the same driver; PD's build does not
  contain whatever it is.
- **The object system.** The `"null"` / `"setting up"` / `"freeing"` /
  `"processing"` / `"exploding"` phase names occupy five pointers beginning at
  `0x8007A220`. They are followed by a separate five-function patch table at
  `0x8007A234`–`0x8007A244`, including `func_8002B280`, `func_8002B314`,
  `mmFree`, and `func_8002B524`. `RevealReturnAddresses` consumes the function
  table to patch return-address sentinels, while the crash reporter consumes the
  phase names. Something at `0x8007A214` relates the two surfaces, but the
  evidence does not justify a combined struct or stronger intent claim.
