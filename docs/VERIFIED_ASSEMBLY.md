# Verified assembly: what stays assembly, and the proof for each routine

Mickey's Speedway USA is 100.00% resolved: every executable byte of the ROM is
either C that rebuilds it byte-identically, or one of 107 routines the original
developers wrote by hand in assembly. This document is the case for the second
group. It says what is claimed, how each routine is proved, where the proof is
thinnest, and how to check all of it yourself. The README's Progress block
carries the byte totals; the tables in §8 are generated from the ROM by
`tools/hasm_audit.py` and carry every per-routine number.

## 1. What is claimed, and what is not

**Claimed.** Each of the 107 routines was written by hand in assembly in the
original program. There is no C source to recover, so writing C for one would
mean inventing a source the original never had, and for most of them no C can
be written at all (§3). Such a routine is finished work: the build assembles it
from the extracted ROM bytes, and `gmake verify` proves the whole ROM rebuilds
byte-identically.

**Not claimed.**

- **That they are C matches.** The README's "functions matched to C" figure
  counts C only and excludes all 107; it is why that figure is not 100%.
- **Original file boundaries or names, for evidence class 3.** Class 3 claims
  authorship only. A run of class 1 routines claims a run of one reference
  object's routines, not that object's file boundary, unless the whole object
  matches.
- **Impossibility for every compiler ever written.** The claim is about the
  toolchain this program was built with, SGI IDO (5.3 here, with 7.1 also
  measured where it matters), and it is measured against that toolchain's real
  output (§3), not assumed.

**Why hand-written assembly counts as resolved.** It is how every project this
one draws on treats the same code. Rare's other decompiled games keep these
routines as hand-written assembly: Diddy Kong Racing and Jet Force Gemini both
carry a `src/hasm/` directory, and several Mickey routines are byte-identical
to files in it. Every decompiled libultra keeps the SDK's assembly routines as
`.s` files, for example the ultralib inside Banjo-Kazooie. The owner set the
standard on 2026-10-14: "100% by the typical community standards, with HASM
etc. handled appropriately". The ledger is evidence, not convenience. A routine
enters it only with proof of its own, and the evidence classes below say what
counts as proof.

The assembly itself is never committed: splat extracts it from your own ROM at
build time (`docs/CLEANROOM.md`).

## 2. The ledger and its three evidence classes

`verified_asm.us.txt` lists 36 splat subsegments. Every routine in a
subsegment takes that subsegment's class, unless a `# routine class N:`
declaration in the ledger files it separately. The only such declaration
covers two routines of `main/math_util` (§4).

| Class | What proves it |
|---|---|
| 1 | Byte-identical to a hand-written assembly object in a permitted reference build. |
| 2 | An IDO-impossible mechanism (§3), plus byte identity to the same routine in a reference build that itself carries it as assembly embedded in C (`GLOBAL_ASM`). |
| 3 | An IDO-impossible mechanism carried by the routine itself, with no reference identity. There are two narrow exceptions, each stated by name: a permitted library routine's built object with instructions deleted by hand, and a routine reached only as another proved routine's private exit. |

**Byte identity means:**

- Mickey's bytes, read from the baserom, equal the reference object's `.text`.
- Only the fields that the reference object's own relocation records patch are
  masked, because addresses differ between games.
- The run occurs exactly once in the ROM.

**Permitted reference builds.** These are the five named in
`docs/CLEANROOM.md`: Diddy Kong Racing, Jet Force Gemini, Perfect Dark,
Banjo-Kazooie and Conker's Bad Fur Day. Each is pinned by commit and by object
digest in `tools/reference-builds.lock`, and the audit flags identity
evidence from any build whose digests have drifted as not reproducible.

Class counts and bytes are in the generated tables in §8. Class 1 carries
most of the bytes, almost all of them in Rare's shared hand-written libraries
(the maths, lighting, inflate and trap routines) and in libultra.

## 3. What IDO never emits: the measured baseline

Mechanism evidence is only as strong as the claim that IDO cannot produce the
mechanism. That claim is measured.

- **The corpus.** `tools/hasm_audit.py --baseline` runs every detector over
  every IDO-compiled function the project can positively identify: Mickey's
  own C-matched resident functions, and every object in the five reference
  builds positively identified as IDO-compiled C: its C source is found, holds
  no assembly and no `GLOBAL_ASM`, and was not built by GCC.
- **The rule.** A shape is usable as evidence only if it fires in none of
  those functions.
- **The result.** The table in §8 gives the per-corpus counts. Every shape
  used as evidence has zero hits, among them:
  - odd single-precision FP registers;
  - trapping signed add/sub;
  - a non-leaf frame below the o32 minimum of 0x18;
  - a leaf that saves a `$ra` it never otherwise writes;
  - a callee-saved FP pair saved out of IDO's slot order;
  - callee-saved registers written without a save;
  - non-argument registers read on entry;
  - a computed jump into the routine's own code;
  - COP0, TLB, cache and system instructions.

**Shapes IDO does produce are refused as evidence,** even where earlier notes
had used them:

- a frame allocated after the first instruction (IDO schedules a load first);
- `$ra` used as scratch;
- a leaf saving `$ra`;
- callee-saved FP saved one word at a time (IDO does this at `-mips1`, the
  default for this program's C);
- dead code behind a branch-likely.

**Odd FP registers.** IDO pairs the 32 FP registers. In compiled code an odd
register appears only as the upper half of a double, in 32-bit loads, stores
and moves, so only odd registers *operated on as singles* count.

**Calibrating the detectors.** The baseline was also used to check the
detectors themselves. Its first run fired two labels in compiled C. Both were
detector bugs: calls reachable only through a switch table made functions look
like leaves, and 64-bit FP moves defined only half a register pair. Both were
fixed before any verdict was read.

## 4. Class 1 and class 2: identity to a reference

**Rare's shared hand-written libraries:**

- **`entrypoint`.** Identical to the hand-written `entrypoint.s` objects of DKR
  and JFG.
- **`main/lights2` (9 routines).** Identical as one whole object to JFG's
  `lights2` assembly object, which JFG files under `asm/hasm`.
  - Four of the masked fields are PC-relative branch fields that the
    reference's relocations cover. In JFG's linked ROM those four words equal
    Mickey's.
  - Eight of the nine routines also carry their own mechanism.
    `lightCreateLightTable` (22 words) carries none, and is covered by the
    whole-object identity.
- **`main/trapDanglingJump`.** Identical to JFG's hand-written
  `trapDanglingJump.s`.
- **`main/gzip_asm` (5 routines).** Identical to DKR's hand-written
  `gzip_asm.s`. JFG's `inflate.s` is a second exact reference. Both carry the
  same three zero alignment words at the end.
- **`main/math_matrix` (4 routines).** Identical to routines two to five of
  JFG's `math_matrix` assembly object, in JFG's order. Every masked field is a
  call to the `Cosf`/`Sinf` pair. All four also carry odd single-precision FP
  operands.
- **`main/refractOutputAssembler`.**
  - Identical to JFG's `src/hasm/refractOutputAssembler.s`, assembled locally.
  - That file is newer than the pinned JFG commit, and is disassembler output
    that JFG's maintainers reclassified as hand-written. It is therefore not
    the strongest kind of reference.
  - The routine also carries a mechanism of its own: a leaf that saves a `$ra`
    it never writes, which the baseline finds in no compiled function.
- **`main/math_util` (39 routines).** These are JFG's hand-written
  `src/hasm/ido/math_util.s`, in JFG's order.
  - The 37 identical routines fall into four runs, starting at ROM 0x2A250,
    0x2A630, 0x2AE50 and 0x2B100.
  - Between the runs are 12 bytes of zero padding and two routines JFG does not
    have: `func_800299E8` and `func_8002A414`. Neither matches anything in any
    reference build.
  - Each carries its own mechanism: a trapping signed add in the first; odd
    single-precision operands and a non-leaf frame below 0x18 in the second.
    The ledger therefore files them at class 3 by a routine declaration.
  - This subsegment was previously described as DKR's whole `math_util.s`
    object. That was wrong (§7).

**libultra (20 subsegments, 29 routines).** Each is identical as a whole
object to the hand-written SDK `.s` object in one or more of BK's ultralib,
DKR, JFG, PD and Conker; most match all five. `exceptasm` and `unmaptlball`
match JFG's revision only (DKR's `exceptasm` differs).

**Class 2 (`main/shadows_fp`, `main/shadows_fp_19254`, 3 routines).**

- **Identity.** Identical to routines one, two and four of JFG's
  `src/shadows_214A0.c`, which JFG carries as assembly embedded in C. JFG's
  object is therefore a reference for the bytes, but not for authorship.
- **Mechanism.** Authorship rests on the mechanism. All three operate on odd
  FP registers as singles, which no IDO function in the baseline does.

## 5. Class 3: proved by mechanism

These seventeen routines have no byte-identical counterpart in any permitted
build. Each one carries, by itself, at least one shape the baseline finds in
no IDO-compiled function. The generated table in §8 lists each mechanism per
routine with its count. In summary:

- **`main/trackasm`, four routines.** All four use trapping signed add/sub (46,
  9, 5 and 6 sites). `trackLightAsm` also operates on odd single-precision
  registers and allocates a second frame. They are shorter revisions of JFG's
  hand-written `trackasm`.
- **`main/shadows_fp_19144`, `func_80018544`.** Odd single-precision
  operands. It is JFG's routine three in a shorter revision, and it sits
  between routines two and four of the same object, which are class 2.
- **`main/matrix`, `func_8002AA50`.**
  - A non-leaf frame below 0x18, holding only `$ra`, with six calls made from
    it.
  - Caller-saved FP registers are held across those calls.
  - Odd single-precision operands.
- **`main/weather_snow_asm`, two routines.**
  - `snow_update` enters an unrolled add ladder through a computed jump into
    its own code (a Duff-style entry with no jump table), and uses trapping
    add/sub.
  - `snow_vertices` operates on odd single-precision registers.
- **`main/conv_mult_matrix`, `func_80058FF0`.** Odd single-precision operands
  and trapping add/sub.
- **`main/gen_anim_data`, five routines.**
  - `func_800591B0` operates on all sixteen odd FP registers as independent
    singles, saves a callee-saved FP pair out of IDO's slot order, and uses
    trapping add/sub.
  - Its helpers take arguments in t-registers (`func_8005A550`,
    `func_8005A5C0`), or write s0-s6 with no save and leave by a jump
    (`func_8005A630`).
  - `func_8005A6F0` is the one exception, below.
- **`main/matrix_2BC40`, `func_8002B040`.** The other exception, below.

### The two exceptions, argued individually

**`func_8002B040` (136 bytes): a compiled library routine with six
instructions deleted by hand.**

- **The reproduction.** Its 34 words are the 40-word guMtxXFMF (transform a
  point by a float matrix) with the translation row removed. Concretely: take
  the built object, delete the three loads of the translation terms and the
  three adds of them, and point each row's store at the row sum's register.
  The result is byte-identical. Rare's own hand-written `math_util.s` (DKR and
  JFG) carries the full routine, and Mickey carries it too, inside the
  JFG-identical `main/math_util`. Rare demonstrably pasted this compiled
  routine into hand-written assembly.
- **Nothing in it is impossible for IDO on its own,** so the case is that no
  compiler produces the shortened form.
- **The search (2026-10-21):**
  - 46,776 compiles of the translation-free body under IDO 5.3 and 7.1;
  - 78 flag sets: `-O1` to `-O3`, `-mips1` to `-mips3`, with and without
    `-Wab,-r4300_mul`, three `-g` levels, `-Olimit`;
  - variations of order, grouping and operand order, zero-weighted terms,
    indexing style, temporaries, `register`, loops, structs and pointers.
- **The best result.** 16 words differ. The instruction schedule, opcodes and
  offsets are identical to the ROM; the 16 words differ only in FP register
  numbers.
- **Why no compiler gets there.**
  - **The allocator model.** IDO's code generator hands out FP temporaries from
    a queue: a result takes the front register, and its inputs' registers
    rejoin at the back. That model reproduces the register fields of every one
    of 3,093 distinct compiles of this body, and of the 40-word reference.
  - **No evaluation order fits.** The ROM's registers fit no evaluation order
    of its own values, over every queue starting order.
  - **The deleted values fit exactly.** The registers fit only if each row also
    held one load and one sum after the row sum. Those are exactly the deleted
    translation-term load and add, so the registers were assigned before the
    six instructions were removed.
  - **Nothing in the toolchain deletes them.** The assembler keeps dead
    instructions, and the optimizer folds identities before registers are
    assigned.

  `tools/fp_queue_model.py` re-derives the model, its validation and the fit
  from your own ROM and compilers.

**`func_8005A6F0` (8 bytes): a private exit.**

- **The routine.** Two instructions on a 16-byte boundary. Its bytes are what
  IDO emits for an empty `void` function.
- **The evidence.** Its only reference anywhere is the closing jump of
  `func_8005A630`, which has no return of its own. The audit checks resident
  code and data, the overlay export table and raw constants in the overlay
  images.
- **What it rests on.** It is admitted as that helper's exit, which is the
  same idiom JFG's hand-written `gen_anim_data.s` uses. Calling it a separate
  C function would claim a file boundary inside this object with no evidence
  but the byte coincidence.

## 6. Where the proof is thinnest, stated plainly

`tools/hasm_audit.py` marks four routines *thin*:

- **`func_8002B040` and `func_8005A6F0`.** Argued above.
- **`sqrtf` (8 bytes) and `__osSetFpcCsr` (16 bytes).** Each is byte-identical
  as a whole object to the SDK's hand-written `.s` object, in several
  reference builds. But it is two or three words long, too short for the bytes
  alone to exclude a compiler intrinsic. These rest on library provenance.

**Size of the residual doubt.** If all four were somehow compiled C, they
total 168 bytes, 0.018% of the program, and the resolved figure would still
round to 99.98%. Every other routine carries byte identity to a hand-written
reference, or a mechanism the baseline finds in no compiled function, or both.

## 7. What this audit corrected (2026-10-21)

The fact-check found the ledger's conclusions right and some of its stated
evidence wrong. The evidence is fixed in `verified_asm.us.txt`,
`mickey.us.yaml`, `symbol_addrs.us.txt`, `docs/modules.md` and
`docs/resident.md`.

- **`main/math_util` was attributed to DKR's whole `math_util.s` object.**
  DKR's object matches only 15 of its routines. The true reference is JFG's
  `math_util.s`: 37 of 39 routines, in four runs. The two routines JFG lacks
  are now filed at class 3 on their own mechanisms.
- **Masking.** The ledger said only call and HI/LO address fields were masked.
  `main/lights2` also masks four PC-relative branch fields that its
  reference's relocations cover.
- **Mechanism wording that IDO contradicts:**
  - `refractOutputAssembler`'s "frame opened five instructions in" is not
    evidence; IDO does that in 154 Mickey functions. Its evidence is the leaf
    `$ra` save.
  - `func_800591B0`'s "FP saved one word at a time" is not evidence, because
    IDO does that at `-mips1`, the default for this program's C. Its evidence
    is the out-of-order FP pair.
- **`func_8002B040`'s note said a compiler "reschedules the rows".** It does
  not: the schedule is identical and only register numbers differ, which is
  what the allocator model explains.
- **Smaller items:**
  - stale odd-FP counts;
  - `gzip_asm`'s object size: 0xA80, including three alignment words;
  - "the helpers jump between each other's interiors": only one jump crosses a
    routine boundary;
  - the shadows counting method.

Three independent passes produced the corrections, and they agree on every
routine:

- the audit tool, which re-derives everything from the bytes and does not read
  the yaml notes;
- a reference reviewer with its own ELF parser and comparison;
- a mechanism reviewer with its own disassembly and a separately measured IDO
  baseline.

## 8. Checking it yourself

You need a legally dumped ROM at `baseroms/mickey.us.z64`, a build, and the
reference builds checked out at the commits in `tools/reference-builds.lock`
(the audit reads `$REFS_ROOT`, defaulting to the documented location).

```sh
gmake verify                                        # the whole ROM rebuilds byte-identically
.venv/bin/python tools/hasm_audit.py --check --strict    # every routine's proof, re-derived
.venv/bin/python tools/hasm_audit.py --baseline          # what IDO never emits, measured
.venv/bin/python tools/fp_queue_model.py --rom func_8002B040   # the guMtxXFMF register proof
.venv/bin/python tools/hasm_audit.py --baseline --write-doc docs/VERIFIED_ASSEMBLY.md  # regenerate the tables below
```

<!-- hasm-audit:start -->
<!-- generated by tools/hasm_audit.py --write-doc; do not edit by hand -->

Generated from ROM SHA-1 `507341c0a40ca3e9a7cee969b396ee53facfb548` against the reference builds pinned in `tools/reference-builds.lock` (2894 objects). Verdicts: supported 107.

| Class | Routines | Bytes |
|---|---|---|
| 1 | 87 | 19,092 |
| 2 | 3 | 528 |
| 3 | 17 | 10,120 |
| **all** | **107** | **29,740** |

| Subsegment | Class | ROM | Routines | Bytes | Run against the reference builds |
|---|---|---|---|---|---|
| `entrypoint` | 1 | 0x1000-0x1050 | 1 | 80 | whole-object, 14 of 14 words covered |
| `main/math_util` | 1 | 0x2a250-0x2b650 | 39 | 5,100 | ordered-runs, 1266 of 1277 words covered |
| `main/math_matrix` | 1 | 0x2b778-0x2bc40 | 4 | 1,224 | single-object-run, 306 of 306 words covered |
| `main/shadows_fp` | 2 | 0x18ff0-0x19144 | 2 | 340 | single-object-run, 85 of 85 words covered |
| `main/shadows_fp_19254` | 2 | 0x19254-0x19310 | 1 | 188 | single-object-run, 46 of 46 words covered |
| `main/lights2` | 1 | 0x1ae60-0x1be50 | 9 | 4,068 | whole-object, 1017 of 1017 words covered |
| `main/trapDanglingJump` | 1 | 0x33fa0-0x34180 | 1 | 480 | whole-object, 119 of 119 words covered |
| `main/gzip_asm` | 1 | 0x4ea60-0x4f4e0 | 5 | 2,676 | whole-object, 669 of 669 words covered |
| `main/refractOutputAssembler` | 1 | 0x50820-0x50c00 | 1 | 992 | whole-object, 245 of 245 words covered |
| `main/trackasm` | 3 | 0x16140-0x16a90 | 4 | 2,384 | multi-segment, 582 of 596 words covered |
| `main/shadows_fp_19144` | 3 | 0x19144-0x19254 | 1 | 272 | multi-segment, 66 of 67 words covered |
| `main/matrix` | 3 | 0x2b650-0x2b778 | 1 | 296 | multi-segment, 54 of 73 words covered |
| `main/matrix_2BC40` | 3 | 0x2bc40-0x2bcd0 | 1 | 136 | multi-segment, 34 of 34 words covered |
| `main/weather_snow_asm` | 3 | 0x3d030-0x3d370 | 2 | 832 | multi-segment, 120 of 208 words covered |
| `main/conv_mult_matrix` | 3 | 0x59bf0-0x59db0 | 1 | 444 | none, 71 of 110 words covered |
| `main/gen_anim_data` | 3 | 0x59db0-0x5b300 | 5 | 5,448 | multi-segment, 651 of 1361 words covered |
| `libultra/exceptasm` | 1 | 0x4fc30-0x505e0 | 9 | 2,476 | whole-object, 619 of 619 words covered |
| `libultra/sqrtf` | 1 | 0x6fc50-0x6fc60 | 1 | 8 | whole-object, 2 of 2 words covered |
| `libultra/bzero` | 1 | 0x701e0-0x70280 | 1 | 156 | whole-object, 38 of 38 words covered |
| `libultra/writebackdcache` | 1 | 0x70910-0x70990 | 1 | 116 | whole-object, 28 of 28 words covered |
| `libultra/invalicache` | 1 | 0x70990-0x70a10 | 1 | 116 | whole-object, 28 of 28 words covered |
| `libultra/invaldcache` | 1 | 0x71430-0x714e0 | 1 | 172 | whole-object, 42 of 42 words covered |
| `libultra/writebackdcacheall` | 1 | 0x714e0-0x71510 | 1 | 40 | whole-object, 9 of 9 words covered |
| `libultra/setintmask` | 1 | 0x71970-0x71a10 | 1 | 160 | whole-object, 39 of 39 words covered |
| `libultra/bcopy` | 1 | 0x72420-0x72730 | 1 | 772 | whole-object, 193 of 193 words covered |
| `libultra/interrupt` | 1 | 0x72810-0x72850 | 2 | 60 | whole-object, 14 of 14 words covered |
| `libultra/bcmp` | 1 | 0x73320-0x73440 | 1 | 284 | whole-object, 71 of 71 words covered |
| `libultra/getcount` | 1 | 0x73730-0x73740 | 1 | 12 | whole-object, 2 of 2 words covered |
| `libultra/probetlb` | 1 | 0x738c0-0x73980 | 1 | 184 | whole-object, 45 of 45 words covered |
| `libultra/setsr` | 1 | 0x73b00-0x73b10 | 1 | 16 | whole-object, 3 of 3 words covered |
| `libultra/getsr` | 1 | 0x73b10-0x73b20 | 1 | 12 | whole-object, 2 of 2 words covered |
| `libultra/setfpccsr` | 1 | 0x73b20-0x73b30 | 1 | 16 | whole-object, 3 of 3 words covered |
| `libultra/unmaptlball` | 1 | 0x73bd0-0x73c20 | 1 | 68 | whole-object, 16 of 16 words covered |
| `libultra/maptlbrdb` | 1 | 0x73c20-0x73c80 | 1 | 88 | whole-object, 21 of 21 words covered |
| `libultra/getcause` | 1 | 0x73c80-0x73c90 | 1 | 12 | whole-object, 2 of 2 words covered |
| `libultra/setcompare` | 1 | 0x75e10-0x75e20 | 1 | 12 | whole-object, 2 of 2 words covered |

Baseline: every detector over IDO-compiled functions (`tools/hasm_audit.py --baseline`).

| Corpus | Objects | Functions | Words |
|---|---|---|---|
| mickey-resident-c | - | 1,342 | 110,957 |
| banjo-kazooie compiled C | 1,046 | 8,361 | 389,251 |
| conker compiled C | 135 | 205 | 5,916 |
| diddy-kong-racing compiled C | 193 | 1,845 | 211,021 |
| jfg compiled C | 120 | 254 | 15,437 |
| perfect_dark compiled C | 428 | 5,591 | 512,640 |
| **all** | | **17,598** | **1,245,222** |

| Shape | mickey-resident-c | banjo-kazooie compiled C | conker compiled C | diddy-kong-racing compiled C | jfg compiled C | perfect_dark compiled C | Verdict |
|---|---|---|---|---|---|---|---|
| 64-bit instruction | 10 | 21 | 0 | 18 | 18 | 3 | information only |
| branch into another routine | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| call to a non-entry address | 12 | 158 | 0 | 21 | 9 | 4 | information only |
| callee-saved FP pair out of IDO's slot order | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| callee-saved FP saved one word at a time | 19 | 79 | 0 | 111 | 1 | 83 | refused: fires in compiled C |
| callee-saved register written unsaved | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| register jump to an arithmetically computed address | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| computed jump into its own code | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| branch or jump in a delay slot | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| falls through into the next routine | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| frame size not 8-aligned | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| frame allocated after the first instruction | 154 | 658 | 2 | 205 | 27 | 897 | refused: fires in compiled C |
| jump into another routine's interior | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| leaf saves $ra | 2 | 2 | 0 | 9 | 0 | 1 | refused: fires in compiled C |
| leaf saves an otherwise-unwritten $ra | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| non-argument register read on entry | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| non-leaf frame below 0x18 | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| non-leaf never saves $ra | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| non-leaf with no frame | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| odd FP register at double width | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| odd FP register in a 32-bit move | 2 | 372 | 0 | 227 | 7 | 1 | information only |
| odd single-precision FP operands | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| $ra set other than by a call or its restore | 35 | 23 | 0 | 58 | 2 | 47 | refused: fires in compiled C |
| register jump to a constant code address | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| k0/k1 use | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| second frame allocation | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| entered at an interior address | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| $sp set other than by a frame adjustment | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| COP0/TLB/cache/system instruction | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| register jump through a table or pointer | 33 | 168 | 0 | 64 | 5 | 223 | information only |
| jump to another routine's entry | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| trapping signed add/sub | 0 | 0 | 0 | 0 | 0 | 0 | usable |
| instructions no path reaches | 284 | 771 | 29 | 114 | 35 | 1,346 | refused: fires in compiled C |

| Routine | Subsegment | Class | ROM | Bytes | Re-derived evidence | Verdict |
|---|---|---|---|---|---|---|
| `entrypoint` | `entrypoint` | 1 | 0x1000 | 80 | identical to `diddy-kong-racing/build/src/hasm/entrypoint.s.o` (handwritten-source), 14 words, 8 masked; register jump to a constant code address x1, $sp set other than by a frame adjustment x1, trapping signed add/sub x2 | supported |
| `interrupts_disable` | `main/math_util` | 1 | 0x2a250 | 44 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 10 words, 2 masked; COP0/TLB/cache/system instruction x2 | supported |
| `interrupts_enable` | `main/math_util` | 1 | 0x2a27c | 40 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 9 words, 2 masked; COP0/TLB/cache/system instruction x2 | supported |
| `func_800296A4` | `main/math_util` | 1 | 0x2a2a4 | 12 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 3 words, 2 masked | supported |
| `mtxf_transform_point` | `main/math_util` | 1 | 0x2a2b0 | 160 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 40 words, 0 masked | supported |
| `mtxf_transform_dir` | `main/math_util` | 1 | 0x2a350 | 124 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 31 words, 0 masked | supported |
| `mtxf_mul` | `main/math_util` | 1 | 0x2a3cc | 264 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 66 words, 0 masked; non-argument register read on entry x1 | supported |
| `mtxf_to_mtx` | `main/math_util` | 1 | 0x2a4d4 | 156 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 38 words, 0 masked | supported |
| `func_80029970` | `main/math_util` | 1 | 0x2a570 | 12 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 3 words, 2 masked | supported |
| `mathRnd` | `main/math_util` | 1 | 0x2a57c | 108 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 26 words, 4 masked; trapping signed add/sub x3 | supported |
| `func_800299E8` | `main/math_util` | 3 | 0x2a5e8 | 72 | trapping signed add/sub x1 | supported |
| `vec3s_reflect` | `main/math_util` | 1 | 0x2a630 | 136 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 34 words, 0 masked; trapping signed add/sub x5 | supported |
| `func_80029AB8` | `main/math_util` | 1 | 0x2a6b8 | 44 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 11 words, 0 masked | supported |
| `mtxf_translate_y` | `main/math_util` | 1 | 0x2a6e4 | 68 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 17 words, 0 masked | supported |
| `mtxf_from_inverse_transform` | `main/math_util` | 1 | 0x2a728 | 484 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 120 words, 6 masked; non-leaf frame below 0x18 x1 | supported |
| `func_80029D0C` | `main/math_util` | 1 | 0x2a90c | 216 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 53 words, 6 masked; non-leaf frame below 0x18 x1 | supported |
| `mathRSMtx` | `main/math_util` | 1 | 0x2a9e4 | 144 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 35 words, 2 masked; non-leaf frame below 0x18 x1, odd single-precision FP operands x7 | supported |
| `mathOneFloatRPY` | `main/math_util` | 1 | 0x2aa74 | 184 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 45 words, 6 masked; non-leaf frame below 0x18 x1 | supported |
| `mathOneFloatYPR` | `main/math_util` | 1 | 0x2ab2c | 184 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 45 words, 6 masked; non-leaf frame below 0x18 x1 | supported |
| `mathOneFloatPY` | `main/math_util` | 1 | 0x2abe4 | 96 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 23 words, 4 masked; non-leaf frame below 0x18 x1 | supported |
| `mathXZInTri` | `main/math_util` | 1 | 0x2ac44 | 240 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 59 words, 0 masked; trapping signed add/sub x16 | supported |
| `mathTranslateMtx` | `main/math_util` | 1 | 0x2ad34 | 60 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 15 words, 0 masked | supported |
| `mathScaleMtx` | `main/math_util` | 1 | 0x2ad70 | 48 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 12 words, 0 masked | supported |
| `coss_s16` | `main/math_util` | 1 | 0x2ada0 | 96 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 23 words, 2 masked; entered at an interior address x1 | supported |
| `coss_2` | `main/math_util` | 1 | 0x2ae00 | 68 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 16 words, 2 masked; entered at an interior address x1 | supported |
| `func_8002A244` | `main/math_util` | 1 | 0x2ae44 | 4 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 1 words, 0 masked; falls through into the next routine x1 | supported |
| `pointListRPY` | `main/math_util` | 1 | 0x2ae50 | 232 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 57 words, 6 masked; non-leaf frame below 0x18 x1, odd single-precision FP operands x27 | supported |
| `splinePos` | `main/math_util` | 1 | 0x2af38 | 220 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 55 words, 20 masked; odd single-precision FP operands x26 | supported |
| `func_8002A414` | `main/math_util` | 3 | 0x2b014 | 236 | non-leaf frame below 0x18 x1, odd single-precision FP operands x28 | supported |
| `matrixScale` | `main/math_util` | 1 | 0x2b100 | 116 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 29 words, 0 masked; odd single-precision FP operands x8 | supported |
| `matrixTranslate` | `main/math_util` | 1 | 0x2b174 | 144 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 36 words, 0 masked; odd single-precision FP operands x22 | supported |
| `func_8002A604` | `main/math_util` | 1 | 0x2b204 | 184 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 45 words, 2 masked; non-leaf frame below 0x18 x1, odd single-precision FP operands x27 | supported |
| `func_8002A6BC` | `main/math_util` | 1 | 0x2b2bc | 184 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 45 words, 2 masked; non-leaf frame below 0x18 x1, odd single-precision FP operands x27 | supported |
| `matrixYaw` | `main/math_util` | 1 | 0x2b374 | 184 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 45 words, 2 masked; non-leaf frame below 0x18 x1, odd single-precision FP operands x27 | supported |
| `func_8002A82C` | `main/math_util` | 1 | 0x2b42c | 76 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 19 words, 0 masked | supported |
| `Powerf` | `main/math_util` | 1 | 0x2b478 | 68 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 16 words, 0 masked | supported |
| `func_8002A8BC` | `main/math_util` | 1 | 0x2b4bc | 4 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 1 words, 0 masked; falls through into the next routine x1 | supported |
| `func_8002A8C0` | `main/math_util` | 1 | 0x2b4c0 | 80 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 19 words, 2 masked; trapping signed add/sub x1 | supported |
| `Arctanf` | `main/math_util` | 1 | 0x2b510 | 252 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 62 words, 5 masked; odd single-precision FP operands x4, trapping signed add/sub x2 | supported |
| `mathDiffAngle` | `main/math_util` | 1 | 0x2b60c | 56 | identical to `jfg/build/src/hasm/ido/math_util.s.o` (handwritten-source), 14 words, 0 masked; trapping signed add/sub x3 | supported |
| `func_8002AB78` | `main/math_matrix` | 1 | 0x2b778 | 268 | identical to `jfg/build/asm/hasm/math_matrix.s.o` (reference-hasm), 66 words, 6 masked; non-leaf frame below 0x18 x1, odd single-precision FP operands x28 | supported |
| `func_8002AC84` | `main/math_matrix` | 1 | 0x2b884 | 396 | identical to `jfg/build/asm/hasm/math_matrix.s.o` (reference-hasm), 98 words, 6 masked; non-leaf frame below 0x18 x1, odd single-precision FP operands x69 | supported |
| `func_8002AE10` | `main/math_matrix` | 1 | 0x2ba10 | 348 | identical to `jfg/build/asm/hasm/math_matrix.s.o` (reference-hasm), 86 words, 6 masked; non-leaf frame below 0x18 x1, odd single-precision FP operands x52 | supported |
| `func_8002AF6C` | `main/math_matrix` | 1 | 0x2bb6c | 212 | identical to `jfg/build/asm/hasm/math_matrix.s.o` (reference-hasm), 53 words, 0 masked; odd single-precision FP operands x24 | supported |
| `shadowBoxPolyOverlap` | `main/shadows_fp` | 2 | 0x18ff0 | 196 | identical to `jfg/build/src/shadows_214A0.c.o` (global-asm), 48 words, 0 masked; odd single-precision FP operands x15 | supported |
| `shadowBoundingBox` | `main/shadows_fp` | 2 | 0x190b4 | 144 | identical to `jfg/build/src/shadows_214A0.c.o` (global-asm), 36 words, 0 masked; odd single-precision FP operands x11 | supported |
| `func_80018654` | `main/shadows_fp_19254` | 2 | 0x19254 | 188 | identical to `jfg/build/src/shadows_214A0.c.o` (global-asm), 46 words, 0 masked; odd single-precision FP operands x10 | supported |
| `lightSingleLight` | `main/lights2` | 1 | 0x1ae60 | 484 | identical to `jfg/build/asm/hasm/lights2.s.o` (reference-hasm), 121 words, 3 masked; leaf saves an otherwise-unwritten $ra x1, trapping signed add/sub x4 | supported |
| `lightTwoLights` | `main/lights2` | 1 | 0x1b044 | 724 | identical to `jfg/build/asm/hasm/lights2.s.o` (reference-hasm), 181 words, 1 masked; leaf saves an otherwise-unwritten $ra x1, trapping signed add/sub x6 | supported |
| `lightMultiLight` | `main/lights2` | 1 | 0x1b318 | 728 | identical to `jfg/build/asm/hasm/lights2.s.o` (reference-hasm), 182 words, 5 masked; leaf saves an otherwise-unwritten $ra x1, trapping signed add/sub x4 | supported |
| `lightCreateLightTable` | `main/lights2` | 1 | 0x1b5f0 | 88 | identical to `jfg/build/asm/hasm/lights2.s.o` (reference-hasm), 21 words, 0 masked | supported |
| `lightMakeJointPositions` | `main/lights2` | 1 | 0x1b648 | 372 | identical to `jfg/build/asm/hasm/lights2.s.o` (reference-hasm), 93 words, 0 masked; callee-saved FP pair out of IDO's slot order x1, odd single-precision FP operands x41 | supported |
| `lightEnviroMap` | `main/lights2` | 1 | 0x1b7bc | 404 | identical to `jfg/build/asm/hasm/lights2.s.o` (reference-hasm), 101 words, 0 masked; leaf saves an otherwise-unwritten $ra x1, trapping signed add/sub x12 | supported |
| `starfieldFastMove` | `main/lights2` | 1 | 0x1b950 | 304 | identical to `jfg/build/asm/hasm/lights2.s.o` (reference-hasm), 75 words, 0 masked; odd single-precision FP operands x33 | supported |
| `cpuLine` | `main/lights2` | 1 | 0x1ba80 | 472 | identical to `jfg/build/asm/hasm/lights2.s.o` (reference-hasm), 117 words, 0 masked; trapping signed add/sub x16 | supported |
| `rainFastDraw` | `main/lights2` | 1 | 0x1bc58 | 492 | identical to `jfg/build/asm/hasm/lights2.s.o` (reference-hasm), 123 words, 3 masked; callee-saved FP pair out of IDO's slot order x4, odd single-precision FP operands x55, second frame allocation x1 | supported |
| `TrapDanglingJump` | `main/trapDanglingJump` | 1 | 0x33fa0 | 480 | identical to `jfg/build/src/hasm/ido/trapDanglingJump.s.o` (handwritten-source), 119 words, 15 masked; non-argument register read on entry x2 | supported |
| `gzip_inflate_block` | `main/gzip_asm` | 1 | 0x4ea60 | 260 | identical to `diddy-kong-racing/build/src/hasm/gzip_asm.s.o` (handwritten-source), 65 words, 19 masked; trapping signed add/sub x3 | supported |
| `gzip_inflate_dynamic` | `main/gzip_asm` | 1 | 0x4eb64 | 1048 | identical to `diddy-kong-racing/build/src/hasm/gzip_asm.s.o` (handwritten-source), 262 words, 32 masked; trapping signed add/sub x25 | supported |
| `gzip_inflate_fixed` | `main/gzip_asm` | 1 | 0x4ef7c | 344 | identical to `diddy-kong-racing/build/src/hasm/gzip_asm.s.o` (handwritten-source), 85 words, 11 masked; trapping signed add/sub x8 | supported |
| `gzip_inflate_stored` | `main/gzip_asm` | 1 | 0x4f0d4 | 268 | identical to `diddy-kong-racing/build/src/hasm/gzip_asm.s.o` (handwritten-source), 67 words, 16 masked; trapping signed add/sub x7 | supported |
| `gzip_inflate_codes` | `main/gzip_asm` | 1 | 0x4f1e0 | 756 | identical to `diddy-kong-racing/build/src/hasm/gzip_asm.s.o` (handwritten-source), 189 words, 20 masked; trapping signed add/sub x15 | supported |
| `refractOutputAssembler` | `main/refractOutputAssembler` | 1 | 0x50820 | 992 | identical to `jfg/src/hasm/refractOutputAssembler.s` (handwritten-source), 245 words, 16 masked; leaf saves an otherwise-unwritten $ra x1 | supported |
| `trackMakePolylist` | `main/trackasm` | 3 | 0x16140 | 1180 | trapping signed add/sub x46 | supported |
| `getXZCompareMask` | `main/trackasm` | 3 | 0x165dc | 284 | trapping signed add/sub x9 | supported |
| `getYCompareMask` | `main/trackasm` | 3 | 0x166f8 | 152 | trapping signed add/sub x5 | supported |
| `trackLightAsm` | `main/trackasm` | 3 | 0x16790 | 768 | odd single-precision FP operands x28, second frame allocation x1, trapping signed add/sub x6 | supported |
| `func_80018544` | `main/shadows_fp_19144` | 3 | 0x19144 | 272 | odd single-precision FP operands x27 | supported |
| `func_8002AA50` | `main/matrix` | 3 | 0x2b650 | 296 | non-leaf frame below 0x18 x1, odd single-precision FP operands x44 | supported |
| `func_8002B040` | `main/matrix_2BC40` | 3 | 0x2bc40 | 136 | `diddy-kong-racing/build/src/hasm/ido/math_util.s.o` mtxf_transform_point with 6 instructions deleted, 3 stores repointed | supported (thin) |
| `snow_update` | `main/weather_snow_asm` | 3 | 0x3d030 | 324 | computed jump into its own code x1, trapping signed add/sub x26 | supported |
| `snow_vertices` | `main/weather_snow_asm` | 3 | 0x3d174 | 508 | odd single-precision FP operands x34, trapping signed add/sub x10 | supported |
| `func_80058FF0` | `main/conv_mult_matrix` | 3 | 0x59bf0 | 444 | odd single-precision FP operands x30, trapping signed add/sub x3 | supported |
| `func_800591B0` | `main/gen_anim_data` | 3 | 0x59db0 | 5024 | callee-saved FP pair out of IDO's slot order x6, odd single-precision FP operands x683, trapping signed add/sub x110 | supported |
| `func_8005A550` | `main/gen_anim_data` | 3 | 0x5b150 | 112 | non-argument register read on entry x13, trapping signed add/sub x8 | supported |
| `func_8005A5C0` | `main/gen_anim_data` | 3 | 0x5b1c0 | 112 | non-argument register read on entry x13, trapping signed add/sub x8 | supported |
| `func_8005A630` | `main/gen_anim_data` | 3 | 0x5b230 | 192 | callee-saved register written unsaved x7, jump to another routine's entry x1, trapping signed add/sub x11 | supported |
| `func_8005A6F0` | `main/gen_anim_data` | 3 | 0x5b2f0 | 8 | only reference is func_8005A630's closing jump | supported (thin) |
| `__osExceptionPreamble` | `libultra/exceptasm` | 1 | 0x4fc30 | 16 | identical to `jfg/build/libultra/src/os/exceptasm.s.o` (handwritten-source), 3 words, 2 masked; register jump to a constant code address x1, k0/k1 use x3 | supported |
| `__osException` | `libultra/exceptasm` | 1 | 0x4fc40 | 1408 | identical to `jfg/build/libultra/src/os/exceptasm.s.o` (handwritten-source), 351 words, 48 masked; branch into another routine x2, callee-saved register written unsaved x2, non-argument register read on entry x30, non-leaf never saves $ra x1, non-leaf with no frame x1, odd FP register at double width x16, k0/k1 use x107, entered at an interior address x2, $sp set other than by a frame adjustment x2, COP0/TLB/cache/system instruction x10, jump to another routine's entry x3, trapping signed add/sub x2 | supported |
| `send_mesg` | `libultra/exceptasm` | 1 | 0x501c0 | 180 | identical to `jfg/build/libultra/src/os/exceptasm.s.o` (handwritten-source), 44 words, 6 masked; callee-saved register written unsaved x1, non-leaf never saves $ra x1, non-leaf with no frame x1 | supported |
| `handle_CpU` | `libultra/exceptasm` | 1 | 0x50274 | 52 | identical to `jfg/build/libultra/src/os/exceptasm.s.o` (handwritten-source), 13 words, 0 masked; branch into another routine x2, non-argument register read on entry x1, k0/k1 use x4 | supported |
| `__osEnqueueAndYield` | `libultra/exceptasm` | 1 | 0x502a8 | 280 | identical to `jfg/build/libultra/src/os/exceptasm.s.o` (handwritten-source), 69 words, 8 masked; non-leaf never saves $ra x1, non-leaf with no frame x1, odd FP register at double width x6, k0/k1 use x23, entered at an interior address x1, COP0/TLB/cache/system instruction x1, jump to another routine's entry x1 | supported |
| `__osEnqueueThread` | `libultra/exceptasm` | 1 | 0x503c0 | 72 | identical to `jfg/build/libultra/src/os/exceptasm.s.o` (handwritten-source), 18 words, 0 masked | supported |
| `__osPopThread` | `libultra/exceptasm` | 1 | 0x50408 | 16 | identical to `jfg/build/libultra/src/os/exceptasm.s.o` (handwritten-source), 4 words, 0 masked | supported |
| `__osDispatchThread` | `libultra/exceptasm` | 1 | 0x50418 | 444 | identical to `jfg/build/libultra/src/os/exceptasm.s.o` (handwritten-source), 111 words, 11 masked; callee-saved register written unsaved x21, non-leaf never saves $ra x1, non-leaf with no frame x1, odd FP register at double width x16, k0/k1 use x91, $sp set other than by a frame adjustment x1, COP0/TLB/cache/system instruction x3 | supported |
| `__osCleanupThread` | `libultra/exceptasm` | 1 | 0x505d4 | 8 | identical to `jfg/build/libultra/src/os/exceptasm.s.o` (handwritten-source), 2 words, 1 masked; falls through into the next routine x1, non-leaf never saves $ra x1, non-leaf with no frame x1 | supported |
| `sqrtf` | `libultra/sqrtf` | 1 | 0x6fc50 | 8 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/gu/sqrtf.o` (handwritten-source), 2 words, 0 masked | supported (thin) |
| `_bzero` | `libultra/bzero` | 1 | 0x701e0 | 156 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/libc/bzero.o` (handwritten-source), 38 words, 0 masked | supported |
| `osWritebackDCache` | `libultra/writebackdcache` | 1 | 0x70910 | 116 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/writebackdcache.o` (handwritten-source), 28 words, 0 masked; COP0/TLB/cache/system instruction x2 | supported |
| `osInvalICache` | `libultra/invalicache` | 1 | 0x70990 | 116 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/invalicache.o` (handwritten-source), 28 words, 0 masked; COP0/TLB/cache/system instruction x2 | supported |
| `osInvalDCache` | `libultra/invaldcache` | 1 | 0x71430 | 172 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/invaldcache.o` (handwritten-source), 42 words, 0 masked; COP0/TLB/cache/system instruction x4 | supported |
| `osWritebackDCacheAll` | `libultra/writebackdcacheall` | 1 | 0x714e0 | 40 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/writebackdcacheall.o` (handwritten-source), 9 words, 0 masked; COP0/TLB/cache/system instruction x1 | supported |
| `osSetIntMask` | `libultra/setintmask` | 1 | 0x71970 | 160 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/setintmask.o` (handwritten-source), 39 words, 4 masked; COP0/TLB/cache/system instruction x2 | supported |
| `_bcopy` | `libultra/bcopy` | 1 | 0x72420 | 772 | identical to `perfect_dark/build/ntsc-final/lib/ultra/libc/bcopy.o` (handwritten-source), 193 words, 0 masked; trapping signed add/sub x3 | supported |
| `__osDisableInt` | `libultra/interrupt` | 1 | 0x72810 | 32 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/interrupt.o` (handwritten-source), 7 words, 0 masked; COP0/TLB/cache/system instruction x2 | supported |
| `__osRestoreInt` | `libultra/interrupt` | 1 | 0x72830 | 28 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/interrupt.o` (handwritten-source), 6 words, 0 masked; COP0/TLB/cache/system instruction x2 | supported |
| `_bcmp` | `libultra/bcmp` | 1 | 0x73320 | 284 | identical to `perfect_dark/build/ntsc-final/lib/ultra/libc/bcmp.o` (handwritten-source), 71 words, 0 masked; non-argument register read on entry x1 | supported |
| `osGetCount` | `libultra/getcount` | 1 | 0x73730 | 12 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/getcount.o` (handwritten-source), 2 words, 0 masked; COP0/TLB/cache/system instruction x1 | supported |
| `__osProbeTLB` | `libultra/probetlb` | 1 | 0x738c0 | 184 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/probetlb.o` (handwritten-source), 45 words, 0 masked; COP0/TLB/cache/system instruction x9, trapping signed add/sub x3 | supported |
| `__osSetSR` | `libultra/setsr` | 1 | 0x73b00 | 16 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/setsr.o` (handwritten-source), 3 words, 0 masked; COP0/TLB/cache/system instruction x1 | supported |
| `__osGetSR` | `libultra/getsr` | 1 | 0x73b10 | 12 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/getsr.o` (handwritten-source), 2 words, 0 masked; COP0/TLB/cache/system instruction x1 | supported |
| `__osSetFpcCsr` | `libultra/setfpccsr` | 1 | 0x73b20 | 16 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/setfpccsr.o` (handwritten-source), 3 words, 0 masked | supported (thin) |
| `osUnmapTLBAll` | `libultra/unmaptlball` | 1 | 0x73bd0 | 68 | identical to `jfg/build/libultra/src/os/unmaptlball.s.o` (handwritten-source), 16 words, 0 masked; COP0/TLB/cache/system instruction x7, trapping signed add/sub x1 | supported |
| `osMapTLBRdb` | `libultra/maptlbrdb` | 1 | 0x73c20 | 88 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/maptlbrdb.o` (handwritten-source), 21 words, 0 masked; COP0/TLB/cache/system instruction x8 | supported |
| `__osGetCause` | `libultra/getcause` | 1 | 0x73c80 | 12 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/getcause.o` (handwritten-source), 2 words, 0 masked; COP0/TLB/cache/system instruction x1 | supported |
| `__osSetCompare` | `libultra/setcompare` | 1 | 0x75e10 | 12 | identical to `banjo-kazooie/lib/ultralib/build/I/libultra_rom/src/os/setcompare.o` (handwritten-source), 2 words, 0 masked; COP0/TLB/cache/system instruction x1 | supported |
<!-- hasm-audit:end -->
