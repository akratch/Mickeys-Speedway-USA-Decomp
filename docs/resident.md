# The resident segment (docs/modules.md section 3)

Split out of `docs/modules.md` on 2026-08-25; evidence tiers and naming rules are defined in that file, section 1. Section numbers below keep their original 3.x identity so existing references resolve.

## 3. The resident segment (`main`)

ROM `0x1000`–`0x86640`, VRAM `0x80000400`–`0x80085A40`, plus `0x52D10` of BSS.
Always present; the boot code jumps straight into it at `0x800211A0`, and
`mainproc` is byte-identical to DKR's at exactly that address.

Named anchors, in address order. **Tier A** rows are byte-identical to a
reference build's objects (DKR's, JFG's, Perfect Dark's, Banjo-Kazooie's or
Conker's; see `docs/references.md`); **tier C** rows are string-correspondence
with JFG; everything else is noted inline. Ranges without a named anchor are
omitted rather than guessed at. 171 translation units are matched whole across
the segment, carrying 194 function names.

| ROM | VRAM | Anchor | Tier | What it establishes |
|---|---|---|---|---|
| `0x1000` | `0x80000400` | `entrypoint` | A | The reset vector's target |
| `0x1AE60`–`0x1BE50` | `0x8001A260` | `main/lights2` | A | **Measured file boundary**: JFG's whole 0xFF0 `hasm/lights2.s`, 9 routines: the lighting pipeline, a starfield mover, a CPU line rasteriser, a rain draw. The first anchor anywhere in `0x16140`–`0x1C790` |
| `0x31C4` | `0x800025C4` | `audspat_jingle_off` | A | Spatial audio, and the thinnest row adopted |
| `0xC9B4`, `0xF520` | — | `"track/track.c"` asserts | — | **`track` code is partly resident** |
| `0x21DA0` | `0x800211A0` | `mainproc`, `thread1_main`, `func_80021290` | A | Compiler/link-exact C in the boot source unit |
| `0x25C20`-`0x263F0` | `0x80025020` | `main/joy` | B | Controller setup, polling, mapping, accessors and CIC helper; §3.4 |
| `0x263F0`-`0x27760` | `0x800257F0` | `main/level` | B | Level lifecycle and metadata accessors; §3.4 |
| `0x27760`-`0x2A250` | `0x80026B60` | `main/main` | B + C | Main state/frame control, identified by call graph and six file-string references; §3.4 |
| `0x27BB4`, `0x28BB8` | — | `"main/main.c"` asserts | — | **`main` code is resident** |
| `0x29FD0` | `0x800293D0` | `"x = %5d"` … `"a = %3.1f"` | — | On-screen coordinate readout |
| `0x2A250`–`0x2AE44` | `0x80029650` | 11 named `math_util.s` routines | A | Matrix / vector / RNG library. 13 routines matched: 11 named here, `rand_range` already carried as `mathRnd`, and `func_80070058` left unnamed as a placeholder |
| `0x2B650`–`0x2BCD0` | `0x8002AA50` | `main/matrix` | — | The parked float TU (§6.2) |
| `0x2C860` | `0x8002BC60` | `align16`/`align8`/`align4` | A | The allocator |
| `0x2C8C0`–`0x2ECA0` | `0x8002BCC0` | `main/saves` | A/B/D | Rumble, EEPROM/save bitstreams, and Controller Pak files (§3.15) |
| `0x2ECA0`–`0x2F0D0` | `0x8002E0A0` | `main/pi` | B | Asset lookup and cartridge DMA (§3.15) |
| `0x2F0D0`–`0x2F400` | `0x8002E4D0` | `main/screen` | B | Compressed screen loading and drawing (§3.15) |
| `0x2F400`–`0x30CD0` | `0x8002E800` | `main/rcpFast3d` | A/B | Fast3D/RCP task and clear helpers (§3.15) |
| `0x30CD0`–`0x323A0` | `0x800300D0` | `main/sched` | A/B/C | The 21-function game scheduler (§3.15) |
| `0x316E8` | `0x80030AE8` | `"SP CRASHED"`, `"Version %s"` | — | The frame loop / RCP watchdog |
| `0x323A0`–`0x323E0` | `0x800317A0` | `main/rsp_segment` | A | Measured file boundary and compiler/link-exact DKR-adapted C (whole `.text`) |
| `0x323E0`–`0x33FA0` | `0x800317E0` | `main/runlink` | A/B/C | **The runtime overlay linker** (§5) |
| `0x33FA0`–`0x34180` | `0x800333A0` | `main/trapDanglingJump` | A | The overlay call trampoline. **Measured file boundary**: JFG's whole 0x1E0 `hasm/ido/trapDanglingJump.s`. Was named at tier B from Mickey's call graph alone; the bytes agree |
| `0x34180`–`0x34E60` | `0x80033580` | `main/gameVi` | B + A landmarks | **Video and framebuffer management** (§3.8). The complete 23-function order and call/global surface establish the TU boundary; four functions inside are independently tier-A JFG skeleton hits |
| `0x342A8` | `0x800336A8` | `"Ntsc LowRes"` … | — | Video-mode table (15 entries) |
| `0x39A1C` | `0x80038E1C` | `"front/front.c"` asserts | — | **`front` code is partly resident** |
| `0x3B1A0` | `0x8003A5A0` | `"UNKNOWN TRACK"` | — | Track selection |
| `0x3B57C` | `0x8003A97C` | `weather_clip_planes` | A | |
| `0x3D5F0`–`0x43470` | `0x8003C9F0` | `main/particles` | A + B + D | 44-function resident particle TU; §3.16 |
| `0x43470`–`0x45760` | `0x80042870` | `main/diprint` | A + B + C | 19-function formatting/debug-text TU; §3.16 |
| `0x459C0`–`0x467BC` | `0x80044DC0` | `diRcpPrintDL`, `diRcpMoveWd`, `diRcpStrName`, `diRcpOtherMode`, `diRcpGeometryMode` | C | **The display-list disassembler**, a full GBI pretty-printer left in the retail build |
| `0x467BC`–`0x47A60` | `0x80045BBC` | `diCpuReportWatchpoint`, plus the memory/module debug pages and the register-dump crash reporter | C | **The debug monitor**, also left in |
| `0x47A60`–`0x47A70` | `0x80046E60` | `main/get_stack_pointer` | A | Measured file boundary |
| `0x4BC40`–`0x4E1E0` | `0x8004B040` | `main/font` | A/D | JFG's `font.c`: six exact function anchors plus source-order and adjacent-function evidence establish the provisional C split; §3.4 |
| `0x4E378` | `0x8004D778` | `byteswap32` | A | Compiler/link-exact C adapted from DKR `src/gzip.c` |
| `0x4EA60`–`0x4F4D4` | `0x8004DE60` | `main/gzip_asm` | A | **Measured file boundary**: DKR's whole 0xA74 inflate core, in one piece |
| `0x4F4E0` | `0x8004E8E0` | `func_8004E8E0` | D | Matched C adapted from DKR `src/game_text.c` `init_dialogue_text`: exact 188 B / 47 words, frame `0x18`, all 28 relocation identities exact, linked ROM exact |
| `0x4F59C` | `0x8004E99C` | `func_8004E99C` | D | Matched C: exact 80 B and 11/11 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x4F5EC` | `0x8004E9EC` | `func_8004E9EC` | D | Matched C: exact 12 B and 2/2 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x4F5F8` | `0x8004E9F8` | `func_8004E9F8` | D | Matched C adapted from DKR: exact 616 B and 24/24 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x4F9A8` | `0x8004EDA8` | `func_8004EDA8` | D | Matched C: exact 296 B and 25/25 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x4FC20` | `0x8004F020` | `func_8004F020` | D | Matched C: exact 8 executable B and zero relocations at `-O2 -mips2 -32`, linked ROM exact; following 8 B are target padding |
| `0x4FC30`–`0x505E0` | `0x8004F030` | `libultra/exceptasm` | A | **Measured file boundary**, 9 routines including `__osException` and `__osDispatchThread`; §4.2. `0x4FC20` before it is the **rejected** `io/leointerrupt` match, and `0x506D0`–`0x50820` after it is a separate unknown |
| `0x505E0` | `0x8004F9E0` | `osCreateThread` | B; JFG `libultra/src/os/createthread.c` body, with Mickey's measured MIPS-II TU flag | Matched C: exact 232 executable B and 6/6 relocation identities at `-O2 -mips2 -32`; following 8 B are target padding |
| `0x50820`–`0x50C00` | `0x8004FC20` | `main/refractOutputAssembler` | A | Measured file boundary (JFG) |
| `0x58E50`–`0x59B90` | `0x80058250` | `main/vehicle_sounds` | B + D | Four-function positional racer-sound block. Calls the resident XYZ sound API to maintain engine handles and derives pitch/volume from racer speed and listener distance. No exact JFG skeleton hit; the name is descriptive and the existing splat boundary is not claimed as measured |
| `0x59B90`–`0x59BF0` | `0x80058F90` | `main/osBootRamTest` | A | Measured file boundary and byte-exact C (JFG): both IPL3 6105 RAM checks compile to the complete 96-byte target object. |
| `0x5B300`–`0x5C310` | `0x8005A700` | `main/models` | B + D; one A island | Animation-table loading, reference-counted animation storage, frame selection and model-matrix construction establish the descriptive TU name. `camConvertMatrixList` at ROM `0x5B778` alone is byte-identical to JFG `camera.c`; no whole-object identity is claimed |
| `0x5C310`–`0x5E6B0` | `0x8005B710` | `main/gsSnd` | A | **The sound player**, 0x23A0 in one piece, 22 named functions. Two of those names were predicted at tier C from error strings and fall inside this TU at exactly the predicted addresses |
| `0x5E6B0`–`0x6AF90` | `0x8005DAB0` | libultra's `n_audio` synthesis library | A | 45 consecutive measured file boundaries, 106 names, plus two JFG maths TUs interleaved (`math_atan`, `math_acosf`); a third, `math_arc`, begins at `0x6AF90` immediately after. §4.2 |
| `0x6B3D0`–`0x6F3E0` | `0x8006A7D0` | Transfer Pak, Rumble Pak, Controller Pak filesystem | A | 18 measured file boundaries, 34 names. The Transfer Pak three come from **Perfect Dark**, the only reference build that has them; §4.2 |
| `0x6F420`–`0x76D10` | `0x8006E820` | the libultra corridor | A | §4.1 |
| `0x76D10`–`0x76E60` | — | non-resident text | — | Indexes off `$at`, loads from address 0; relocated before it runs. Still `bin` |
| `0x76E60`–`0x81590` | `0x80076260` | `.data`, mostly undifferentiated; selected SDK tails are TU-owned | — | §6.3 |
| `0x81590`–`0x86640` | `0x80080990` | `.rodata`, mostly anonymous; selected SDK tails are TU-owned | — | §6.3 |

### 3.1 Which modules are resident

The ROM carries `__FILE__` path strings from `assert`-style call sites, and
where those strings are *referenced from* is direct evidence of where a
module's code lives.

| Module | Path string copies | Referenced from | Conclusion |
|---|---|---|---|
| `main` | 6, at `0x80081B0C`–`0x80081B48` | resident: `0x80026FB4`, `0x80027FB8` | Fully resident |
| `track` | 14, at `0x80081540`–`0x80081610` | resident: `0x8000BDB4`, `0x8000E920` | **Partly resident** |
| `front` | 2, at `0x800826C0`, `0x800826D0` | resident: `0x80038E1C` | **Partly resident** |
| `clone` | 2, at ROM `0x188B4D0`, `0x188B4E0` | not referenced from the resident segment at all | **Overlay-only**: both strings sit in **overlay 43**'s `.data`, at offsets `0x1500`/`0x1510` from that module's base (§5.3) |

`main` is the permanently resident module; `front` and `track` straddle the
boundary, with resident stubs or shared helpers that carry their own assert
strings; `clone` exists only inside the overlay region. The scheduler's task
taxonomy agrees independently: `SC_TASK_CLONE` is one of its seven task types
(`include/game/sched.h`), so `clone` is a task, i.e. something scheduled rather
than something always present.

`main`, `front`, `track` and `clone` are **source-file names, not overlay
boundaries**. The ROM's overlay segmentation is 107 modules (§5.3); `clone`'s
two `__FILE__` copies land in one of them, and nothing says a source module maps
to one overlay.

### 3.2 What the debug content says about this build

Two substantial debug subsystems survive into the retail ROM: a complete GBI
display-list disassembler (`0x459C0`–`0x467BC`, every `G_*` and `RM_*` name
spelled out) and a debug monitor with memory-region pages, a module list and a
full register-dump crash reporter (`0x467BC`–`0x47A60`). Together that is
roughly 8KB of code plus 4KB of strings, and it is why so much of the resident
segment can be identified from strings alone. It also means the *linker* is
observable from the outside: the crash reporter calls `runlinkGetAddressInfo`
to turn a faulting address into "Module %d at %08x".

### 3.3 Resident TU map from skeleton donors

A masked-instruction-skeleton scan (registers, immediates and jump targets
masked; opcode/funct/fmt kept, so same-source-same-compiler code matches
regardless of register allocation) was run against every function of at
least 10 words in Jet Force Gemini's and Banjo-Kazooie's built objects, over
the still-unnamed code of ROM `0x1000`–`0x6F420`. It found 88 unambiguous
hits (one candidate reference name apiece); two, both inside ROM
`0x76D10`–`0x86990` (the `.data`/`.rodata` tail, not code), turned out to be
JFG float-literal symbols placed inside `.text` and were discarded as
meaningless there. Four more (`0x8001A2C4`, `0x8001A4BC`, `0x8001A774`,
`0x8001A9A4`) land inside the already-measured `main/lights2` whole-file
boundary (§3, table) and are not new. Eleven did not survive independent
re-verification with `tools/find_known_objects.py` against the same
reference build (real relocation-record masking, not the coarse opcode-class
mask): no exact byte match exists at that address once actual register
allocation is compared, so the coarse scan's hit there is a same-shape
coincidence, not a same-source one. They are not adopted:
`Sinf`, `fmvInit`, `camStopShakes`, `camSetZoom`, PD's `osCreatePiManager`,
and six JFG `func_`-placeholder hits.

The remaining 71 were independently re-verified byte-for-byte (masked words
under real relocation records, `romocc` computed): 68 cleared the tier-A bar
in full (docs/modules.md 1.2) and were adopted. In that scan, one at
`0x8002D824` failed on uniqueness (Banjo-Kazooie's `unallocUnusedBlock`,
`romocc=4`) and two more fell short of the 6-unmasked-word floor or left
`romocc` unresolved (`texLoadTextureAddr` at 5 words; `viFrameRateReset`,
`romocc=?`). The 2026-08-27 final JFG source pass subsequently compiled
`texLoadTextureAddr` from the donor C and proved its complete function and
linked-ROM bytes exact; that source-level proof supersedes the earlier
name-only threshold refusal. The other two remain unadopted. Four further hits
(`matrix_RPY_XYZ`,
`matrix_XYZ_YPR_SCL`, `matrix_XYZ_YPR`, `matrixTransposeVectorMultiply`, all
inside `main/matrix`) clear the bar but are **not** written into
`symbol_addrs.us.txt`: their C is parked non-matching, and 1.5 forbids naming
a symbol whose C is not in the ROM. They are recorded instead in §6.2.

The adjacent anonymous `func_8002B040` is separately authenticated over ROM
`0x2BC40..0x2BCD0`: 34 frameless words, no padding before `mmInit`, and no
relocations. Its configured guarded C emits 35 words and matches 1/34
positionally from `+0x0`; IDO spills and reloads the second `f32` formal where
the target transfers all three incoming GPR bit patterns directly to FP
registers. The complete 119-row flag lattice is nonexact (`-O2 -mips1 -32`
improves only to 2/34 while remaining one word long), as are ten bounded
source-faithful type, matrix-shape, temporary, K&R, register, operand-order,
and uopt-O3 forms. Four shipped callers in Overlays 69,
88, and 94 authenticate the scalar-vector ABI. DKR/JFG matrix-transform
assembly supplies structural context only; it does not justify a friendly
name or a C-origin claim. The assembly fallback remains canonical.

Of the 68 adopted, 46 carry a real JFG/BK function name (adopted verbatim,
`symbol_addrs.us.txt`); 22 are JFG placeholder names (`func_8xxxxxxx`), which
1.5 forbids importing, so Mickey's own `func_<VRAM>` stands and the comment
records only the donor translation unit.

**What this adds to the TU picture**, one row per donor TU, functions found
in each and the ROM span of just those functions (not a boundary claim --
see the caveat below):

| Donor TU | Functions found | ROM span of finds | Status |
|---|---|---|---|
| `libultra/n_csplayer.c.o` | 5 | `0x5E970`–`0x61828` | Inside the already-measured `libultra/n_csplayer` boundary (§ table); corroborates it |
| `gsSnd.c.o` | 6 | `0x5C578`–`0x5DFA4` | Inside the already-measured `main/gsSnd` boundary; corroborates it |
| `libultra/n_drvrNew.c.o` | 1 | `0x659C0` | At the exact start of the already-measured `libultra/n_drvrNew` boundary; corroborates it |
| `libultra/n_env.c.o` | 1 | `0x6910C` | Inside the already-measured `libultra/n_env` boundary; corroborates it |
| `libultra/n_load.c.o` | 1 | `0x6A634` | Inside the already-measured `libultra/n_load` boundary; corroborates it |
| `hasm/ido/math_util.s.o` | 15 | `0x2A9E4`–`0x2B644` | Inside the already-measured `main/math_util` boundary; corroborates it |
| `src/menu.c.o` | 6 | `0x3A184`–`0x3B008` | Inside yaml's unnamed `0x37D50`–`0x3B480` block. No whole-`.text` match found, so no boundary is claimed |
| `src/gameVi.c.o` | 4 | `0x34B68`–`0x34E60` | Inside the now-split `main/gameVi` TU (§3.8). The four exact skeleton hits are landmarks; the boundary is separately established at tier B from the complete ordered function/call surface, not claimed as a whole-`.text` byte match |
| `src/menu.c.o` | 6 | `0x3A184`–`0x3B008` | The automated pass found only interior anchors. A later function-order and call-graph census established the narrower `0x39350`–`0x3B1A0` ownership (§3.11); no whole-`.text` match is claimed |
| `src/gameVi.c.o` | 4 | `0x34B68`–`0x34E60` | Inside yaml's unnamed `0x34180`–`0x37D50` block. No whole-`.text` match; no boundary claimed |
| `src/anim.c.o` | 3 | `0x50D7C`–`0x51D28` | Inside yaml's `main/anim` source-owning block at `0x50C00`–`0x58570`. No whole-`.text` match; the individual hits do not establish an internal boundary |
| `src/models.c.o` | 3 | `0x20020`–`0x21710` | Inside yaml's unnamed `0x20020`–`0x21DA0` block, starting exactly at its boundary. No whole-`.text` match; no boundary claimed |
| `src/font.c.o` | 2 | `0x4BC70`–`0x4C884` | The original >=10-word scan found two anchors. The later complete census in §3.4 found four more exact short functions and split `main/font` provisionally; no whole-`.text` match is claimed |
| `src/audio_manager_4C50.c.o` | 2 | `0x45F0`–`0x4F3C` | Starts exactly at yaml's `0x45F0` boundary; ends inside the unnamed `0x4F40`–`0xC950` block. No whole-`.text` match; no boundary claimed |
| `src/audio_manager_1050.c.o` | 3 | `0x12BC`–`0x22C8` | Inside yaml's unnamed `0x1050`–`0x45F0` block. Wide span for 3 hits -- other code plainly sits between them; no boundary claimed |
| `src/charControl.c.o` | 2 | `0x1CED4`–`0x1FFAC` | Inside yaml's former unnamed `0x1C790`–`0x20020` block. This scan alone claimed no boundary; §3.4 records the later TU split and its additional evidence |
| `src/camera.c.o` | 2 | `0x23360`, `0x5B778` | 230KB apart -- evidently not one placed TU here; treat as two independent identifications, not a span |
| `src/memory.c.o` | 2 | `0x2BCD0`–`0x2C3AC` | Starts exactly at yaml's `0x2BCD0` boundary (end of `main/matrix`); the already-named `align16`/`align8`/`align4` (tier A, `memory.c.o`) sit at `0x2C860`, past this span. Consistent with one TU, no boundary claimed |
| `src/shadows_214A0.c.o` | 2 | `0x18FF0`–`0x19144` | Inside yaml's unnamed `0x18FF0`–`0x1AE60` block, starting exactly at its boundary. No boundary claimed |
| `src/saves.c.o`, `src/rcpFast3d.c.o` | 1 each | single points | These were the Tier A seeds expanded by the later call-graph census in §3.15 |
| `src/track.c.o`, `src/textures.c.o`, `src/diCpu.c.o`, `src/objects.c.o`, `libultra/src/flash/flashreadid.c.o`, `us.v10/src/core1/code_1D00.c.o` (BK) | 1 each | single points | Isolated identifications, no span to claim |

**Why this table originally added no `mickey.us.yaml` splits.** §1's
"measured file boundary" tier requires a whole-`.text` match; this pass only
matched individual functions (`tools/find_known_objects.py --sections` found
no whole-object match for any of the not-yet-named TUs above). The later
`main/font` split is explicitly provisional, not a tier-A measured-file claim:
§3.4 records its additional endpoint and ordering evidence. The already
measured TUs above (`n_csplayer`, `gsSnd`, `n_drvrNew`, `n_env`, `n_load`,
`math_util`) needed no new split; they already have one.

### 3.4 `main/font` census

ROM `0x4BC40`–`0x4E1E0`, VRAM `0x8004B040`–`0x8004D5E0`, is split as
`main/font`. The split is provisional (tier D at the file-boundary level), not
a whole-object match. It begins with JFG's byte-identical `fontSetWindow0`,
contains JFG `font.c` functions in source order, and ends after the
`fontYSpacing`-shaped leaf; the next function is the independently identified
`osCreatePiManager`. A supplemental all-size object scan found six exact JFG
anchors in the range: `fontSetWindow0`, `fontSetWindowNoise`, `fontColour`,
`fontWindowColour`, `fontWindowFontColour`, and
`fontWindowFontBackground`. Each has one ROM occurrence; the four colour
setters have at least 7 unmasked words and the two already adopted functions
have at least 10. JFG's complete `font.c.o` does not match Mickey's complete
range.

PROVENANCE: the TU identity, candidate names, declarations, and struct-layout
starting point come from Jet Force Gemini's public decompilation
(`src/font.c`, `src/font.h`, and its built object), a permitted published
retail-derived decomp under `docs/CLEANROOM.md`. Mickey's instructions,
relocations, call graph, and ROM comparison remain authoritative. A
PROVENANCE note is carried at the point of use in `src/main/font.c`.

The table is the complete original `0x4BC40`–`0x4EA60` block census. "A" is
an exact object/ROM skeleton identity; "B" is a call-graph role; "D" is only
source order and structure. D-only JFG placeholders remain Mickey
`func_<VRAM>` names. Calls list in-range callees; `ext` means only resident or
overlay callers/callees outside the range were observed.

| ROM | Size | Mickey symbol | JFG correspondence | Evidence | Calls |
|---|---:|---|---|---|---|
| `0x4BC40` | `0x24` | `fontSetWindow0` | same | A, matched C | leaf; ext callers |
| `0x4BC64` | `0x0C` | `func_8004B064` | `fontSetButtonMode` | D, matched C | leaf; overlay caller |
| `0x4BC70` | `0x34` | `fontSetWindowNoise` | same | A, matched C | leaf |
| `0x4BCA4` | `0x14` | `func_8004B0A4` | `fontUseFont` | D, matched C | leaf; text-setup callers |
| `0x4BCB8` | `0x24` | `fontColour` | same | A, matched C | leaf; text-setup callers |
| `0x4BCDC` | `0x1C` | `func_8004B0DC` | `fontBackground` | B/D, matched C | leaf; text-setup callers |
| `0x4BCF8` | `0x44` | `func_8004B0F8` | `fontPrintXY` | B/D, matched C | calls `0x4BD3C` |
| `0x4BD3C` | `0xA0` | `func_8004B13C` | `fontPrintWindowXY` | B/D, matched C | calls `0x4BDDC` |
| `0x4BDDC` | `0x8B0` | `func_8004B1DC` | JFG `func_80070518` | D, plateau | calls `0x4DF9C`, `0x4C68C`, `0x4D290`, ext |
| `0x4C68C` | `0xB8` | `func_8004BA8C` | `fontStringWidth` | B/D, reproof | calls `0x4DF9C`; called twice by `0x4BDDC`, once by `0x4C8C4`, and by overlays 41 and 45 |
| `0x4C744` | `0x9C` | `func_8004BB44` | `fontWindowSize` | D, matched C | leaf; ext callers |
| `0x4C7E0` | `0x1C` | `func_8004BBE0` | `fontWindowUseFont` | D, matched C | leaf; ext callers |
| `0x4C7FC` | `0x40` | `fontWindowColour` | same | A, matched C | leaf; ext callers |
| `0x4C83C` | `0x48` | `fontWindowFontColour` | same | A, matched C | leaf; ext callers |
| `0x4C884` | `0x40` | `fontWindowFontBackground` | same | A, matched C | leaf; ext callers |
| `0x4C8C4` | `0x2A0` | `func_8004BCC4` | `fontWindowAddStringXY` | B, plateau | calls `0x4D1A4`, `0x4C68C`; ext callers |
| `0x4CB64` | `0x4C` | `func_8004BF64` | `fontWindowFlushStrings` | D, matched C | leaf; ext callers |
| `0x4CBB0` | `0x28` | `func_8004BFB0` | `fontWindowEnable` | D, matched C | leaf; ext callers |
| `0x4CBD8` | `0x28` | `func_8004BFD8` | `fontWindowDisable` | D, matched C | leaf; ext callers |
| `0x4CC00` | `0xC4` | `func_8004C000` | `fontStringAddNumber` | D, matched C | leaf; called by `0x4D1A4` |
| `0x4CCC4` | `0x7C` | `func_8004C0C4` | `fontWindowsDraw` | B | calls `0x4CE00`; ext caller |
| `0x4CD40` | `0xC0` | `func_8004C140` | JFG `func_80071564` | D | ext callee; called by `0x4CE00` |
| `0x4CE00` | `0x3A4` | `func_8004C200` | `fontWindowDraw` | B | calls `0x4CD40`, `0x4D1A4`, `0x4BDDC` |
| `0x4D1A4` | `0xEC` | `func_8004C5A4` | JFG `func_80071A0C` | D, matched C | calls `0x4CC00`; in-range callers |
| `0x4D290` | `0x248` | `func_8004C690` | JFG `func_80071B08` | D | ext callee; called by `0x4BDDC` |
| `0x4D4D8` | `0xA54` | `func_8004C8D8` | `fontCreateDisplayList` | D | ext callee |
| `0x4DF2C` | `0x70` | `func_8004D32C` | no JFG counterpart | D | leaf; ext caller |
| `0x4DF9C` | `0x70` | `func_8004D39C` | `fontConvertString` | D, plateau | leaf; in-range callers |
| `0x4E00C` | `0x1B4` | `func_8004D40C` | `fontGetLine` | D | leaf |
| `0x4CCC4` | `0x7C` | `func_8004C0C4` | `fontWindowsDraw` | B/D, matched C | calls `0x4CE00`; ext caller |
| `0x4CD40` | `0xC0` | `func_8004C140` | DKR `render_fill_rectangle` | B/D, matched C | ext callee; called by `0x4CE00` |
| `0x4CE00` | `0x3A4` | `func_8004C200` | `fontWindowDraw` | B/D, matched C | calls `0x4CD40`, `0x4D1A4`, `0x4BDDC` |
| `0x4D1A4` | `0xEC` | `func_8004C5A4` | JFG `func_80071A0C` | D, matched C | calls `0x4CC00`; in-range callers |
| `0x4D290` | `0x248` | `func_8004C690` | JFG `func_80071B08` | D, plateau | ext callee; called by `0x4BDDC` |
| `0x4D4D8` | `0xA54` | `func_8004C8D8` | `fontCreateDisplayList` | B/D, matched C | ext callee |
| `0x4DF2C` | `0x70` | `func_8004D32C` | no JFG counterpart | D, matched C | leaf; ext caller |
| `0x4DF9C` | `0x70` | `func_8004D39C` | `fontConvertString` | B/D, matched C | leaf; in-range callers |
| `0x4E00C` | `0x1B4` | `func_8004D40C` | `fontGetLine` | A, matched C | exact words, frame, relocations, and linked bytes |
| `0x4E1C0` | `0x20` | `func_8004D5C0` | `fontYSpacing` | D, matched C | leaf |
| `0x4E1E0` | `0x170` | `func_8004D5E0` | `osCreatePiManager` | A, matched C | IDO C exact; SDK calls; ext callers |
| `0x4E350` | `0x28` | `func_8004D750` | `rzipInit` | A, matched C | IDO C exact; allocator call; ext caller |
| `0x4E378` | `0x30` | `byteswap32` | DKR `byteswap32`; JFG `rzipUncompressSize` | A, matched C | leaf; ext callers |
| `0x4E3A8` | `0x38` | `func_8004D7A8` | `rzipUncompressSizeROM` | A, matched C | IDO C exact; calls `byteswap32`, ext |
| `0x4E3E0` | `0x60` | `func_8004D7E0` | `rzipUncompress` | A, matched C | IDO C exact; calls `gzip_inflate_block`; ext callers |
| `0x4E440` | `0x620` | `func_8004D840` | `huft_build` | Tier B, matched C | DKR `src/gzip.c`; all 390 words, frame, relocations, and linked bytes exact; called by `main/gzip_asm` |

`func_8004B1DC` has a readable DKR-JP-derived candidate under
`NON_MATCHING`. Its best stock-flag build has the target's 128-byte frame and
matches through function offset `+0x2C`, but is 28 instructions short with
broad control-flow divergence after the initial null check. The flag lattice
kept `-O2 -mips2` best; the unresolved issue is source organization and live
ranges across the scissor and glyph loops, not a compiler-flag mismatch.

`func_8004C690` remains tier-D `NON_MATCHING`: 105/146 words differ, 144/146 instructions, frame -112, first `+0x0`.
Levers covered flags, widths/qualifiers, direct-global/font-index, copy AST/volatile, and block/pad homes.
Remaining: initial pool setup and saved-header copy schedule.

`func_8004D40C` is Evidence A exact C: all 109 instruction words, its `0x18`
frame, relocation identities, and linked ROM bytes match after bounded
permutation resolved the final temp web.

`func_8004BA8C` owns VRAM `0x8004BA8C..0x8004BB44`, ROM
`0x4C68C..0x4C744`: 184 bytes/46 words, frame `0x30`, and no target padding.
The isolated object's trailing eight alignment bytes are outside the function.
Historical policy-defective configured isolated and full-TU C are byte-identical
at 38/46 raw and relocation-normalized words, first `+0x30`; that body used an
artificial stack pad and empty condition. Clean current C is uncompiled, so its
score, frame, extent, first mismatch, and emitted tuples are unknown. Ordinary
46/46 object and linked equality are assembly fallback only. The historical
eight residual sites are the `fontData` carrier, `fontData`/`spacing` spill
homes, and two equivalent operand orders.

All nine target tuples were exact in the historical C: pairs to `D_800D60E4` at
`+0x04/+0x08`, `D_800D6628` at `+0x14/+0x28`, and `D_800D6644` at
`+0x34/+0x38` and `+0x48/+0x54`, plus the `func_8004D39C` call at `+0x40`.
ORT 880 exports resident offset `0x4B63C`. Its five callers are
`func_8004B1DC+0x1E4/+0x294`, `func_8004BCC4+0x12C`,
`overlay41DrawItem+0x4C`, and `overlay45ConfigureLayout+0x9C`; there are no
other direct, runtime-table, overlay-SYMBOL, or stored-pointer inbounds. The
existing Overlay 41 rename and Overlay 45 proxy preserve the runtime carriers;
current candidate tuples still await configured V0.

The retained 38/46 score is diagnostic: `stackPad` and its empty condition were
artificial frame/allocation aids and are now removed, leaving clean V0
uncompiled. JFG's same-sized function is an assembly-backed NON_EQUIVALENT
draft and DKR's Japanese structural branch was not the built US producer.
Retain exactly 119 flags, capture one allocator trace, try a replacing scoped
`glyphIndex` and one trace-selected `fontData`/`spacing` lifetime form, then
combine only independent gains. Hard cap 122 stock builds plus one trace; no
generic permutation without a policy-clean natural gain.

The font subsegment's FP-register census contains only even-numbered single-
precision registers (`$f0`, `$f4`, `$f6`, `$f8`, `$f10`, `$f16`, and `$f18`),
so no function in this TU was excluded by the odd-register rule in section
6.2.

There are no direct string-literal references in this block. Its data
relocations address font/window state, a font-cache jump table, and rzip
state; consequently no tier-C names are available. ROM `0x4E1E0`–`0x4EA60`
is deliberately outside `main/font`: it is the PI-manager/rzip prefix of the
inflate subsystem, immediately followed by `main/gzip_asm` at `0x4EA60`.

### 3.5 The resident shadows and lights TUs

ROM `0x18FF0`–`0x1AE60` contains two source units followed by the already
measured `main/lights2` hand-written assembly object. The boundary is ROM
`0x19310` (VRAM `0x80018710`), a 16-byte-aligned function boundary. JFG's US
layout puts `shadows_214A0.c` immediately before `lights.c`; Mickey reproduces
the same function order, with the exact `shadowMakeYs` body ending at this
boundary and the allocation/free call graph of `lights.c` beginning there.
The far end is fixed independently by the whole-object tier-A `main/lights2`
match at `0x1AE60`. This is tier B boundary evidence, not a claim that either
C object is whole-object-identical to JFG's.

PROVENANCE DISCLOSURE. The comparison names below come from JFG's public
decomp, `src/shadows_214A0.c`, `src/lights.c`, `src/lights.h`, and its built
objects. JFG is a permitted published retail-derived decomp under
`docs/CLEANROOM.md`. A name marked "comparison" is navigation evidence only:
it is not adopted into `symbol_addrs.us.txt` until it earns one of §1's name
tiers and its C is no longer parked behind `GLOBAL_ASM` (§1.5).

The four shadows functions contain all 88 odd-FP operands in this range and
remain `main/shadows` assembly under §6.2. The 28-function `main/lights` TU
contains none and is split to C with assembly fallbacks. The complete census:

| Mickey VRAM | Size | JFG namesake | Evidence / disposition |
|---:|---:|---|---|
| `0x800183F0` | `0xC4` | `shadowBoxPolyOverlap` | Tier A: 49/49 unmasked words, ROM-wide unique; already adopted |
| `0x800184B4` | `0x90` | `shadowBoundingBox` | Tier A: 36/36 unmasked words, ROM-wide unique; already adopted |
| `0x80018544` | `0x110` | `shadowYHeight` | comparison only: unique nearest 4-gram skeleton, 0.919; remains `func_80018544` |
| `0x80018654` | `0xBC` | `shadowMakeYs` | Tier-A candidate: 47/47 unmasked words, ROM-wide unique; assembly pending a function-sized naming commit |
| `0x80018710` | `0x8C` | `freeLights` | tier-B comparison: three frees and the JFG TU position; C still `func_80018710` |
| `0x8001879C` | `0x130` | `setupLights` | Tier B: JFG-adapted C is compiler/link exact at 76/76 words |
| `0x800188CC` | `0xB0` | JFG placeholder `func_80020D94` | placeholder names are prohibited by §1.5; remains `func_800188CC` |
| `0x8001897C` | `0x238` | `addRomdefLight` | tier-B comparison from TU order and light-update callees; C still `func_8001897C` |
| `0x80018BB4` | `0x200` | `addObjectLight` | tier-B comparison from TU order and light-update callees; C still `func_80018BB4` |
| `0x80018DB4` | `0x10` | `turnLightOff` | Tier A: adapted JFG body is compiler-exact under canonical flags and linked byte-identically |
| `0x80018DC4` | `0x10` | `turnLightOn` | Tier A: adapted JFG body is compiler-exact under canonical flags and linked byte-identically |
| `0x80018DD4` | `0x10` | `toggleLight` | Tier A: adapted JFG body is compiler-exact under canonical flags and linked byte-identically |
| `0x80018DE4` | `0x2C` | `changeLightColour` | Tier A: adapted JFG body is compiler-exact under canonical flags and linked byte-identically |
| `0x80018E10` | `0x20` | `changeLightColourCycle` | Tier A: 7 unmasked of 8 words, ROM-wide unique; adapted C is linked byte-identically and adopted |
| `0x80018E30` | `0x4C` | `changeLightIntensity` | Tier A: adapted JFG body is compiler-exact under canonical flags and linked byte-identically |
| `0x80018E7C` | `0x8C` | `lightUpdateLights` | tier-B comparison: loop calls the following per-light updater |
| `0x80018F08` | `0x334` | JFG placeholder `func_80021444` | Evidence D: Mickey reconstruction is compiled as a candidate: 199/205 instructions, 166 differing words, first `+0x0`, frame `-0x60` versus `-0x48`, and 14 relocation-symbol sites differ; not shape-exact. |
| `0x8001923C` | `0x104` | `killLight` | tier-B comparison from free/update call graph and TU order |
| `0x80019340` | `0x18` | `lightGetLights` | Tier A: adapted JFG body and both global relocations are linked byte-identically |
| `0x80019358` | `0x13C` | `lightGetStrongestEffect` | tier-B comparison: square-root distance calculation and TU order |
| `0x80019494` | `0xA8` | `lightUpdateObjects` | tier-B comparison: calls the following object-light helper |
| `0x8001953C` | `0x3F8` | JFG placeholder `func_80021B9C` | placeholder prohibited; remains `func_8001953C` |
| `0x80019934` | `0xF0` | `lightDistanceCalc` | Tier A: JFG-adapted C and the five-entry compiler-owned switch table are linked byte-identically |
| `0x80019A24` | `0x94` | `lightDirectionCalc` | unique nearest skeleton (0.432) and exact JFG size; comparison only |
| `0x80019AB8` | `0x2E0` | `lightObject` | Evidence D: Mickey reconstruction is compiled as a candidate: 191/184 instructions, 178 differing words, first `+0x0`, frame `-0xD0` versus `-0xC8`, and 54 relocation-symbol sites differ; not shape-exact. |
| `0x80019D98` | `0x50` | `lightDefaultObjectLight` | tier-B comparison: delegates to the following setter |
| `0x80019DE8` | `0xFC` | `lightSetObjectLight` | tier-D boundary; matched exactly. The two ring temporaries the target spends before the delta web are the two field re-reads in `state->valueDelta = state->endValue - state->startValue`. |
| `0x80019EE4` | `0x98` | `lightSetupLightSources` | tier-B comparison: loop calls the adopted `addObjectLight` comparison |
| `0x80019F7C` | `0x8C` | `lightSetupFlareSources` | tier-B comparison: adjacent setup loop and flare helper |
| `0x8001A008` | `0x14C` | `lightInitObjectLighting` | tier-B comparison; `NON_MATCHING` plateau after the flag lattice and nine source/declaration forms: exact 83-word frame/opcode/register/FP/relocation shape, but 4 positional words differ, first `+0x70`, because the call-live result spills at `0x28(sp)` instead of `0x2C(sp)`; the permuter importer scores the isolated function zero, but the required full-TU build retains this mismatch |
| `0x8001A154` | `0xE8` | `lightAdjustGlowingLight` | tier-B comparison; `NON_MATCHING` workbench `register-ring-only` plateau after lifetime/mask/scaled-size levers and a 30-minute permuter batch: exact 58-word shape and call relocation, 13 register-only differences from `+0x1C`; the isolated permuter lead regresses in the full TU |
| `0x8001A23C` | `0x24` | `lightKillGlowingLight` | tier-B comparison: calls the paired delete helper and returns success |
### 3.6 The resident allocator (`main/memory`)

ROM `0x2BCD0`–`0x2C8C0`, VRAM `0x8002B0D0`–`0x8002BCC0`, `0xBF0` bytes.
This boundary is measured from the linked pre-split ELF: `2BCD0.s.o` owns one
`0xBF0`-byte text section, its last function ends at `0x2C8B4`, and its final
12 bytes are alignment before `2C8C0.s.o` begins. This corrects the earlier
provisional task range ending at `0x2C950`, which crosses into the next object.
The TU has no floating-point instructions and no string references.
It owns BSS `0x800D1C60`-`0x800D21C0` (`0x560` bytes): four pool records,
the pool count and pad, 256 queued pointers, 256 delay bytes, four counters,
and final section alignment.

**PROVENANCE:** the correspondence names below were read from Jet Force
Gemini's published `src/memory.c`, `src/memory.h`, built `memory.c.o`, and
public symbol map. The two exact skeleton hits are tier A; the remaining JFG
correspondences are tier B call-graph arguments and stay beside their Mickey
`func_` symbols until matched C justifies adoption under §1.5. JFG lacks
Mickey's 8-byte alignment helper and has a trailing `mmSlotPrint` routine that
Mickey lacks. No distinctive string is referenced, so there is no tier C row.

| ROM | Mickey symbol | JFG correspondence | Tier and evidence |
|---|---|---|---|
| `0x2BCD0` | `mmInit` | `mmInit` | A: unique 30-word skeleton with 14 relocated words; linked C exact |
| `0x2BD48` | `mmExtended` | `mmExtended` | B: returns the expansion-memory flag consumed by `mmInit`; matched C exact |
| `0x2BD54` | `func_8002B154` | `mmAllocRegion` | B: allocates slot storage, then calls the pool initializer with it; linked C exact |
| `0x2BDA0` | `func_8002B1A0` | `mempool_init` | B: shared callee of `mmInit` and the region allocator; initializes the 0x10-byte pool and 0x14-byte slot records; linked C exact |
| `0x2BE80` | `func_8002B280` | `mmAlloc` | B: main-pool wrapper that derives a caller colour tag and calls the slot finder; linked C exact |
| `0x2BF14` | `func_8002B314` | `mmAlloc2` | B: second wrapper with the same calls and result role; linked C exact |
| `0x2BFA8` | `func_8002B3A8` | `mempool_slot_find` | B: common worker used by all three allocation wrappers and the fixed-address allocator; linked C exact |
| `0x2C0C0` | `func_8002B4C0` | `mmAllocR` | B: selects a pool by its slot-array pointer, then calls the common worker; linked C exact |
| `0x2C124` | `func_8002B524` | `mmAllocAtAddr` | B: fixed-address allocator; retained pre-cleanup C is diagnostic 102/116 raw/normalized words, frame `0x58`, first `+0xE0`, with all 12 tuples exact. Artificial volatile pad is removed; clean V0 is uncompiled and linked equality is fallback-only. ORT 547 has nine calls plus one stored pointer. |
| `0x2C2F4` | `mmSetDelay` | `mmSetDelay` | B: writes the deferred-free delay used by `mmFree`; matched C exact |
| `0x2C300` | `func_8002B700` | `mmFlushFreeStack` | B: drains queued addresses through the address-free worker; linked C exact |
| `0x2C368` | `mmFree` | `mmFree` | A: unique 17-word skeleton with four relocated words masked; linked C exact |
| `0x2C3AC` | `func_8002B7AC` | `mmFreeTick` | B: services the delayed-free queue; linked C exact |
| `0x2C4A8` | `func_8002B8A8` | `mempool_free_addr` | B: finds an address's pool and clears its matching live slot; linked C exact |
| `0x2C53C` | `func_8002B93C` | `mempool_free_queue` | B: appends an address and delay to the deferred-free arrays; linked C exact |
| `0x2C578` | `func_8002B978` | `mempool_get_pool` | B: reverse-searches the pool table for the containing address range; linked C exact |
| `0x2C5D0` | `func_8002B9D0` | `mempool_slot_clear` | B: frees a slot and coalesces adjacent free records; linked C exact |
| `0x2C720` | `mmGetSlotPtr` | `mmGetSlotPtr` | B: returns one pool's slot-array pointer; matched C exact |
| `0x2C734` | `mmGetDelay` | `mmGetDelay` | B: returns the deferred-free delay; matched C exact |
| `0x2C740` | `func_8002BB40` | `mempool_slot_assign` | B: JFG role/call-graph donor; canonical C is exact for all 72 frameless words, all eight relocation tuples, and the linked resident range. Four direct callers; no export/runtime/pointer inbound. |
| `0x2C860` | `align16` | `mmAlign16` | A: existing exact 7-word `memory.c.o` match; JFG corroborates the role |
| `0x2C87C` | `align8` | — | A: existing exact 7-word `memory.c.o` match; no JFG counterpart |
| `0x2C898` | `align4` | `mmAlign4` | A: existing exact 7-word `memory.c.o` match; JFG corroborates the role |

`func_8002B524` owns VRAM `0x8002B524..0x8002B6F4`, ROM
`0x2C124..0x2C2F4`: 464 bytes/116 words, frame `0x58`, saves `s0` through
`s3` and `ra`, and has no target padding. Retained pre-cleanup full-TU and
isolated C agree at diagnostic 102/116 raw/normalized words, first `+0xE0`;
ordinary object/TU/ROM equality is assembly fallback only. Its `volatile pad`
was an artificial stack-home aid and is restored to JFG's plain unused local,
leaving clean V0 uncompiled. The return-address sentinel remains authentic:
`RevealReturnAddresses` rewrites it from the function patch table.

All 12 target tuples were exact in diagnostic C: pairs to `D_8007A278` at
`+0x04/+0x08`, `D_8007A270` at `+0x1C/+0x3C`, `D_8007A27C` at
`+0x4C/+0x50`, and `D_800D1C60` at `+0x90/+0x94`; a
`runlinkGetAddressInfo` call at `+0x74`; and `func_8002BB40` calls at
`+0x134,+0x160,+0x180`. ORT 547 exports offset `0x2B0D4`. Resident calls are
`runlinkSuspendCode+0xB4`, `runlinkResumeCode+0xD8`, and
`func_80034448+0x12C/+0x1D0`; Overlay 2/18/19/35 provide five more calls, and
ROM `0x7AE40` stores one function pointer used by `RevealReturnAddresses`.
No resident runtime-table record targets the export.

The clean body uses ordinary `-O2 -mips2 -32`; pinned JFG authenticates its
role and plain local but is a larger MIPS-I structural donor, not exact C.
Retain clean V0 and 119 flags, try scoped direct slot-data and split-guard forms
plus an independently improving combination, then one trace-selected natural
lifetime form. Hard cap 123 stock builds plus one trace; no generic batch.

`func_8002B7AC` owns VRAM `0x8002B7AC..0x8002B8A8`, ROM
`0x2C3AC..0x2C4A8`: 252 bytes/63 words, frame `0x30`, saves `s0` through
`s5` and `ra`, and has no target padding. It is exact canonical C, all 63
words and all 12 HI16/LO16 and R_MIPS_26 identities at the target offsets,
with the linked owned range and the full ROM byte-identical.

The target owns 12 records. Pairs are `D_800D21B0` at `+0x08/+0x0C`,
`D_800D21A8` at `+0x44/+0x48`, first `D_800D20A8` at `+0x50/+0x5C`, second
`D_800D20A8` at `+0x60/+0x6C`, and `D_800D1CA8` at `+0x64/+0x68`; calls are
`ReleaseUnusedLinkSlots` at `+0x3C` and `func_8002B8A8` at `+0x8C`. Historical
prose reports the same identities, with eleven records four bytes early and the
first `D_800D20A8` LO16 twelve bytes early; conflicting old-source claims count
ten or eleven aligned residual rows, and neither has an attributable report.
ORT 593 exports resident offset `0x2B35C` but has no
runtime-table or overlay inbound; `func_80026FB4+0x5F8` is the sole direct
caller. The friendly `ReleaseUnusedLinkSlots` name remains tier-D/invented.

The current C is policy-clean and uses ordinary `-O2 -mips2 -32`. JFG
`mmFreeTick` and DKR `mempool_free_queue_clear` are structural relatives with
different size/control flow, not exact donors. Retain fresh V0, all 119 flags,
and one allocator trace; try JFG-faithful lexical layout and distinct scoped
early-base/later-delay-cursor lifetimes, combining only independent gains.
Hard cap 122 deterministic builds plus one trace; no generic batch absent a
policy-clean natural gain.

`func_8002BB40` owns VRAM `0x8002BB40..0x8002BC60`, ROM
`0x2C740..0x2C860`: 288 bytes/72 words, frameless, with no padding before
`align16`. Canonical `-O2 -mips2 -32` C reproduces all 72 instruction words
and the linked owned range; there is no target padding to inflate the credit.

All eight tuples are exact in canonical C: `D_8007A270` at
`+0x08/+0x64`, `D_800D21B0` at `+0x14/+0x18`, and two `D_800D1C60` pairs at
`+0x28/+0x2C` and `+0x40/+0x44`; count, offset, type, and symbol identity agree
with the assembled target. The serialized ELF row order differs for the second
`D_800D1C60` pair, but relocation order is not part of link identity and the
linked bytes are exact. There is no resident runtime record, ORT export,
overlay inbound, or stored pointer. Direct callers are
`func_8002B3A8+0xE0` and `func_8002B524+0x134/+0x160/+0x180`.

The former 42/72 body first becomes exact through three source-authentic
mechanisms: dead `slotIsTaken` carries the slot count, array-equivalent 20-byte
indexing preserves one temporary chain, and dead `slotSize` carries the
remainder link. A fidelity-gated allocator capture identified that final web;
no forced-color output enters the canonical object. JFG's genuine
`mempool_slot_assign` and DKR's related allocator remain structural donors with
different sizes and control flow, not exact Mickey donors.

Matched C: `align16` is exact for all `0x1C` bytes and has no relocations.
The canonical `-O2 -mips2 -32` flags reproduce the target; JFG's
`mmAlign16` body is the adapted donor.
`align8` is likewise exact for `0x1C` relocation-free bytes with canonical
flags; it is the Mickey-only member derived from the same alignment family.
`align4` completes the family with seven exact instruction words and no
relocations. Its compiled body is `0x1C` bytes; the flag sweep's only reported
delta is the separate 12-byte TU alignment tail already excluded above.
`mmExtended` is exact for `0xC` bytes with the canonical flags. Its two data
relocations retain the target HI16/LO16 offsets and bind `D_8007A274`; the JFG
body and `mmInit` flag role support the tier B name.
`mmSetDelay` is exact for `0xC` bytes under the same flags. Its target-matching
HI16/LO16 pair binds the deferred-free state at `D_800D21AC`.
`mmGetDelay` is the exact `0xC`-byte getter for that same state, with the
target HI16/LO16 relocation pair and canonical flags.
`mmGetSlotPtr` is exact for `0x14` bytes; its HI16/LO16 pair binds the pool
slot-pointer anchor at `D_800D1C64`. The 0x10-byte stride and neighboring
allocator accesses establish Mickey's 16-bit counts at `+0/+2`, slot pointer
at `+4`, size at `+8`, and free-size field at `+0xC`; these differ from JFG's
starting declaration and are reflected in `include/game/memory.h`.
`mmFree` is exact for all `0x44` bytes with canonical flags. Its branch and
two call relocations reproduce the target's immediate-free/deferred-free
selection, using the body adapted from JFG `src/memory.c`.
`func_8002B93C` is exact for all `0x3C` bytes with canonical flags; its queue
address, delay, and count accesses reproduce the JFG `mempool_free_queue`
role without the donor's diagnostic overflow branch.
`func_8002B978` is exact for all `0x58` bytes with canonical flags. Its reverse
pool-table scan is adapted from JFG `mempool_get_pool` and preserves Mickey's
pool count and 16-byte record layout.
`func_8002B700` is exact for all `0x68` bytes with canonical flags. The JFG
`mmFlushFreeStack` loop reproduces Mickey's LIFO queue drain and its call
relocation to the immediate-free worker.
`func_8002B8A8` is exact for all `0x94` bytes with canonical flags. The JFG
`mempool_free_addr` search matches after expressing Mickey's 20-byte slot
stride explicitly and retaining the linked list index at its 16-bit width.
`func_8002B9D0` is exact for all `0x150` bytes with canonical flags. Its JFG
coalescing body matches Mickey after preserving direct pool-table expressions
and natural 20-byte indexing for the allocator's recycled-slot tail.
`func_8002B4C0` is exact for all `0x64` bytes with canonical flags. The JFG
`mmAllocR` reverse pool search and zero colour tag reproduce Mickey's target
and its call relocation to the shared slot finder.
`func_8002B3A8` is exact for all `0x118` bytes with canonical flags. Its JFG
best-fit search matches with Mickey's 16-bit traversal index, retained stack
pad, and natural 20-byte slot indexing at the selected-address return.
`func_8002B1A0` is exact for all `0xE0` bytes with canonical flags. JFG's
pool initializer reproduces the pool/slot setup after applying Mickey's
byte-sized slot flags and colour index and retaining the repeated pool-table
expressions that determine IDO's schedule.
`func_8002B280` is exact for all `0x94` bytes with canonical flags. Its JFG
allocation wrapper matches after retaining Mickey's caller-colour global and
expressing the address/module scratch area as a padded stack record.
`func_8002B314` is exact for all `0x94` bytes with canonical flags. It is the
instruction-identical duplicate of the preceding JFG allocation wrapper and
uses the same padded stack-record spelling.
`func_8002B154` is exact for all `0x4C` bytes with canonical flags. JFG's
region-allocation size calculation and allocator/initializer call sequence
reproduce Mickey's target and both call relocations.
`mmInit` is exact for all `0x78` bytes with canonical flags. The JFG donor's
extended-RAM choice, main-pool construction, deferred-free delay, and queue
reset reproduce all 30 words and the linked global/call relocations.

`func_8002B524`: the retained configured `NON_MATCHING` full-TU object and
isolated candidate are byte-identical and represent the current function body.
They own 116 words with the exact `0x58` frame, all 12 target relocation
tuples, and 102 exact raw and relocation-normalized words. The fourteen
differences are
`+0xE0,+0xE8,+0xEC,+0xF0,+0xF4,+0xF8,+0x104,+0x110,+0x114,+0x118,+0x138,
+0x13C,+0x150,+0x1A4`: ten slot-record/data-carrier sites, two lower-bound
branch/delay-slot sites, and the call-live record home at candidate `sp+0x38`
versus target `sp+0x3C`. The ordinary object, linked owned range, complete
memory TU, and exact ROM contain `GLOBAL_ASM`; no linked C candidate survives.
Use the bounded five-build cache/guard/combination/spill ladder and park if
flat.

`func_8002BB40` is exact canonical C for all 72 frameless words. Its eight
HI16/LO16 tuples bind `D_8007A270`, `D_800D21B0`, and two references to
`D_800D1C60` at the target offsets. The linked owned resident range and full
ROM are byte-identical; this is no longer assembly-fallback evidence.

`func_8002B7AC` closed on the size question, not on allocation. The candidate
was one instruction short because a single source reference to `D_800D21B0`
lets IDO fold the `%lo` into the load, where the target materializes the
address into a callee-saved register and loads through it -- the two-reference
signature. JFG's `mmFreeTick` has a second low-memory tier that re-reads the
same global under a `0xC000` guard and calls a module Mickey never links, so
the guarded block is empty here and the read is the only thing it contributes.
With the size right, the last two words were the loop-guard delay slot: taking
the counter reset out of the `for` header onto its own line puts it ahead of
the preheader's hoisted `D_800D20A8` address instead of into the delay slot.
The empty guard is recorded in `docs/cleanup-queue.md`.

The `models` block is now the deliberate exception to that earlier scheduling
rule: it has been split as a **working decompilation TU**, not promoted to a
tier-A original-file-boundary claim. The evidence and the distinction are
recorded below.

### 3.7 `main/models` working split

ROM `0x20020`-`0x21DA0`, VRAM `0x8001F420`-`0x800211A0`, 19 functions.
The start is an existing 16-byte-aligned yaml boundary and the first function
is an exact, ROM-wide-unique JFG `src/models.c.o` skeleton match. Two later
functions in the same block are exact matches to that object too. The
intervening call graph stays within model allocation, texture ownership and
matrix generation, and the next boundary is the independently tier-A
`mainproc`/`thread1_main` anchor. That supports a practical source split, but
not the stronger statement that every byte came from one original object.

JFG names below are **correspondences, not adopted Mickey symbols** unless a
later matched-C row says otherwise. Tier A means exact masked-skeleton
identity; tier B means the external calls and role agree; tier D means only
function order and local structure agree. There are no distinctive string
references in this block, so it has no tier-C rows. Reference placeholders are
never imported as names, and uncertain rows retain Mickey's `func_` spelling.

| ROM | Mickey symbol / size | JFG correspondence | Tier and evidence |
|---|---|---|---|
| `0x20020` | `func_8001F420`, `0x3C` | JFG placeholder in `models.c.o` | A: exact 15-word skeleton and linked C match; placeholder retained |
| `0x2005C` | `modInitModels`, `0xC4` | `modInitModels` | B: same allocation/table-initialisation calls and TU position; linked C match |
| `0x20120` | `func_8001F520`, `0x644` | `modLoadModel` | B: same cache, decompression, texture and instance-helper call graph |
| `0x20764` | `func_8001FB64`, `0x68` | JFG placeholder in `models.c.o` | A: exact 26-word skeleton and linked C match; placeholder retained |
| `0x207CC` | `func_8001FBCC`, `0x84` | JFG placeholder helper | D: function order and allocation/copy structure; linked C match |
| `0x20850` | `func_8001FC50`, `0x534` | JFG placeholder helper | D: function order and model-instance construction; non-matching C plateau |
| `0x20D84` | `modFreeModel`, `0xF4` | `modFreeModel` | B: instance free followed by model-reference/resource release; linked C match |
| `0x20E78` | `func_80020278`, `0x168` | JFG placeholder resource-free helper | B: texture free plus the same family of owned allocations; linked C match |
| `0x20FE0` | `func_800203E0`, `0xD8` | no adoptable name | D: model helper calls only; linked C match, placeholder retained |
| `0x210B8` | `func_800204B8`, `0xAC` | no adoptable name | D: texture/allocation release structure only; linked C exact |
| `0x21164` | `modelSetModelFlags`, `0xC` | `modelSetModelFlags` | B: paired global setter and observed callers; linked C match |
| `0x21170` | `modelGetModelFlags`, `0xC` | `modelGetModelFlags` | B: paired global getter; linked C match |
| `0x2117C` | `func_8002057C`, `0x558` | `makeModelGfx` | B: texture/display-list construction call graph and TU order; non-matching C plateau |
| `0x216D4` | `func_80020AD4`, `0x3C` | JFG placeholder in `models.c.o` | A: exact 15-word skeleton and linked C match; placeholder retained |
| `0x21710` | `func_80020B10`, `0x27C` | JFG placeholder helper | D: adjacent table-builder structure; non-matching C plateau |
| `0x2198C` | `func_80020D8C`, `0xC0` | `modSetTextureFrame` | B: model texture-frame traversal and matching TU position |
| `0x21A4C` | `func_80020E4C`, `0x1C4` | `modSuspendModelTextures` | B: allocate/save/free texture ownership sequence |
| `0x21C10` | `modResumeModelTextures`, `0x8C` | `modResumeModelTextures` | B: reload/free saved texture ownership sequence; linked C match |
| `0x21C9C` | `func_8002109C`, `0xF8` + `0xC` alignment | no adoptable name | D: model point/matrix traversal; linked C exact, JFG candidates diverge |

**PROVENANCE.** JFG's public `src/models.c`, its built `src/models.c.o`, its
`asm/nonmatchings/models/` filenames, and its published symbol map supplied
the correspondence vocabulary above. The three tier-A rows are measurements
against Mickey's ROM; every other row is explicitly an argument. No JFG body
is present in the initial all-`GLOBAL_ASM` split.
`func_800204B8` is a Mickey-only exact reconstruction for all `0xAC` bytes
under canonical `-O2 -mips2 -32`. Directly reloading the model's byte-sized
texture count reproduces the target register allocation; its texture releases,
two allocation frees, call relocations, and nulling stores are linked exact.
`func_8002109C` is exact for all `0xF8` executable bytes under the same flags;
the following `0xC` bytes are TU alignment, not function text. Its typed loop
uses a four-byte point-index record to select ten-byte signed-coordinate
records, transforms each point into a three-float output, and preserves both
call relocation identities. JFG's neighboring model helpers remain assembly,
so the body and tier-D role are reconstructed from Mickey alone.
`func_80020B10` p7 verdict: structure-mismatch, 160/159 instructions, first `+0x0`, frame `0x20` versus `0x10`.
Tried constant audit/context lint, scoped-cache/loop-local, direct-array, register-hint, and Gfx command forms; the six-save web keeps `GLOBAL_ASM` canonical.
`func_8001FC50` plateaus after ten coherent allocation-layout, stack-home,
zeroing-loop, and copy-loop spellings plus a bounded ten-minute permutation.
The best canonical candidate is 330 instructions against 333, has 300
differing positional words, and first differs at `+0x0`: its frame is `0x88`
instead of `0x78`. A function-local `-Wo,-loopunroll,2` diagnostic was also
non-exact and cannot establish a TU-wide override for the already-proven
canonical consumers.
`func_8002057C` plateaus after the complete 119-combination flag lattice,
ten coherent command-emission, measured-type, copy-loop, and lifetime
spellings, and a bounded permutation. Its best canonical candidate has the
target's exact 342-instruction size but 257 positional words differ from
`+0x0`; its frame is `0xC8` rather than `0xD0`, with a different saved-register
and stack-home allocation. The permuter's lower-scoring candidate reused the
last texture parameter as a command-word temporary and would corrupt the next
part's cache comparison, so it was rejected.
`func_80020D8C` owns ROM `0x2198C..0x21A4C`, 48 words with no padding before
`func_80020E4C`. Policy-clean configured full-TU C has its exact 48-word body,
frame `0x8`, and zero relocations, with 33/48 raw and normalized words matching
and first mismatch `+0x38`. All fifteen residuals are register fields.
`remainingCopy` now carries the frame load so `lh` uses t1; the `outputValue`
phantom is gone. ORT 374 authenticates
resident `func_8001BB10+0x60`, Overlay 57
`overlay57ApplyValue+0x50`, Overlay 60
`func_overlay_060_F0000334_18BA10C+0x1F34`, and six sites in `overlay82Update`,
for nine calls across four functions. The resident caller passes owner/context
in `a3`, which the callee overwrites without consuming. Linked function, TU,
and ROM equality proves fallback only; the guarded C remains the bounded
plateau.
The September 6 four-case carrier matrix reproduces the old retained
12-word residual without a new gain. Only combined product staging and later
constant-index reuse changes initial register reservation. Product-only keeps
baseline initial membership and coloring but changes downstream allocation;
constant-only is byte-identical to baseline. Meaningful-section, symbol and
relocation fidelity passes for every stock/trace-off/trace-on control; no target
compiler trace or complete dynamic FIFO proof exists. The per-symbol
`docs/matching-triage-handoffs/func_80020D8C.md` records the source-bound
evidence and stop condition. The canonical body remains unchanged.
**Why most rows have no new `mickey.us.yaml` split.** §1's "measured file
boundary" tier requires a whole-`.text` match; this pass only matched
**Why the original scan added no `mickey.us.yaml` splits.** §1's "measured
**Why the original skeleton table did not itself produce a split.** §1's "measured
| `src/saves.c.o`, `src/rcpFast3d.c.o`, `src/track.c.o`, `src/textures.c.o`, `src/diCpu.c.o`, `src/objects.c.o`, `libultra/src/flash/flashreadid.c.o`, `us.v10/src/core1/code_1D00.c.o` (BK) | 1 each | single points | Isolated identifications at the time of this scan; §3.4 subsequently measures the complete `diCpu` span |

**Why no new `mickey.us.yaml` split accompanies this table.** §1's "measured
file boundary" tier requires a whole-`.text` match; this pass only matched
individual functions (`tools/find_known_objects.py --sections` found no
whole-object match for any of the not-yet-named TUs above). Asserting a yaml
`asm`/`c` split from function-level hits alone would claim more than was
measured, exactly the mistake 1.2's uniqueness clause exists to prevent one
level up. `gameVi` is the exception added later: §3.8 supplies independent
tier-B boundary evidence from its complete ordered function and call surface.
The already-measured TUs above (`n_csplayer`, `gsSnd`, `n_drvrNew`, `n_env`,
`n_load`, `math_util`) needed no new split; they already have one.

### 3.8 `gameVi`: ROM `0x34180`–`0x34E60`

This is the resident video-interface and framebuffer translation unit,
`src/main/gameVi.c`. **PROVENANCE:** the TU and function names used as matching
candidates come from Jet Force Gemini's public decompilation,
`src/gameVi.c`/`src/gameVi.h`; Mickey's ROM decides every boundary, body and
verdict.

The boundary is tier B rather than a whole-object tier-A claim. Mickey has the
same complete ordered sequence of 23 functions as JFG, from the video
initialiser through the byte-copy leaf. Their call and global-access roles
agree: the first routine creates the video message queue and scheduler client,
the mode routine owns buffer allocation and timing, and the last routine is
`fb_memcpy`. The preceding range is the independently measured
`trapDanglingJump` TU, while ROM `0x34E60` starts the `texInitTextures`-shaped
function and the following `textures.c` sequence. Both ends are 16-byte
aligned.

Four landmarks inside the TU are independently tier A in
`symbol_addrs.us.txt`: `viSetWideAdjust`, `viDisplayingScreen0`, the
placeholder-retaining `func_80034018`, and `fb_memcpy`. Unmatched functions
retain their Mickey `func_` labels as §1.5 requires; a JFG counterpart is not
promoted merely because it occupies the same position in the sequence.

`fb_memcpy` is now canonical C, adapted from JFG with a point-of-use
PROVENANCE note. IDO 5.3 under the resident `-O2 -mips2 -32` flags emits all
12 instruction words exactly, with no relocations; the linked range and full
ROM are byte-identical.

`viDisplayingScreen0` is also canonical C under the same flags and provenance:
all 11 instruction words and the four HI16/LO16 relocation records to Mickey's
framebuffer globals are exact. Its linked range and the full ROM are
byte-identical.

`viSetWideAdjust` is canonical C as well. The adapted clamp/store/timing-call
body emits all 16 instruction words and its HI16, LO16 and call relocations
exactly under the resident flags.

`func_800336A8` is canonical C at tier A (Mickey byte identity), retaining its
address label. The owned range is ROM `0x342A8`–`0x345B4`, VRAM
`0x800336A8`–`0x800339B4`: 780 executable bytes, no padding before
`func_800339B4`, and a `0x28` frame. The configured full TU emits all 195 words
and all 77 static relocation records exactly, including offset, type and
identity; the linked owned range and full ROM are byte-identical.

**PROVENANCE:** the mode body follows Jet Force Gemini's public
`src/gameVi.c:viChangeMode` at `efd5abb`. Mickey's extra PAL subcase,
allocation constants and final state writes remain ROM-derived. The reopened
baseline had 71 differing words and 75 candidate relocations against 77.
Removing its cached triple-buffer local alone regressed to 110 differing words
and 196 words, despite restoring the relocation count. Adding the donor's
empty `if (1) {}` immediately before the non-widescreen framebuffer assignment
then made the entire function exact. The empty statement evaluates only a
constant and has no memory access or side effect; its spelling is disclosed
in source and recorded in the cleanup queue. The triple-buffer flag is read
again after the conditional allocation, as Mickey's relocation surface requires.
No flag or post-compile instruction change was used.

The placeholder-retaining `func_80034018` is canonical C. JFG's public decomp
provides the framebuffer-fill body but not a descriptive function name, so
§1.5 keeps Mickey's address label. All 31 instruction words and its two
global-address plus cache-flush-call relocations are exact.

`viGetVideoMode` is adopted at tier B after its five-word accessor body became
canonical C. Eleven same-address Mickey callers use the returned low mode bits
to choose display dimensions or compare them with a requested mode before
calling the mode-change routine; this is the exact role of JFG's same-position
function. Its HI16/LO16 relocation pair and linked bytes are exact.

`viGetWideAdjust` is adopted at tier B with its three-word canonical accessor:
`frontSetWideAdjust` calls the already tier-A setter, immediately reads this
value back, and stores it as the front-end's current setting. The getter's
HI16/LO16 relocation pair and linked bytes are exact. The public declarations
for the matched named surface now live in `include/game/gameVi.h`.

`viSetTrippleBuffer` is adopted at tier B with JFG's original spelling. The
front-end passes a requested resolution mode, then reads the current video mode
and tests whether the buffer configuration changed before calling the mode
changer. The four-word setter and its HI16/LO16 relocation pair are exact.

`viChangeBuffers` is the seven-word predicate used by that same caller. It
compares the active and requested triple-buffer flags, and the caller invokes
the mode changer exactly when it returns true. That pins the JFG name at tier B;
both HI16/LO16 relocation pairs and the linked body are exact.

`viFrameRateReset` is adopted at tier B. The mode changer and two runtime
state-reset paths call it before frame pacing resumes, while the canonical body
resets the skip-adjust flag, delta counter, delta interval and one-frame mode.
All 11 instruction words and four HI16/LO16 relocation pairs are exact.

`viInit` is adopted at tier B. Resident startup passes its scheduler at the
same point where JFG initializes video, immediately before the PI/RCP sequence.
The canonical body is exact at 74 words and all 52 relocation sites.

`func_800339B4` retains JFG's `viReset`-shaped, linked-exact 50-word candidate;
its literal omits framebuffer relocations from `+0x1C`, while extern/array forms
add address formation and disrupt the schedule, so the asm remains canonical.

`viAllocateZBuffer` and `viFreeZBuffer` are adopted at tier B as the paired
allocation lifecycle around mode changes. Their canonical bodies are exact at
22/20 words and 7/9 relocation sites respectively.

`viGetCurrentSize` is adopted at tier B: its callers pass two output pointers
and consume the active display dimensions written through them. Its 18 words
and four relocation sites are exact. `viConvertXY` is likewise pinned by
callers that pass coordinate pairs and immediately consume the scaled values;
all 21 words and four relocation sites are exact.

`viSetTiming` is adopted at tier B. Both the mode changer and the tier-A
wide-adjust setter call this same-position JFG role after changing video state.
The adapted body is canonical C at all 102 words and all 28 relocation sites;
its linked range and full-ROM hash are exact under `-O2 -mips2 -32`.

`viFrameSync` is adopted at tier B. The resident game loop passes its
buffer-swap message, stores the returned update rate, then bounds that rate
before the next update; this pins JFG's same-position role. The adapted body is
canonical C at all 106 words and all 26 relocation sites, with exact symbol
identities, linked bytes and full-ROM hash under the resident flags.

`fb_swap` is adopted at tier B. `viFrameSync` calls it for each non-skip frame,
and the mode changer calls the same routine after rebuilding its buffers. The
canonical body is exact at 56 words and all 18 relocation sites.

`func_80033D58` is canonical C: all seven words and the two scale globals'
HI16/LO16 relocation pairs are exact. JFG calls the equivalent body
`viGetScaleXY`, but only three words are unmasked and no same-address Mickey
caller pins the role, so the public name is recorded only in the source comment
and not adopted.

`func_80033FB8` is canonical C at three words with an exact HI16/LO16 pair.
JFG calls the equivalent accessor `viGetTrippleBuffer`, but no same-address
Mickey caller pins that public name and the body is below the tier-A threshold,
so it remains an address label.

`func_80033FE0` is likewise canonical C at three words with an exact
HI16/LO16 pair. JFG calls the store-only helper `viNoClear`, but no
same-address Mickey caller pins that public name and the body is below the
tier-A threshold, so the address label remains canonical.

| Function | Exact result |
|---|---|
| `func_80036A80` | 48 bytes under `-O2 -mips2 -32`; JFG `src/textures.c::resetColourCycle` body, all 12 instruction words and zero relocations exact. |
| `func_80036C60` | 76 bytes under `-O2 -mips2 -32`; JFG `src/textures.c::resetMixCycle` body, all 19 instruction words and zero relocations exact. |
| `func_80036CAC` | 292 bytes under `-O2 -mips2 -32`; JFG `src/textures.c::updateMixCycle` body, all 73 instruction words and zero relocations exact. |
| `func_80036DD0` | 312 bytes under `-O2 -mips2 -32`; JFG `src/screen.c::screenLoad` body, all 78 instruction words and ten relocation identities exact. |
| `func_80036F08` | 576 executable bytes plus 8-byte target padding under `-O2 -mips2 -32`; JFG `src/screen.c::screenDraw` body with Mickey command data, all 144 instruction words exact. |
| `func_80037150` | 108 bytes under `-O2 -mips2 -32`; buffer-release body, all 27 instruction words and its relocation surface exact. |
| `func_80037658` | 12 bytes under `-O2 -mips2 -32`; direct store-only body, all 3 instruction words and one HI16/LO16 relocation identity exact. |
| `func_80037664` | 104 bytes under `-O2 -mips2 -32`; state predicate body, all 26 instruction words and eight relocation identities exact. |
| `func_800376CC` | 472 bytes under `-O2 -mips2 -32`; fade state machine, all 118 instruction words and its relocation surface exact (matched 2026-09-19: leftover time in the parameter, duplicated overflow clears, L100 identity so the remainder keeps a2). |
| `func_80037A78` | 116 bytes under `-O2 -mips2 -32`; frontend transition body, all 29 instruction words and its relocation surface exact. |
| `func_8003A680` | 48 bytes under `-O2 -mips2 -32`; bounded character-buffer append body, all 12 instruction words and four relocation identities exact. |
| `func_8003A6B0` | 80 bytes under `-O2 -mips2 -32`; character-code mapping body, all 20 instruction words and zero relocations exact. |
| `func_8003A700` | 84 bytes under `-O2 -mips2 -32`; inverse character-code mapping body, all 21 instruction words and zero relocations exact. |

| Candidate | Verdict |
|---|---|
| `func_80036AB0` | 432 bytes under `-O2 -mips2 -32`; colour-cycle interpolation in JFG `updateMixCycle` form with DKR `update_colour_cycle` channel locals, all 108 instruction words exact (frame 0x28, no relocations). The typed `frames[]` index forwards the frame index in a register while re-reading the time at the loop bottom; declaring `table` after the channel locals keeps its web out of the first spill slot. |
| `func_8003A5A0` | Structure-mismatch, 19 differing words, first `+0x0`; target/candidate 56 words, with the lookup/end pointer carrier and relocation web unresolved. |
| `func_8003A754` | 124 bytes under `-O2 -mips2 -32`; record-clearing double loop, all 31 instruction words and its one relocation exact (matched 2026-09-11: copy-initialised inner counter, call result kept live to the exit, offset defined before the pointer copies). |
| `func_8003A7D0` | Structure-mismatch, 36 differing words, first `+0x10`; target 43/candidate 41 words, with count-carrier and first-loop address formation unresolved. |
| `func_800371BC` | Structure-mismatch, 148 differing words, first `+0x28`; target 150/candidate 159 words and 0x38-byte frames, with grid induction and record-store schedule unresolved. |
| `func_80037414` | Structure-mismatch, 59 differing words, first `+0x10`; target 145/candidate 146 words, target frame 0x30 versus candidate 0x40. |
| `func_800378A4` | Structure-mismatch candidate: 103 differing words, first `+0x14`; target 117 instructions/0x68 frame versus candidate 112/0x68, with radial-gradient FP lifetimes and clamp/control-flow shape unresolved. |
| `func_80037AEC` | Allocation-mismatch, 7 differing words, first `+0x54`; target/candidate 66 words and 0x40-byte frames, with only register allocation unresolved. |
| `func_80037BF4` | Matched: the retained NON_MATCHING body was already byte-exact (32/32 words, 0x18 frame, 6 relocations); the recorded 17-word structure-mismatch was a stale verdict from an earlier candidate. |
| `func_80037C74` | Structure-mismatch, 324 differing words, first `+0x0`; target 327/candidate 308 words, with Gfx emission and render-loop shape unresolved. |
| `func_80038190` | Structure-mismatch, 365 differing words, first `+0x0`; target 368/candidate 342 words and 0xE8-byte frames, with Gfx emission, relocation web, and render-loop shape unresolved. |

### 3.9 `main/track`: ROM `0xC950`-`0x16140`

This 0x97F0-byte, 66-function block is one resident translation unit. The
identification is stronger than the isolated `trackSetFogOff` row in §3.3:

- **Tier A:** `trackSetFogOff` at ROM `0x151A0` is byte-identical to JFG's
  function, with 27 unmasked words, two masked words, and one ROM occurrence.
- **Tier B:** callers use the block as one track-rendering, collision-query,
  lighting, and fog API. Internal calls stay within those same clusters; the
  block's first large routine orchestrates its later helpers.
- **Tier C:** the routines at `0x8000BDB4` and `0x8000E920` reference all 14
  resident copies of `"track/track.c"` at `0x80081540`-`0x80081610`.
- **Tier D:** the complete function order follows JFG's built `src/track.c.o`:
  update/draw/sky, texture scrolling, track lights, spatial queries, and fog,
  ending with the corresponding display-list helper. Running
  `tools/skeleton_scan.py similar --target <vram> --top 5` for every Mickey
  function puts a JFG `track.c.o` member first for 39 of the 62 functions large
  enough for the default ten-word index, and in the top five for 40. Exact
  sizes drift, as expected for a different engine revision, but the order does
  not.

The 16-byte-aligned yaml boundaries agree with that sequence: ROM `0xC950`
starts at the function corresponding to JFG's first track routine, and the
last Mickey function ends at `0x16134`, leaving only 12 bytes of compiler
alignment before the next subsegment. No routine in the block uses an odd
single-precision FP register, so none is classified as hand-written assembly
under §6.2.

**PROVENANCE:** the TU name, comparison order, and reference function names
come from Jet Force Gemini's public decomp, `src/track.c` and its built
`src/track.c.o`, a permitted published retail-derived source under
`docs/CLEANROOM.md`. Mickey's ROM supplies the boundaries, call graph, string
references, and matching verdicts; JFG is a starting point, never authority
over a disagreement.

Matched C in this TU:

| Function | ROM | Bytes | Flags | Donor and verdict |
|---|---:|---:|---|---|
| `func_8000BD50` | `0xC950` | 0x64 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `trackUpdateFX` three-module structure at the established tier-D TU position, with Mickey's module IDs and unresolved calls; public name deliberately not adopted; 25/25 instruction words and all six call relocations exact, linked ROM exact |
| `func_8000BDB4` | `0xC9B4` | 0x64C | `-O2 -mips2 -32 -Wab,-r4300_mul` | Tier A byte-identity: JFG `src/track.c:trackDraw` at `efd5abb` supplies the texture loop, display-list macros, camera loop and explicit sky-mask carriers. Mickey's revised branches, fields and call order remain authoritative. All 403 words, the 0x38 frame and 100 relocation offsets/types are exact; 93 identities resolve statically and seven through the unchanged resident runtime table plus exact linked ROM bytes. The owned range ends at ROM `0xD000`, with no padding credit. |
| `func_8000C400` | `0xD000` | 0x140 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG texture-animation loop at the established tier-D TU position, revised to Mickey's segment, batch, texture, and flag layout; donor placeholder deliberately not adopted; 80/80 instruction words and all five relocation records exact, linked ROM exact |
| `func_8000C540` | `0xD140` | 0xA8 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `initSky` body structure at the established tier-D TU position, with Mickey's player-count guard and object layout; public name deliberately not adopted; 42/42 instruction words and all 11 relocation records exact, linked ROM exact |
| `trackSkySet` | `0xD1E8` | 0xC | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `src/track.c` body; tier B role and tier D TU position; 3/3 instruction words and relocation layout exact, linked ROM exact |
| `func_8000C5F4` | `0xD1F4` | 0x684 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG flashy-sky builder at the established tier-D TU position, with DKR's published workbench documenting the donor family's load-bearing expression forms and local padding; Mickey's level-data offsets, display-list bindings, and geometry layouts are authoritative; donor placeholder deliberately not adopted; 417/417 instruction words and all 28 relocation records exact, linked ROM exact |
| `func_8000CC78` | `0xD878` | 0x258 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG background-gradient builder and display-list command forms at the established tier-D TU position, revised to Mickey's ten-byte vertex layout and resident bindings; donor placeholder deliberately not adopted; 150/150 instruction words and all 24 relocation records exact, linked ROM exact |
| `func_8000CED0` | `0xDAD0` | 0x13C | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `func_80013478` supplies tier-D sky-object update structure; Mickey proves the revised mode test, fields, calls, and final draw condition, so the donor placeholder is deliberately not adopted; 79/79 instruction words and all 19 relocation records exact, linked ROM exact |
| `func_8000D00C` | `0xDC0C` | 0xC | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction; JFG's corresponding `trackGetSky` is only tier D and is deliberately not adopted; 3/3 instruction words and relocation layout exact, linked ROM exact |
| `func_8000D018` | `0xDC18` | 0x154 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `func_800135E0` supplies the camera/update skeleton; public name deliberately not adopted. The camera-position dangling call uses a typed `trackCamPosTrap` weak alias, canonicalized back to `TrapDanglingJump` with `objcopy --redefine-sym` in the track.c.o rule, so its three f32 args pass single-precision (no double promotion) while every other `TrapDanglingJump` call site and the canonical relocation identity are preserved; 85/85 instruction words and all 32 relocation records exact, linked ROM exact |
| `func_8000D16C` | `0xDD6C` | 0x4C | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction; JFG's corresponding `trackAddTextureScroll` is tier D only and its public name is deliberately not adopted; 19/19 instruction words and both HI16/LO16 relocation pairs exact, linked ROM exact |
| `func_8000D570` | `0xE170` | 0xBC | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `trackLightFreeMem` supplies tier-D role/TU and control-flow context; Mickey reconstruction retains the placeholder; 47/47 instruction words and all 17 relocation records exact, linked ROM exact |
| `func_8000D62C` | `0xE22C` | 0xFC | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `trackLightAdd` supplies tier-D role/TU and the 0x80-byte pool stride; Mickey's stores establish the typed light record and body, so the public name is deliberately not adopted; 63/63 instruction words and all nine relocation records exact, linked ROM exact |
| `func_8000D728` | `0xE328` | 0x40 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction; JFG's corresponding `trackLightDelete` is tier D only and its public name is deliberately not adopted; 16/16 instruction words and the D_800792FC HI16/LO16 pair exact, linked ROM exact |
| `func_8000D768` | `0xE368` | 0x90 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the colour-ramp loop; JFG's assembly-only `trackLightColour` supplies tier-D role/TU context and its public name is deliberately not adopted; 36/36 instruction words, no relocation records, linked ROM exact |
| `func_8000D7F8` | `0xE3F8` | 0x28 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction; JFG's corresponding `trackLightMove` is tier D only and its public name is deliberately not adopted; 10/10 instruction words, no relocation records, linked ROM exact |
| `func_8000D978` | `0xE578` | 0x1BC | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `trackUpdateLighting` supplies tier-D role/TU and the alternating segment-lighting structure; Mickey proves the revised module path, fields, and calls, so the public name is deliberately not adopted; 111/111 instruction words and all 28 relocation records exact, linked ROM exact |
| `func_8000DDE4` | `0xE9E4` | 0x1D8 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the priority-record filter and adjacent-swap ordering; the reference scan found no credible donor and the placeholder is retained. All 119 flag combinations were nonexact, while one codegen-faithful allocator trace identified the cross-phase carrier; reusing dead `recordIndex` for `passCount` produces 118/118 instruction words, the exact `0x28` frame, and exact R_MIPS_26 records for `runlinkIsModuleLoaded` at `+0x114` and `TrapDanglingJump` at `+0x128`. Runtime record 157 resolves the latter role through ORT 1315 to Overlay 21 `+0x10C`; the owned range and linked ROM are exact. |
| `func_8000F57C` | `0x1017C` | 0x2B0 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the bounded visible-segment distance list and adjacent-swap ordering; JFG's assembly-only `trackGetBlockList` supplies tier-D role/TU context and its public name is deliberately not adopted; 172/172 instruction words and all eight relocation records exact, linked ROM exact |
| `func_8000F82C` | `0x1042C` | 0x200 | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `traverse_segments_bsp_tree` body adapted to Mickey's global camera/result state; JFG independently supplies tier-D TU-position context, but neither donor name is adopted; 128/128 instruction words and all 18 relocation records exact, linked ROM exact |
| `func_8000FA2C` | `0x1062C` | 0xB4 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the camera/BSP range setup wrapper; the reference scan found no credible donor and the placeholder is retained; 45/45 instruction words and all 19 relocation records exact, linked ROM exact |
| `func_8000FBD8` | `0x107D8` | 0xCC | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `check_if_inside_segment` bounding-box containment structure adapted to Mickey's direct coordinates and inclusive bounds; donor name deliberately not adopted; 51/51 instruction words and the `D_800792E8` HI16/LO16 relocation pair exact, linked ROM exact |
| `func_8000FCA4` | `0x108A4` | 0xC4 | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `get_inside_segment_count_xz` body adapted to Mickey's 16-bit output indices and resident bindings; donor name deliberately not adopted; 49/49 instruction words and both `D_800792E8` HI16/LO16 relocation pairs exact, linked ROM exact |
| `func_8000FD68` | `0x10968` | 0x14C | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `get_inside_segment_count_xyz` body adapted to Mickey's resident track and bounding-box types; JFG independently supplies tier-D `trackGetCubeBlockList` context, but the public name is deliberately not adopted; 83/83 instruction words and both `D_800792E8` HI16/LO16 relocation pairs exact, linked ROM exact |
| `func_8000FEB4` | `0x10AB4` | 0x38 | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `block_get` accessor structure with Mickey's stricter upper bound and 0x40-byte segment layout; donor name deliberately not adopted; 14/14 instruction words and the `D_800792E8` HI16/LO16 relocation pair exact, linked ROM exact |
| `func_8000FEEC` | `0x10AEC` | 0x40 | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `block_boundbox` body and 12-byte bounding-box layout; donor name deliberately not adopted; 16/16 instruction words and the `D_800792E8` HI16/LO16 relocation pair exact, linked ROM exact |
| `func_8000FF2C` | `0x10B2C` | 0x24C | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of three transformed plane equations; JFG's same-position assembly-only placeholder supplies tier-D structure/TU context and is deliberately not adopted; 147/147 instruction words and all nine relocation records exact, linked ROM exact |
| `func_80010178` | `0x10D78` | 0x25C | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the visibility gate and three-plane AABB test; the reference scan found no credible donor and the placeholder is retained; 151/151 instruction words and all 11 text relocation records exact, linked ROM exact |
| `func_800131AC` | `0x13DAC` | 0x178 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `trackClip3D` supplies the tier-D six-plane clipping structure and paired helper context; Mickey proves the shorter boundary and exact body, so the public name is deliberately not adopted; 94/94 instruction words and all ten relocation records exact, linked ROM exact |
| `func_80013324` | `0x13F24` | 0xD8 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the interval-clipping helper; the reference scan found no credible donor and the placeholder is retained; 54/54 instruction words and all four relocation records exact, linked ROM exact |
| `trackGetTrack` | `0x14AB4` | 0xC | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction with JFG name (tier B callers); 3/3 instruction words and relocation layout exact, linked ROM exact |
| `func_80013EC0` | `0x14AC0` | 0x20C | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG's assembly-only `trackFreeAll` supplies tier-D teardown structure/TU context; Mickey proves the resident calls, fields, and source spelling, so the public name is deliberately not adopted; 131/131 instruction words and all 56 relocation records exact, linked ROM exact |
| `trackSetFog` | `0x15030` | 0xF8 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `src/track.c` body with tier B callers and tier D TU order; 62/62 instruction words and relocation layout exact, linked ROM exact |
| `trackGetFog` | `0x15128` | 0x78 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG direct-path body with tier B caller and tier D TU order; 30/30 instruction words and relocation layout exact, linked ROM exact |
| `trackSetFogOff` | `0x151A0` | 0x74 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `src/track.c`; 29/29 instruction words and relocation layout exact, linked ROM exact |
| `func_80014614` | `0x15214` | 0x190 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction of the fog-state updater; JFG same-position skeleton is the 0.733 top hit but its placeholder is not imported; 100/100 instruction words and relocation layout exact, linked ROM exact |
| `func_800147A4` | `0x153A4` | 0x13C | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction using the SDK fog-colour/position macros; JFG same-size top skeleton supplies structural context but its placeholder is not imported; 79/79 instruction words and relocation layout exact, linked ROM exact |
| `func_800148E0` | `0x154E0` | 0x2CC | `-O2 -mips2 -32 -Wab,-r4300_mul` | DKR `obj_loop_fogchanger` body and declaration order adapted to Mickey's direct player-list call, 0x54 fallback stride, object offsets, and 0x40 fog records; JFG independently supplies tier-D `trackChangeFog` TU context, but the public name is deliberately not adopted; 179/179 instruction words and all three text relocation records exact, linked ROM exact |
| `func_80014BAC` | `0x157AC` | 0x238 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `trackFadeFog` body at the established tier-D TU position; its public name is deliberately not adopted; 142/142 instruction words and both HI16/LO16 relocation pairs exact, linked ROM exact |
| `func_80014DE4` | `0x159E4` | 0xC8 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction; JFG supplies only tier-D transform-role context and no public name is adopted; 50/50 instruction words and relocation layout exact, linked ROM exact |
| `func_80014EAC` | `0x15AAC` | 0x20 | `-O2 -mips2 -32 -Wab,-r4300_mul` | JFG `func_8001C550` is a tier-A 8/8-word TU donor, unique in the ROM; JFG placeholder not imported; linked ROM exact |
| `func_80014ECC` | `0x15ACC` | 0x668 | `-O2 -mips2 -32 -Wab,-r4300_mul` | Mickey reconstruction using the SDK GBI display-list macros; JFG's assembly-only final `track.c.o` helper supplies tier-D TU-position and structural context, but its placeholder is deliberately not adopted; 410/410 instruction words and all 22 relocation records exact, linked ROM exact |

The 2026-09-08 `func_8000BDB4` donor packet re-derived a configured full-TU
baseline of 402 versus 403 words, frame `0x38`, 301 raw and masked differences,
first `+0x48`, and 100 relocations with 51 stable identities aligned. Three
source attempts produced successively 151, 1 and 0 differing words. JFG's
texture loop and lexical display-list macro temporaries recovered exact
geometry; reusing its explicit width/height mask carrier removed the remaining
register differences; its unsigned scroll-enable flag agreed with Mickey's
byte load. Mickey keeps its own early overlay dispatch, culling branches,
camera-count thresholds and update-rate choices. The flag sweep initially
failed closed on the missing sized owner and compiled no configurations;
no flag change or permuter was used. The final sized symbol row, unguarded C,
`tools/promotion_proof.py` receipt, extracted linked range and `gmake verify`
prove 1,612 new executable bytes. Seven generic trap calls retain distinct
retail runtime identities: Overlay 66 `+0x34` and `+0x40`, Overlay 71 `+0x7A8`,
Overlay 100 `+0x318` and `+0x50C`, and Overlay 40 `+0x2E4` and `+0x534`.
Source snapshots, objects and comparison receipts for all attempts remain in
ignored lane evidence; the retired plateau is preserved in Git history.

Current matching plateau:

| Function | Target | Best attempt | First mismatch and blocker |
|---|---:|---|---|
| `func_80010B4C` | 0xA98 / 678 words | The guarded definition/fallback, its 678 contiguous generated fallback rows, the linked ELF and `src/main/track.c.o` FUNC sizes, linker-map ownership, and adjacent `func_800115E4` boundary independently authenticate VRAM `0x80010B4C..0x800115E4` / ROM `0x1174C..0x121E4` under `-O2 -mips2 -32 -Wab,-r4300_mul`. The TU's sole postprocess is a metadata-only symbol rename unrelated to this range. Configured C has exact 678-word geometry, frame `0x158` versus target `0x148`, 668 differing words, and 9/9 relocation count. The tier-D metadata adds no match credit. | `+0x0`: the recovered retry/failure CFG still needs original declaration/lifetime evidence for the 16-byte frame excess and FP web. The now-bounded skeleton scan ranks JFG's assembly-only `trackGetPlayerIntersect` first at 0.086 Jaccard, structural corroboration only; four-project coddog returns no candidate line and only its known overlay-end warnings. Preserve the fallback pending new lifetime evidence. |
| `func_80012574` | 0xE4 / 57 words | JFG's assembly-only `trackSphereIntersect` corroborates the role/structure only. Historical pre-cleanup configured full-TU C was measured at **50/57 raw and normalized words**, first `+0x50`, frame `0x48`, with `sqrtf` at `+0xA4`; no candidate object/hash survives. Its two dead stack-home assignments were diagnostic and are removed, so clean V0's score/shape/tuple are unknown. Five retail calls come from `func_80011CDC` (two) and `func_800563B4` (three); ORT 308 has no runtime/overlay/pointer inbound. | Compile clean V0; use one historical control only on structural regression. Retain 119 configurations including V0, one trace, at most two trace-supported natural forms, and an improving-only combination. Cap 122 deterministic builds plus trace, 123 only for the control; permit one 20-minute/2,000-candidate stack-aware batch only after a legal gain. |
| `func_80011CDC` | 0x558 / 342 words | The guarded definition/fallback, its 342 contiguous generated fallback rows, the linked ELF's `0x558` FUNC size, the linker map's `src/main/track.c.o` owner, and adjacent `func_80012234` boundary independently authenticate VRAM `0x80011CDC..0x80012234` / ROM `0x128DC..0x12E34` under `-O2 -mips2 -32 -Wab,-r4300_mul`. Configured C remains 344 words with frame `0xD0`, 15/342 positional words, and 11 relocations versus 15 target records. The metadata adds no match credit. | `+0x0`: plane-difference locals recover the target saved-register shape, but texture-global hoisting still spills the loop counter. `skeleton_scan.py similar` found no credible donor: the top result is an unrelated Conker placeholder at 0.0803 Jaccard, while JFG `hitGetLedgeCrossed` is only rank 10 at 0.0549 and is structural context. Four-project coddog returned no candidate line for the exact slice while reporting its known out-of-bounds overlay-end warnings. Preserve the fallback; resume with new declaration/lifetime evidence, not donor adoption. |
| `func_8001291C` | 0x890 / 548 words | The guarded definition/fallback, its 548 contiguous generated fallback rows, the linked ELF and `src/main/track.c.o` FUNC sizes, linker-map ownership, and adjacent `func_800131AC` boundary independently authenticate VRAM `0x8001291C..0x800131AC` / ROM `0x1351C..0x13DAC` under `-O2 -mips2 -32 -Wab,-r4300_mul`. The TU's sole postprocess is a metadata-only symbol rename unrelated to this range. Configured C is 543 words with frame `0x2B0` versus target `0x288`, 527 differing words, and 15 relocations versus 13 target records. The tier-D metadata adds no match credit. | `+0x0`: recovered insertion, hit-cap, early-out, and distance CFG still need declaration/lifetime layout for the 40-byte frame excess and one global pair. The now-bounded skeleton scan ranks JFG's assembly-only `trackNearestIntersection` first at 0.140 Jaccard, structural corroboration only; four-project coddog returns no candidate line and only its known overlay-end warnings. Preserve the fallback pending new lifetime evidence. |
| `func_8000FAE0` | 0xF8 / 62 words | Type pass confirms `TrackBoundingBox` as six signed 16-bit fields and leaves the 62-word, 0x10-frame candidate unchanged: 43 positional differences before and after. | Workbench verdict remains `structure-buckets`; first mismatch `+0x1C` colors the segment count into `a0` instead of target `t0`, then diverges in branch scheduling. Preserve `GLOBAL_ASM`; original local/loop declaration evidence remains missing. |
| `func_8000D820` | 0x158 / 86 words | Type pass adds `TrackSegment.lightBatchCount` at `+0x20`, types the source/mask record, and widens `D_800C95B4` to `s32[]`; candidate size moves 112 -> 84 words versus target 86. | Workbench verdict is `structure-buckets`, first divergence at `+0x0`: target is frameless and has a different global/pointer web; residual is 57 register-class differences plus 32 structural. Preserve `GLOBAL_ASM`; original declaration/lifetime evidence is needed. |
| `func_800133FC` | 0x180 / 96 words | Mickey's reconstructed three-point plane helper under `-O2 -mips2 -32 -Wab,-r4300_mul` reaches the exact 96-word length, 0xA0 frame, and `sqrtf` relocation. The required 119-mode flag sweep uniquely selected the R4300 multiply schedule, and a bounded ten-minute permuter plus ten source/lifetime hypotheses reduced the residual to 58 positional words. | `+0x18`: IDO loads the point coordinates in a different order and gives the retained integer coordinates and pre-normalized components different stack homes, cascading through the GPR and FP webs. The reference scan found no credible source donor; another attempt needs original declaration/lifetime evidence rather than more register-order guessing. |
| `func_8001357C` | 0x410 / 260 words | The `NON_MATCHING` body and adjacent `func_8001398C` boundary authenticate a unique `src/main/track.c.o` owner under `-O2 -mips2 -32 -Wab,-r4300_mul`; the retained configured candidate is 321 words with the exact 0x138 frame and 289 differing words. | `+0x8`: an isolated no-unroll diagnostic reaches 261 words and 229 differences, but it is not promotable without function-local flag isolation and full impact proof. The ownership row adds no match credit. |
| `func_8001398C` | 0x528 / 330 words | The guarded definition/fallback, its 330 contiguous generated fallback rows, the linked ELF and `src/main/track.c.o` FUNC sizes, the linker-map owner, and adjacent `trackGetTrack` boundary independently authenticate VRAM `0x8001398C..0x80013EB4` / ROM `0x1458C..0x14AB4` under `-O2 -mips2 -32 -Wab,-r4300_mul`. Configured C has exact geometry and frame `0x140`, with 168/330 positional words and 21/21 relocation count; 19 identities align. The tier-D metadata adds no match credit. | `+0x60`: the remaining mismatch is allocator scheduling. The now-bounded skeleton scan ranks JFG's assembly-only `trackGetHeights` first at 0.194 Jaccard, which corroborates the structural family but supplies no adoptable source; four-project coddog returns no candidate line and only its known overlay-end warnings. Preserve the fallback pending procedure-scoped lifetime evidence. |
| `func_8000DFBC` | 0x630 / 396 words | The `NON_MATCHING` body and adjacent `func_8000E5EC` boundary authenticate a unique `src/main/track.c.o` owner under `-O2 -mips2 -32 -Wab,-r4300_mul`; the configured candidate is 398 words with the exact 0x70 frame, 304 differing words, and the same 51-record relocation count. | `+0x48`: batch/display-list initialization and register scheduling remain structurally different; 36 relocation offset/type sites and 35 stable identities align. The ownership row unlocks the flag lattice but adds no match credit. |
| `func_8000D3B8` | 0x1B8 / 110 words | **Exact.** 110 words, frame `0x38`, all 16 relocations, ROM `0xDFB8..0xE170` byte-identical; `wb_compare --rom` reports `instruction-words-identical` and `gmake verify` still prints the expected hash. | Closed by one expression: the pool size is `D_800792F8 * sizeof(TrackLight)`, read back from the global the line above rather than computed from the `lightCount` parameter. The parameter form lets IDO write the product straight into `a0`; re-reading the global keeps the stored value's carrier live, so the product takes a temporary and is copied into `a0` -- the missing 111th word. That one change took the candidate from 109/110 words and 105 differing to exact, so the `byteCount` local was never the pool size's carrier. |
| `func_80020E4C` | 0x1C4 / 113 words | The `NON_MATCHING` body and adjacent `modResumeModelTextures` boundary authenticate the unique resident range `0x80020E4C..0x80021010` / ROM `0x21A4C..0x21C10` in `src/main/models.c.o` under `-O2 -mips2 -32`; the configured candidate has exact 113-word geometry and a 0x40 frame. | `+0xC`: 25 words differ. Explicit byte-scaled indexing remains best; pointer-cursor and declaration-order probes did not improve it, while exception-loop/pool-slot 1 and temp-slot 3 allocation remain. The ownership row unlocks flag and skeleton queries but adds no match credit. |
| `func_8000D1B8` | 0x200 / 128 words | Mickey's packed-scroll and nested segment/batch/vertex reconstruction under `-O2 -mips2 -32` compiles to 124 instructions with a `-40` frame; workbench reports 121 differing words, 98 aligned structural, 40 register, 1 constant, and seven relocation-site differences. | `+0x04`: the candidate takes a different prologue/global-register path and then differs through the scroll-offset and vertex-update webs; target is 128 instructions, so this remains a structural candidate rather than permuter-ready. |
| `func_80010900` | 0x24C / 147 words | Mickey's reconstructed repeated segment-intersection wrapper under `-O2 -mips2 -32 -Wab,-r4300_mul` identifies the 0x20-byte callback record and reaches the exact 147-word opcode schedule, 0xB8 frame, every stack offset, FP allocation, and all five call relocations. The 119-mode flag sweep found no better mode; a bounded ten-minute permuter and ten type, declaration, lifetime, and call-schedule hypotheses leave 17 register-only words. | `+0x14`: one clean saved-register bijection assigns the direction pointer, intersection pointer, and secondary result to `s4`, `s5`, and `s6` instead of the target's `s5`, `s6`, and `s4`. The reference scan found no credible donor, and explicit pointer/return-category variants reproduced the same allocator basin; another attempt needs original declaration or forced-color evidence rather than more register-order guessing. |
| `func_800103D4` | 0x280 / 160 words | Evidence D: JFG's assembly-only object-alpha role, DKR's visibility helper, and Mickey's m2c draft establish the switch, fade, and plane loop. | Candidate: structure-mismatch, 177/160 instructions, 175 differing words, first `+0x0`, frame `-0x58` versus `-0x38`; switch/FP saved-register shape remains unresolved. |
| `func_80010654` | 0x2AC / 171 words | Evidence D: DKR's collision resolver and Mickey's m2c draft establish the candidate-stream, base-plane, signed-edge, and nearest-intersection algorithm. | Candidate: structure-mismatch, 174/171 instructions, 172 differing words, first `+0x0`, frame `-0x80` versus `-0x98`; three instruction/FP-home residuals remain. |
| `func_8000DB34` | 0x2B0 / 172 words | The `NON_MATCHING` body and adjacent `func_8000DDE4` boundary authenticate a unique `src/main/track.c.o` owner under `-O2 -mips2 -32 -Wab,-r4300_mul`; Mickey's m2c draft establishes the inverse segment-order table, descending object scan, integer AABB test, and eight-byte results. | The integrated candidate remains 267/172 instructions with 267 differing words, first `+0x0`, and frame `0x188` versus `0x190`; the ownership row unlocks the flag lattice but adds no match credit. |
| `func_80012658` | 0x2C4 / 177 words | Evidence D: JFG's collision-edge builder and Mickey's typed reconstruction establish the encoded-node, three-corner, and output-stride logic. | Candidate: structure-mismatch, 174/177 instructions, 157 differing words, first `+0x0`, frame `-0x38` versus `-0x40`; three instruction/frame-padding residuals remain. |
| `func_8000E5EC` | 0x334 / 205 words | The `NON_MATCHING` body and adjacent `func_8000E920` boundary authenticate a unique `src/main/track.c.o` owner under `-O2 -mips2 -32 -Wab,-r4300_mul`; Mickey's reconstruction and DKR's level-geometry renderer establish visible-segment selection, visibility-map setup, particle passes, and dispatch. | The configured candidate is 209 words with a 0xE8 frame versus target 0xD8, 185 differing words, and the same 56-record relocation count. First mismatch is `+0x0`; 14 relocation offset/type sites and 9 stable identities align. The ownership row unlocks the flag lattice but adds no match credit. |
| `func_8000E920` | 0x878 / 542 words | The guarded definition/fallback, its 542 contiguous generated fallback rows, the linked ELF and `src/main/track.c.o` FUNC sizes, the linker-map owner, and adjacent `func_8000F198` boundary independently authenticate VRAM `0x8000E920..0x8000F198` / ROM `0xF520..0xFD98` under `-O2 -mips2 -32 -Wab,-r4300_mul`. The TU's `trackCamPosTrap` postprocess is a metadata-only symbol redefine for `func_8000D018` and does not change this owned range. The tier-D metadata adds no match credit. | The integrated configured plateau has frame `0xF8`, all 114 target relocation records, and 491 differing words from `+0x38`; reverse-pass lifetime scoping remains the next source lever. The bounded skeleton scan's top result is JFG's assembly-only `func_8001424C` placeholder at 0.1623 Jaccard; every other hit is below 0.08. Four-project exact-slice coddog emits no candidate line. No provenance-safe source donor is available; preserve the fallback and closed source plateau. |
| `func_80012234` | 0x340 / 208 words | Evidence D: JFG's cylinder-intersection role and Mickey's m2c draft establish the interval and square-root paths. | Candidate: structure-mismatch, 175/208 instructions, 208 differing words, first `+0x0`, frame `-0xA0` versus `-0x60`; FP lifetime and cross-product schedule remain unresolved. |
| `func_80011980` | 0x35C / 215 words | Evidence D: Mickey's encoded polygon/edge draft and JFG's same-region context establish the ray-query control flow. | Candidate: structure-mismatch, 214/215 instructions, 208 differing words, first `+0x0`, frame `-0xD8` versus `-0xC8`; FP schedule and local lifetime remain unresolved. |
| `func_800140CC` | 0x364 / 217 words | Evidence D: DKR's shadow renderer, JFG's assembly-only helper immediately before `trackSetFog`, and Mickey's display-list draft establish the two-range renderer. | Candidate: structure-mismatch, 217/217 instructions, 187 differing words, first `+0x0`, frame `-0x90` versus `-0xA8`; 3/4 relocation placements align, while frame/local layout remains unresolved. |
| `func_800115E4` | 0x39C / 231 words | Evidence D: Mickey's collision-response draft and JFG's corresponding helper establish the three surface branches and normalization. | Candidate: structure-mismatch, 236/231 instructions, 232 differing words, first `+0x0`, frame `-0xA0` versus `-0x98`; five-instruction FP schedule residual remains. |

### 3.10 Resident camera: ROM `0x21EE0`–`0x25C20`

This whole `0x3D40`-byte block is the resident camera TU: **69 functions,
`0x3D3C` executable bytes and four bytes of terminal alignment**. Its ordered
systems are camera/FOV state, user viewports, projection setup, sprite and
model matrices, projection helpers, then screen shake. The split is
`main/camera`; flags are `-O2 -mips2 -32 -Wab,-r4300_mul`, with the multiply
scheduler mode fixed by the exact `camGetProjZ` projection-depth dot product.

**PROVENANCE.** The TU identity, source order and borrowed names below come
from Jet Force Gemini's public retail-derived decomp, `src/camera.c`, permitted
by `docs/CLEANROOM.md`. JFG is a starting point only; the tiers say which parts
Mickey's own bytes establish.

| Evidence | Result |
|---|---|
| **A — byte identity** | `camSetWaterLine` at ROM `0x225B0` has 6 unmasked words of 8 and `romocc=1`; `camGetPlayerProjMtx` at `0x23360` has 8 unmasked words of 13 and `romocc=1`. Both clear §1.2. |
| **B — role/call graph** | `camInit` opens the block and ranks JFG `camInit` at 0.3125 masked 4-gram Jaccard versus 0.0851 for the runner-up. `camOverrideProjScales`, `camGetProjOrgMtx`, `camStopShakes`, and `camSetZoom` have the same state effects and ordered camera roles as JFG; their comments in `symbol_addrs.us.txt` retain why they are below tier A. |
| **C — strings** | None. The resident strings `"Camera Error: Illegal mode!"` and `"Cam do 2D sprite called with NULL pointer!"` are not addressed by resident code (§7); this identification does not use them. |
| **D — structure** | The existing endpoints are 16-byte aligned, the first function is the `camInit` nearest neighbour, and the last function ends four bytes before `0x25C20`. The 69-function call/data sequence follows one camera-state cluster throughout. |

The call census finds 15 direct internal edges. Representative chains are
the view setup calling the window-limit, projection and viewport routines;
the reset path calling scissor and viewport setup; the sprite helpers sharing
matrix conversion; and the shake updater calling the shake initializer.
External callers span resident render, track, menu and update code, while the
TU calls the matrix library, video helpers, `sqrtf`, and `Arctanf`. There are
no odd single-precision FP registers anywhere in the block, so none of these
functions is classified as handwritten assembly under §6.2.

| Matched C function | ROM | Tier | Exact executable bytes | Proof |
|---|---:|---|---:|---|
| `camInit` | `0x21EE0` | B — JFG role/call graph and nearest camera skeleton | 344 | JFG body adapted to Mickey's six-camera array, reset routine and projection globals; configured object, 25 text relocations, linked range and full ROM exact. |
| `func_80021438` | `0x22038` | D — retained Mickey auto-name; camera TU position only | 12 | Mickey-only global read; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camUseShake` | `0x22084` | B — role/order | 16 | Configured object, relocation pair, linked range and full ROM exact. |
| `camOverrideProjScales` | `0x220E4` | B — role/order (named above) | 32 | Configured object, six relocations, linked range and full ROM exact. |
| `func_800217AC` | `0x223AC` | D — retained Mickey auto-name; camera TU position only | 12 | Mickey-only matrix pointer getter; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camDistance` | `0x223B8` | D — JFG TU role/order and camera-position dataflow | 128 | JFG body adapted to Mickey's active-camera array; configured object, five relocations, linked range and full ROM exact. |
| `camSetWaterLine` | `0x225B0` | A — byte identity (named above) | 32 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetProjOrgMtx` | `0x25270` | B — role/order (named above) | 28 | Configured object, two relocation pairs, linked range and full ROM exact. |
| `camSetZoom` | `0x258C8` | B — role/order (named above) | 56 | Configured object, two relocation pairs, linked range and full ROM exact. |
| `camGetPlayerProjMtx` | `0x23360` | A — byte identity (named above) | 52 | Configured object, five relocations, linked range and full ROM exact. |
| `camStopShakes` | `0x25754` | B — role/order (named above) | 76 | Configured object, three relocation pairs, linked range and full ROM exact. |
| `camStartShake` | `0x256C4` | D — JFG TU role/order and shake-record dataflow | 144 | JFG body adapted to Mickey's six-camera bound; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_80024ED8` | `0x25AD8` | D — retained Mickey auto-name; fixed-distance camera-transform dataflow | 324 | Mickey-only body; 81 executable instructions and nine text relocations are exact. The following four-byte TU alignment NOP is reproduced by compiler section padding; linked range and full ROM exact. |
| `camIgnoreShake` | `0x22094` | B — role/order | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetFOV` | `0x220A0` | B — role/order | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `func_80021444` | `0x22044` | D — retained Mickey auto-name; paired camera-state effect only | 64 | Mickey-only bounded state setter; configured object, two HI16/LO16 relocation pairs, linked range and full ROM exact. |
| `func_800214AC` | `0x220AC` | D — retained Mickey auto-name; active-camera state effect only | 56 | Mickey-only active-camera byte toggle; configured object, two HI16/LO16 relocation pairs, linked range and full ROM exact. |
| `func_80021504` | `0x22104` | D — retained Mickey auto-name; JFG `camSetFOV` role/order | 532 | Matched-C, reproof-only. Retained configured C owns 133 words with frame `0x28` and 43 candidate relocation tuples. Its 90 non-relocation words agree with the linked ELF and the remaining differences are exactly those 43 sites; linked ROM `0x22104..0x22318`, the complete camera TU, and resident `.main` are exact. Three resident direct calls and five overlay relocation records are proven. The exact whole `.bin` predates the object and no independent target relocation object survives. |
| `func_80021718` | `0x22318` | D — retained Mickey auto-name; DKR `cam_reset_fov` projection-reset role | 148 | DKR projection-reset body adapted to Mickey's matrix globals. Retained configured C owns 37 words, frame `0x28`, and 14 candidate relocation tuples; applying those tuples reproduces all 37 linked ELF words, and the post-object ELF range plus complete camera TU are byte-identical to ROM. No direct, object, main-reloc, overlay-reloc, or absolute-pointer caller is proven. ROM-table row 453 exports the address but is not inbound evidence. The exact full `.bin` predates the object and no independent target relocation object survives, so one fresh reproof remains required. |
| `func_80021838` | `0x22438` | D — retained Mickey auto-name; DKR reset role and JFG camera TU position only | 224 | DKR reset body adapted to Mickey's extended camera fields and store order; configured object, six relocations, linked range and full ROM exact. |
| `camGetWaterLine` | `0x225A0` | D — TU order only, no per-symbol callgraph argument recorded | 16 | Configured object, relocation pair, linked range and full ROM exact. |
| `camGetMode` | `0x22518` | D — TU order only, no per-symbol callgraph argument recorded | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `camSetMode` | `0x22524` | D — TU order only, no per-symbol callgraph argument recorded | 64 | Configured object, two relocation pairs, linked range and full ROM exact. |
| `camGetNo` | `0x22564` | D — TU order only, no per-symbol callgraph argument recorded | 12 | Configured object, relocation pair, linked range and full ROM exact. |
| `func_80021970` | `0x22570` | D — retained Mickey auto-name; indexed camera-array role only | 36 | Mickey-only indexed camera-array getter; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camSetNo` | `0x22594` | D — TU order only, no per-symbol callgraph argument recorded | 12 | Configured object, relocation pair, linked range and full ROM exact; Mickey omits JFG's bounds guard. |
| `func_800219D0` | `0x225D0` | D — retained Mickey auto-name; DKR `copy_viewports_to_stack` body and JFG `camUserViewTick` role/order | 416 | Matched-C, reproof-only. Retained pre-comment ordinary and `NON_MATCHING=1` objects agree on 104 words, frame `0x8`, and eight HI16/LO16 records: `D_80079D48` twice, `D_80079C10`, and `D_80079D58`. The object-to-linked differences are exactly those eight words; linked ROM `0x225D0..0x22770`, the complete camera TU, and resident `.main` are exact. The retained whole `.bin` is exact but predates this object. Proven callers are `func_80026FB4+0x530` and `func_8000BDB4+0x390`, both assembly-backed in ordinary builds. Linked storage makes `D_80079D58[20]` resolve to adjacent `D_80079E98[0]`; intent is not independently established. |
| `camEnableUserView` | `0x22770` | D — JFG TU role/order and viewport-flag dataflow | 116 | JFG body adapted to Mickey's viewport array; configured object, two HI16/LO16 relocation pairs, linked range and full ROM exact. |
| `camDisableUserView` | `0x227E4` | D — JFG TU role/order and viewport-flag dataflow | 120 | JFG body adapted to Mickey's viewport array; configured object, two HI16/LO16 relocation pairs, linked range and full ROM exact. |
| `camIsUserView` | `0x2285C` | D — JFG TU role/order | 44 | JFG body adapted to Mickey's viewport-flags symbol; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_80021C88` | `0x22888` | D — retained Mickey auto-name; JFG `camSetUserView` role/order | 364 | DKR `viewport_menu_set` body adapted to Mickey's video-size call and viewport layout; configured object, seven text relocations, linked range and full ROM exact. |
| `camSetUserViewSpecial` | `0x229F4` | D — JFG TU role/order and viewport-field dataflow | 252 | JFG body adapted to Mickey's viewport array; configured object, four relocations, linked range and full ROM exact. |
| `camGetVisibleUserView` | `0x22AF0` | D — JFG TU role/order and viewport-scissor dataflow | 120 | JFG body adapted to Mickey's viewport array; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camGetUserView` | `0x22B68` | D — JFG TU role/order and viewport-field dataflow | 72 | JFG body adapted to Mickey's viewport array; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_80021FB0` | `0x22BB0` | D — retained Mickey auto-name; JFG `camGetWindowLimits` role/order | 568 | JFG body adapted for Mickey's inset margins and split-orientation state; configured object, nine text relocations, linked range and full ROM exact. |
| `func_800221E8` | `0x22DE8` | D — retained Mickey auto-name; JFG `camSetView` role/order | 1,052 | JFG body adapted for Mickey's region flag, half-resolution and zoom state; configured object, 19 text relocations, linked range and full ROM exact. |
| `func_80022604` | `0x23204` | D — retained Mickey auto-name; camera TU position only | 12 | Mickey-only global setter; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camSetScissor` | `0x23210` | D — JFG TU role/order and scissor-command dataflow | 336 | JFG role adapted to Mickey's window-limit helper and scissor encoding; configured object, five text relocations, linked range and full ROM exact. |
| `func_80022794` | `0x23394` | D — retained Mickey auto-name; JFG `camSetProjMtx` role/order | 676 | JFG body adapted for Mickey's extra camera-state FOV check; configured object, 40 text relocations, linked range and full ROM exact. |
| `camOrthoYAspect` | `0x23638` | D — JFG TU role/order | 12 | JFG body and masked skeleton exact; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_80022A44` | `0x23644` | D — retained Mickey auto-name; camera TU position only | 12 | Mickey-only float-state setter; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camStandardOrtho` | `0x23650` | D — JFG TU role/order and orthographic viewport dataflow | 324 | JFG body adapted for Mickey's half-resolution alternate viewport bank; configured object, 18 text relocations, linked range and full ROM exact. |
| `camStandardPersp` | `0x23794` | D — JFG TU role/order and perspective-matrix dataflow | 196 | JFG body adapted to Mickey's camera transform and matrix globals; configured object, 17 relocations, linked range and full ROM exact. |
| `camSetViewport` | `0x23858` | D — JFG TU role/order and viewport dataflow | 200 | JFG body adapted for Mickey's alternate viewport bank and horizontal region flip; configured object, 10 relocations, linked range and full ROM exact. |
| `func_80022D20` | `0x23920` | D — retained Mickey auto-name; JFG `camResetView` role/order | 352 | JFG body adapted to Mickey's viewport flags and region-flip argument; configured object, 12 text relocations, linked range and full ROM exact. |
| `func_80022E80` | `0x23A80` | D — retained Mickey auto-name; camera-relative billboard-offset dataflow | 340 | Mickey-only body; configured object, 23 text relocations, linked range and full ROM exact. |
| `func_80023A08` | `0x24608` | D — retained Mickey auto-name; JFG `camDoSprite` role/order | 708 | JFG body adapted for Mickey's one-shot projection flip and display-list encoding; configured object, 24 text relocations, linked range and full ROM exact. |
| `func_80023CCC` | `0x248CC` | D — retained Mickey auto-name; JFG `camDoSpriteDirect` role/order | 696 | JFG body adapted for Mickey's secondary matrix scale, one-shot projection flip and display-list encoding; configured object, 21 text relocations, linked range and full ROM exact. |
| `func_80023F84` | `0x24B84` | D — retained Mickey auto-name; JFG `camDo2DSprite` role/order | 640 | JFG body adapted to Mickey's 10-byte vertex layout, resident transforms and display-list encoding; configured object, 23 text relocations, linked range and full ROM exact. |
| `camPushFloatModelMtx` | `0x24E04` | D — JFG TU role/order and float-model matrix dataflow | 220 | JFG body adapted to Mickey's matrix globals and display-list encoding; configured object, 14 relocations, linked range and full ROM exact. |
| `camPushMuzzleMtx` | `0x24EE0` | D — JFG TU role/order and muzzle-matrix dataflow | 332 | JFG body adapted to Mickey's matrix globals and display-list encoding; configured object, 16 text relocations, linked range and full ROM exact. |
| `camScaleModelMtx` | `0x2502C` | D — JFG TU role/order and model-scale matrix dataflow | 192 | JFG body adapted to Mickey's matrix globals and display-list encoding; configured object, 15 relocations, linked range and full ROM exact. |
| `camPushModelMtx` | `0x250EC` | D — JFG TU role/order and model-matrix dataflow | 256 | JFG body adapted to Mickey's transform, matrix globals and display-list encoding; configured object, 21 relocations, linked range and full ROM exact. |
| `camRestoreModelMtx` | `0x251EC` | D — JFG TU role/order | 32 | JFG display-list body adapted to Mickey's Gfx layout; configured object, relocation-free linked range and full ROM exact. |
| `camPopModelMtx` | `0x2520C` | D — JFG TU role/order | 32 | JFG display-list body adapted to Mickey's Gfx layout; configured object, relocation-free linked range and full ROM exact. |
| `camGetPtr` | `0x2522C` | B — Mickey/JFG weather call-graph correspondence and active-camera dataflow | 44 | JFG body adapted to Mickey's 0x54-byte Camera stride; configured object, two HI16/LO16 relocation pairs, linked range and full ROM exact. |
| `camGetListPtr` | `0x25258` | D — JFG TU role/order | 12 | JFG body and masked skeleton exact; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camGetInvProjMtx` | `0x25264` | D — JFG TU role/order and Mickey matrix dataflow | 12 | JFG body and masked skeleton exact; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_8002468C` | `0x2528C` | D — retained Mickey auto-name; camera matrix dataflow only | 12 | Mickey standalone perspective-matrix getter; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camGetRotationMtx` | `0x25298` | B — Mickey/JFG weather call-graph correspondence and camera matrix dataflow | 12 | JFG body and masked skeleton exact; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `camGetProjectionMtx` | `0x252A4` | D — JFG role and Mickey final projection-matrix dataflow | 12 | JFG body and masked skeleton exact; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_800246B0` | `0x252B0` | D — retained Mickey auto-name; JFG `camProjectPoint` role/order | 388 | Mickey matrix/viewport reconstruction; configured object, 13 text relocations, linked range and full ROM exact. |
| `func_80024834` | `0x25434` | A — JFG `camReversePoint` source adapted to Mickey's symbols and ABI | 260 | JFG's matched repeated viewport-load spelling reproduces all 65 instructions, the `0x38` frame and nine relocations; linked range and full ROM exact. |
| `camGetProjZ` | `0x25538` | D — JFG TU role/order and projection-depth dataflow | 64 | JFG body adapted to Mickey's rotation matrix; configured object, HI16/LO16 relocation pair, linked range and full ROM exact. |
| `func_80024BA0` | `0x257A0` | D — retained Mickey auto-name; JFG `camScreenShake` role/order | 296 | Mickey distance-based shake reconstruction; configured object, 74 instruction words and 10 text relocations exact. |
| `func_80024D00` | `0x25900` | D — retained Mickey auto-name; JFG `camTick` role/order | 472 | Mickey shake-envelope tick reconstruction for six cameras; configured object, 11 text relocations, linked range and full ROM exact. |

Bounded plateau:

| Function | ROM | Evidence and retained result |
|---|---:|---|
| `func_80024978` | `0x25578` | D — retained Mickey auto-name; JFG `camCopyOrthoMatrix` supplies the role and loop body, with Mickey adding its projection scale. The full flag lattice, eight coherent source/type/indexing variants, bounded permuter batch, and 2026-08-26 separate-scalar/fixed-loop forms leave the best candidate at 84 instructions against 83, with 59 positional words from `+0x5C`; the extra third-coefficient address materialization and extern-array ownership blocker remain. |
| `func_80022FD4` | `0x23BD4` | D — workbench `structure-mismatch`; the best `-Wab,-r4300_mul` candidate has the exact `0xB0` frame, 365/369 instructions and 217 positional differences from `+0x2C`. The 2026-08-26 flag sweep and block-scoped horizontal/transform probes found no improvement; the coordinate-home shift, four-instruction deficit, final Gfx schedule and downstream relocation alignment remain. |
| `func_80023598` | `0x24198` | D — retained Mickey auto-name; camera-TU placement and call to the matched sprite-direct helper. The full flag lattice, ten coherent control-flow/type/lifetime/parameter variants, and 2026-08-26 direct Gfx post-increment form leave the best candidate at 286 instructions against 284 and 275 positional words from `+0x0`; IDO retains `dlist` in `$s1` with a `0xA0` frame while the target homes it with `0x90`. |

level up. The already-measured TUs above (`n_csplayer`, `gsSnd`, `n_drvrNew`,
`n_env`, `n_load`, `math_util`) needed no new split; they already have one.
The later menu census below adds independent boundary evidence rather than
retroactively treating the six hits as a whole-object match.

### 3.11 Resident front-end menu: ROM `0x39350`–`0x3B1A0`

This range is `main/menu`, corresponding to JFG's `src/menu.c`. The identity
uses permitted JFG material and is disclosed here: names, declarations,
function order, and starting bodies are compared against JFG's public
decompilation; Mickey's ROM remains authoritative for every match.

The six exact masked-skeleton anchors in §3.3 are **tier A** evidence for the
TU identity, but not its boundaries. The boundaries are a separate **tier B/D**
argument from the full function census. At ROM `0x39350`, the code begins a
sequence structurally corresponding to JFG's `setLanguage`, `initFront`,
`frontFreeMode`, `frontInitMode`, and `frontSetMode`; the same order continues
through the six tier-A anchors and the settings accessor family. The last menu
routine is the short setter at `0x3B190`, in JFG's
`frontCharSelectSetQuitMode` position. The next function, at aligned ROM
`0x3B1A0`, searches the table associated with the distinctive `"UNKNOWN
TRACK"` string and begins a different subsystem. The preceding aligned
function start at `0x39350` likewise follows texture/screen code whose JFG
ordering is outside `menu.c`. Thus the split claims only `0x39350`–`0x3B1A0`,
not the surrounding yaml block.

The source began as 41 `GLOBAL_ASM` functions. Six already have tier-A names
in `symbol_addrs.us.txt`; other JFG names remain a navigation crosswalk until
an exact body is promoted, so the unresolved symbols keep their `func_` names
per §1.5. Flags are the resident game-code defaults, `-O2 -mips2 -32`. The
`-Wo,-loopunroll,0` override the TU carried until 2026-09-09 was measured
byte-inert for every function in it except `func_80038878`, whose target
unrolls both its clearing loops; it was dropped when that function matched.

`func_80038750` adds **0x128 bytes / 74 words** at ROM `0x39350`. Matched C:
exact object words, jump table, and linked ROM range at `-O2 -mips2 -32
-Wo,-loopunroll,0`. The CDX allocator trace showed `destination` carries the
`piRomLoadSection` `a1`-argument affinity, so the relocation loop re-caches the
table base into it each iteration (base rides `a1`, the element stays a junior
temp on `a0`), and the `-1 ==` spelling keeps the hoisted constant first. The
TU now owns its five-entry language jump table (`.rodata` carve moved to
`0x83334`, trim `0xAC`); the table bytes also corrected the language mapping to
`assetIndex = language + 1` in descending case order, which the old
JFG-adapted reversal got wrong while matching `.text` by coincidence.

`func_80038878` remains tier-D `NON_MATCHING`. Its retained p6 result is
diagnostic: target 85 versus candidate 86 instructions, frame `0x18`, 66/85
positional target-word differences, first `+0x14`. The empty address condition
was introduced only as an allocation lever and is removed; clean V0 is
uncompiled and historical flag/source/permuter exhaustion is scheduling
evidence only. JFG's assembly-backed `initFront` remains a structural lead.
Remaining work is the initial global-address allocation, later loop webs, and
relocation identities.

| Function | Exact result |
|---|---|
| `func_80038878` | 340 bytes under `-O2 -mips2 -32` (default unroller); JFG PR #37 `src/menu.c::initFront` body adapted, all 85 instruction words exact. Needs `D_800D3044[4]` (the play choices) defined in the TU: the four stores share one `lui $at`, which IDO emits only for an owned object; the Makefile weakens the definition so the bss gap's retail label wins. |
| `func_80038E1C` | 1116 bytes under `-O2 -mips2 -32`; JFG PR #37 `src/menu.c::frontUpdate` body adapted, all 279 instruction words exact, with its 76-byte compiler-owned switch table at 0x800827E0 (the TU's `.rodata` carve grew from 0xAC to 0xFC, taking the table and the ROM's one zero pad word from the following data segment). Three facts closed the 219-word plateau: the overlay trap's nonzero result is an early `return 0` statement, not a shared exit; the donor's `case 0: break;` widens the jump table to 19 entries; and `func_80000510` takes one argument, so the `-1` web is free to colour `a1`. |
| `func_800389CC` | 504 bytes under `-O2 -mips2 -32` (unroll flag measured inert); JFG `src/menu.c::frontFreeMode` body, all 126 instruction words exact, with its 76-byte compiler-owned switch table. |
| `func_80038BC4` | 488 bytes under `-O2 -mips2 -32` (unroll flag measured inert); JFG `frontInitMode` role/order comparison and Mickey-derived body, all 122 instruction words exact, with its 76-byte compiler-owned switch table. |

The tier-B `frontSetMode` adds **0x64 bytes / 25 words** at ROM `0x399AC`.
Its exact free/init/reset call sequence, mode-state store, and ordered pairing
with `frontGetMode` establish the JFG role. The name, role, and shared control
flow carry point-of-use `PROVENANCE`; Mickey supplies the exact state surface.
The default flags, three calls plus five data-relocation pairs, object words, and
linked ROM range are exact without post-processing.

The tier-B `frontGetMode` adds **0xC bytes / 3 words** at ROM `0x39A10`.
Its exact byte getter, ordered position between the front-end mode setter and
update routine, and the update's dispatch on the same state establish the JFG
name. The adapted body carries point-of-use `PROVENANCE`; the default flags,
HI16/LO16 data relocations, object words, and linked ROM range are exact
without post-processing.

`func_80038E1C` retains a Mickey-derived `NON_MATCHING` candidate with the
exact **0x45C-byte / 279-word** size, `0x28`-byte frame, case count, and
high-level control flow. It plateaus at **248/279 differing words**, first
`+0x24`: IDO assigns the persistent fade-state address to `a0` rather than the
target's `v1`, then cascades into a different register and switch schedule.
The full 119-combination flag lattice keeps the resident defaults best; JFG
has no C donor body for the nearest front-end routine. Canonical code remains
assembly.

The tier-B `frontDemoMessage` adds **0x108 bytes / 66 words** at ROM
`0x39E78`. Its exact size, 16-tick blink gate, localized-language setup,
shadow/main text pair, and position immediately after `frontUpdate` establish
JFG's same-name role. JFG supplies the name and semantic comparison but keeps
its body in assembly; Mickey supplies the C body and message-record field, as
the point-of-use `PROVENANCE` note records. Spelling the byte timer as a direct
compound assignment recovers the target's `v0` address lifetime and temporary
register ring. The default flags, nine call relocations, three data-relocation
pairs, object words, and linked ROM range are exact without post-processing.

The tier-B `frontDrawRectangles` adds **0x204 bytes / 129 words** at ROM
`0x39F80`. Its ordered rectangle-batcher role, screen clipping, colour-change
batching, display-list state setup, and pairing with `frontDrawRectangle`
establish JFG's same-name role. JFG supplies the name and assembly-level
comparison; Mickey supplies the C body and exact render commands, as the
point-of-use `PROVENANCE` note records. Unsigned screen dimensions prevent
IDO from caching the clamped values, and their declaration order recovers the
target's `0x58`/`0x54` stack homes. The resident default flags, all 129 words,
three call relocations, the display-list data relocation pair, configured
object, linked ROM range, and full ROM are exact without post-processing.

The tier-A `frontDrawRectangle` adds **0x50 bytes / 20 words** at ROM
`0x3A184`. Its 19 unmasked words are byte-identical to JFG's uniquely
identified same-named skeleton; the only masked word is the call to the paired
rectangle-list renderer. The name/order carry point-of-use `PROVENANCE`, while
the body and 12-byte rectangle record are Mickey-derived. Full-width coordinate
parameters narrowed into that record recover the exact target schedule. The
default flags, call relocation, object words, and linked ROM range are exact
without post-processing.

The tier-B `frontPlayerScreenLimits` adds **0xB8 bytes / 46 words** at ROM
`0x3A1D4`. Its player-indexed screen-limit table, two-player split adjustment,
two coordinate-pair conversions, and front-end ordering establish the JFG
role. JFG supplies the name and semantic comparison but keeps its body in
assembly; Mickey supplies this C body and table indexing, as the point-of-use
`PROVENANCE` note records. Explicit shifts preserve `(cameraCount - 1)` against
algebraic reassociation. The default flags, three call relocations, data pair,
object words, and linked ROM range are exact without post-processing.

`func_8003968C`: workbench `structure-mismatch`, **28/37 words**, first `+0x0`;
the menu TU now owns the measured `0x30`-byte repeat-state BSS carve and its
individual labels, while external array aliases keep `func_80039720` exact.

The tier-D `func_80039720` adds **0x320 bytes / 200 words** at ROM `0x3A320`.
Its Mickey-derived body updates four controllers' held, pressed, stick, and
repeat state, then aggregates the enabled controllers. Direct array indexing
recovers IDO's seven induction pointers and exact `0x40`-byte frame; JFG's
nearest menu routine remains assembly, so no donor body or descriptive name is
used. The resident defaults are exact in the full flag lattice. All 53 text
relocations match the target's offsets, kinds, and symbol identities, and the
configured object and linked ROM range are byte-identical without
post-processing.

The tier-B `freeFrontEndList` adds **0x5C bytes / 23 words** at ROM `0x3A640`.
Its exact sentinel-list loop calls the immediately following per-item routine,
matching JFG's ordered `freeFrontEndList`/`freeFrontEndItem` pair. The body is
adapted from DKR's public `menu_assetgroup_free` with point-of-use
`PROVENANCE`; JFG supplies the role and name. The default flags, call
relocation, object words, and linked ROM range are exact without
post-processing. A zero-byte weak alias preserves the anonymous name used by
the overlay caller.

The tier-B `freeFrontEndItem` adds **0xEC bytes / 59 words** at ROM
`0x3A69C`. Its four-way dispatch by the resource table's `0xC000` type bits,
loaded-slot clearing, live-resource decrement, and position immediately after
`freeFrontEndList` establish the JFG role. JFG supplies the name and semantic
comparison but keeps its body in assembly; Mickey supplies the C body, as the
point-of-use `PROVENANCE` note records. Treating the polymorphic resource table
as raw 32-bit handles until each typed release call preserves the target's
`v0` lifetime. The default flags, five call relocations, four data-relocation
pairs, object words, and linked ROM range are exact without post-processing.
A zero-byte weak alias preserves the anonymous name used by the list wrapper.

The parallel tier-B `loadFrontEndList` adds **0x5C bytes / 23 words** at ROM
`0x3A788`. Its exact sentinel-list loop calls the immediately following
per-item load routine, matching JFG's ordered `loadFrontEndList`/
`loadFrontEndItem` pair. The body is adapted from DKR's public
`menu_assetgroup_load` with point-of-use `PROVENANCE`; JFG supplies the role
and name. The default flags, call relocation, object words, and linked ROM
range are exact without post-processing.

The tier-B `loadFrontEndItem` adds **0x16C bytes / 91 words** at ROM
`0x3A7E4`. Its four-way dispatch by the resource table's high bits, texture,
sprite, object, and model call surface, loaded-slot bookkeeping, exact size,
and position after `loadFrontEndList` establish JFG's same-name role. The body
is adapted from DKR's public `src/menu.c::menu_asset_load`; Mickey supplies
the 13-byte spawn packet and the spawned-object fields at `0x40`, `0x22`,
`0x68`, and `0x08`, as the point-of-use `PROVENANCE` note records. A named
inner pointer recovers the target's pooled `v1`, while direct active-array
indexing recovers IDO's `0x1C` compiler spill. The resident default flags, all
91 words, four call relocations, seven data-relocation pairs, configured object,
and linked ROM range are exact without post-processing. A zero-byte weak alias
preserves the anonymous name used by the list wrapper.

The third tier-B sentinel wrapper, `setupFrontEndList`, adds **0x5C bytes /
23 words** at ROM `0x3A950`. Its call to the immediately following
`setupFrontEndObject` and JFG's same ordered pair establish the role. The body
is adapted from DKR's public `menu_imagegroup_load` with point-of-use
`PROVENANCE`; JFG supplies the name/order. The default flags, call relocation,
object words, and linked ROM range are exact without post-processing.

The tier-A `setupFrontEndObject` adds **0x88 bytes / 34 words** at ROM
`0x3A9AC`. Its complete masked instruction skeleton is identical to JFG's
same-name function, while the explicit typed record copy and signed trailing
bytes are derived from Mickey. The point-of-use `PROVENANCE` note records that
split. Computing the destination pointer before the source pointer recovers
the target's `v1`/`a1` allocation. The default `-O2 -mips2 -32` flags, both
data-relocation pairs, object words, and linked ROM range are exact without
post-processing.

`func_80039E34` retains a Mickey-derived `NON_MATCHING` draw candidate with
the target's exact **0xB8-byte frame** and local homes from `0x7C` through
`0xAC`. Its best object is one word longer than the target's **0x418 bytes /
262 words** and plateaus at **242/262 differing words**, first `+0x14`: IDO
assigns the persistent `D_800D31C8` base and selected object to `t2`/`a3`
instead of `t5`/`t0`, cascading through the command-building paths. Pointer,
volatile-access, and stack-layout variants did not recover the allocation;
the full flag lattice keeps the resident defaults best. JFG's corresponding
front-end draw routine also has no C donor body. Canonical code remains
assembly.

The tier-B `frontGetLanguage` adds **0x14 bytes / 5 words** at ROM
`0x3AE4C`. Its packed-field getter, position immediately before the paired
setter and screen-mode accessors, and the resident caller's use of its result
establish the JFG role. The JFG-derived name carries point-of-use
`PROVENANCE`, while the body is Mickey-derived. An unsigned expression feeding
the signed API recovers IDO's temporary return register; the default flags,
HI16/LO16 data relocations, object words, and linked ROM range are exact
without post-processing. A zero-byte weak alias preserves the anonymous name
used by resident assembly.

The paired tier-B `frontSetLanguage` adds **0x38 bytes / 14 words** at ROM
`0x3AE60`. Its byte-wide read/modify/write of the same six-bit field, ordered
position after `frontGetLanguage`, and call to the front-end language refresh
routine establish the JFG role. The name carries point-of-use `PROVENANCE`,
while the field layout and body are Mickey-derived. Default flags, the refresh
call and data relocations, object words, and linked ROM range are exact without
post-processing.

`frontSetWideAdjust` is the first exact C promotion: **0x2C bytes / 11 words**
at ROM `0x3AFDC`, with the target's four relocation-bearing words resolving
at their real linked addresses. Its body is adapted from JFG's public
`src/menu.c` and carries the required point-of-use `PROVENANCE` note. A flag
sweep confirmed that the default `-O2 -mips2 -32` spelling is exact; no
per-file override or post-compile instruction edit is involved.

`frontGetWideAdjust` adds **0xC bytes / 3 words** at ROM `0x3AFD0`. The name
is explicitly **tier B**, not tier A: the body is too short for the standalone
skeleton threshold, but its exact byte-return of the setter's stored state and
its position immediately before `frontSetWideAdjust` establish the same role
as JFG's ordered pair. The adapted body has a point-of-use `PROVENANCE` note,
and the default flags are byte-exact in the flag lattice.

The tier-B `frontGetStereoMode` adds **0x14 bytes / 5 words** at ROM
`0x3B008`. Its two-bit packed getter and ordered position between the
wide-adjust and SFX-volume accessors establish the JFG role. The name carries
point-of-use `PROVENANCE`; Mickey's paired getter/setter encoding supplies the
bitfield and `u32` ABI. The canonical flags, HI16/LO16 data relocations, object
words, and linked ROM range are exact without post-processing.

The paired tier-B `frontSetStereoMode` adds **0x60 bytes / 24 words** at ROM
`0x3B01C`. JFG supplies the name, clamp, output-type table lookup, audio call,
and ordered position; Mickey's paired accessors supply the packed two-bit
storage. The adapted logic carries point-of-use `PROVENANCE`. The default
flags, two data pairs plus call relocation, object words, and linked ROM range
are exact without post-processing.

The tier-A-named `frontGetScreenMode` adds **0x30 bytes / 12 words** at ROM
`0x3AE98`. Mickey's draft established the two tests; JFG's published
`Resbitfield` declaration supplied the original source shape needed to recover
the compiler's temporary-register order. Mickey has two adjacent mode bits,
confirmed by the paired writes in the following setter. The adapted type has a
point-of-use `PROVENANCE` note, and the default flags, object words, and linked
ROM range are exact without post-processing.

`func_8003A2C8` remains guarded and exact-sized. The configured full-TU
alias-backed mode-byte candidate matches 31/32 raw and relocation-normalized
words, first `+0x14`, with no frame or padding. It carries
the target's six HI16/LO16 records at `+0x00/+0x04`, `+0x24/+0x28`, and
`+0x3C/+0x40` for `D_8007C090` and `D_800D3128`. The complete pool and temp
lanes match; only the equality branch reads the narrowed v0 carrier instead of
the target's v1 carrier.

Exactly 119 configurations were retained. Thirteen O2/MIPS-II rows tie clean
V0 and every other viable family regresses. A second full-TU pass reproduced
that lattice and used an instrumented proc-20 build whose `.text`, `.data`,
`.rodata`, symbols, and relocations all pass the stock-toolchain fidelity gate.
The trace separates the full mode, narrowed byte, state address, and comparison
carrier webs. All six declaration orders, direct-global access, and a narrow-
first split assignment are byte-flat. A bounded permuter pass exposed the
natural address alias plus refreshed-byte lifetime, improving seven words; its
reseeded pass moved score 10 to 5 without reaching zero. Splitting the repeated
mask regresses to 27/32, and the remaining single branch word is classified as
an unreachable uopt coalescing tie without a forced-colour oracle. ORT 606
exports the function;
authenticated callers remain `func_80027EC0+0x80`,
`overlay46ReleaseState+0x70`, and
`func_overlay_060_F0000334_18BA10C+0x1238`. Exact ordinary-object, linked-
function, complete-menu-TU, and ROM evidence continues to prove assembly
fallback only.

The tier-A-named `frontGetLevelScreenMode` adds **0x68 bytes / 26 words** at
ROM `0x3AF68`. Its JFG source body is still `GLOBAL_ASM`, so the C body was
derived from Mickey's own draft and control flow rather than borrowed. The
four cases return fixed mode 1, level mode with bit 1 set, fixed mode 3, or the
current level mode. The canonical flags, two call relocations, object words,
and linked ROM range are exact without post-processing.

The tier-B-named `frontStoreScreenMode` adds **0x14 bytes / 5 words** at ROM
`0x3AF48`. Its copied-byte store, its position in the ordered screen-mode
accessor family, and the matching JFG source body establish the role; the body
therefore carries a point-of-use `PROVENANCE` note. The default flags and both
global-data relocations are exact without post-processing.

The adjacent tier-B `frontRecallScreenMode` adds **0xC bytes / 3 words** at
ROM `0x3AF5C`. Its byte return reads the state written by
`frontStoreScreenMode`, reproducing JFG's ordered accessor pair. The adapted
body carries a point-of-use `PROVENANCE` note; the default flags and data
relocations are exact without post-processing.

The tier-B `frontGetSfxVolume` adds **0xC bytes / 3 words** at ROM `0x3B07C`.
The halfword getter's ordered JFG position and its adjacent setter's call to
`gsSndpSetGlobalVolume` identify the state as the SFX volume. The adapted JFG
body has a point-of-use `PROVENANCE` note; default flags and the linked global
relocation are exact without post-processing. A zero-byte weak alias retains
the anonymous spelling still referenced by resident assembly.

The paired tier-B `frontSetSfxVolume` adds **0x3C bytes / 15 words** at ROM
`0x3B088`. JFG's body accounts for both bounds clamps, the halfword store, and
the `gsSndpSetGlobalVolume` call, so the adapted body carries point-of-use
`PROVENANCE`. The canonical flags, call and data relocations, object words, and
linked ROM range are exact; a zero-byte weak alias preserves the anonymous
name used by the remaining assembly caller.

The tier-B `frontGetBgmVolume` adds **0xC bytes / 3 words** at ROM `0x3B0C4`.
Its halfword getter follows the completed SFX pair at the exact JFG menu
position, while the adjacent clamp-and-audio-call setter confirms the paired
BGM state. The adapted body carries point-of-use `PROVENANCE`; its linked data
relocation is exact, and a zero-byte weak alias preserves the anonymous name
used by resident assembly.

The paired tier-B `frontSetBgmVolume` adds **0x3C bytes / 15 words** at ROM
`0x3B0D0`. The JFG body exactly accounts for Mickey's two bounds clamps,
halfword store, and corresponding audio-volume call, and carries point-of-use
`PROVENANCE`. Canonical flags, call and data relocations, object words, and the
linked ROM range are exact; a zero-byte weak alias preserves its assembly
caller's anonymous spelling.

The tier-B `frontGet2PlayerSplit` adds **0x14 bytes / 5 words** at ROM
`0x3B10C`. Its extracted flag bit, its paired byte-preserving setter, and its
exact JFG menu position establish the role; the body itself was derived from
Mickey because JFG's remains `GLOBAL_ASM`. The extended bitfield declaration
carries point-of-use `PROVENANCE`. A local result recovers IDO's target `v1`
live range; the default flags, both data relocations, object words, and linked
ROM range are exact without post-processing.

`func_8003A520` adds **0x24 bytes / 9 words** at ROM `0x3B120`. Mickey's code
is the byte-preserving `twoPlayerSplit` setter paired with the preceding
getter. The instrumented-ugen free-list trace showed the target's temp ring
rotated one pop past the plain bitfield assignment; a `& 1` redundant with the
1-bit field insert supplies that pop (field-guide lever 16) and the object
words, relocations, and linked ROM range are exact without post-processing.

The tier-D `func_8003A544` adds **0xC bytes / 3 words** at ROM `0x3B144`.
Mickey's code is the single-word setter paired with the following getter; no
published donor body or descriptive-name evidence is used. The default flags,
HI16/LO16 data relocations, object words, and linked ROM range are exact
without post-processing.

The paired tier-D `func_8003A550` adds **0xC bytes / 3 words** at ROM
`0x3B150`. Mickey's code returns the same word written by `func_8003A544`;
no donor body or descriptive-name evidence is used. The default flags, both
data-relocation words, object words, and linked ROM range are exact without
post-processing.

`func_8003A2C8` remains guarded `NON_MATCHING` over ROM `0x3AEC8..0x3AF48`.
The configured alias-backed candidate matches 31/32 raw and relocation-
normalized words, first `+0x14`, with no frame or padding and all six
relocations exact. Its complete pool and temp lanes match; the lone difference
is the equality branch reading v0 instead of the target's v1. A ten-minute
bounded sweep improved score 60 to 10 and exposed the natural alias/refreshed-
byte form; a reseeded ten-minute sweep reached score 5 but no zero. The clean
score-5 spelling remains 31/32, while splitting the repeated mask regresses to
27/32. Earlier flags, trace, declaration, direct-global, width, and shared-byte
forms remain closed; the last word is an unreachable uopt coalescing tie until
a forced-colour oracle becomes available.
Ordinary and linked exact output remains assembly fallback only. ORT 606 and
exactly three callers are authenticated: resident `func_80027EC0+0x80`, Overlay
46 `overlay46ReleaseState+0x70`, and Overlay 60
`func_overlay_060_F0000334_18BA10C+0x1238`. There are no runtime records inside
the function, extra direct calls, stored pointers, or other overlay inbounds.
JFG's ordered `frontSetScreenMode` peer is assembly-backed accessor-family role
evidence, not donor C.

The tier-D `func_8003A55C` adds **0x34 bytes / 13 words** at ROM `0x3B15C`.
Its Mickey-derived body stops the active tune, stores the caller's byte, and
sets the paired halfword timer to `0x78`. The adjacent JFG menu names do not
pin that Mickey-specific behavior, so the address label remains. The default
flags, call relocation, two data-relocation pairs, object words, and linked ROM
range are exact without post-processing.

The tier-D `func_8003A590` adds **0x10 bytes / 4 words** at ROM `0x3B190`.
Mickey's body writes `-1` to a resident halfword. A short-function skeleton
collision with an unrelated published routine is rejected as naming evidence,
so the address label remains. The default flags, HI16/LO16 relocation pair,
object words, and linked ROM range are exact without post-processing.

### 3.12 Track assembly and shadows (`0x16140`–`0x18FF0`)

This block contains two JFG-lineage translation units. The boundary claims are
explicitly **not tier A whole-object matches**:

- `main/trackasm`, ROM `0x16140`–`0x16A90`: **tier B** from the track callers
  and helper call graph, plus **tier D** from JFG's exact four-function order
  (`trackMakePolylist`, `getXZCompareMask`, `getYCompareMask`,
  `trackLightAsm`). JFG carries the same run in `asm/hasm/trackasm.s`.
- `main/shadows`, ROM `0x16A90`–`0x18FF0`: **tier B** from allocation/free and
  track-render call relationships, plus **tier D** from JFG `src/shadows.c`'s
  order and the per-function masked-skeleton results. Its upper boundary is
  independently corroborated at **tier A**: `shadowBoxPolyOverlap` begins at
  `0x18FF0`, the first function of JFG's next TU, `shadows_214A0.c`.

**PROVENANCE:** the TU names and descriptive function names are borrowed from
Jet Force Gemini's public decomp (`asm/hasm/trackasm.s`, `src/shadows.c`, and
their `asm/nonmatchings/` file names), a permitted retail-derived source under
`docs/CLEANROOM.md`. Mickey's own bytes determine the bodies. JFG's
address-placeholder helper names are not imported.

| ROM | Size | Symbol | Evidence / disposition |
|---|---:|---|---|
| `0x16140` | `0x49C` | `trackMakePolylist` | B; extractor-marked handwritten, stays `asm` |
| `0x165DC` | `0x11C` | `getXZCompareMask` | B; extractor-marked handwritten, stays `asm` |
| `0x166F8` | `0x98` | `getYCompareMask` | B; extractor-marked handwritten, stays `asm` |
| `0x16790` | `0x300` | `trackLightAsm` | B; uses odd single-precision FP registers, stays `asm` |
| `0x16A90` | `0x12C` | `shadowInitBuffers` | B name; `NON_MATCHING` relocation-identity plateau: all 75 linked words and shadows' `0x50`-byte `.data` input are exact, but the sentinel pair binds `D_80079434 + 0xC` where target metadata names `D_80079440` |
| `0x16BBC` | `0x78` | `shadowFreeBuffers` | B name; JFG-adapted exact C, 30 words, 15 relocs under O2/mips2 |
| `0x16C34` | `0x18` | `shadowChangeBuffer` | B name; exact C, 6 words, 2 relocs |
| `0x16C4C` | `0x4C` | `shadowGetBuffers` | B name; exact C, 19 words, 8 relocs |
| `0x16C98` | `0x7F8` | `shadowGenerate` | B; `NON_MATCHING` candidate at 445 differing words, 520/510 instructions, frames `-0x150`/`-0x138` |
| `0x17490` | `0x8B0` | `func_80016890` | D; `NON_MATCHING` resident fallback and adjacent `func_80017140` boundary establish `src/main/shadows.c.o` ownership under `-O2 -mips2 -32 -Wab,-r4300_mul`; candidate at 563 differing words, exact `-0x190` frame, first `+0x4`; its target carries eight FP multiply-hazard nops; no match credit |
| `0x17D40` | `0x520` | `func_80017140` | D candidate: 300 differing words, 325/328 instructions, exact `-0x140` frame with the polygon buffer at the target's own offset, first `+0x44` |
| `0x18260` | `0x56C` | `func_80017660` | **A**; exact C, 347 words, 4 relocations under `-O2 -mips2 -32 -Wab,-r4300_mul`; the owned ROM range `0x18260`-`0x187CC` is byte-identical to the baserom |
| `0x187CC` | `0x4E8` | `func_80017BCC` | D candidate: 270 differing words, 316/314 instructions, exact `-0x108` frame after deleting four decompiler-only locals, first `+0x4`; its target carries four FP multiply-hazard nops |
| `0x18CB4` | `0x33C` | `func_800180B4` | Evidence D candidate: structure-mismatch, 206/206 instructions, 101 differing words, first `+0x34`, exact `-0x90` frame, every stack home and 7/8 relocation identities aligned; the one open mechanism is the sector index, which the target holds in a caller-saved register and spills across `getXZCompareMask` |

`src/main/shadows.c` compiles with `-Wab,-r4300_mul`. Three of its unmatched
functions carry, in the ROM's own bytes, the scheduler nop IDO emits only under
that flag between two adjacent single-precision multiplies; the four functions
the TU already matches contain no such pair and are insensitive, so the ROM
rebuild is consistent either way and the target bytes are the deciding
evidence.

There are no string references in either TU. The only resident-tail anchors
are `D_800817A0` and `D_800817A4`, both floating-point constants. Of the four
extractor-marked handwritten track routines, only `trackLightAsm` uses odd FP
registers; the other three contain non-compiler instruction shapes and remain
assembly with it. No function in `main/shadows` uses an odd FP register.

### 3.13 Camera lights and sprite animation (`0x1BE50`–`0x1C790`)

The eight entry points at `0x1BE50`–`0x1BEA0` are Mickey's disabled
`main/camlight` implementation: each is a return-only or argument-spilling
stub, but their exact order and signatures follow JFG's `src/camlight.c`.
That ordering, the object-system call sites, and the clean handoff to
`spranimInit` make the boundary **tier B/D**, not a whole-object tier-A hit.
`main/spranim` then occupies the remainder of this assigned block. Its first
five functions follow JFG's `src/spranim.c` order; `texscrollControl` and
`rangetriggerControl` are additionally identified by their masked skeletons
and texture-scroll/volume-trigger callees. Helpers without that evidence keep
their Mickey address names.

**PROVENANCE:** the TU and descriptive function names are borrowed from Jet
Force Gemini's public retail-derived `src/camlight.c`, `src/spranim.c`, and
their `asm/nonmatchings/` names, as permitted by `docs/CLEANROOM.md`. No JFG
body is copied by this split; Mickey's own bytes remain authoritative.

| ROM | Size | Symbol | Evidence / disposition |
|---|---:|---|---|
| `0x1BE50` | `0x8` | `camlightInit` | D name; exact C, 2 words, 0 relocs |
| `0x1BE58` | `0x8` | `camlightFlush` | D name; exact C, 2 words, 0 relocs |
| `0x1BE60` | `0x10` | `camlightAdd` | D name; exact C, 4 words, 0 relocs |
| `0x1BE70` | `0x8` | `camlightDelete` | D name; exact C, 2 words, 0 relocs |
| `0x1BE78` | `0x8` | `camlightUpdateAll` | D name; exact C, 2 words, 0 relocs |
| `0x1BE80` | `0x8` | `camlightUpdate` | D name; exact C, 2 words, 0 relocs |
| `0x1BE88` | `0x8` | `camlightVisibilityCheck` | D name; exact C, 2 words, 0 relocs |
| `0x1BE90` | `0x10` | `camlightDraw` | D name; exact C, 4 words, 0 relocs |
| `0x1BEA0` | `0x74` | `spranimInit` | D name; JFG-adapted exact C, 29 words, 0 relocs under O2/mips2 |
| `0x1BF14` | `0x4C` | `spranimControl` | D name; JFG-adapted exact C, 19 words, 1 call reloc under O2/mips2 |
| `0x1BF60` | `0x48` | `sprasjiInit` | D name; exact C, 18 words, 0 relocs under O2/mips2 |
| `0x1BFA8` | `0x78` | `spranimOnceControl` | D name; JFG-adapted exact C, 30 words, 2 call relocs under O2/mips2 |
| `0x1C020` | `0x304` | `effectboxControl` | Evidence D candidate: structure-mismatch, 193/193 instructions, 65 differing words, first `+0x0`, frame `-0x98` versus `-0x80`, exact relocation identities, four opcode residuals |
| `0x1C324` | `0x74` | `texscrollControl` | B name; JFG-adapted exact C, 29 words, 1 call reloc under O2/mips2 |
| `0x1C398` | `0x2BC` | `func_8001B798` | Evidence D candidate: structure-mismatch, 171/175 instructions, 131 differing words, first `+0x0`, frame `-0xD0` versus `-0xE0`, and 11 relocation-symbol sites differ |
| `0x1C654` | `0x90` | `rangetriggerControl` | B; `NON_MATCHING` plateau after flag sweep and 10 stack-layout hypotheses: 34/36 words exact with both call relocs exact, first mismatch `+0x50`; IDO homes the entry pointer at `sp+0x44` instead of target `sp+0x40` |
| `0x1C6E4` | `0x14` | `func_8001BAE4` | exact C, 5 words, 0 relocs; role unresolved |
| `0x1C6F8` | `0xC` | `func_8001BAF8` | exact C, 3 words, 0 relocs; role unresolved |
| `0x1C704` | `0xC` | `func_8001BB04` | exact C, 3 words, 0 relocs; role unresolved |
| `0x1C710` | `0x78` | `func_8001BB10` | plateau: 8/30 words differ; first `+0x3C`, load scheduling |

No function in this range uses an odd single-precision FP register, and there
are no string references. All twenty functions are compiler-generated. ROM
`0x1C788`-`0x1C790` is alignment padding and receives no function credit.

### 3.14 Weather (`0x3B480`–`0x3D5F0`)

This run is `main/weather`. The second function, `weather_clip_planes`, was
already a unique tier-A DKR byte match. The preceding `initWeather` and the
remaining snow/rain call graph agree with JFG and DKR at **tier B/D**; the
masked-skeleton scan independently selected their weather counterparts for
all public entry points and most helpers. The upper boundary is fixed
independently at **tier A** by `reset_particles`, the first function of the
following `particles.c` run at `0x3D5F0`. The lower boundary is structural,
not a claimed whole-object match.

The logical TU is represented by three physical splat fragments:
`main/weather` (`0x3B480`–`0x3D030`), the hand-written
`main/weather_snow_asm` island (`0x3D030`–`0x3D370`), and
`main/weather_tail` (`0x3D370`–`0x3D5F0`). This keeps the hand-written pair
out of asm-processor while retaining C ownership around it.

**PROVENANCE:** the TU and descriptive function names are borrowed from Jet
Force Gemini's and Diddy Kong Racing's public retail-derived `src/weather.c`
files and JFG's `asm/nonmatchings/weather/` names, as permitted by
`docs/CLEANROOM.md`. The matched `initWeather`, `weather_clip_planes`,
`freeWeather`, `setupWeather`, `snow_init`, `changeWeather`, and `rainDensity`
bodies, plus `rain_set`, `rainSetFog`, `rain_update`, `rain_lightning`, and
`rain_sound`, are adapted from those disclosed sources and carry point-of-use
notes; Mickey's own bytes remain authoritative.

The tier-B/D `initWeather` adds **0xFC bytes / 63 words** at ROM `0x3B480`.
JFG's initialization and asset-table walk reproduce Mickey's instruction
stream at the canonical `-O2 -mips2 -32` flags, with all 25 relocations and
the linked ROM range agreeing.

The tier-B/D `freeWeather` adds **0x120 bytes / 72 words** at ROM `0x3B5D0`.
JFG's release sequence maps directly onto Mickey's global layout; the
canonical `-O2 -mips2 -32` object is instruction-exact with all 34 relocations
and the linked ROM range agreeing.

The tier-B/D `setupWeather` adds **0x420 bytes / 264 words** at ROM `0x3B6F0`.
JFG's declaration order and control spelling reproduce Mickey's 0x60-byte
frame, while Mickey's own rain-init arguments, random bounds, texture layout,
and buffer-end sentinel settle the revision differences. The canonical
`-O2 -mips2 -32` object is instruction-exact with all 41 relocations agreeing,
and the linked ROM range is exact without post-processing.

The tier-B/D `changeWeather` adds **0x1EC bytes / 123 words** at ROM
`0x3BC30`. JFG supplies the state transition; Mickey's combined condition and
assignment order compile instruction-exact at the canonical `-O2 -mips2 -32`
flags, with all 5 relocations and the linked ROM range agreeing.

The tier-B/D `snow_init` adds **0x120 bytes / 72 words** at ROM `0x3BB10`.
DKR supplies the circular position loop; Mickey's scale constants and texture
loader compile instruction-exact at canonical `-O2 -mips2 -32`, with all 8
relocations and the linked ROM range agreeing.

The tier-B/D `rain_lightning` adds **0x128 bytes / 74 words** at ROM
`0x3CE48`. The DKR/JFG timer structure plus Mickey's transition arguments and
thresholds compile instruction-exact at canonical `-O2 -mips2 -32`, with all
17 relocations and the linked ROM range agreeing.

The tier-B/D `rain_update` adds **0x144 bytes / 81 words** at ROM `0x3C6B4`.
JFG's transition and dispatch structure, including Mickey's unresolved
rain-movement binding, compiles instruction-exact at canonical
`-O2 -mips2 -32`, with all 27 relocations and the linked ROM range agreeing.

The tier-B/D `rain_set` adds **0x104 bytes / 65 words** at ROM `0x3C468`.
JFG's TV-rate-dependent transition setup compiles instruction-exact at
canonical `-O2 -mips2 -32`, with all 18 relocations and the linked ROM range
agreeing.

The tier-B/D `rainSetFog` adds **0xD0 bytes / 52 words** at ROM `0x3C56C`.
JFG's level-flag guard and fog calculation compile instruction-exact at
canonical `-O2 -mips2 -32`, with all 7 relocations and the linked ROM range
agreeing.

The tier-B/D `rain_sound` adds **0xC0 bytes / 48 words** at ROM `0x3CF70`.
JFG's camera-relative sound positioning compiles instruction-exact at canonical
`-O2 -mips2 -32`, with all 13 relocations and the linked ROM range agreeing.

The tier-B/D `doWeather` adds **0x2A4 bytes / 169 words** at ROM `0x3BE1C`.
JFG's weather transition and render dispatch compile instruction-exact at
canonical `-O2 -mips2 -32`, with all 61 relocations and the linked ROM range agreeing.

`rain_init` and `free_rain_memory` share a synthetic static
`TrapDanglingJump` binding with `rain_update`, but require incompatible integer,
void, and float call signatures inside the consolidated TU. Their JFG bodies
otherwise reproduce all 59 and 33 instruction words and every relocation kind;
the best candidates retain one relocation-identity mismatch each, at `+0xA0`
and `+0x68` respectively. Direct calls, typed function-pointer casts, weak
aliases, and three- versus four-parameter `rain_init` declarations were tested;
the candidates remain preserved behind `NON_MATCHING`.

| ROM | Size | Symbol | Evidence / disposition |
|---|---:|---|---|
| `0x3B480` | `0xFC` | `initWeather` | D |
| `0x3B57C` | `0x54` | `weather_clip_planes` | A donor; exact C, 21 words, 2 relocs |
| `0x3B5D0` | `0x120` | `freeWeather` | D |
| `0x3B6F0` | `0x420` | `setupWeather` | D |
| `0x3BB10` | `0x120` | `snow_init` | D |
| `0x3BC30` | `0x1EC` | `changeWeather` | D |
| `0x3BE1C` | `0x2A4` | `doWeather` | D |
| `0x3C0C0` | `0x238` | `snow_render` | D |
| `0x3C2F8` | `0xEC` | `rain_init` | D |
| `0x3C3E4` | `0x84` | `free_rain_memory` | D |
| `0x3C468` | `0x104` | `rain_set` | D |
| `0x3C56C` | `0xD0` | `rainSetFog` | D |
| `0x3C63C` | `0x78` | `rainDensity` | D name; exact C, 30 words, 4 relocs |
| `0x3C6B4` | `0x144` | `rain_update` | D |
| `0x3C7F8` | `0x650` | `rain_render_splashes` | D |
| `0x3CE48` | `0x128` | `rain_lightning` | D |
| `0x3CF70` | `0xC0` | `rain_sound` | D |
| `0x3D030` | `0x144` | `snow_update` | D; handwritten asm |
| `0x3D174` | `0x1FC` | `snow_vertices` | D; odd-FP handwritten asm |
| `0x3B480` | `0xFC` | `initWeather` | B/D name; exact C, 63 words, 25 relocs |
| `0x3B57C` | `0x54` | `weather_clip_planes` | A donor; exact C, 21 words, 2 relocs |
| `0x3B5D0` | `0x120` | `freeWeather` | B/D name; exact C, 72 words, 34 relocs |
| `0x3B6F0` | `0x420` | `setupWeather` | B/D name; exact C, 264 words, 41 relocs |
| `0x3BB10` | `0x120` | `snow_init` | B/D name; exact C, 72 words, 8 relocs |
| `0x3BC30` | `0x1EC` | `changeWeather` | B/D name; exact C, 123 words, 5 relocs |
| `0x3BE1C` | `0x2A4` | `doWeather` | B/D name; exact C, 169 words, 61 relocs |
| `0x3C0C0` | `0x238` | `snow_render` | B/D name; exact C, 142 words, 18 relocs; linked ROM proved |
| `0x3C2F8` | `0xEC` | `rain_init` | B/D; plateau, 59 words exact, one reloc identity at `+0xA0` |
| `0x3C3E4` | `0x84` | `free_rain_memory` | B/D; plateau, 33 words exact, one reloc identity at `+0x68` |
| `0x3C468` | `0x104` | `rain_set` | B/D name; exact C, 65 words, 18 relocs |
| `0x3C56C` | `0xD0` | `rainSetFog` | B/D name; exact C, 52 words, 7 relocs |
| `0x3C63C` | `0x78` | `rainDensity` | B/D name; exact C, 30 words, 4 relocs |
| `0x3C6B4` | `0x144` | `rain_update` | B/D name; exact C, 81 words, 27 relocs |
| `0x3C7F8` | `0x650` | `rain_render_splashes` | B/D |
| `0x3CE48` | `0x128` | `rain_lightning` | B/D name; exact C, 74 words, 17 relocs |
| `0x3CF70` | `0xC0` | `rain_sound` | B/D name; exact C, 48 words, 13 relocs |
| `0x3D030` | `0x144` | `snow_update` | B/D; handwritten asm |
| `0x3D174` | `0x1FC` | `snow_vertices` | B/D; odd-FP handwritten asm |
| `0x3D370` | `0x9C` | `func_8003C770` | D; Mickey-only reconstruction, exact C, 39 words, 15 relocs |
| `0x3D40C` | `0x1E4` | `func_8003C80C` | `NON_MATCHING` diagnostic draft; 106 positional words differ, first `+0x14`, target/candidate 118/117 instructions. Its unused eight-byte frame pad is removed; clean shape/frame are uncompiled, with structural/register gaps remaining. |

`snow_render` now reproduces all **568 bytes / 142 words** at ROM
`0x3C0C0`–`0x3C2F8`, with the exact `0x58` frame and all 18 static relocation
identities. The JFG donor's typed texture access, early returns and scoped
command expansions remove the prior size/frame gap; Mickey's pre-call caches
and decrementing loop remain required by its ROM. The final matrix command
preserves the donor macro's shared statement line. The canonical linked owned
range and full US ROM both compare exactly. See the
[resolved handoff](matching-triage-handoffs/snow_render.md) for the measured
attempts and provenance.

There are no string references. `snow_vertices` is the range's only function
using odd single-precision FP registers; it and extractor-marked
`snow_update` remain source assembly permanently rather than matching targets.
The independent boundary evidence used later for `saves`, `pi`, `screen`,
`rcpFast3d` and `sched` is recorded separately below rather than retroactively
attributed to this scan.

### 3.15 Save, PI, screen, RCP and scheduler census

ROM `0x2C8C0`–`0x323A0` contains **86 functions and 23,264 bytes** in five
consecutive JFG-lineage translation units. This is a function-order and call-
graph census, not a claim that any whole JFG object is byte-identical. The
source files carry the required `PROVENANCE` disclosure; Mickey's own bytes
remain authoritative for every body.

| ROM range | Size | Functions | TU | Evidence |
|---|---:|---:|---|---|
| `0x2C8C0`–`0x2ECA0` | 9,184 | 42 | `main/saves` | **A:** `rumbleKill` is byte-identical to JFG. **B:** ordered rumble calls, save checksums, and the contiguous Controller Pak API (`osPfs*`). **D:** uncertain Mickey-only helpers retain `func_<VRAM>` names. |
| `0x2ECA0`–`0x2F0D0` | 1,072 | 7 | `main/pi` | **B:** the exact JFG function order `piInit`, four asset lookups/loaders, two accessors, `romCopy`; the last routine owns the `osPiStartDma` loop. |
| `0x2F0D0`–`0x2F400` | 816 | 2 | `main/screen` | **B:** load/decompress followed by draw/VI calls, matching JFG's two-function `screen.c` order. |
| `0x2F400`–`0x30CD0` | 6,352 | 14 | `main/rcpFast3d` | **A:** `rcpInit` and the existing border-colour routine are byte-identity anchors; masked skeletons also reproduce JFG's `rcpFast3d`/screen-colour shapes. **B:** queue/RCP calls and the ordered init helpers. |
| `0x30CD0`–`0x323A0` | 5,840 | 21 | `main/sched` | **A:** the two queue accessors and the ROM-exact C reconstruction of `__scHandleRetrace`. **B:** the complete JFG scheduler call graph from `osCreateScheduler` through `__scSchedule`. **C:** `osScGetTaskType`'s seven task-name strings and `__scHandleRetrace`'s `"SP CRASHED"`/`"Version %s"`. |

The function boundaries are the extracted labels cross-checked against the
linked ELF's symbol sizes. All 86 functions were queried with
`tools/skeleton_scan.py similar --target <vram> --top 5`; the useful exact
anchors are the ones stated above, while the remaining results are near-match
context rather than naming evidence. No function in either original block
uses an odd single-precision FP register, so §6.2's hand-written-assembly
criterion identifies **zero** forced-ASM functions here.

The table above states each TU's evidence *categories*, not a per-function
verdict; §1 requires the latter. Per §1's rule, only the functions named in
the A/B/C cells above carry that individual argument (`rumbleKill`: tier A;
`piInit` and the rest of `main/pi`'s seven-function order: tier B; both
`main/screen` functions: tier B; `rcpInit` and the border-colour routine:
tier A, the remaining `main/rcpFast3d` functions: tier B; the two queue
accessors and `osScGetTaskType`/`__scHandleRetrace`: tier A/A/A respectively,
the rest of `main/sched`'s scheduler call graph: tier B). Every other
function in these five TUs, without an individual argument beyond TU
membership and order, is tier D. `symbol_addrs.us.txt` carries the resulting
per-symbol tier token for each of the 86 functions; this table is the
TU-level summary, not a substitute for it.

Exact C reconstructions in this census currently include the still-unnamed
`func_8002BCC0` (ROM `0x2C8C0`–`0x2C8FC`, 60 bytes), `rumbleRumbles`
(ROM `0x2C8FC`–`0x2C908`, 12 bytes), `rumbleProcessing` (ROM `0x2C908`–
`0x2C958`, 80 bytes), `rumbleStart` (ROM `0x2C958`–`0x2CA74`, 284 bytes),
`rumbleStop` (ROM `0x2CA74`–`0x2CB00`, 140 bytes),
`rumbleKill` (ROM `0x2CB00`–`0x2CB44`, 68 bytes), `rumbleUpdate` (ROM
`0x2CB44`–`0x2CB54`, 16 bytes), the still-unnamed rumble reinitializer
`func_8002BF54` (ROM `0x2CB54`–`0x2CC98`, 324 bytes),
`packCalculateGameChecksum` (ROM `0x2D3BC`–
`0x2D3EC`, 48 bytes), `packCalculateGlobalFlagsChecksum` (ROM `0x2DA2C`–
`0x2DA54`, 40 bytes), `packClose` (ROM `0x2DED4`–`0x2DF00`, 44 bytes),
`packOpen` (ROM `0x2DCCC`–`0x2DED4`, 520 bytes),
the still-unnamed chunked save-device transfer `func_8002C7EC` (ROM
`0x2D3EC`–`0x2D4B4`, 200 bytes),
`packIsPresent` (ROM `0x2E0CC`–`0x2E128`, 92 bytes),
`packDirectory` (ROM `0x2E128`–`0x2E424`, 764 bytes),
`packDirectoryFree` (ROM `0x2E424`–`0x2E458`, 52 bytes),
`packFreeSpace` (ROM `0x2E458`–`0x2E56C`, 276 bytes),
`packDeleteFile` (ROM `0x2E56C`–`0x2E620`, 180 bytes),
`packOpenFile` (ROM `0x2E620`–`0x2E74C`, 300 bytes),
`packReadFile` (ROM `0x2E74C`–`0x2E810`, 196 bytes),
`packWriteFile` (ROM `0x2E810`–`0x2EA50`, 576 bytes),
`packFileSize` (ROM `0x2EA50`–`0x2EAB4`, 100 bytes),
the still-unnamed `func_8002C5F4` (ROM `0x2D1F4`–`0x2D20C`, 24 bytes),
the still-unnamed bitstream allocator `func_8002C60C` (ROM `0x2D20C`–
`0x2D29C`, 144 bytes),
the still-unnamed `func_8002C788` (ROM `0x2D388`–`0x2D390`, 8 bytes),
the still-unnamed `func_8002C790` (ROM `0x2D390`–`0x2D39C`, 12 bytes),
the still-unnamed `func_8002C79C` (ROM `0x2D39C`–`0x2D3BC`, 32 bytes),
the still-unnamed `func_8002C8B4` (ROM `0x2D4B4`–`0x2D54C`, 152 bytes),
the still-unnamed `func_8002CCE4` (ROM `0x2D8E4`–`0x2D96C`, 136 bytes),
the still-unnamed game-state writer `func_8002CD6C` (ROM `0x2D96C`–
`0x2DA2C`, 192 bytes),
the still-unnamed global-flags loader `func_8002CE54` (ROM `0x2DA54`–
`0x2DB0C`, 184 bytes),
the still-unnamed `func_8002CF0C` (ROM `0x2DB0C`–`0x2DB6C`, 96 bytes),
the still-unnamed `func_8002E020` (ROM `0x2EC20`–`0x2ECA0`, 128 bytes),
`piInit` (ROM `0x2ECA0`–`0x2ED48`, 168 bytes),
`piRomLoad` (ROM `0x2ED48`–`0x2EDE4`, 156 bytes),
`piRomLoadCompressed` (ROM `0x2EDE4`–`0x2EEE0`, 252 bytes),
`piRomLoadSection` (ROM `0x2EEE0`–`0x2EF5C`, 124 bytes),
`piRomGetSectionPtr` (ROM `0x2EF5C`–`0x2EFA4`, 72 bytes),
`piRomGetFileSize` (ROM `0x2EFA4`–`0x2EFE0`, 60 bytes),
`romCopy` (ROM `0x2EFE0`–`0x2F0D0`, 240 bytes),
`screenLoad` (ROM `0x2F0D0`–`0x2F1D4`, 260 bytes),
`screenDraw` (ROM `0x2F1D4`–`0x2F3FC`, 552 bytes),
`rcpWaitDP` (ROM `0x2F6A0`–`0x2F76C`, 204 bytes),
`rcpSetScreenColour` (ROM `0x2F76C`–`0x2F794`, 40 bytes),
`bgdraw_fillcolour` (ROM `0x2F794`–`0x2F7D4`, 64 bytes), and the still-
unnamed global setter `func_8002EBD4` (ROM `0x2F7D4`–`0x2F7E0`, 12 bytes),
plus `rcpClearScreen` (ROM `0x2FD88`–`0x30068`, 736 bytes),
`rcpInitDp` (ROM `0x30068`–`0x30118`, 176 bytes),
`rcpInitDpNoSize` (ROM `0x30118`–`0x3013C`, 36 bytes), and
`rcpInitSp` (ROM `0x3013C`–`0x30160`, 36 bytes), and
`rcpInit` (ROM `0x30160`–`0x30218`, 184 bytes), and
`osCreateScheduler` (ROM `0x30CD0`–`0x30E2C`, 348 bytes),
`osScGetAudioSPStats` (ROM `0x30F20`–
`0x30F38`, 24 bytes), `osScGetCmdQ` (ROM `0x30F10`–`0x30F18`, 8 bytes),
`osScGetInterruptQ` (ROM `0x30F18`–`0x30F20`, 8 bytes), `__scMain` (ROM
`0x30F38`–`0x310E0`, 424 bytes), and the still-unnamed
no-op `func_80030608` (ROM `0x31208`–`0x31210`, 8 bytes), plus
the still-unnamed scheduler helper `func_800304E0` (ROM `0x310E0`–
`0x31180`, 160 bytes),
`osScAddClient` (ROM `0x30E2C`–`0x30E88`, 92 bytes), `osScRemoveClient`
(ROM `0x30E88`–`0x30F10`, 136 bytes), `__scHandleRetrace` (ROM
`0x316E8`–`0x31D4C`, 1,636 bytes), `__scHandleRSP` (ROM `0x31D4C`–
`0x31E74`, 296 bytes), `__scHandleRDP` (ROM `0x31E74`–`0x31EFC`, 136
bytes), `__scTaskReady` (ROM `0x31EFC`–`0x31F4C`, 80 bytes),
`__scTaskComplete` (ROM `0x31F4C`–`0x3204C`, 256 bytes),
`__scAppendList` (ROM `0x3204C`–`0x320AC`, 96 bytes), `__scExec` (ROM
`0x320AC`–`0x3216C`, 192 bytes), and `__scYield` (ROM `0x3216C`–`0x321B8`,
76 bytes). All were compiled
with the resident `-O2 -mips2 -32` flags. The saves TU additionally disables
loop unrolling: the full flag
lattice selects the target's scalar 24-record reset loop, and the full ROM
comparison confirms the setting leaves its other exact functions unchanged.
The named bodies are adapted from
JFG's `src/saves.c`, `src/pi.c`, `src/rcpFast3d.c`, and `src/sched.c`; the
Mickey `rumbleStart` body specifically corresponds to JFG's `rumbleMax`; the
still-unnamed leading rumble gate also adapts its JFG body while retaining
Mickey's placeholder name, and the still-unnamed global-flags loader adapts
JFG's load-and-default-copy flow to Mickey's 24-byte resident record and I/O
helpers. `bgdraw_fillcolour` adapts Diddy Kong Racing's
public `src/rcp_dkr.c` body. The anonymous setter, dual-global reset,
record-field accessors, allocation wrapper, and no-op are reconstructed from
Mickey's own bodies. `screenLoad` is likewise reconstructed from Mickey's
display-list command writes; JFG supplies its existing TU/name association,
not its C body. `rcpInitDp` is likewise reconstructed from Mickey's own
display-list command flow; JFG supplies its name and ordered TU position, not
its C body. `rcpClearScreen` adapts DKR's public `bgdraw_render` display-list
macro spelling to Mickey's guards, helpers, and coordinates; JFG supplies its
name and ordered TU position while retaining assembly. `rcpInit` reconstructs
Mickey's six message queues while JFG's public source supplies its name and
prototype and its object supplies the exact skeleton anchor, not a C body. All
configured object ranges and the final linked ROM are byte-exact.

| Function | ROM | Bytes | Flags | Verdict |
|---|---:|---:|---|---|
| `rcpFast3d` | `0x2F400` | 0x2A0 | `-O2 -mips2 -32` | Mickey/JFG task construction; 168/168 instruction words and all relocations exact after metadata-only weak-alias rebinding, linked ROM exact |

`rcpClearZBuffer` owns ROM `0x2FBDC..0x2FD88`, 107 words with no padding before
`rcpClearScreen`. Tier A: untouched IDO `-O2 -mips2 -32` C is frameless and
exact over all 428 owned bytes after linking at its real address. Loading the
display-list cursor between the two aligned-coordinate assignments closes the
register-allocation residual without changing guards, command order, or stores.
The existing fill-cycle macro is available in the ordinary build with its
replacement text unchanged. Actual IDO preprocessing confirms unchanged
prepared context; raw compiler and configured full-TU output agree, as do all
13 neighboring function ranges and the TU's data sections. Both relocations
(`+0x00` HI16 and `+0x04` LO16) exactly identify resident BSS `D_800D2FAC`.
The linked owned range and full ROM are byte-identical. JFG supplies the
existing name and ordered TU correspondence, not a C body; the source retains
its DKR command-sequence provenance. ORT 765 and the existing callers remain
unchanged. Earlier nonexact attempts are retained in Git history and ignored
diagnostic artifacts.

`func_8002EBE0` retains a Mickey-derived `NON_MATCHING` eight-band gradient
renderer after the 119-combination flag lattice and ten source-shape
hypotheses. The nearest skeleton is Diddy Kong Racing's `bgdraw_render` at only
0.055 similarity; JFG has no function in the corresponding ordered gap. The
best faithful MIPS II candidate has 249 instructions versus the target's 255
and a 0x50-byte frame versus 0x88, with 242 positional word mismatches and the
first at function `+0x0`. SDK scissor, fill-colour, fill-rectangle, and pipe
macros close the body-size gap, but IDO retains 56 fewer bytes of non-save stack
and assigns the display-list cursor and colour-step webs differently from the
prologue onward. A bounded permuter import selected MIPS I and was rejected as
non-canonical; its pack-expression lead also failed when recompiled with the
resident MIPS II flags. The assembly fallback remains canonical.

| Function | ROM | Bytes | Flags | Verdict |
|---|---:|---:|---|---|
| `__scHandleRetrace` | `0x316E8` | 1,636 | `-O2 -mips2 -32` | **A:** 409/409 owned words, 114/114 relocation offsets/types/identities, linked range and full ROM exact |

`__scHandleRetrace` closes the previous 408-word, 84-difference plateau by
re-deriving JFG's permitted `src/sched.c` body at upstream `efd5abb`.
The donor's plain byte flag and polygon-overflow reset replace the earlier
padded volatile diagnostic and explicit coordinate narrowing. Its separate
counter read/write names reproduce the target's address materialization:
`D_8007A660` owns its existing eight-byte, zero-initialized data slot, and
`schedRetraceCounterRead` is a native weak alias of the same storage. The
`.data` carve is exactly ROM `0x7B260..0x7B268`; only eight trailing bytes of
compiler section padding are trimmed. The counter earns no executable credit.

The original fallback object and configured C have all 114 relocation
records at identical offsets with identical types and effective identities.
Both frames are `0xE8`; the owned range ends exactly where `__scHandleRSP`
begins, with no padding. Extracting that range from the linked ELF reproduces
the same baserom offsets byte for byte, and `gmake verify` reproduces the full
US ROM. All 21 scheduler function boundaries and the TU's BSS/rodata sizes
remain unchanged. Raw compiler fidelity and prepared-input self-context
checks pass. The earlier plateau and its source are retained in Git history;
the five donor-mechanism attempts and their objects remain ignored evidence.

The still-unnamed scheduler diagnostic `func_80030610` retains a Mickey-
derived `NON_MATCHING` display-list bisection body after the 119-combination
flag lattice, nine serious source/layout hypotheses, and a bounded two-worker
permuter batch. The best candidate has 194 instructions versus the target's
192 and first diverges at function `+0x5C`; 148 positional words differ. Its
0x90-byte frame is eight bytes shorter than the target's 0x98-byte frame: IDO
homes the received message at stack `+0x78` instead of `+0x70` and the saved
second-command pointer at `+0x48` instead of `+0x4C`. Scalar, `s64` backup,
macro, and explicit-pointer formulations either retain those homes or add
more instructions. JFG supplies the exact assembly skeleton and scheduler
position but no C body. The assembly fallback remains canonical pending a
source spelling that reproduces both stack homes without synthetic padding.

`__scSchedule` is exact at **488 bytes / 122 instruction words** under the
resident flags. Its JFG-derived body and compiler-owned switch table are now
owned by `main/sched` and link with the exact ROM range and relocation surface.

`__scYield` is exact for all 19 instruction words after assigning scheduler's
`0x800D2D40`–`0x800D2D50` BSS to this TU. The owned `u64` at `0x800D2D48`
places its low-word store at `0x800D2D4C`; the final linked ROM is exact.

`osScGetTaskType` plateau: workbench reports exact instructions and known relocation layout.
Removing the wrapper fails the full link because `jtbl_800823D8` references seven assembly-local labels.
The remaining lever is coordinated rodata ownership; assembly stays canonical.

The still-unnamed bit writer `func_8002C69C` is exact at **112 bytes / 28
instruction words**, frameless, with zero relocations. Indexed
`writer->cursor[0]` stores let IDO generate the walking pointer; on that
shape the bit advance belongs before the mask store. ORT 727 has five direct
calls in matched `func_8002C94C`.
The assembly fallback remains canonical. A subsequent source-correctness
repair uses an unsigned initial bit and count-minus-one, defined for counts
1..32 while preserving the zero-count no-op. The five authenticated direct
calls use counts 4, 5 and 18. Configured full-TU text, relocations and data
remain unchanged; this repair adds no matching credit.

The 124-byte `func_8002C70C` is exact under canonical `-Wo,-loopunroll,0 -O2 -mips2 -32`; its 31 words and relocation-free linked range match.

`func_8002CF6C` owns ROM `0x2DB6C..0x2DCCC`, 88 words with no padding before
`packOpen`. A stack-homed `SavesWipeState` (L112 unobservable array lengths,
L97 region around the footer stores, s0 reuse of the buffer after the flags
die) matches frame `0x48` and the target slot ladder. Configured C emits 86
instructions, 54/88 positional words, first `+0x20`, and all 11 relocation
identities. Two target-only words remain: an addiu of the buffer copy by
`0x1C0` that uopt deletes, and a nop in `mainResetPressed`'s jal delay.
The target records remain calls at
`+0x0C/+0x18/+0x28/+0x54/+0x60/+0xC4/+0x120/+0x13C/+0x144` plus the
`D_8007A304` pair at `+0x80/+0x90`. ORT 505 at ROM `0x1849F14` exports it;
`joyRead+0x130` is the sole caller. The assembly fallback remains canonical.
Resume with a source form that keeps the dead increment without spanning a
later call.

The save-window serializer `func_8002C94C` is now **matched** (tier-A byte-identity).
The residual was a pure `schedule-mismatch` (exact 115-word shape, `0x70` frame, relocations already agreeing); the decomp-permuter closed it, finding an `if (1) { ... }` grouping around the entry initialization that resolves the callee-saved slot/counter scheduling tie-break. The C in `src/main/saves.c` now compiles byte-identical to the ROM; no assembly fallback remains.

| Function | ROM | Bytes | Flags | Verdict |
|---|---:|---:|---|---|
| `func_8002CB18` | `0x2D718` | `0x1CC` | `-O2 -mips2 -32 -Wo,-loopunroll,0` | Mickey-derived save-window loader; exact 115 instruction words, frame, relocations, and linked ROM bytes after the stack-home census and unsigned width rematerialization |

`packInit` retains a `NON_MATCHING` body adapted from Diddy Kong Racing's
public `src/save_data.c:init_controller_paks` after the 119-combination flag
lattice and ten loop, local-order, pointer, and controller-limit hypotheses.
Its best candidate has the exact 115 instructions, 96-byte frame, reset loop,
calls, and relocations; 34 words differ, first at `+0xA0`. IDO delays the PFS
base's low half into the loop, rotating address temporaries and serialising the
rumble-success stores. Explicit pointers revert to a multiply, grow the frame,
and score worse. The assembly fallback remains canonical.

The scheduler display-list trace helper `func_80030910` retains a
`NON_MATCHING` body adapted from Jet Force Gemini's public
`src/sched.c:func_8004FF64_50B64` after the 119-combination flag lattice and
ten declaration-order, aggregate-layout, pointer, and address-expression
hypotheses. Its best candidate has the exact 152-byte frame, local addresses,
control flow, calls, and relocations, but 117 instructions versus 118; 20 words
differ, first at `+0x11C`. IDO keeps `0x80000000` live once in `$v1`, while the
target rematerialises it twice through `$at`; the missing word changes branch
spans. Other signed, unsigned, additive, and bitwise spellings retain that CSE
or emit OR. The assembly fallback remains canonical.

| Function | Exact result |
|---|---|
| `font_codes_to_string` | 176 bytes under `-O2 -mips2 -Wo,-loopunroll,0`; JFG `src/saves.c` donor with a hoisted remainder lifetime, untouched IDO output byte-identical. |
| `string_to_font_codes` | 188 bytes under `-O2 -mips2 -Wo,-loopunroll,0`; paired JFG donor with a hoisted remainder and shared bulk-loop entry, untouched IDO output byte-identical. |

### 3.16 Particle and debug-print translation units

ROM `0x3D5F0`–`0x45760` contains two aligned resident C subsegments.
`symbol_addrs.us.txt` records every function's exact size and evidence tier.
Unresolved functions remain `GLOBAL_ASM`, so the split claims no matched bytes.

| Mickey TU | ROM / VRAM | Functions | Evidence |
|---|---|---:|---|
| `main/particles` | `0x3D5F0`–`0x43470` / `0x8003C9F0` | 44 | **A:** DKR's built `particles.c.o` identifies `reset_particles` byte-for-byte. **B:** the internal call graph and external particle callers. **D:** the full function order and masked-skeleton sequence track JFG's 42-function `particles.c.o` from `partFreeLib` through `partNullifyCircularParticleParents`; Mickey inserts two extra 12-byte state setters before `partUpdateTriggers`, after which the sequences reconverge. |
| `main/diprint` | `0x43470`–`0x45760` / `0x80042870` | 19 | **A:** DKR objects identify `strcpy`, `memset`, and `sprintf`; diprint's formatter data/tables and the linked `vsprintf`/`diPrintfAll`/`debug_text_parse` are exact. **B:** `diPrintf` brackets `vsprintf` with `sprintfSetSpacingCodes`, `diPrintfAll` drives the parse/background/character/bounds/origin helpers, and later `diRcp*` routines call `sprintf`. The order matches JFG's `diprint.c.o`, with DKR's `debug_text_width` inserted between `diPrintfSetXY` and `debug_text_parse`. |

**PROVENANCE.** Names/TU attribution use JFG's public `src/particles.c`,
`src/diprint.c`, and objects; DKR's `src/printf.c` supplies
`debug_text_width`, and its `unused_string.c.o`, `printf.c.o`, and
`particles.c.o` supply the stated tier-A rows. Donor placeholders stay
excluded; Mickey's bytes/call graph decide disagreements.

The table tiers are TU-level. Per symbol, byte-identical functions and
compiler-owned formatter data are tier A; named call-graph functions are tier
B; remaining JFG/DKR order/skeleton attributions are tier D. Each of the 82
`symbol_addrs.us.txt` rows in this block carries its tier token.

Exact C matches banked in these TUs: `partAdjustScaling` (ROM `0x3F9C8`,
`0xC` bytes, default resident flags, JFG body donor) and `func_8003EDD4`
(ROM `0x3F9D4`, `0xC` bytes, default resident flags, Mickey-only
reconstruction) and `func_8003EDE0` (ROM `0x3F9E0`, `0xC` bytes, default
resident flags, Mickey-only reconstruction); `strcpy` (ROM `0x43470`, `0x34`
bytes, default resident flags, DKR body donor) and `memset` (ROM `0x434A4`,
`0x34` bytes, default resident flags, DKR body donor); `reset_particles` (ROM
`0x3D5F0`, `0x30` bytes, default resident flags, DKR body donor); `sprintf`
(ROM `0x435A4`, `0x2C` bytes, default resident flags, DKR body donor);
`debug_text_origin` (ROM `0x45710`, `0x24` bytes, default resident flags, JFG
body donor); `sprintfSetSpacingCodes` (ROM `0x43598`, `0xC` bytes, default
resident flags, JFG body donor); `debug_text_newline` (ROM `0x45734`, `0x28`
owned bytes, default resident flags, JFG body donor; the following 4-byte TU
alignment pad is excluded from match credit); `debug_text_bounds` (ROM
`0x45680`, `0x90` bytes, default resident flags, JFG body donor); `diPrintfInit`
(ROM `0x448E0`, `0x54` bytes, default resident flags, JFG body donor);
`diPrintfSetXY` (ROM `0x44D48`, `0x8C` bytes, default resident flags, JFG body
donor); `diPrintfSetCol` (ROM `0x44C10`, `0x9C` bytes, default resident flags,
JFG body donor); `diPrintfSetBG` (ROM `0x44CAC`, `0x9C` bytes, default resident
flags, JFG body donor); `diPrintf` (ROM `0x44934`, `0x9C` bytes, default
resident flags, JFG body donor with its stubbed diagnostic call omitted);
`diPrintfAll` (ROM `0x449D0`, `0x240` bytes, default resident flags, JFG body
donor with diprint-owned BSS, linked exact);
`_itoa` (ROM `0x434D8`, `0xC0` bytes, default resident flags, identical JFG and
DKR glibc-derived body donor); `func_8003CCE4` (ROM `0x3D8E4`, `0x44` bytes,
default resident flags, Mickey-only reconstruction); `func_8003E730` (ROM
`0x3F330`, `0x88` bytes, default resident flags, Mickey-only reconstruction);
`func_80041C50` (ROM `0x42850`, `0x94` bytes, default resident flags,
Mickey-only reconstruction);
`func_80041F48` (ROM `0x42B48`, `0xA4` bytes, default resident flags,
Mickey-only reconstruction);
`func_8004233C` (ROM `0x42F3C`, `0xB0` bytes, default resident flags, DKR
`move_particle_basic` body donor);
`partInitTriggerPos` (ROM `0x3F270`, `0xC0` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_8003CD28` (ROM `0x3D928`, `0xE8` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_80041FEC` (ROM `0x42BEC`, `0xF4` bytes, default resident flags, DKR
`move_particle_basic_parent` body donor);
`func_800423EC` (ROM `0x42FEC`, `0x108` bytes, default resident flags, DKR
`move_particle_forward` body donor);
`func_800420E0` (ROM `0x42CE0`, `0x114` bytes, default resident flags, DKR
`move_particle_attached_to_parent` body donor);
`func_8003CA20` (ROM `0x3D620`, `0x11C` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_8003E7B8` (ROM `0x3F3B8`, `0x120` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_8003CB3C` (ROM `0x3D73C`, `0x1A8` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_80041388` (ROM `0x41F88`, `0x1A8` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_8003EF80` (ROM `0x3FB80`, `0x1D4` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_8003EB08` (ROM `0x3F708`, `0x184` bytes, default resident flags,
Mickey reconstruction with the JFG assembly sibling as a structural oracle);
`func_800421F4` (ROM `0x42DF4`, `0x148` bytes, default resident flags, DKR
`move_particle_with_acceleration` body donor);
`partDraw` (ROM `0x43264`, `0x160` bytes, default resident flags, Mickey
reconstruction with the JFG assembly sibling as a structural oracle);
`partUpdateParticles` (ROM `0x430F4`, `0x170` B, default flags, Mickey/JFG
assembly reconstruction); `func_80040878` (ROM `0x41478`, `0x310` B,
`-O2 -mips2 -32`, DKR body/JFG assembly oracle); `func_80041040` (ROM
`0x41C40`, `0x348` B, default flags, Mickey body/JFG assembly oracle);
`partInitTriggerSPPos`
(ROM `0x3F224`, `0x4C` bytes, default resident flags, JFG-named Mickey
reconstruction); `partInitTrigger` (ROM `0x3F1AC`, `0x78` bytes, default
resident flags, JFG-named Mickey reconstruction); `debug_text_background`
(ROM `0x452F8`, `0xA0` bytes, resident flags plus `-Wab,-r4300_mul`, JFG body
donor); `func_8003EC8C` (ROM `0x3F88C`, `0xBC` bytes, default resident flags,
Mickey pointer-first reconstruction with the JFG assembly sibling as oracle).

`func_8003D4FC`: before/after no C candidate for the `0x10B0`-byte range; shared aggregate types do not change the asm-only body.
Type lever: global and resource declarations; no C promotion is possible. Remains canonical `GLOBAL_ASM`; JFG `func_8005E3DC` is the donor label.

`func_8003F5F8`: Evidence A exact C; all 276 instruction words, the configured
relocation surface, and linked ROM bytes match. JFG `func_800608EC` remains the
structural oracle, not the byte-identity authority.

`func_8003F154`: before/after structure mismatch, 297 instructions and 39 raw words, first `+0x204`.
Type lever: Basic/emitter/vector aggregates; no structure movement. Remains the zero-vector/header-copy/FP-normalization cluster; JFG `func_80060400` is the oracle.

`partNullifyCircularParticleParents` is opcode/size-exact with 25/42 positional
residuals from `+0x0`; implicit loop bounds seed both carrier pairs oppositely.
An explicit end pointer changes the frame, so asm stays canonical.

`debug_text_width` owns ROM `0x44DD4..0x44EDC`, 66 words with no padding before
`debug_text_parse`. Fresh configured full-TU C is 59/66 raw and
relocation-normalized words under canonical game-code flags, with exact frame
`0x138` and first mismatch `+0x38`. Exact records are calls to
`sprintfSetSpacingCodes` at `+0x18/+0x30`, `vsprintf` at `+0x28`, and the
`D_8007CE98` pair at `+0x4C/+0x50`. All 119 flag rows were attempted; thirteen
O2/MIPS-II rows tie and none is exact. One allocator trace and workbench isolate
a five-site current-byte `v1`/`v0` web plus two structural newline-lowering
words. A separate classification scalar regressed to 57/66 through a stack
home, while an explicit newline-tail jump was flat; neither justified a
combination or batch. ORT 862 exports the function, but exhaustive resident,
overlay, direct-jal, literal-pointer, and source scans found no caller. Ordinary
66/66 and linked equality prove fallback only.

`func_8004054C` is an exact 125-word, frameless match over ROM
`0x4114C..0x41340`, with both relocation identities exact. Two source
artefacts held it at a 42-word plateau, and neither was an allocator wall:

- A `u32 *freeBits` local. The target reads `pool->freeBits` afresh at each of
  the eight uses; the local pins all of them to one pool colour and forces the
  descending scan's cursor onto that colour instead of the forward scan's.
  Dropping the local entirely -- both scans read the field -- closes the four
  structural words, the hoisted shift base, and seven register words at once.
  Inlining only one of the two scans is strictly worse than either extreme.
- `1U` versus `1` in the bit-scan test. `!(bits & (1 << bitIndex))` and
  `!(bits & (1U << bitIndex))` are the same value and the same instruction, but
  IDO canonicalises the `and`'s operand order on the operands' signedness, so
  only the unsigned literal puts `bits` in `rs`. Two words, in both scans, and
  no rewriting of the test -- `== 0`, reversed operands, a cast on `bits` --
  reaches it; the literal's own type is the whole lever.

`func_8003E8D8` reaches a bounded configured full-TU plateau at 139/140 raw
and relocation-normalized words, first `+0x38`, with exact `0x230` size,
`0x38` frame, no padding, and all ten target relocation tuples. Declaration
homes now place `entry` at `sp+0x24` and `result` at `sp+0x34` around both
calls, eliminating the prior eight call-live differences; only the zero-count
branch target at `+0x38` remains. Exact linked function/TU/ROM evidence proves
`GLOBAL_ASM` only.
The sole authenticated caller is `func_8003E7B8+0xE4`; there is no ORT export,
resident runtime record, overlay `SYMBOL` inbound, or stored-pointer inbound.
JFG's ordered peer is assembly-backed and 0x24C bytes, so it is structural
evidence only. The 119-configuration lattice produced seven ties at the
131/140 V0 and otherwise regressed. One fidelity-clean allocator trace, three
natural declaration forms, and one trace-selected CFG form completed the
124-build cap; the declaration forms moved 9 differences to 20, 5, and 1,
while the CFG form was byte-identical. The one gain-gated bounded permuter
batch reported a false score-zero for an isolated 147-instruction, 0x24C-byte
candidate with relocation drift, so it yielded no legal candidate. A future
pass needs a source-faithful first-loop CFG spelling that retains the redundant
pool-base reset; do not repeat flags, declaration homes, the explicit reset
form, or this batch route.

| Newly matched function | ROM / size | Match evidence |
|---|---:|---|
| `func_8003CE10` | ROM `0x3DA10`, `0x44C` B | IDO 5.3 exact under resident `-O2 -mips2 -32`; Mickey reconstruction with JFG assembly sibling as structural oracle |
| `func_80040740` | ROM `0x41340`, `0x138` B | IDO 5.3 exact under resident `-O2 -mips2 -32`; Mickey reconstruction with JFG structural oracle |

`partModelObjEmitModelPart` reached a bounded size-exact 84-word plateau under
the default resident flags. The best compliant Mickey reconstruction differs
in 19 words, first at function offset `0x58`, where the candidate performs the
final trigger-stride shift before the descriptor-table add/load while the
target performs it afterward. The residual continues through the trigger
initialization stores; the call and complete post-call FP control flow are
exact. The flag lattice found no exact alternative. A bounded permuter import
selected `-mips1` instead of this TU's configured `-mips2`, so its output was
inadmissible and discarded. The candidate remains under `NON_MATCHING` and the
original asm body remains canonical.

`partUpdateTriggers` is an exact 101-word match under `-Wab,-r4300_mul`; JFG
supplies the body donor and a bounded canonical-flag permuter found the final
loop-condition web coalescing.

`func_80041CE4` owns VRAM `0x80041CE4..0x80041F48`, ROM
`0x428E4..0x42B48`: 612 bytes/153 words, frame `0x80`, and no target padding.
Evidence A exact C: all 153 instruction words, the `-0x80` frame, all nine
relocations, and the linked ROM bytes match. The isolated section's final 12
bytes are alignment outside the function.

All nine target records are exact in genuine C: pairs to `D_8007C894` at
`+0x04/+0x08`, `D_8007C88C` at `+0x48/+0x4C` and `+0x1E4/+0x1F0`, and
`D_7C900` at `+0x9C/+0xA4`, plus the `func_800349A4` call at `+0x158`.
Runtime/export evidence is empty: zero resident records, no ORT row at offset
`0x41894`, no overlay SYMBOL inbound, and no stored pointer. `partDraw+0xEC`
is the sole direct caller and passes typed `Gfx **`/`ParticleLineVertex **`.

The configured flags are `-O2 -mips2 -32 -Wab,-r4300_mul`. JFG `func_80063514`
is a larger assembly-backed structural peer, not genuine donor C.

The 27-word register residual that stood here was four separate source
artefacts, not one allocator phase, and each is worth carrying forward:

- The inner vertex fill walked `line` cast to a `ParticleLinePoint **` cursor.
  The source indexes `line->points[j]` from the `for` counter and lets uopt
  build the cursor itself; the vertex pointer stays a real cursor because
  `ParticleLineVertex` is 20 bytes and indexing it costs a multiply. The loop
  must be a `for`, not the equivalent guard-plus-`do`: the indexed access under
  a `do` costs one instruction, under a `for` it costs none. 27 -> 24.
- The two display-list commands are two carriers, not one reused local. A
  single `command` coalesces them into one colour; two names give the target's
  two. 24 -> 12 once the declaration count is paid for.
- `vertexAddress` was never a local. Spelling `(s32)vertexStart + 0x80000000`
  at both uses frees the tenth declaration slot the second carrier needs, and
  the frame stays `0x80`: eleven locals move it to `0x88`.
- `displayList` is the *fifth* declared local. Its home is the function's only
  addressed stack slot (`&displayList` reaches `func_800349A4`), so the
  declaration order alone moves it between `sp+104` and the target's `sp+108`;
  only one of the ten positions is right. 12 -> 3.
- The last three words were a redundant read: a pre-guard
  `pointCount = line->pointCount` gives the point count a pool colour before
  the guard. Testing `line->pointCount >= 2` directly and reading the count
  only after the call is what puts it in the target's colour. 3 -> 0.

`func_8003D25C`: before/after allocation mismatch, 168 instructions and 70 register-only words, first `+0x50`.
Type lever: pool/render-resource aggregates and vector aliases; no allocation movement. Remains temp slot 0 and pool substitutions; asm stays canonical.

| Function | Result |
|---|---|
| `vsprintf` | **tier-A**, ROM `0x435D0`, `0x1310` bytes, `-Wab,-r4300_mul`; JFG body and formatter data/tables are compiler-owned by `main/diprint`, linked exact. |
| `debug_text_parse` | **tier-A**, ROM `0x44EDC`, `0x41C` bytes, `-Wab,-r4300_mul`; JFG body with diprint BSS/rodata ownership, linked exact; asm-object aliases remain compiler-local. |
| `diPrintfAll` | **tier-A**, ROM `0x449D0`, `0x240` bytes, default resident flags; JFG body with diprint-owned BSS, linked exact. |

| Function | Exact result |
|---|---|
| `debug_text_character` | **tier-A**, ROM `0x45398`, `0x2E8` bytes, `-Wab,-r4300_mul`; an aligned width union recovers the target `0x18` frame and all 186 instructions/relocations. JFG body donor. |

`func_80040B88`: before/after structure mismatch, 302 instructions and 160 words; frame remains `0x70` vs target `0x68`.
Type lever: line-entry/config/vector aggregates; trigger spill and temporary lanes did not move. Remains the pool web; DKR `update_line_particle` and JFG are the oracles.

`func_80041530`: before/after structure mismatch, target 456/C 457 instructions and 280 words; frame remains `0x160` vs `0x168`.
Type lever: model particle pointers and vector arrays; the software pipeline did not change. Remains the input-vector construction gap; JFG `func_80062BFC` is the oracle.

`func_8003FB98` reached a bounded near-exact plateau at the target's full 621
instructions, `0x38` frame, and relocation surface. Ten words remain, first at
function offset `0x10C`: six stack operands place the reused scale value at
`sp+0x20` rather than the target's `sp+0x24`, while two adjacent FP instruction
pairs schedule the Z square before the X/Y partial sum. Ten coherent source,
type, and expression-shape attempts plus the full flag lattice found no exact
form. The bounded permuter used inadmissible `-mips1` and only moved its internal
score from 19985 to 19900. The body is adapted from DKR
`create_general_particle` and cross-checked against JFG `func_80060ED4`; it
remains under `NON_MATCHING` and asm remains canonical.

No function in either range uses an odd single-precision floating-point
register. None is therefore classified as handwritten assembly by §6.2's
criterion.
`func_800336A8` is now exact canonical C; see the tier-A proof in §3.8.

| Function | Exact result |
|---|---|
| `func_800336A8` | Tier A: 780 executable bytes, 195 exact words, `0x28` frame, all 77 static relocation identities exact, no padding before `func_800339B4`; linked ROM `0x342A8`–`0x345B4` and full ROM exact. JFG `src/gameVi.c:viChangeMode` donor pair, with Mickey-specific paths retained (§3.8). |
| `func_80034094` | 188 bytes under `-O2 -mips2 -32`; JFG `src/gameVi.c::viGetOsViMode` body, all 47 instruction words exact, with its 48-byte compiler-owned switch table in `main/gameVi` `.rodata`. |
| `src/saves.c.o`, `src/rcpFast3d.c.o`, `src/track.c.o`, `src/textures.c.o`, `src/diCpu.c.o`, `src/objects.c.o`, `libultra/src/flash/flashreadid.c.o`, `us.v10/src/core1/code_1D00.c.o` (BK) | 1 each | single points | Isolated identifications, no span to claim |

**Why the rows do not establish new internal boundaries.** §1's "measured file
boundary" tier requires a whole-`.text` match; this pass only matched individual
functions (`tools/find_known_objects.py --sections` found no whole-object match
for any of the not-yet-named TUs above). The later `main/anim` split (§3.4)
therefore preserves the pre-existing, 16-byte-aligned `0x50C00`–`0x58570`
block in one piece. It is source ownership, not a claim that the whole range is
JFG's `anim.c`. The already-measured TUs above (`n_csplayer`, `gsSnd`,
`n_drvrNew`, `n_env`, `n_load`, `math_util`) needed no new split; they already
have one.

### 3.4 The `main/anim` source-owning block

ROM `0x50C00`–`0x58570`, VRAM `0x80050000`–`0x80057970`, is now one C
subsegment with 55 function starts. This is deliberately a source-ownership
boundary, not a donor-TU identity claim. The first 34 functions follow JFG's
`anim.c` family; the following code has JFG `hit.c` shapes; the final function
has the exact masked skeleton of JFG's `fmvInit`. There is no proved,
16-byte-aligned internal object boundary, so the old yaml block remains intact.

PROVENANCE: the comparison names below come from Jet Force Gemini's public
decompilation (`src/anim.c`, `src/hit.c`, `src/fmv.c`, their built objects, and
their public declarations), permitted by `docs/CLEANROOM.md`. They are
comparison labels, not silently adopted Mickey symbols. Tier A rows were
rechecked with relocation-aware byte comparison and `romocc=1`; tier B rows
are pinned by the within-block call graph; tier D rows are structural
similarity or source-order evidence only. Section 1.5 keeps the Mickey
`func_<VRAM>` name for every still-assembly function, and JFG address
placeholders are never imported.

| Mickey VRAM | Size | JFG comparison | Evidence / status |
|---|---:|---|---|
| `0x80050000` | `0x24` | `func_800767A0` | D naming; placeholder retained. Matched C: exact 36 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050024` | `0x80` | `func_800767C4` | D naming; placeholder retained. Matched C: exact 128 B and relocation surface at `-O2 -mips2 -32` |
| `0x800500A4` | `0x98` | `func_80076840` | D naming; placeholder retained. Matched C: exact 152 B and relocation surface at `-O2 -mips2 -32` |
| `0x8005013C` | `0x40` | `func_800768D4` | D naming; placeholder retained. Matched C: exact 64 B and relocation surface at `-O2 -mips2 -32` |
| `0x8005017C` | `0x30` | `func_80076918` | A; exact 48 B, masked `6/12`, placeholder retained. Matched C: exact 48 B and relocation surface at `-O2 -mips2 -32` |
| `0x800501AC` | `0x1C` | `func_80076948` | D naming; placeholder retained. Matched C: exact 28 B and relocation surface at `-O2 -mips2 -32` |
| `0x800501C8` | `0xB4` | `func_80076968` | D; 0.653 skeleton similarity, placeholder retained. Fresh Mickey m2c/JFG CFG reconstruction matches all 45 instruction words and relocation layout at `-O2 -mips2 -32 -Wo,-loopunroll,0` |
| `0x8005027C` | `0x50` | `func_80076A20` | A; exact 80 B, masked `9/20`, placeholder retained. Matched C: exact 80 B and relocation surface at `-O2 -mips2 -32` |
| `0x800502CC` | `0x7C` | `func_80076A70` | B; same cleanup callees and position, placeholder retained. Matched C: exact 124 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050348` | `0x214` | `animseqInitPath` | B; exact `animseqInitGroup` calls this function. Matched C: exact 532 B and relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; the canonical symbol records object and linked-ROM byte identity |
| `0x8005055C` | `0x12C` | `animseqResetPath` | B; reset/process callers and trap/audio call shape. Matched C: exact 300 B and relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0` |
| `0x80050688` | `0x7C` | `animseqStartPath` | B; process-command call position, adopted name. Matched C: exact 124 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050704` | `0x78` | `animseqStopPath` | B; process-command call position, adopted name. Matched C: exact 120 B and relocation surface at `-O2 -mips2 -32` |
| `0x8005077C` | `0x40` | no unique candidate | D; placeholder retained. Matched C: exact 64 B and relocation surface at `-O2 -mips2 -32` |
| `0x800507BC` | `0x88` | `animseqHoldPath` | B; process-command call position, adopted name. Matched C: exact 136 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050844` | `0x38` | `animseqLockPath` | B; paired process-command calls, adopted name. Matched C: exact 56 B and relocation surface at `-O2 -mips2 -32` |
| `0x8005087C` | `0x38` | `animseqUnLockPath` | B; paired process-command calls, adopted name. Matched C: exact 56 B and relocation surface at `-O2 -mips2 -32` |
| `0x800508B4` | `0x20` | no unique candidate | D; placeholder retained. Matched C: exact 32 B and relocation surface at `-O2 -mips2 -32` |
| `0x800508D4` | `0x200` | `func_800772C4` | B; JFG structural oracle. Matched C: exact 512 B and relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; the unsigned scale is the TU's own 0.01f literal (ROM `0x84BA8`), whose lwc1 joins the invariant group in the ROM's f26/f24/f22/f20 order |
| `0x80050AD4` | `0x120` | `animseqLinkNodes` | D; nearest ordered `anim.c` function. Matched C: exact 288 B and `D_800D6B00` relocation pair at `-O2 -mips2 -32 -Wo,-loopunroll,0` |
| `0x80050BF4` | `0x15C` | `animseqInit` | tier-D; type pass unchanged: structure-mismatch, 15 words at the exact 87-instruction/0x18-frame shape; lever earliest clear-loop branch, residual `li/bne` versus target `slti/bnez` plus address schedule |
| `0x80050D50` | `0x58` | `func_80077784` | D; nearest `anim.c` skeleton, placeholder retained. Matched C: exact 88 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050DA8` | `0x48` | `animseqFreeLevelData` | B; frees storage then the group, adopted name. Matched C: exact 72 B and relocation surface at `-O2 -mips2 -32` |
| `0x80050DF0` | `0xAC` | `animseqLoadLevelData` | tier-D; nearest ordered `anim.c` function, placeholder retained. Matched C: exact 172 B and relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; declaring the source scalar before the bounds pointer recovers the target spill home |
| `0x80050E9C` | `0x168` | `animseqFreeGroup` | Tier A: ordinary full-TU C and linked ROM are exact over 360 executable bytes / 90 words, frame `0x20`, no padding, and all 41 relocation identities. Same-line cursor setup plus an integer XOR-zero loop test and algebraically zero reset value preserve the required allocation with unchanged configured flags and untouched compiler instructions. Tier B: JFG cleanup role correspondence remains separate from byte proof |
| `0x80051004` | `0xE4` | `animseqSetupGroup` | B; calls free/init/reset group family. Matched C: exact 228 B and relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; explicit four-byte directory and signed level-header types resolve the scan allocation |
| `0x800510E8` | `0x40` | `animseqInitGroup` | A; exact 64 B, masked `1/16`, adopted name. Matched C: exact 64 B and relocation surface at `-O2 -mips2 -32` |
| `0x80051128` | `0x9C` | `animseqResetGroup` | B; calls reset-path family, adopted name. Matched C: exact 156 B and relocation surface at `-O2 -mips2 -32` |
| `0x800511C4` | `0x1A0` | `func_80077BE8` | A; exact 416 B and 11 relocation sites at `-O2 -mips2 -32 -Wo,-loopunroll,0`; matched C and linked ROM, comparison placeholder retained |
| `0x80051364` | `0x47C` | `animseqUpdate` | tier-D; before/after type pass: structure-mismatch, 192 words and 287 instructions; lever frame/lifetime, residual candidate `0x48` frame versus target `0x40` |
| `0x800517E0` | `0x1C40` | `animseqProcessCommandList` | tier-B; GLOBAL_ASM-only, shared type pass not applicable; residual direct command dispatcher assembly |
| `0x80053420` | `0x90` | `animseqCamera` | D; ordered tail and nearest same-family shape. Matched C: exact 144 B and relocation surface at `-O2 -mips2 -32` |
| `0x800534B0` | `0x10` | `animseqPlay` | D adoption; ordered JFG tail and the `playing = 1` store. Matched C: exact 16 B and relocation surface at `-O2 -mips2 -32`; skeleton remains too short for tier A |
| `0x800534C0` | `0x2C` | `animseqPause` | D; ordered `anim.c` tail only, so the placeholder remains. Matched C: exact 44 B and relocation surface at `-O2 -mips2 -32`; the overwritten formal counter is required for IDO's target `$a0` allocation and has no static Mickey caller |
| `0x800534EC` | `0x64` | no unique `hit.c` candidate | D; placeholder retained at the start of collision-shaped code. Matched C: exact 100 B and relocation surface at `-O2 -mips2 -32` |
| `0x80053550` | `0x318` | `hitInitObjectHit` | tier-B; before/after type pass: operand-mismatch, 16 stack-offset words with exact 198-instruction schedule; lever stack homes, residual target/candidate workspace and hit-pointer homes |
| `0x80053868` | `0x12D4` | `hitUpdate` | tier-B; GLOBAL_ASM-only, shared type pass not applicable; residual collision dispatcher assembly |
| `0x80054B3C` | `0x5C8` | no unique `hit.c` candidate | tier-D; GLOBAL_ASM-only, shared type pass not applicable; residual collision/vector assembly |
| `0x80055104` | `0x6F4` | no unique `hit.c` candidate | tier-D; GLOBAL_ASM-only, shared type pass not applicable; residual collision/vector assembly |
| `0x800557F8` | `0x178` | no unique `hit.c` candidate | D; collision handler family. Matched C: exact 376 B and eight-call relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; reconstructed from Mickey's resident state/counter/audio ABI after no external skeleton exceeded 0.070 similarity |
| `0x80055970` | `0x1B4` | no unique `hit.c` candidate | D; placeholder retained. Matched C: exact 436 B and relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; uopt homes declared locals at descending offsets in declaration order, so firstVehicle declared second lands its call-crossing spill at the target `sp+0x48` |
| `0x80055B24` | `0x1E4` | no unique `hit.c` candidate | tier-D; matched C: exact 484 B, 121 instructions, `0x50` frame, FP schedule, and nine-call relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`. A typed `void` weak alias for the first collision callback removes the generic trap placeholder's phantom return web; metadata-only rebinding restores the measured `TrapDanglingJump` relocation identity. |
| `0x80055D08` | `0x148` | no unique `hit.c` candidate | D; collision handler family. Matched C: exact 328 B and three-call relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; composed from Mickey-local exact state-update and normalization patterns |
| `0x80055E50` | `0x114` | no unique `hit.c` candidate | D; collision handler family. Matched C: exact 276 B and three-call relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0` |
| `0x80055F64` | `0x16C` | no unique `hit.c` candidate | D; collision handler family. Matched C: exact 364 B and three-call relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; declare volatile `secondZ` before `secondY` to retain the target stack home |
| `0x800560D0` | `0x1A4` | no unique `hit.c` candidate | D; collision handler family. Matched C: exact 420 B and three-call relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; composed from Mickey-local dual-state advance and normalization patterns |
| `0x80056274` | `0x140` | no unique `hit.c` candidate | D; collision handler family. Matched C: exact 320 B and three-call relocation surface at `-O2 -mips2 -32 -Wo,-loopunroll,0`; the Mickey-led declaration order fixes the two target-pointer spill homes without importing a donor body |
| `0x800563B4` | `0xA24` | `hitVectorCheck` | tier-B; GLOBAL_ASM-only, shared type pass not applicable; residual vector/cylinder/sphere assembly |
| `0x80056DD8` | `0x394` | no unique `hit.c` candidate | tier-D; before/after type pass: structure-mismatch, 214 words with 226 versus 229 instructions; lever local/frame lifetime, residual candidate `0x80` frame versus target `0x70` and FP phase |
| `0x8005716C` | `0x140` | no unique `hit.c` candidate | tier-D; the `NON_MATCHING` body and adjacent `func_800572AC` boundary authenticate a unique `src/main/anim.c.o` owner under `-O2 -mips2 -32 -Wo,-loopunroll,0`. Matched C: exact 320 B, `0x28` frame, exact 2-record relocation surface. Seven declarations put the one spilled cell at the target's home, and an explicit `(f32)` cast moves the x-axis product's left operand -- workbench L92 read on a float multiply. |
| `0x800572AC` | `0xA4` | no unique `hit.c` candidate | D; collision handler, placeholder retained. Matched C: exact 164 B and relocation surface at `-O2 -mips2 -32` |
| `0x80057350` | `0x78` | no unique `hit.c` candidate | D; collision handler, placeholder retained. Matched C: exact 120 B and relocation surface at `-O2 -mips2 -32` |
| `0x800573C8` | `0x3A4` | no unique `hit.c` candidate | tier-D; before/after type pass: structure-mismatch, 203 words at 231 versus 233 instructions and exact `0x88` frame; lever pointer/FP phase, residual two folded pointer initializations |
| `0x8005776C` | `0x1A4` | `hitPlayer` | tier-B; before/after type pass: mixed structural/register, 51 words at exact 105 instructions/`0xC0` frame; lever radius/call/register web, residual target workspace/register rotation |
| `0x80057910` | `0x5C` + `0x4` pad | `fmvInit` | A; exact masked JFG skeleton and C donor, adopted name. Matched C: exact 92 executable B and relocation surface at `-O2 -mips2 -32`; trailing 4 B is compiler alignment padding and earns no function credit |
| `0x80057970` | `0x50` | `osFlashReInit` | A; JFG `libultra/src/flash/flashreinit.c` body. Matched C: exact 80 B and 2/2 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x800579C0` | `0xF4` | `osFlashInit` | B; JFG `libultra/src/flash/flashinit.c` name/body and flash-init call role; JFG built object is 0xF0 B with different TU scheduling. Matched C: exact 244 B and 16/16 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x80057AB4` | `0x90` | `osFlashReadStatus` | A; JFG `libultra/src/flash/flashreadstatus.c` body. Matched C: exact 144 B and 6/6 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x80057B44` | `0xCC` | `osFlashReadId` | B; JFG `libultra/src/flash/flashreadid.c` name/body and flash-ID call role; JFG built object is 0xD4 B with different TU scheduling. Matched C: exact 204 B and 21/21 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x80057C5C` | `0xCC` | `osFlashAllErase` | B; JFG `libultra/src/flash/flashallerase.c` name/body and chip-erase role; JFG built object is 0xD0 B with different TU scheduling. Matched C: exact 204 B and 7/7 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x80057D28` | `0xD4` | `osFlashSectorErase` | B; JFG `libultra/src/flash/flashsectorerase.c` name/body and sector-erase role; JFG built object is 0xD8 B with different TU scheduling. Matched C: exact 212 B and 7/7 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x80057E84` | `0xD0` | `osFlashWriteArray` | B; JFG `libultra/src/flash/flashsectorerase.c` name/body and page-program role; JFG built object is 0xD4 B with different TU scheduling. Matched C: exact 208 B and 6/6 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x80057DFC` | `0x88` | `osFlashWriteBuffer` | B; JFG `libultra/src/flash/flashsectorerase.c` name/body and DMA-write role; JFG built object is 0x84 B. Matched C: exact 136 B and 6/6 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x80057F54` | `0xB4` + `0x8` pad | `osFlashReadArray` | B; JFG `libultra/src/flash/flashreadarray.c` name/body and read-DMA role; JFG built object is 0xB4 B. Matched C: exact 180 executable B and 11/11 relocation identities at `-O2 -mips2 -32`, linked ROM exact; following eight bytes are target padding |
| `0x80058010` | `0x6C` | `func_80058010` | D; initializes/checks the Transfer Pak connector and records presence. Matched C: exact 108 B and 12/12 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x800581BC` | `0x1C` | `func_800581BC` | D; sets the Transfer Pak save-action flag. Matched C: exact 28 B and 2/2 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x800581D8` | `0x34` | `func_800581D8` | D; records one Transfer Pak save-action state triple and sets its flag. Matched C: exact 52 B and 8/8 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x8005820C` | `0x34` | `func_8005820C` | D; records the other Transfer Pak save-action state triple and sets its flag. Matched C: exact 52 B and 8/8 relocation identities at `-O2 -mips2 -32`, linked ROM exact |
| `0x80058240` | `0x10` | `func_80058240` | D; returns the resident Transfer Pak pointer slot. Matched C: exact 16 B and 2/2 relocation identities at `-O2 -mips2 -32`, linked ROM exact |

No function in this block directly references a distinctive string. Its
references into `0x80083FA8`–`0x80084218` are floating-point constants, so no
tier-C name is available. A scan of every function found no odd
single-precision FP register operand; §6.2 therefore parks none of this block
as hand-written assembly on that criterion.

### 3.4 The resident shadows and lights TUs

ROM `0x18FF0`–`0x1AE60` holds `main/shadows` and `main/lights`. JFG's exact
`shadowMakeYs` ends at ROM `0x19310` (VRAM `0x80018710`), where its `lights.c`
call graph begins; Mickey shares the order and alignment (tier B). Shadows has
all 88 odd-FP operands and stays assembly under §6.2; lights has none.

PROVENANCE DISCLOSURE. Comparisons use JFG's permitted public
`src/{shadows_214A0,lights}.c` and `src/lights.h`.

`func_80019DE8` owns VRAM `0x80019DE8..0x80019EE4`, ROM
`0x1A9E8..0x1AAE4`: 252 bytes/63 words, frame `0x38`, and no padding before
`lightSetupLightSources`. Matched exactly under `-O2 -mips2 -32
-Wab,-r4300_mul`, with the `mathOneFloatRPY` R_MIPS_26 record at `+0xBC` and
the `D_800CB290` HI16/LO16 pair at `+0xC8/+0xE0` at the target offsets. ORT
358 exports offset `0x19998`, with zero resident-runtime or overlay inbounds;
direct callers are `lightDefaultObjectLight+0x38` and
`func_8001A008+0x74/+0xC4`, and no stored pointer exists.

The last residual was a uniform two-slot rotation of ugen's temporary ring
from the delta web onward, with every opcode, offset and relocation already
exact. Reading the two bytes back out of the structure --
`state->valueDelta = state->endValue - state->startValue` -- spends the two
ring temporaries the target spends there; uopt forwards both stores, so the
two loads leave no instruction behind and only the ring position moves. JFG's
`lightSetObjectLight` remains assembly-backed structural evidence, not donor C.

| Mickey VRAM | Size | JFG namesake | Evidence / disposition |
|---:|---:|---|---|
| `0x800183F0` | `0xC4` | `shadowBoxPolyOverlap` | Tier A: 49/49 unmasked words, ROM-wide unique; already adopted |
| `0x800184B4` | `0x90` | `shadowBoundingBox` | Tier A: 36/36 unmasked words, ROM-wide unique; already adopted |
| `0x80018544` | `0x110` | `shadowYHeight` | comparison only: unique nearest 4-gram skeleton, 0.919; remains `func_80018544` |
| `0x80018654` | `0xBC` | `shadowMakeYs` | Tier-A candidate: 47/47 unmasked words, ROM-wide unique; assembly pending a function-sized naming commit |
| `0x80018710` | `0x8C` | `freeLights` | Tier A: JFG-adapted C is compiler/link exact |
| `0x8001879C` | `0x130` | `setupLights` | Tier B: JFG-adapted C is compiler/link exact at 76/76 words |
| `0x800188CC` | `0xB0` | JFG placeholder `func_80020D94` | Tier A: Mickey/DKR-adapted C is compiler/link exact; placeholder remains prohibited by §1.5 |
| `0x8001897C` | `0x238` | `addRomdefLight` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x80018BB4` | `0x200` | `addObjectLight` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x80018DB4` | `0x10` | `turnLightOff` | Tier A: JFG-adapted C is compiler/link exact |
| `0x80018DC4` | `0x10` | `turnLightOn` | Tier A: JFG-adapted C is compiler/link exact |
| `0x80018DD4` | `0x10` | `toggleLight` | Tier A: JFG-adapted C is compiler/link exact |
| `0x80018DE4` | `0x2C` | `changeLightColour` | Tier A: JFG-adapted C is compiler/link exact |
| `0x80018E10` | `0x20` | `changeLightColourCycle` | Tier A: 7/8 unmasked words, ROM-unique; linked C is byte-exact and adopted |
| `0x80018E30` | `0x4C` | `changeLightIntensity` | Tier A: JFG-adapted C is compiler/link exact |
| `0x80018E7C` | `0x8C` | `lightUpdateLights` | Tier A: JFG-adapted C is compiler/link exact |
| `0x80018F08` | `0x334` | JFG placeholder `func_80021444` | Evidence D candidate: 199/205 instructions, 166 differing words, first `+0x0`, frame `-0x60` versus `-0x48`, and 14 relocation-symbol sites differ; not shape-exact |
| `0x8001923C` | `0x104` | `killLight` | Tier A: Mickey/DKR-adapted C is compiler/link exact |
| `0x80019340` | `0x18` | `lightGetLights` | Tier A: JFG C and both global relocations are link exact |
| `0x80019358` | `0x13C` | `lightGetStrongestEffect` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x80019494` | `0xA8` | `lightUpdateObjects` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x8001953C` | `0x3F8` | JFG placeholder `func_80021B9C` | placeholder prohibited; remains `func_8001953C` |
| `0x80019934` | `0xF0` | `lightDistanceCalc` | Tier A: JFG-adapted C and the five-entry compiler-owned switch table are linked byte-identically |
| `0x80019A24` | `0x94` | `lightDirectionCalc` | Tier A: JFG C is compiler/link exact |
| `0x80019AB8` | `0x2E0` | `lightObject` | Evidence D: Mickey reconstruction is compiled as a candidate: 191/184 instructions, 178 differing words, first `+0x0`, frame `-0xD0` versus `-0xC8`, and 54 relocation-symbol sites differ; not shape-exact |
| `0x80019D98` | `0x50` | `lightDefaultObjectLight` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x80019DE8` | `0xFC` | `lightSetObjectLight` | tier-D boundary; matched exactly. The two ring temporaries the target spends before the delta web are the two field re-reads in `state->valueDelta = state->endValue - state->startValue`. |
| `0x80019EE4` | `0x98` | `lightSetupLightSources` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x80019F7C` | `0x8C` | `lightSetupFlareSources` | Tier A: Mickey/JFG-adapted C is compiler/link exact |
| `0x8001A008` | `0x14C` | `lightInitObjectLighting` | tier-B comparison; `NON_MATCHING` plateau after the flag lattice and nine source/declaration forms: exact 83-word frame/opcode/register/FP/relocation shape, but 4 positional words differ, first `+0x70`, because the call-live result spills at `0x28(sp)` instead of `0x2C(sp)`; the permuter importer scores the isolated function zero, but the required full-TU build retains this mismatch |
| `0x8001A154` | `0xE8` | `lightAdjustGlowingLight` | tier-B comparison; `NON_MATCHING` workbench `register-ring-only` plateau after lifetime/mask/scaled-size levers and a 30-minute permuter batch: exact 58-word shape and call relocation, 13 register-only differences from `+0x1C`; the isolated permuter lead regresses in the full TU |
| `0x8001A23C` | `0x24` | `lightKillGlowingLight` | Tier A: Mickey/JFG-adapted C is compiler/link exact; Mickey uses a no-argument delete wrapper |

### 3.4 Resident controller, level and main TUs

ROM `0x25C20`-`0x2A250` is 17,968 bytes (`0x4630`) containing 108
functions. A per-function census recorded every boundary, direct caller and
callee, string reference, and the top five masked n-gram neighbours from
`tools/skeleton_scan.py`. No function in the range uses an odd-numbered
single-precision floating-point register, so §6.2 does not force any of these
functions to remain hand-written assembly.

| Canonical TU | ROM / VRAM | Bytes | Functions | Evidence |
|---|---|---:|---:|---|
| `main/joy` | `0x25C20`-`0x263F0` / `0x80025020`-`0x800257F0` | 2,000 | 19 | **Tier B:** exact ordered correspondence to JFG's controller setup/read, map accessors, stick clamp and CIC helper; Mickey's callers agree |
| `main/level` | `0x263F0`-`0x27760` / `0x800257F0`-`0x80026B60` | 4,976 | 21 | **Tier B:** exact ordered correspondence to JFG `level.c`; Mickey omits `levelGetWorldRegions` and four donor tail accessors |
| `main/main` | `0x27760`-`0x2A250` / `0x80026B60`-`0x80029650` | 10,992 | 68 | **Tiers B + C:** ordered main-state call graph plus six references to `main/main.c`; the last routine references the `x/y/z/a` coordinate readout strings |

The boundaries are all 16-byte aligned and are evidence-backed TU splits, but
they are not tier-A whole-object matches: no complete JFG object was
byte-identical. The strongest masked-skeleton anchors include
`levelUpdateColourCycling` (0.622), `levelGetNextOfWorld` (0.615),
`mainCPUeffects` (0.671), and the unnamed `func_80027EC0` (0.837). Three tiny
controller routines compare byte-identically with JFG under relocation masks,
but each has fewer than six unmasked words and therefore remains tier B under
§1.2 rather than being promoted to tier A.

The direct-call census supplies independent anchors. `joyResetMap` is called
by `joyInit`; the stick accessors converge on `joyClamp`; `levelInit` owns the
subsystem initialization/free fanout and `levelFreeAll` is reached from the
main-state loop; `mainThread` reaches `mainInitGame`, `joyRead`,
`mainChangeLevel` and `mainPreNMI`. The two large routines at `0x80026FB4` and
`0x80027FB8` build the six `main/main.c` string addresses. Placeholder-named
JFG functions were not imported: unresolved routines retain Mickey's own
`func_<VRAM>` symbol.

One initial combined symbol was corrected during reconstruction: the
108-byte routine at `0x80028E2C` has JFG `mainFrontInit`'s exact size,
top-ranked skeleton and call role; the independent return stub at
`0x80028E98` occupies JFG's following `mainStartGame` slot. Both names are
tier B because the complete donor bodies are not byte-identical.

The original 24-byte `func_80028F3C` range was likewise split at tier-D
structural boundaries: it consists of three consecutive independent
return/delay-slot islands at `0x80028F3C`, `0x80028F44`, and `0x80028F4C`.
Their placeholder names remain because JFG role attribution is not unique.

**Matching progress.** Ninety-six functions / 8,844 bytes compile exactly
under the resident `-O2 -mips2 -32` flags. Owned bytes, relocation identity,
linked ranges and the full ROM are exact.

- `main/joy` (17 / 1,032 bytes): `joyMessageQ`, `joyResetMap`, `joyDisable`, `joyEnable`,
  `joyCreateMap`, `joyGetController`, `joyGetButtons`, `joyGetPressed`,
  `joyGetReleased`, `joyGetStickX`, `joyGetAbsX`, `joyGetStickY`, `joyGetAbsY`,
  `joyClamp`, `joySetSecurity`, `arithmeticFunction`, and `joyCharVal`.
- `main/level` (18 / 1,404 bytes): `levelNGetType`, `levelGetTune`,
  `levelGetWorld`, `levelGetRegionNo`, `levelGetScreenMode`,
  `levelGetBlurEffect`, `levelGetGfxIndex`, `levelGetColourCycling`,
  `levelGetNumber`, `levelGetLevel`, `levelGetType`, `levelGetCamera`,
  `levelTunePlay`, `levelUpdateColourCycling`, `levelGetName`,
  `levelGetNextOfWorld`, `levelGetPrevOfWorld`, and `levelInitRegionFlags`.
- `main/main` (61 / 6,408 bytes): `RevealReturnAddresses`, `mainGetZBCheck`,
  `mainGameWindowChanging`,
  `mainGameWindowSize`, `mainCPUeffects`, `mainSetGameWindow`, `func_80027D14`,
  `mainSetAnimGroup`,
  `mainGetAnimGroup`,
  `mainChangeCameras`, `mainGetNextCharacter`, `mainGetNextLevel`,
  `func_80027628`, `mainAddZBCheck`,
  `func_80027EC0`, `func_80027FB8`, `func_800282C8`,
  `mainResetPressed`, `mainPreNMI`, `mainInitGame`, `mainChangeLevel`,
  `mainSyncNextLevel`,
  `mainGetMode`, `mainSetMode`,
  `mainTitlePageInit`,
  `mainFrontInit`, `mainStartGame`,
  `mainGetNumberOfCameras`, `func_80028DE4`, `func_80028EA0`,
  `func_80028EFC`, `func_80028F3C`,
  `func_80028F44`, `func_80028F4C`, `func_80028F54`,
  `func_80028F60`, `func_80028F98`,
  `func_80028FA8`,
  `func_80028FB8`,
  `func_80029038`, `func_8002904C`, `func_8002905C`, `func_80029084`,
  `func_800290A0`, `func_800290AC`,
  `func_80029090`, `func_800290EC`, `func_800290F8`, `func_80029104`,
  `func_80029120`, `func_80029144`, `func_80029160`, `func_8002917C`,
  `func_80029198`,
  `func_800291B4`,
  `func_800291C4`,
  `func_800291D0`, `func_800291D8`, `func_800291E4`, `func_800291FC`, and
  `func_80029240`, and `func_800293D0`.

The exact source preserves Mickey's six-byte level-summary and controller-pad
layouts, packed flag extractions, bounded/wraparound searches, and guarded
input calls. `arithmeticFunction` binds its three unavailable CIC-overlay
calls to Mickey's existing `TrapDanglingJump` relocations. `mainCPUeffects`
reproduces its cropped-framebuffer register allocation by beginning the
product-plus-base update before the independent height calculation; its typed
rain callback alias is folded back to the target `TrapDanglingJump` symbol in
ELF metadata without changing an instruction word.

Two ABI/name exceptions remain explicit. Mickey's `mainGameWindowChanging`
returns a 32-bit word, not JFG's declared `s16`; the JFG signature changed the
load and was rejected. `mainGetMode` is a tier-D paired-getter name correcting
an earlier positional setter attribution. `mainGetNumberOfCameras` is tier B
from JFG tail order plus the `levelGetGfxIndex` caller. `func_800291C4` is
consistent with `mainGetGameArrayPtr`, but not uniquely; `func_80028F54` has
the tier-B `mainGetGame` role but retains its placeholder because renaming it
would require out-of-scope overlay edits. The inherited `levelInitRegionFlags`
name is suspect: Mickey's exact 56-byte body is a boolean query over the level
type byte and `D_8007BF08`, not JFG's region-table initializer.

**Bounded plateaus (all remain assembly):**

- `joyInit`, eight source/storage hypotheses, the full flag lattice and a
  bounded two-worker canonical-MIPS-II permuter batch, first mismatch
  `+0x11C`: the JFG-shaped candidate is exact through the controller scan but
  compiles to 86 rather than 83 instructions. External `D_800CF3B4` storage
  makes IDO materialize four HI16/LO16 pairs for the final byte clears; the
  target shares one HI16 and names `D_800CF3B4` through `D_800CF3B7` in four
  distinct LO16 relocations. Alternative scalar and aggregate declarations
  disrupt the otherwise exact loop; exposing the named bytes in a block-scoped
  comma expression expands the function to 115 instructions. The permuter
  found no improvement from its base score of 325.
- `joyRead`, six loop/storage/type hypotheses, the full flag lattice and a
  bounded two-worker permuter batch, first mismatch `+0x18`: the JFG-shaped
  candidate has the exact 636-byte size, 159-instruction schedule and `-0x38`
  frame, but differs in 48 words. Original TU-local adjacency lets IDO name
  `D_800CF388`, `D_800CF3BC` and `D_800CF3B0` as three loop endpoints; the
  split extern layout materializes the preceding bases plus their array sizes,
  leaving six relocation-identity mismatches. The permuter's 5,795-to-5,305
  improvement required an invented do-while guard and was rejected.
- `func_80026FB4`, nine structural/display-command hypotheses, the full flag
  lattice and a bounded two-worker resident-MIPS-II permuter batch, first
  mismatch `+0x48`: the Mickey-derived main-loop candidate needs
  `-Wo,-Olimit,100` to reproduce the target's `-0x28` frame and transition
  result at `sp+0x24`, but compiles to 418 rather than 413 instructions. IDO
  assigns the first display-list pointer store through `$at` instead of the
  target's `$a0`; the remaining five-word structural excess is concentrated
  in the two end-of-frame display commands. The valid permuter score improved
  from 3,620 to 3,050 by introducing a matrix-array temporary, not identity.
- `func_80028564`: p2 workbench structure-mismatch, 492/489 instructions and 426 differing positional words.
  First mismatch `+0x4`; the exact frame still carries one unused saved-register web.
  Lever 1 and volatile/address-alias probes were byte-identical, leaving that web unresolved.
- `mainThread`: exact **0xC8 bytes / 50 words** under `-Wo,-Olimit,100`; the
  literal RAM-end loop is compiled untouched, and a digest-guarded metadata
  pass restores `D_803FFFFC` HI16/LO16 at `+0x18`/`+0x28`.
- `mainUpdateZBCheck`: Evidence A exact C after bounded permutation; all 63
  instruction words, the `-0x48` frame, relocation identities, and linked ROM
  bytes match.
- `levelGetCounts`: historically measured pre-endpoint full-TU C owns exactly
  259 words, frame `0x58`, and all 37 offsets/types, scoring 255/259 raw and
  256/259 relocation-normalized; no attributable C object survives. Only 35/37
  identities were exact: raw first `+0x50` bound the endpoint pair to
  address-equivalent but identity-wrong `D_800CF3E0+0x40`. Normalized first
  `+0x13C`, with `+0x148/+0x154`, was one world-value carrier. A reformatted
  isolated import measured only 251/259 raw and 252/259 normalized. Current
  `D_800CF420` source is uncompiled, so its score, frame, and identities are
  unknown. ORT 518 has exactly one inbound, `overlay18Initialize+0x8`; the
  owned ROM `0x263F0..0x267FC` has no padding. Pinned JFG keeps the body in
  assembly; public JFG `src/level.c` is disclosed provenance input, not an
  exact donor. Linked equality proves fallback only. Run current V0, one
  historical control only on regression, exactly 119 configurations only if
  the three-site carrier reproduces, then one trace and one natural carrier
  form. A strict gain alone permits a 2,000-candidate/20-minute batch.
- `levelInit`, ROM `0x26A18..0x27228`, owns 516 words with no following padding.
  Retained pre-cleanup configured full-TU and isolated C reproduce 394/516 raw
  words, frame `0x80`, first `+0x238`, and all 110 static tuples (70 calls and
  20 HI16/LO16 pairs). That producer's unused volatile two-word stack pad makes
  the score diagnostic; the pad is removed and seven runtime trap identities
  now use their authenticated ABIs, so clean V0's score/frame are unknown.
  The first retained residual is in the level-header resource-list selector
  carrier, not a fog-load delay slot. Historical 12,975-to-12,580 MIPS-I
  permuter numbers are objective scores, not current word evidence.
  ORT 526 has sole caller `func_80028564+0x5F8`, with no runtime/overlay/pointer
  inbound. Eight runtime records inside the function resolve seven overlay
  callees; metadata-only alias rebinding preserves the shipped trap identity.
  Pinned JFG keeps the body in assembly; later public JFG C is structural and
  lifetime evidence only. Compile pad-free V0 and typed-alias V1, then one
  donor-supported `s16 tune` lifetime. If still nonexact, trace once and try at
  most one trace-supported natural form; no generic lattice/permutation absent
  a legal gain. No linked C/ROM proof survives.
  Proven level `.bss` ownership is `0x800CF3E0..0x800CF490`; the preceding
  configured `bss_gap_D800CF3C0..D800CF3E0` remains unresolved.
- `func_80028FCC`: fresh configured full-TU C with canonical
  `-Wo,-Olimit,100` reproduces 17/27 words, exact 108-byte boundary/frame
  `0x18`, first `+0x1C`, and all three `R_MIPS_26` calls to `func_80028FB8` at
  `+0x14/+0x30/+0x4C`. The natural normalized-result/shared-epilogue probe
  regressed to 25 words and moved the latter two calls, so it did not unlock
  another flag lattice or trace. ORT 663 is real, but exhaustive resident,
  overlay, direct-JAL, and pointer scans authenticate no caller. There is no
  padding, and linked equality proves fallback only. JFG's pinned
  `src/main.c::mainAnyoneHas` remains assembly and supplies only a structural
  analogue/TU-role comparison; no C body was adapted. Park pending a new
  source-faithful Boolean-lowering mechanism; do not repeat flags, trace, or
  generic permutation without a strict structural gain.
- `levelFreeAll`: fresh configured full-TU C reaches 112/117 words with the
  exact 468-byte extent, frame `0x28`, and first mismatch `+0x13C`. The
  `for`/parallel-indexed-array spelling closes the CFG, saved-register lanes,
  and all 36 static relocation offset/type/identity records. The five residual
  words are one ugen temp-FIFO swap between the resource-table base and masked
  index. All 119 flag configurations and fifteen bounded declaration, loop,
  join-store, expression, and temp-lifetime forms are nonexact. Linked equality
  remains fallback-only; resume with a bounded permuter or new FIFO-trace
  evidence, not another generic source/flag sweep.
- `func_80029274`, seventeen control-flow/parameter/register-lifetime
  hypotheses and the full flag lattice: the best canonical candidate has the
  exact 348-byte, 87-instruction boundary and `-0x10` frame, but differs in 39
  words, first at `+0x8`. Initializing the accumulators before copying the
  velocity correctly anchors `$f2`; IDO still hoists the first float argument,
  colors the velocity/distance webs as `$f12`/`$f16` rather than `$f14`/`$f12`,
  and reshapes the negative-velocity return path. The size-exact `-g3` probe
  reaches 38 differing words, first at `+0x14`, but is not exact and does not
  justify a TU flag override.

The full flag lattice produced no exact result for any of these plateaus; the
single one-word `-g3` improvement is recorded above and was not adopted.

**PROVENANCE.** TU identities and adopted function names are adapted from Jet
Force Gemini's published `src/{joy,level,main}.c` and built
`src/{controller,level,main}.c.o`, a permitted public retail-derived decomp
under `docs/CLEANROOM.md`. The tier-B/C evidence above comes independently
from Mickey's own function order, callers/callees and strings. Any C body
adapted during matching carries the same disclosure at its point of use.

### 3.4 The resident debug and effects run

The four assigned ROM runs in `0x45760`–`0x4BC40` total 25,808 bytes. Including
the already-measured 16-byte `main/get_stack_pointer` island, their continuous
span is 25,824 bytes in five source units. The four new C splits below own 75
functions; none uses an odd single-precision register,
so the hand-written-assembly test in §6.2 excludes none of them. The `fx` range
has 16,840 executable bytes and eight bytes of compiler alignment padding.

| ROM | Source unit | Functions | Tier | Evidence |
|---|---|---:|---|---|
| `0x45760`–`0x459C0` | `main/diRcpTrace` | 4 | B | JFG's `src/diRcpTrace.c` has the same four-function order and near-identical sizes. Mickey's scheduler/track callers and the trace-buffer consumer establish the roles. |
| `0x459C0`–`0x465B0` | `main/diRcp` | 18 | B/C | The complete GBI opcode/mode string set identifies the disassembler (C); `diRcpPrintDL` calls the same ordered helper family as JFG's `src/diRcp.c` (B). |
| `0x465B0`–`0x47A60` | `main/diCpu` | 14 | A/B/C | `diCpuTraceInit` is a 21-word Tier-A skeleton/object hit; `diCpuThread` is linked exact for 340 bytes under the resident flags; the exception/watchpoint strings identify the monitor (C), and the OS-thread/debug call graph follows JFG's `src/diCpu.c` (B). The end is pinned by the measured `get_stack_pointer` TU at `0x47A60`. |
| `0x47A70`–`0x4BC40` | `main/fx` | 39 | B/D | Mickey begins where JFG's `src/fx.c` reaches `fxFreeCone`: the cone and wake routines have the same allocator, texture, trigonometry and draw call graph in the same order (B). The later unresolved effects retain Mickey `func_` names (D). The next block contains JFG `font.c` hits, independently fixing the far end. |

The strongest `fx` call-graph pairs are structural rather than merely
positional: `fxAllocateCone` calls the allocator, texture loader and the same
three cone builders; `wakeSetupRipple` calls the alignment helper, texture
loader and `wakeAllocate`; `wakeUpdateRipple` calls `Arctanf` and
`wakeUpdate`; and `wakeDrawRipple` calls the texture setup/draw pair and
`wakeDraw`. The earlier JFG level-effect functions are absent, which is why
Mickey's TU begins at `fxFreeCone` instead of JFG's first `fx.c` symbol.

Pre-existing assembly callers still spell 18 of these targets as
`func_<VRAM>`. Those exported labels are retained in `symbol_addrs.us.txt`,
with the JFG identity and tier on the same row, until each function or its
caller becomes C-owned; this avoids pretending that a source-level rename is
already available to the stale generated caller assembly.

FX type-pass inventory (target widths/offsets; no source-body promotion):

| Function | Target-derived aggregate/global surface | Before → after; lever; remaining |
|---|---|---|
| `func_80046EC4` | `FxCone`: pointer words `+0,+4,+8,+C,+10`; bytes `+14..17`; floats `+18,+1C`; halfwords `+20..2A`; colors `+2C..32`. | GLOBAL_ASM → GLOBAL_ASM; cone aggregate; allocator/body not re-derived. |
| `func_800470B0` | `FxCone` bytes `+14..17`, pointer/output words `+8,+C,+10,+15`; generated vertex records; `func_8002A8BC/C0`. | Evidence D plateau: exact 149-word size and `-0x168` frame, 90 differing words, first `+0x44`; all three call identities are present but their offsets differ. The fixed-bound register web remains the next allocator lever. |
| `func_80047304` | `FxCone` `+10,+14,+15`; generated vertex records; `D_80083DE4`; trig helpers. | Evidence D candidate: structure-mismatch, 183/185 instructions, 176 differing words, first `+0x4`, exact `-0x180` frame, and 57 structural words remain; not shape-exact. |
| `func_800475E8` | `FxCone.texture +0`, `segmentCount +15`, `vertices +10`; `FxConeTextureInfo` `+6/+8`; `FxConeVertex` byte `+0/+1..3`, halfwords `+4..E`; `D_80083DE8`. | structure plateau → structure plateau (`440` raw words, first `+0`); structure-buckets; frame/code shape remains. |
| `func_800479D4` | `FxCone` `+8,+15..17,+18,+1C,+20..2A`; generated vertex records; trig helpers. | Evidence D candidate: structure-mismatch plateau, 179/193 instructions, 176 differing words, first `+0x40`, exact `-0x150` frame, and 80 structural words remain. |
| `func_80047CD8` | `FxGfx **`; `FxCone` words `+0..10`, bytes `+14..17,+2C..32`; `func_800349A4`. | Tier A: ordinary full-TU C is ROM-exact at `0x80047CD8..0x80048080` / ROM `0x488D8..0x48C80`: 936 executable bytes, 234 words, frame `0x68`, no padding, and four exact call relocation identities. Preserved display-list block grouping, a white RGB XOR-zero identity and a logical-negation test close the prior eight-word allocation residual. No compiler flags or post-compile instructions changed. Tier B: JFG `fxDrawCone` role correspondence remains separate from byte proof. |
| `func_80048080` | Output vertex records: bytes `+6..9`, halfwords `-A,-8,-6`; trig helpers. | **A**; exact C, 89 words, frame `-0x48`, 4 relocations; ROM `0x48C80`-`0x48DE4` byte-identical. Both cursors are the parameters, promoted into registers for the loop and written back at its exit; the rotated *y* component is a named local like the rotated z, and that fifth (coalesced, instruction-free) FP web is what puts x in f2 and z in f14. |
| `wakeAllocate` | `Wake`: bytes `+0..3,+38..3B`; float `+4,+C`; halfwords `+8`; pointers `+10,+14,+18..2C,+30`; halfwords `+34,+36`; word `+3C`. | GLOBAL_ASM → GLOBAL_ASM; `Wake` layout; allocation/initialization CFG remains. |
| `func_80048760` | `WakeRipple` 0x88-byte setup; texture/link `+70`; bytes `+74,+75`; halfwords `+76,+78,+7A`; floats `+7C,+80`; nested `Wake *+84`; source fields `+40,+54`. | **A**; exact C, 121 words, frame `-0x48`, 4 relocations; ROM `0x49360`-`0x49544` byte-identical. The two texture extents are `s32` locals, which is what orders the ugen temp ring; the white-fill loop indexes the record from `i` rather than carrying a cursor, which is what orders the loop preheader. |
| `wakeUpdate` | `Wake` `+0..14,+30,+34..3C`; `WakeRipple` texture/config `+70..84`; generated display records. | GLOBAL_ASM → GLOBAL_ASM; wake aggregate; update/draw scheduling remains. |
| `func_80049000` | `WakeRipple` `+54` link, `+70,+74..84`; `Wake` `+80,+84`; `Arctanf`, `wakeUpdate`. | Evidence D candidate: structure-mismatch, 150/149 instructions, 125 differing words, first `+0x0`, frame `-0x30` versus `-0x38`; ripple update/call surface remains but is not shape-exact. |
| `wakeDraw` | `Wake` `+30,+34,+38`; `FxGfx **`; `func_800349A4`. | Evidence D candidate: structure-mismatch, 163/177 instructions, 177 differing words, first `+0x0`, frame `-0x38` versus `-0x88`; command-loop structure remains unresolved. |
| `func_80049518` | `WakeRipple` `+70,+74,+76,+78,+84`; nested `Wake +3C`; `FxGfx **`; draw helpers. | GLOBAL_ASM → GLOBAL_ASM; ripple aggregate; display-list schedule remains. |
| `func_800498FC` | `FxRecord` `+0,+1,+2,+14,+16,+18,+1A..1F`; `D_800D5F58[5]`; camera helpers. | **A**; exact C, 100 words, 5 relocations, frame `-0x30`; ROM `0x4A4FC`-`0x4A68C` byte-identical. The two packed flag tests read the `u8` fields back (`record->value1E`), not the argument (`(u8)(flags & 0x80)`): the argument form CSEs a second long-lived web into the pool and rotates four registers. |
| `func_80049B14` | `D_800D5F50`; `FxRecord` `+0,+1,+2,+14,+16,+18,+1E,+1F`; five-record stride `0x20`. | Evidence D candidate: structure-mismatch, 219/206 instructions, 216 differing words, first `+0x8`, exact `-0x18` frame; switch-state schedule remains unresolved. |
| `func_80049E4C` | `D_800D5F50`, `D_800D5F58`, `D_800D5FD8`; `FxRecord` bytes/halfwords; `FxGfx`; VI/scissor helpers. | **A**; exact C, 169 words, 9 relocations, frame `-0x60`; ROM `0x4AA4C`-`0x4ACEC` byte-identical. Three source facts, in order: the record loop is `while (count--)` over one counter, not a counter plus a separate `remaining`; `count` is declared *before* the two address-taken VI-size locals, which is what puts their homes at `sp+0x58`/`sp+0x54`; and the single-record `count = 1` lives in the `else` arm, not ahead of the display-list setup. 37 -> 7 -> 3 -> 0 words. |
| `func_8004A10C` | `D_8007D320` u32 table, `D_8007D364` u8 glyph table; VI size; `FxGfx` command words. | Evidence D candidate: structure-mismatch, 156/157 instructions, 155 differing words, first `+0x0`, frame `-0x60` versus `-0x58`; glyph loop/VI relocation surface is preserved but not exact. |
| `func_8004A380` | `D_8007D364[12]` bytes, `D_80083DE0` text, `D_800D2FA0` screen pointer; local text buffer. | Evidence A exact C: all 76 instruction words, the `-0x80` frame, all 9 relocations, and linked ROM bytes match. |
| `fxSPDPRipple` | `D_8007D370[2]`, `D_8007D374[2]`, `D_8007D378[4]`; `FxGfx **`; level/draw helpers. | GLOBAL_ASM → GLOBAL_ASM; global table widths; ripple display CFG remains. |
| `fxScreenEffect` | `D_8007D380[10]`, `D_8007D3D0[7]`, `D_8007D408[14]` `FxGfx`; VI video mode and display helpers. | GLOBAL_ASM → GLOBAL_ASM; dlist aggregate; effect command CFG remains. |
| `func_8004ACC4` | `D_800D60A8` word; three parallel four-element slot arrays at `D_800D60B0`, `D_800D60C0` and `D_800D60D0`; `D_8007D47C` callback array; `TrapDanglingJump`. | **A**; exact C, 28 words, frameless, 12 relocations; ROM `0x4B8C4`-`0x4B934` byte-identical. The four cursors are IDO's own strength-reduced induction variables: the source indexes the three arrays with one loop variable, which is what puts the now-dead copy of that variable in the first pool colour and the synthesised trip counter in the second. |
| `func_8004ADE8` | `D_800D60A8`, `D_800D6098[4]`, `D_800D60B0[4]`, `D_800D60C0[4]`, `D_8007D47C[4]`; texture info `+6/+8`. | Tier A: ordinary full-TU C is ROM-exact at `0x8004ADE8..0x8004AF68` / ROM `0x4B9E8..0x4BB68`: 384 executable bytes / 96 words, frame `0x40`, no padding, and all 15 relocation offsets, types, and identities exact. Removing a never-read offset local recovered the JFG-homologous stack-home layout without changing semantics. Tier B: JFG `fxCpuTextureRequired` supplies the role and source-topology context, not Mickey's byte proof. |
| `func_8004AF68` | the four parallel slot arrays `D_800D60B0`, `D_800D60C0`, `D_800D60D0` and `D_8007D47C`; `D_800D60A8`; `TrapDanglingJump`; `mmFree`. | **A**; exact C, 52 words, frame `-0x38`, 12 relocations; ROM `0x4BB68`-`0x4BC38` byte-identical. One `while (i--)` index over all four arrays; uopt builds every cursor, shares one byte offset between `D_800D60C0` and `D_8007D47C`, and keeps `D_800D60C0`'s base inside the loop. |

Exact C closures in these splits begin with 680 bytes across seven `diCpu`
functions: the 8-byte `func_80046504` (`diCpuTraceGetFault` in JFG) and the
60-byte `func_8004650C` (`diCpuTraceTick`). Their natural return-zero and
60-tick counter bodies are identical under the resident `-O2 -mips2 -32`
rule; the getter has no relocations and the tick routine retains both exact
HI16/LO16 data pairs. Five JFG `diRcp` return-eight leaves are also exact at
the resident defaults with no relocations: 16-byte `diRcpTexDma`, 20-byte
`diRcpStrNameMacro`, 12-byte `diRcpPrimColor`, 20-byte `diRcpColor`, and
12-byte `diRcpDmaOffsets`. Six 52-byte JFG unpack-and-return bodies,
`diRcpVertex`, `diRcpPolygon`, `diRcpMatrix`, `diRcpDPBlock`,
`diRcpViewport`, and `diRcpDisplayList`, are exact at the same defaults,
including their helper-call relocations and source-specific stack frames. The
52-byte `diRcpStrName` formatter is exact as well, including its format-string
and `sprintf` relocations. The 44-byte `func_80044B9C` (`diRcpTraceReset`) is
exact too, including both data-symbol relocation pairs.
The 60-byte `diRcpTraceInit` is likewise exact, preserving both allocator
calls and their call/data relocations. The 60-byte JFG-identified `wakeFree`
is exact after resolving `func_800347A0` as a one-argument call; its two call
relocations and the wake-linked field access match without normalization. The
same ABI resolves the adjacent 72-byte `func_80048980` (`wakeFreeRipple`),
which is exact with both its linked-release and nested-wake call relocations.
The 204-byte `func_80044BC8` (`diRcpTrace` in JFG) is exact too. Its typed
three-word trace entries and direct global-index expressions reproduce all 51
target words, including the repeated buffer/count reloads, the 100-entry
limit, and all four data relocation pairs at the resident defaults.
The 100-byte `stop_all_threads_except_main` is exact on Mickey's active-thread
walk: it filters priorities 1 through 127, passes the thread itself to
`osStopThread`, and retains the exact call relocation and 32-byte frame under
the resident defaults.
The 60-byte `Decode_gDma1p` display-list unpacker is exact too: its four typed
word extractions reproduce all 15 target instructions at the resident defaults
and have no relocation surface.
Its adjacent 60-byte `Decode_gMoveWd` unpacker is likewise exact: the alternate
24/16/8-bit field split retains all 15 target instructions and has no
relocations under the same flags.
| Function | Exact result |
|---|---|
| `diRcpPrintDL` | 1,540 bytes under `-O2 -mips2 -32`; JFG `src/diRcp.c` body, all 385 instruction words exact, with three compiler-owned switch tables in `main/diRcp` `.rodata`. |
| `diRcpMoveWd` | 156 bytes under `-O2 -mips2 -32`; JFG `src/diRcp.c` body, all 39 instruction words exact, with its compiler-owned switch table in `main/diRcp` `.rodata`. |
| `diRcpOtherMode` | 520 bytes under `-O2 -mips2 -32`; Ryan Myers' JFG commit `db755880d` body, all 130 instruction words exact, including the `sprintf` relocation and 128-byte frame. |

The 268-byte `diRcpGeometryMode` helper is exact at the resident defaults.
JFG's object-like `stubbed_printf` macro preserves the target's empty geometry-
flag switch and its otherwise-unused saved registers, reproducing all 67 owned
instructions plus the `sprintf` call and format-symbol relocations. The two
following target words are end-of-TU alignment padding outside the function;
IDO supplies them through normal section alignment, and the linked range is
byte-identical without post-compile editing.
The same JFG update moves the diagnostic strings into natural source
expressions. IDO emits Mickey's 2,876-byte string block at ROM
`0x83940`–`0x8447C` byte-identically, followed by the four exact switch tables.
The four bytes after the complete `0xC3C` object-owned `.rodata` range are
linker padding and remain outside the input section.
The 84-byte `diCpuTraceInit` is exact at the resident defaults. Keeping JFG's
distinct thread-control-block and stack-top declarations reproduces the target
evaluation schedule; Mickey resolves both operands to the same address, so the
linked function and its three call/data relocation pairs are exact.
The 88-byte `func_80046E00` screen-clear helper is also exact at the resident
defaults. JFG's natural framebuffer pointer/countdown loop reproduces all 22
owned instructions and the `viGetCurrentSize` plus framebuffer relocations;
the two following target words are alignment padding outside the function.
The 136-byte `func_80045CAC` active-thread scanner is exact as well. JFG's
natural thread-list loop reproduces all 34 owned instructions, including both
branch-likely paths, the 32-byte frame, the active-queue relocation, and both
crash-handler call relocations at the resident defaults.
The 140-byte `cpuXYPrintf` formatter is exact too. JFG's 255-byte local text
buffer and natural varargs setup reproduce the 288-byte frame, all 35 target
instructions, both call relocations, and the display-mode data relocation at
the resident defaults.
The 164-byte `diCpuReportWatchpoint` reporter is exact as well. JFG's natural
100-iteration clear loop, address-information query, two diagnostic prints,
and terminal wait reproduce all 41 target words, the 56-byte frame,
and every call and string relocation at the resident defaults.
The 84-byte `func_80046E70` (`fxFreeCone`) is exact too: two distinct texture
handle locals reproduce the target's direct second argument register and
branch-delay schedule, with both texture-free calls and the allocator call
retaining their exact relocations under the resident defaults.
The adjacent 52-byte `func_8004707C` is exact without relocations: its six
full-width value parameters are stored into byte fields only after the null
check, preserving the target's leaf schedule under the same default flags.
The 108-byte JFG-identified `fxQueueScreenEffect` is also exact: expressing
the four-entry queue selection as an array subscript with a post-incremented
global count reproduces the target's 20-byte offset schedule and both data
relocation pairs under the resident defaults.
Its 172-byte dequeue sibling `func_8004A9CC` (`fxUnQueueScreenEffect`) is exact
on the natural pointer/count loop, including the 64-byte frame, all nine
arguments to `fxScreenEffect`, the call relocation, and both queue-global
relocation pairs.
The 60-byte Mickey-named `func_80049828` bounds-checks one of five effect
records and tests a caller-supplied flag mask; its natural 32-byte-stride
record access is exact at the resident defaults, including the data-symbol
relocation pair. Its adjacent 56-byte `func_80049864` sibling tests a byte
status field with the same bounds and stride and is exact under the same flags,
also with the target's data-symbol relocation pair. The following 96-byte
`func_8004989C` packs the record's RGB bytes into a duplicated 16-bit color;
the typed record body, expression schedule, and data relocation are exact at
the resident defaults. The 28-byte `func_8004A0F0` clears two adjacent effect
queue words and their index; its three stores and both data relocation pairs
are exact under the same defaults. The 108-byte `func_8004A4B0` appends a
compact eight-byte record to one of two four-entry effect queues. Its natural
post-incremented count subscript and field-order assignments reproduce all 27
target instructions plus the three data relocation pairs under the resident
defaults.
The 164-byte `func_8004A51C` consumes the completed half of that double-buffered
queue and clears the newly selected half. Keeping the record's final fields
unsigned and spelling the queue toggle as a global assignment followed by a
global-indexed clear reproduces all 41 target words, the 40-byte frame, the
five-argument helper call, and all data/call relocations at the resident
defaults.
The 76-byte JFG-identified `fxInit` is exact as well: its post-decrement loop
clears all five 32-byte records, resets the global state, and preserves the
callee plus two data relocation pairs without normalization.
The 136-byte Mickey-named `func_80049A8C` resets either one record or all five,
clearing state/status and two flag bits. Its selection branches, stack home,
countdown loop, and data relocation pair are exact at the resident defaults.
The 156-byte `func_8004978C` remains exact in 37/39 positions after a fresh m2c pass with the proven 32-byte `FxRecord` layout and a new flag sweep.
The first mismatch is `+0x4`: IDO chooses an 8-byte leaf frame while the target uses 16 bytes; only the prologue/epilogue adjustments differ.
Hypothesis: an optimized-out original local enlarged the frame; prior padding/aggregate/qualifier forms disturb otherwise-exact allocation, so it remains `NON_MATCHING`.
The 180-byte `func_8004AD34` (`fxGenerateTextures` in JFG) is exact too. Its
four-entry descending callback loop, flag test, callback-table refresh, and
indirect call retain all target instruction words and relocation identities at
the resident defaults; spelling the constant-count loop as `while (index--)`
reproduces IDO's rotated `3`-through-`0` schedule without normalization.

`func_8004ACC4` is exact. Hand-written cursors were the defect: with four
source pointers the loop's dead counter copy is compiler-generated and takes
the *last* pool colour, and no arrangement of the setup statements moves it
(720 statement orders x 32 physical-line groupings, all flat at ten words).
Indexing three parallel four-element arrays from one `while (i--)` variable
instead lets uopt build the four cursors itself as strength-reduced induction
variables, which numbers the source index's now-dead copy first and the
synthesised trip counter second -- the target's colouring exactly. splat had
minted only `D_800D60D3`, the one element the assembly addresses directly;
`symbol_addrs.us.txt` now names the `D_800D60D0` base the C needs.

`func_8004AF68` is exact, and by the same edit as `func_8004ACC4`. The
hand-written-cursor candidate floored at 15 words: sharing one byte offset in
source hoists `D_800D60C0`'s base into an eighth saved register and costs
three, indexing the two arrays separately emits two shifts and costs three,
and no arrangement of the cursors reaches the target's `move v0, s3`. Indexing
all four arrays from one `while (i--)` variable hands the whole induction
problem to uopt, which produces the shared offset, the in-loop base and the
dead index copy by itself. 15 -> 0.

`tier-A func_80045BBC` is exact C: 240 bytes/60 words at VRAM
`0x80045BBC..0x80045CAC`, ROM `0x467BC..0x468AC`, frame `0x30`, with no
padding. Reading the saved stack pointer as a 64-bit context field and then
converting its low word to an N64 pointer closes the two temporary-register
words. Mickey's exception handler saves/restores SP at thread offset `0xF0`,
and `osCreateThread` independently defines that field as `u64`. The shared
header retains its opaque context; this function reads the established field
at context offset `0xD0`.

The stock configured TU uses unchanged `-O2 -mips2 -32`. Its 18 emitted static
relocations have exact offsets, types and identities. The fallback's six
additional records describe three fixed dump-header addresses; the C embeds
those same addresses literally. Resolving both surfaces gives the same owned
bytes, and the canonical C rebuild passes the full US ROM hash. There are no
resident runtime relocation records, ORT exports or overlay inbounds for this
range. Its sole authenticated caller remains `func_80045CAC+0x64`.

The permitted JFG draft supplies disclosed structural context. The closing
field-width correction follows Mickey's own thread representation and bytes.
No compiler force, flag change, instruction edit or relocation patch is used.

The 292-byte `func_80046AA8` packed-glyph renderer is Evidence A exact C after
bounded permutation: all 73 instruction words, its 72-byte frame, relocation
identities, and linked ROM bytes match. JFG's assembly-only
`func_800680B0_68CB0` corroborates the role and loop structure. Its inert
permuter-forced spelling remains in `docs/cleanup-queue.md` for a readability
follow-up, not as a matching deficit.

The 300-byte `func_80044C94` trace-neighbor lookup is exact C. JFG's newly
matched `diRcpTraceGetInfo` source supplied the original array-index loop
spelling; with Mickey's own symbols and ABI, IDO emits all 75 target
instructions and the exact relocation layout. The inactive-buffer scan and
nearest-lower/nearest-upper selection use Mickey's seven-argument ABI and
three-word trace entry; all six data-relocation pairs bind to Mickey's own
trace globals.

The 1,836-byte `func_80045D34` crash-screen controller also remains
`NON_MATCHING`. Supplying its jump table recovered a complete Mickey-derived
draft, but JFG's closest 1,888-byte peer (`func_80067880`) is assembly-only and
offers no source body. At the resident defaults the best typed candidate is
eight instructions short (451 versus 459), uses a 176-byte frame instead of
168 bytes, and differs in 432 positional words from function `+0x0`; the full
flag lattice's smaller MIPS I result cannot be adopted for a TU containing
existing MIPS II exact matches. Source/lifetime and named-string experiments
remained structural mismatches, so the target assembly stays canonical.

**PROVENANCE.** The TU identities and descriptive names in this subsection,
`symbol_addrs.us.txt`, and the four `src/main/*.c` files are adapted from Jet
Force Gemini's public decompilation (`src/diRcpTrace.c`, `src/diRcp.c`,
`src/diCpu.c`, and `src/fx.c`). JFG is a permitted published decomp under
`docs/CLEANROOM.md`; Mickey's own bytes, strings and linked call graph decide
every mapping. JFG address placeholders are not imported.

### 3.4 `main/charControl`

ROM `0x1C790`–`0x20020`, VRAM `0x8001BB90`–`0x8001F420`, is split as
`src/main/charControl.c`. The endpoints retain splat's original aligned
file-boundary candidates, but the assignment no longer rests on that heuristic
alone. At the start, the first six functions follow JFG's `charControl.c`
camera-control cluster by masked-skeleton similarity and call graph. Inside the
block, `func_8001C2D4` and `controlSetPlayerSetup` are tier-A skeleton anchors
from JFG's built `src/charControl.c.o`. At the tail, the latter is followed by
the setup getter and clearer behavior in JFG's order. The next yaml block
begins at `0x20020` with a separately tier-A function from JFG's `models.c.o`.
Together these are **B/D TU-boundary evidence**, not a whole-`.text` tier-A
match; the distinction is why §3.3's original two-hit row did not itself draw
the split.

**PROVENANCE:** JFG's public `src/charControl.c`, `src/charControl.h`, built
object, public symbol map, and `asm/nonmatchings/charControl` filenames supplied
the names in the comparison column below. Only the two tier-A rows were already
adopted in `symbol_addrs.us.txt`. Tier-B/D names remain comparison leads while
their functions use `GLOBAL_ASM`; §1.5 therefore keeps Mickey's `func_` names
until matching C independently establishes a role strongly enough to adopt
one. A dash means that neither the JFG order, masked similarity, nor the current
call graph isolates one namesake.

| Mickey VRAM | Size | JFG comparison lead | Evidence / current disposition |
|---|---:|---|---|
| `0x8001BB90` | `0x24` | `cameraGetBlend` | D + matched C: exact 0x2C-stride float getter under O2/mips2; JFG comparison remains structural, so retain `func_` |
| `0x8001BBB4` | `0x258` | `func_8002B378` | Bounded plateau: configured C is exact-sized at 150 words, with 112/150 positional words, first `+0x0`, and frame `0x30` versus target `0x38`. All 18 relocation offsets/types align; 15 identities are statically stable and the three overlay calls are runtime-authenticated. The 119-row lattice and ten coherent forms are nonexact; local structs improve to 34 differences only by overshooting the frame. Resume with a natural local/declaration layout that adds exactly eight non-save bytes, then revisit the post-decrement loop CFG. |
| `0x8001BE0C` | `0x248` | `func_8002EDA0` | Type pass: before=GLOBAL_ASM; after=GLOBAL_ASM (no C candidate).<br>Access: `ControlActor` f32 `0x10/0x14`; `ControlPlayer` s8 `0`, u8 `0x16F`; `ControlCameraState` s16 `0/2/4`, f32 `0x10/0x18/0x1C/0x20/0x24/0x28/0x40`, u8 `0x3D/0x44-0x49`; `CameraOverrideSlot` ptr `0/4`, f32 `0x8-0x28`; globals `D_800CB300/D_800CB368`.<br>Lever/remains: typed the 0x54-byte camera and 0x2C-byte override records; body remains assembly. |
| `0x8001C054` | `0x34` | `cameraAddOverrideObject` | D + matched C: exact 24-entry append under O2/mips2; JFG comparison remains structural, so retain `func_` |
| `0x8001C088` | `0x8C` | `cameraDeleteOverrideObject` | D + matched C: exact 24-entry search-and-delete under O2/mips2; JFG comparison remains structural, so retain `func_` |
| `0x8001C114` | `0x1B0` | `func_8002F0E8` | D + Mickey-derived exact C: 108/108 words, frame `0x18`, and all six static relocation offsets, types, and identities agree. Grouping the zero index and array cursor initializations in one ordered expression gives IDO the target `a1`/`v1` allocation and late schedule without a forced compiler result. The function maintains a camera override by rejecting an out-of-range tracked object and selecting an eligible replacement. JFG's corresponding `func_8002F0E8` remains assembly-only, so this body is a direct clean-room candidate at that insertion point. |
| `0x8001C2C4` | `0x8` | — | Matched C: exact empty routine under O2/mips2; retain `func_` |
| `0x8001C2CC` | `0x8` | — | Matched C: exact empty routine under O2/mips2; retain `func_` |
| `0x8001C2D4` | `0x4C` | `func_80031F60` | A + matched C: 19/19 unmasked JFG words and independently reconstructed byte-clear C are exact; placeholder rule retains Mickey's `func_` |
| `0x8001C320` | `0x1A0` | `controlPlayerReInit` | B + matched C: exact 104-instruction save/clear/reinitialize/restore wrapper under O2/mips2 with `-Wab,-r4300_mul`; its role and call graph mirror JFG, so the name is adopted |
| `0x8001C4C0` | `0x64C` | `controlPlayerInit` | Type pass: before=GLOBAL_ASM; after=GLOBAL_ASM (no C candidate).<br>Access: `ControlActor` s16 `0/2/4`, s8 `0x3A`, ptr `0x64/0x68`; `ControlPlayer` byte/halfword fields through `0x456`, f32 `0x10/0x38-0x54/0x174-0x17C/0x188/0x3EC/0x3F0/0x444-0x450`; `ControlGravityVector` `0/4/8` at 0x10 stride; `ControlParticleSlot` `0/1/2/3/8`.<br>Lever/remains: added the shared player, point-table, particle, level, and camera types; initializer body remains assembly. |
| `0x8001CB0C` | `0x78` | — | Matched C: exact one-point transform setup under O2/mips2; no unique JFG comparison, so retain `func_` |
| `0x8001CB84` | `0x71C` | `controlPlayer` | **A**; exact C, 455 words, 41 relocations, frame `-0x80` under `-O2 -mips2 -32 -Wab,-r4300_mul`; the owned ROM range `0x1D784`-`0x1DEA0` is byte-identical to the baserom. One scratch float local carries the ballistic step and is then reused for the cone factor; a second declared local for the step moves every compiler temp 4 bytes down the frame |
| `0x8001D2A0` | `0x17C` | — | Type pass: before=mixed(structural:1, register:8), 96/95 instructions; after=same, first +0xE0.<br>Access: `ControlActor` s16 `0/2/4`, f32 `0x8-0x14`, ptr `0x64`; `ControlPlayer` s8 `0`, s16 `0x14C/0x154/0x158/0x160/0x162/0x164/0x3FA/0x43C-0x440`, f32 `0x444-0x450`, u16 `0x1A8`, ptr `0xD4`; global `D_800CB300` is a 0x54-byte camera record.<br>Lever/remains: typed the camera-list stride; IDO CSE still adds one address instruction before the camera-count call. |
| `0x8001D41C` | `0x21C` | — | Matched C: exact 135-instruction timer, effect-spawn, and action-callback body under O2/mips2 with `-Wab,-r4300_mul`; the mandatory 119-combination sweep found no alternate flag improvement and no unique JFG comparison, so retain `func_` |
| `0x8001D638` | `0x58` | `controlFrozen` | B + matched C: exact pause/input gate under O2/mips2; calls the following restart routine as JFG does; name adopted |
| `0x8001D690` | `0x194` | `controlRestartPlayer` | B + matched C: exact 101-instruction multiplayer respawn-point search and single-player restart fallback under O2/mips2 with `-Wab,-r4300_mul`; JFG has the same role and nearest charControl skeleton, but Mickey retains `func_` because IDO's allocation changes under the public name |
| `0x8001D824` | `0x5C` | `dAngle` | B + matched C: same wrapped-angle role/body as JFG, whose MIPS-I conversion sequence is longer; adapted Mickey C is ADR 0001 exact under O2/mips2 |
| `0x8001D880` | `0x90` | `controlMakeV` | Bounded plateau: fresh configured full-TU C is exact-sized and frameless at 28/36 positional words, first `+0x4`, with no padding or relocations. Indexing the table (`table[index]`) rather than walking an `entry` pointer makes the address operand order `a2,t7` on both sides, and naming the upper table entry in `value` before the lerp takes the residual to 28/36. Twenty-five further source forms and 210 permuter candidates were flat or worse. The residual is a uopt colour rotation: the target colours only `10.0f` out of {`f16`,`f18`} and hands `f18` to a five-wide FP ring, this candidate colours both and runs the ring four wide. `x * 1.0f` on an `f32` folds to no instruction under IDO 5.3 `-O2` and is fully allocation-inert here, so it is not an FP analogue of the L65 phantom mask. Thirteen natural builds plus four fidelity-clean trace diagnostics found no strict gain; the prior 119-flag sweep remains exhausted.<br>Access/ABI: `f32 (f32, f32, f32 *, f32)` with a 4-byte-stride table. ORT 405 has runtime inbounds from Overlay 26 module `+0xA0C` and Overlay 80 `+0xD0`, but their friendly caller ABIs conflict with the target FP ABI and are identity-only evidence.<br>Lever/remains: proc 21 shows the second-base FP web occupying the target constant color and cascading the final schedule. Preserve V0 and fallback; resume only with a source-authentic base pool-to-temp/web-formation mechanism. |
| `0x8001D910` | `0x50` | `controlFSUvels` | B + matched C: JFG rotation-vector role/body with Mickey's output at player `+0x14`; adapted C is ADR 0001 exact under O2/mips2 |
| `0x8001D960` | `0x370` | `controlUpdateJetFlames` | **A**; exact C, 220 words, 6 relocations, frame `-0x60` under `-O2 -mips2 -32 -Wab,-r4300_mul`; the owned ROM range `0x1E560`-`0x1E8D0` is byte-identical to the baserom |
| `0x8001DCD0` | `0xA0` | — | Matched C after the type pass: `ControlVector3` f32 `0/4/8`, s16 rotation/output pointers; `tools/wb_compare.sh --rom` reports instruction-words-identical, 40 instructions. |
| `0x8001DD70` | `0x854` | `controlGroundHits` | Type pass: before=GLOBAL_ASM; after=GLOBAL_ASM (no C candidate).<br>Access: `ControlActor` transform f32 `0xC/0x10/0x14`; `ControlPlayer` f32 `0x38-0x40/0x74-0x90`, u8 `0x166/0x16C/0x173/0x181/0x18E/0x198/0x349-0x34B`, s32 `0x334/0x344`, point ptr `0x2B8`, count `0x2BC`; `ControlGravityVector` 0x10-byte records; globals `D_80081850-0x81860`.<br>Lever/remains: **frame closed 2026-09-09** -- the `NON_MATCHING` candidate now matches the target at 533 instructions and frame `-0x268` exactly. The 0x38-byte excess was a declaration census, not allocation: fourteen m2c-only carriers sized the local block in 8-byte steps, and dropping them also cut the alignment gaps 260 -> 86 at 433 relocation-masked words. Remaining is block geometry inside the collision loop (238 opcode differences).
| `0x8001E5C4` | `0x680` | `controlHangOK` / `controlGrabOK` | Type pass: before=GLOBAL_ASM; after=GLOBAL_ASM (no C candidate).<br>Access: `ControlVector3` f32 `0/4/8`; `ControlPlayer` f32 `0x4/0x8/0x38-0x40/0x74-0x90/0x2F0-0x2F8`, u8 `0x166/0x16C/0x173/0x181/0x18E/0x198/0x349-0x34B`, s32 `0x334/0x344`; `ControlCollisionState` `0x00-0x3D`; globals `D_800CB2C0-0xCB2FD`.<br>Lever/remains: collision-state aggregate is typed. Declaration census 2026-09-09 took the `NON_MATCHING` candidate frame `-0x100` -> `-0xE0` against a `-0xD0` target at 410 -> 408 relocation-masked words and an unchanged 412 instructions. The exact `-0xD0` frame is reachable but costs +4 words and 46 more alignment gaps: a frame too LARGE while the instruction count is four SHORT means this candidate homes locals the target does not and misses spills the target has, so the instruction count is the binding constraint, not the frame. |
| `0x8001EC44` | `0x3B8` | `controlSquashCheckPrior` | Type pass: before=GLOBAL_ASM; after=GLOBAL_ASM (no C candidate).<br>Access: `ControlVector3` f32 `0/4/8`; `ControlCollisionPlane` f32 `0/4/8/0xC/0x10/0x14/0x18`, s32 `0x20`, u8 `0x24`; `ControlCollisionState` f32 `0x04-0x24`, s32 `0x38`, u8 `0x3D`; globals `D_8008187C-0x81890/D_800CB2C4-0xCB2FD`.<br>Lever/remains: typed the collision callback inputs/state; collision helper remains assembly. |
| `0x8001EFFC` | `0xA0` | — | Matched C: exact point-list transform and translation loop under O2/mips2; no unique JFG comparison, so retain `func_` |
| `0x8001F09C` | `0xB0` | `func_800370D8` | D + matched C: exact target-smoothing body under O2/mips2 with `-Wab,-r4300_mul`; JFG placeholder comparison remains structural, so retain `func_` |
| `0x8001F14C` | `0x110` | `controlCeiling` | D + matched C: exact offset/spawn/effect body under O2/mips2; JFG comparison remains positional, so retain `func_` |
| `0x8001F25C` | `0x8` | `controlDisableJoypad` | B + matched C: caller supplies player and boolean, next routine tests the stored state; JFG has the same role but a one-argument global implementation |
| `0x8001F264` | `0xBC` | `controlReadJoypad` | B + matched C: calls all seven stick/button readers in JFG order; adapted per-player C is ADR 0001 exact under O2/mips2 |
| `0x8001F320` | `0x44` | `controlSetRumble` | B + matched C: sole call is the rumble dispatcher under player-state guards; Mickey-derived wrapper is ADR 0001 exact under O2/mips2 |
| `0x8001F364` | `0x8` | — | Matched C: empty routine, ADR 0001 byte-identity; retain `func_` |
| `0x8001F36C` | `0x40` | `controlSetPlayerSetup` | A + matched C: 6 unmasked of 16 JFG words established the name; Mickey-derived four-halfword/valid-byte body is ADR 0001 exact |
| `0x8001F3AC` | `0x5C` | `controlGetPlayerSetup` | B + matched C: consumes and clears the exact state written by the tier-A setter; adopted with point-of-use JFG provenance and ADR 0001 byte-identity |
| `0x8001F408` | `0xC` + `0xC` padding | `controlClearPlayerSetup` | B + matched C: clears the setup-valid byte; adopted with point-of-use JFG provenance and ADR 0001 byte-identity |

No function in this TU uses an odd single-precision FP register, so §6.2 does
not classify any of them as hand-written assembly. The `0xC` bytes after
`func_8001F408` are alignment padding, not executable ownership.

#### Audio-manager census and conservative source split

Census of yaml's former `0x1050`-`0xC950` assembly surface found **152
functions**: 82 in `0x1050`, 5 in `0x45F0`, and 65 in `0x4F40`. The assigned
JFG audio-manager family covers 74; the remainder has separate lineage.

| Mickey ROM range | Functions | Attribution and evidence | Canonical treatment |
|---|---:|---|---|
| `0x1050`-`0x2340` | 49 | JFG `audio_manager_1050.c`: **tier A** at `amTuneSetFadeScaled`, `amSndSetPan`, `forcelink`; **tier B** API order/calls. The aligned end precedes JFG's separate `audiomgr` initializer | `src/main/audio_manager_1050.c` |
| `0x2340`-`0x3100` | 13 | JFG `audiomgr.c`; **tier B** allocator/queue/scheduler/DMA/frame-state calls; outside the assigned TUs | `src/main/audiomgr.c`, `src/main/audiomgr_30E0.c`; boundaries recorded |
| `0x3100`-`0x45F0` | 20 | JFG `audio_manager_36D0.c`; **tier B** start allocator, 20-function order, positional setters, and terminal volume calculation. `audspat_jingle_off`/JFG `amAmbientPause` is a title-specific naming divergence | `src/main/audio_manager_36D0.c` |
| `0x45F0`-`0x4F40` | 5 | JFG `audio_manager_4C50.c`; **tier A** endpoints (`amVibratoInit`, `_depth2Cents`), five-function order, and `0xC` terminal alignment | `src/main/audio_manager_4C50.c` |
| `0x4F40`-`0xC950` | 65 | JFG `objects.c` lineage follows the oscillator TU; **tier A** `GetRomlistInfo`, but no whole-object match or promoted boundary | assembly |

Matched C bodies in these new TUs:

All rows use IDO 5.3 `-O2 -mips2 -32` and are linked-ROM exact unless noted.
The final column records owned object words and relocation coverage.

| Mickey routine | ROM / size | Name evidence | Match evidence |
|---|---:|---|---|
| `func_80000450` | `0x1050` / `0xC0` | **tier B**: JFG supplies the `amSetMuteMode` body and exact audio-manager order; Mickey's segment-start placeholder is retained for existing address arithmetic and overlay declarations | Exact 48 object words and all data/call relocations |
| `func_80000510` | `0x1110` / `0x84` | **tier B**: JFG supplies the `amTunePlay` control flow and exact audio-manager order; Mickey's external placeholder is retained, and Mickey's shorter target omits JFG's later tempo/count updates | Exact 33 object words and all data/call relocations |
| `amTuneVoiceLimit` | `0x1194` / `0x38` | **tier B**: JFG supplies the complete body and official name; the block flag, tune-player call, and exact audio-manager order agree | Exact 14 object words and all data/call relocations |
| `func_800005CC` | `0x11CC` / `0xF0` | **tier B**: JFG supplies the `amTuneSetFade` body and exact audio-manager order; Mickey's externally used placeholder is retained | Exact 60 object words and all data/call relocations |
| `amTuneResetFade` | `0x1330` / `0xC` | **tier B**: exact JFG routine order and the adjacent tune-fade controller role | Exact object words and linked ROM bytes |
| `amAmbientSetFade` | `0x133C` / `0xF0` | **tier B**: JFG supplies the complete body and official name; the paired fade-state globals, TV-rate paths, and exact audio-manager order agree | Exact 60 object words and all data/call relocations |
| `amAmbientResetFade` | `0x142C` / `0xC` | **tier B**: exact JFG routine order and the adjacent ambient-fade controller role | Exact object words and linked ROM bytes |
| `amAudioTick` | `0x1438` / `0x284` | **tier B**: JFG supplies the official name, fade controllers, delayed-sound queue, and exact audio-manager order; Mickey's two sequence-init calls and master-volume fade tail remain authoritative | Exact 161 object words and all message-queue, fade, delayed-sound, sequence, and master-volume relocation identities |
| `amWaitForMidiSync` | `0x16BC` / `0x80` | **tier B**: JFG supplies the official name and exact audio-manager order; Mickey's own code pins the pending-sync flag, blocking receive loop, and pre-NMI call | Exact 32 object words and all flag/queue/call relocations |
| `amResetMidiSync` | `0x173C` / `0xC` | **tier B**: JFG supplies the official name and exact audio-manager order; Mickey's own code clears the same pending-sync flag consumed by `amWaitForMidiSync` | Exact 3 object words and data relocation identity |
| `func_80000B48` | `0x1748` / `0xA0` | **tier B**: JFG supplies the `amTuneSetChlMask` name and exact audio-manager order; Mickey's external placeholder is retained, while the body and `u8` call ABI come from Mickey-only evidence | Exact 40 object words and all player/mask/call relocations |
| `amTuneMuteChl` | `0x17E8` / `0x8` | **tier B**: exact JFG routine order between the channel-mask setter and its paired unmute leaf | Exact object words and linked ROM bytes |
| `amTuneUnmuteChl` | `0x17F0` / `0x8` | **tier B**: exact JFG routine order immediately after its paired mute leaf | Exact object words and linked ROM bytes |
| `amTuneSetChlVolume` | `0x17F8` / `0x40` | **tier B**: JFG routine order and exact channel-bound/call role; its 1.000 skeleton is ambiguous with DKR's pan/volume/fade wrappers and is not tier A | Exact 16 object words and both data/call relocation identities |
| `amTuneResetChls` | `0x1838` / `0x64` | **tier B**: exact JFG routine order and the paired unmute/full-volume loop role | Exact 25 object words and all global/call relocation identities |
| `amAmbientPlay` | `0x189C` / `0x50` | **tier B**: JFG and DKR agree on the official role; the current-sequence assignment, ambient player, playing guard, and sequence-start call match exactly | Exact 20 object words and all global/call relocation identities |
| `amTuneStop` | `0x18EC` / `0x30` | **tier B**: JFG and DKR agree on the official role; the tune-change block and tune-player stop call pin the identity | Exact 12 object words and both global/call relocation identities |
| `amAmbientStop` | `0x191C` / `0x38` | **tier B**: JFG and DKR agree on the official role; the playing guard, ambient-ID reset, and ambient-player stop call pin the identity | Exact 14 object words and all global/call relocation identities |
| `amTuneGetSeqNo` | `0x1954` / `0x3C` | **tier B**: JFG and DKR agree on the official role; the current-tune guard and tune-player `AL_PLAYING` state check pin the identity | Exact 15 object words and all data relocation identities |
| `amAmbientGetSeqNo` | `0x1990` / `0x0C` | **tier B**: JFG and DKR agree on the official return role; Mickey returns the same current-ambient global used by the play/stop pair | Exact 3 object words and data relocation identity |
| `amTuneSetVolume` | `0x199C` / `0x6C` | **tier B**: JFG supplies the full body and official name; the clamp, saved base volume, scaled tune-player call, and update flag agree exactly | Exact 27 object words and all data/call relocation identities |
| `amTuneSetGlobalVolume` | `0x1A08` / `0x5C` | **tier B**: JFG supplies the full body and official name; the global-volume clamp, saved scale, and recalculated tune-player call agree exactly | Exact 23 object words and all data/call relocation identities |
| `amTuneGetVolume` | `0x1A64` / `0x0C` | **tier B**: JFG and DKR agree on the official return role; Mickey returns the base-volume global written by `amTuneSetVolume` | Exact 3 object words and data relocation identity |
| `amAmbientSetVolume` | `0x1A70` / `0x4C` | **tier B**: JFG supplies the full body and official name; the saved relative volume and sound-global-scaled ambient-player call agree exactly | Exact 19 object words and all data/call relocation identities |
| `amDittyPlay` | `0x1ABC` / `0x64` | **tier B**: JFG has the same exact boundary and sequence-table guard/current-ID/player-start role; `skeleton_scan.py` ranks it first at 0.571, not tier-A identity | Exact 25 object words and all data/call relocation identities |
| `amDittyPlaying` | `0x1B20` / `0x54` | **tier B**: JFG has the same exact boundary and DKR supplies the official role; current-ID, enabled, and ambient-player-state guards agree exactly | Exact 21 object words and all data relocation identities |
| `amSndStop` | `0x1B74` / `0x20` | **tier B**: JFG supplies the complete one-call body and official name; the target is below the skeleton oracle's 10-word confidence floor | Exact 8 object words and call relocation identity |
| `amSndPlay` | `0x1B94` / `0x104` | **tier B**: JFG has the same exact boundary and direct-player call shape; DKR supplies the official role and `SoundData` interpretation | Exact 65 object words and all data/call relocation identities |
| `amSndPlayDirect` | `0x1C98` / `0xAC` | **tier B**: JFG supplies the official name, parameter roles, range check, scaler, and direct-player call shape; Mickey's branch-likely form is four bytes shorter | Exact 43 object words and all data/call relocation identities |
| `amSndSetVol` | `0x1D44` / `0xC0` | **tier B**: JFG and DKR agree on the official role; base-volume lookup, relative scaling, resident scaler, and volume-parameter call agree exactly | Exact 48 object words and all data/call relocation identities |
| `amSndSetPitchDirect` | `0x1E2C` / `0x2C` | **tier B**: JFG and DKR agree on the official name and parameter role; the handle guard and pitch-parameter call agree exactly | Exact 11 object words and call relocation identity |
| `amGetSfxCount` | `0x1E58` / `0x18` | **tier B**: JFG supplies the complete body and official name; the bank/instrument traversal and sound-count field agree exactly | Exact 6 object words and data relocation identity |
| `amGetSfxSettings` | `0x1E70` / `0x38` | **tier B**: JFG supplies the complete body and official name; the optional table/size/count outputs and their globals agree exactly | Exact 14 object words and all data relocation identities |
| `amSoundIsLooped` | `0x1EA8` / `0x60` | **tier B**: JFG and DKR agree on the official role and body; the sound-count bound, sound-array traversal, and infinite-decay test agree exactly | Exact 24 object words and data relocation identity |
| `func_80001308` | `0x1F08` / `0x74` | **tier B**: JFG supplies the alternate sequence-initializer role and audio-manager order; Mickey's placeholder is retained, while its body and resident sequence-count field are reconstructed from Mickey-only evidence | Exact 29 object words and all sequence-count, player-state, and call relocation identities |
| `func_8000137C` | `0x1F7C` / `0x1EC` | **tier B**: JFG supplies the `music_sequence_init` role and exact audio-manager order; Mickey's placeholder is retained, while its body and resident metadata types are independently reconstructed from Mickey-only evidence | Exact 123 object words and all sequence-table, player-state, channel-mask, and call relocation identities |
| `stop_ALSeqp` | `0x2168` / `0x88` | **tier B**: JFG name/body and Mickey's two-player stop state machine agree | Exact 34 object words and all call/data relocations |
| `amTuneSetReverbOnOff` | `0x21F0` / `0x8` | **tier B**: JFG supplies the name and no-op body | Exact 2 object words; no relocations |
| `func_800015F8` | `0x21F8` / `0x10` | **tier D**: direct write of one to the resident audio flag; no external name is asserted | Exact 4 object words and data relocation identity |
| `func_80001608` | `0x2208` / `0xC` | **tier B**: overlay 46 calls this routine at its sequence-transition exit, corroborating the direct resident audio-flag clear; no external name is asserted | Exact 3 object words and data relocation identity |
| `func_80001614` | `0x2214` / `0xC` | **tier B**: a resident caller branches on this direct audio-flag read; no external name is asserted | Exact 3 object words and data relocation identity |
| `func_80001620` | `0x2220` / `0x48` | **tier B**: a resident caller consumes the range-checked sound-table volume; no external name is asserted | Exact 18 object words and both data relocations |
| `func_80001668` | `0x2268` / `0x30` | **tier D**: guarded sound-volume parameter wrapper; no external name is asserted | Exact 12 object words and call relocation |
| `scalevol` | `0x22C8` / `0x24` | **tier B**: JFG supplies the complete body and official name | Exact 9 object words; no relocations |
| `func_800016EC` | `0x22EC` / `0x1C` | **tier B**: overlay 49 supplies mode-call context; no external name is asserted | Exact 7 object words and two data relocations |
| `func_80001708` | `0x2308` / `0x38` | **tier B**: a resident caller pins the master-volume reset role; no external name is asserted | Exact 14 object words, two calls, and data relocation |
| `func_80001740` | `0x2340` / `0x344` | **tier B:** DKR/JFG supply the SGI audio-manager initializer order and queue/DMA roles; Mickey's config and state fields remain authoritative | Candidate: 203 vs 209 instructions, frame -344 vs -336, first structural divergence at `+0x0` |
| `func_80001A84` | `0x2684` / `0x11C` | **tier A:** DKR's public SGI `__amMain` body supplies the scheduler message-loop shape; Mickey's queue globals and message flow remain authoritative | Exact 71 object words and linked ROM bytes |
| `func_80001BA0` | `0x27A0` / `0x24` | **tier B:** JFG supplies the `amGo` role and audio-manager order; the placeholder is retained | Exact 9 object words and linked ROM bytes |
| `func_80001BC4` | `0x27C4` / `0x24` | **tier B:** JFG supplies the `amStop` role and audio-manager order; the placeholder is retained | Exact 9 object words and linked ROM bytes |
| `func_80001BE8` | `0x27E8` / `0x0C` | **tier A:** Mickey-derived C body and linked ROM identity; placeholder retained because no external name is asserted | Exact 3 object words and linked ROM bytes |
| `func_80001BF4` | `0x27F4` / `0x540` | **tier B:** DKR/JFG supply the SGI frame-handler/task-builder role; Mickey's manager and task globals remain authoritative | Candidate: 372 vs 336 instructions, frame -96 vs -88, structure mismatch from `+0x0` |
| `func_80002134` | `0x2D34` / `0x54` | **tier B:** SGI audio-manager underrun-handler role and Mickey's AI/EFC state identify the body; no external name is asserted | Exact 21 object words, frame 0x18, and all seven call/data relocation identities |
| `func_80002188` | `0x2D88` / `0x1CC` | **tier B:** DKR's public SGI `__amDMA` body supplies the DMA-list search/allocation shape; Mickey's DMA state and calls remain authoritative | Exact 115 object words, all 22 relocation records, and linked ROM bytes |
| `audioManager_DMAInitProc` | `0x2F54` / `0x38` | **tier A:** BK `src/core1/code_1D00.c` supplies the SGI DMA-initializer body and name | Exact 14 object words and linked ROM bytes |
| `func_8000238C` | `0x2F8C` / `0x130` | **tier A:** DKR's public SGI `__clearAudioDMA` body supplies the DMA-list cleanup shape; Mickey's queue and DMA state remain authoritative | Exact 76 object words and linked ROM bytes |
| `func_800024E0` | `0x30E0` / `0x0C` | **tier A:** Mickey-derived C return body; the placeholder is retained because no external name is asserted | Exact 3 object words and linked ROM bytes |
| `func_800024EC` | `0x30EC` / `0x14` | **tier A:** Mickey-derived zero-test return body; the placeholder is retained because no external name is asserted | Exact 5 object words and linked ROM bytes |
| `func_80002500` | `0x3100` / `0xC4` | **tier B**: JFG supplies the `amInitAudioMap` role and exact audio-manager order; Mickey's externally visible placeholder is retained, while its pool sizes, point stride, handle field, and body come from Mickey-only evidence | Exact 49 object words and all sound-table, allocator, point-pool, count, and reset-call relocation identities under `-Wab,-r4300_mul` |
| `audspat_jingle_off` | `0x31C4` / `0x28` | existing **tier A** audio-spatial object identity; JFG's `amAmbientPause` supplies the complete body while Mickey's title-specific name remains authoritative | Exact 10 object words and both call/data relocations |
| `amAmbientRestart` | `0x31EC` / `0xC` | **tier B**: JFG supplies the complete body and official name; the paired ambient-pause flag and exact audio-manager order agree | Exact 3 object words and data relocation identity |
| `amResetAudioMap` | `0x31F8` / `0x170` | **tier B**: JFG supplies the official name, reset role, and exact audio-manager order; Mickey's point/free-pool layout, queue slots, module ID, and body remain authoritative | Exact 92 object words and all point/free-pool, active-count, queue-slot, sound-stop, module-query, and trap relocation identities under `-Wab,-r4300_mul` |
| `amPlayAudioMap` | `0x3368` / `0x720` | **tier B**: JFG supplies the official name and exact audio-manager order; DKR supplies the related positional-point update body, while Mickey's reduced point-only update and dynamic-module tail remain authoritative | Exact 456 object words and all camera, sound-player, point-heap, and dynamic-module relocation identities under `-Wab,-r4300_mul` |
| `amCalcSfxStereo` | `0x3A88` / `0x158` | **tier B**: JFG supplies the complete body and official name; the single-camera transform, signed angle bands, and exact audio-manager order agree | Exact 86 object words and all matrix/call relocation identities |
| `func_80002FE0` | `0x3BE0` / `0xBC` | **tier B**: JFG supplies the `amSndPlayXYZ` body, `SoundData` layout, and exact audio-manager order; the Mickey placeholder is retained | Exact 47 object words and all table/call relocations |
| `func_8000309C` | `0x3C9C` / `0x18` | **tier B**: JFG supplies the `amSndSetVolXYZ` body and exact audio-manager order; the externally visible Mickey placeholder is retained | Exact 6 object words; no relocations |
| `func_800030B4` | `0x3CB4` / `0x18` | **tier B**: JFG supplies the `amSndSetPitchXYZ` body and exact audio-manager order; the Mickey placeholder is retained | Exact 6 object words; no relocations |
| `func_800030CC` | `0x3CCC` / `0xF4` | **tier B**: JFG supplies the `amSndPlayDirectXYZ` body, prototype, and exact audio-manager order; the Mickey placeholder is retained | Exact 61 object words and call relocation identity |
| `func_800031C0` | `0x3DC0` / `0x28` | **tier B**: JFG supplies the `amSndSetXYZ` body and exact audio-manager order; Mickey's external placeholder is retained | Exact 10 object words; no relocations |
| `func_800031E8` | `0x3DE8` / `0x68` | **tier B**: JFG supplies the `amSndStopXYZ` body and exact audio-manager order; Mickey's widely used external placeholder is retained | Exact 26 object words and all heap/count/call relocations |
| `amSndUnlinkHandleXYZ` | `0x3E50` / `0x4C` | **tier B**: JFG supplies the official name and exact audio-manager order; Mickey's own body pins the heap search and handle unlink field | Exact 19 object words and all heap/count relocation identities |
| `func_8000329C` | `0x3E9C` / `0x114` | **tier B**: JFG supplies the `amCreateAudioPoint` role, prototype, and exact audio-manager order; Mickey's placeholder is retained, while its high-water mark, free/used pools, point-field layout, and body come from Mickey-only evidence | Exact 69 object words and all count, high-water, free-pool, and used-pool relocation identities under `-Wab,-r4300_mul` |
| `func_800033B0` | `0x3FB0` / `0xD0` | **tier B**: JFG supplies the ordered `amSndSetEcho` role; Mickey's placeholder, echo-surface layout, and body remain authoritative | Exact 52 object words, frame, call relocation, and linked ROM bytes under `-Wab,-r4300_mul`; stack-home census plus dead-parameter reuse closed the prior schedule residual |
| `func_800035F8` | `0x41F8` / `0x168` | **tier B**: JFG supplies the ordered positional-update placeholder peer; Mickey's placeholder is retained, while the per-group queue, point layout, and sound-parameter update body come from Mickey-only evidence | Exact 90 object words and all group-count, update-entry, sound-start, parameter, priority, and echo relocation identities under `-Wab,-r4300_mul` |
| `func_800037C4` | `0x43C4` / `0x128` | **tier B**: Mickey callers pin the used-pool removal role and JFG supplies the ordered placeholder peer; no donor placeholder is adopted | Exact 74 object words and all sound-stop, auxiliary-cleanup, free/used-pool, and count relocation identities under `-Wab,-r4300_mul` |
| `func_800038EC` | `0x44EC` / `0xF8` | **tier B**: JFG supplies the `amSndGetXYZVolume` role and terminal audio-manager order; Mickey's placeholder is retained, while the coordinate distance, sound-setting layout, and attenuation body come from Mickey-only evidence | Exact 62 object words and all settings-table, `sqrtf`, and floating-constant relocation identities under `-Wab,-r4300_mul`; the following `0xC` bytes are TU alignment padding, not function credit |
| `amSndSetPan` | `0x1E04` / `0x28` | existing **tier A** JFG byte identity | Exact object words and relocation identity |
| `forcelink` | `0x2298` / `0x30` | existing **tier A** JFG byte identity | Exact object words and both call relocations |
| `amVibratoInit` | `0x45F0` / `0x90` | existing **tier A** JFG byte identity; BK supplies the matching free-list source shape | Exact 36 object words and linked ROM bytes. Relocation count/type/offset are exact; splat's per-element pool symbols resolve identically to the C array-base relocations plus their element addends |
| `amInitOsc` | `0x4680` / `0x2D8` | **tier B**: JFG supplies the official name and exact oscillator-TU order; Perfect Dark supplies the related oscillator-init vocabulary and source shape, while Mickey's eight-case state initialization and layout remain authoritative | Exact 182 object words and all free-list, depth-conversion, cents-ratio, and state-field relocation identities |
| `amUpdateOsc` | `0x4958` / `0x574` | **tier B**: JFG supplies the official name and exact oscillator-TU order; Perfect Dark supplies the related oscillator-update vocabulary and source shape, while Mickey's eight-case state machine and layout remain authoritative | Exact 349 object words and all sine, cents-ratio, constant, and state-field relocation identities |
| `amStopOsc` | `0x4ECC` / `0x18` | **tier B**: JFG supplies the official name and exact oscillator-TU order; Mickey's own body returns the state to the free-list head | Exact 6 object words and both free-list relocation identities |
| `_depth2Cents` | `0x4EE4` / `0x50` | existing **tier A** JFG byte identity, independently corroborated by BK's compiled object | IDO 5.3, `-O2 -mips2 -32 -Wab,-r4300_mul`; exact object words/relocations, with `0xC` target padding excluded |
| `func_80006C40` | `0x6C40` / `0xC` | **tier D**: object-TU order and +0x58 field-store structure; placeholder retained | Exact 3 object words and no relocations under IDO 5.3 `-O2 -mips2 -32` |
| `func_8000A6DC` | `0xA6DC` / `0xC` | **tier B**: `src/main/level.c` fixes the setter's existing `s32` call prototype; placeholder retained | Exact 3 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_8000485C` | `0x485C` / `0x10` | **tier D**: object-TU order and `s8` global-setter structure; placeholder retained | Exact 4 object words and no relocations under IDO 5.3 `-O2 -mips2 -32` |
| `func_80008118` | `0x8118` / `0x10` | **tier D**: object-TU order and byte-global initializer structure; placeholder retained | Exact 4 object words and no relocations under IDO 5.3 `-O2 -mips2 -32` |
| `func_80005750` | `0x5750` / `0x18` | **tier B**: resident callers agree on the count-output/list-return accessor role; placeholder retained | Exact 6 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_80005768` | `0x5768` / `0x30` | **tier B**: `src/main/anim.c` supplies the typed `AnimPathObject *` append caller; placeholder retained | Exact 12 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_80005798` | `0x5798` / `0x70` | **tier D**: object-list removal loop and object-TU order; placeholder retained | Exact 28 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_80009E78` | `0x9E78` / `0x90` | **tier B**: resident track/menu callers fix the display-list, matrix, vertex, and object wrapper interface; placeholder retained | Exact 36 object words, 13 relocation identities, and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_8000A62C` | `0xA62C` / `0xB0` | **tier B**: JFG supplies the `setObjectViewNormal` body and `src/main/track.c` fixes the vector-setter interface; Mickey's target globals remain authoritative | Exact 44 object words, target relocation surface, and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32 -Wab,-r4300_mul` |
| `func_8000572C` | `0x572C` / `0x24` | **tier B**: resident callers agree on the start/end/list accessor role; placeholder retained | Exact 9 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `GetRomlistInfo` | `0xBC88` / `0x28` | existing **tier A** JFG identification; indexed romlist-table accessor | Exact 10 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_800069C0` | `0x69C0` / `0x28` | **tier D**: nested object/data field offsets and output-record initialization; placeholder retained | Exact 10 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_8000A244` | `0xA244` / `0x158` | **tier D**: object-list active/inactive partition and +0x14 header flag reconstructed from the DKR ancestor and Mickey target | Exact 86 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_80005808` | `0x5808` / `0x18` | **tier B**: resident callers agree on the second count-output/list-return accessor role; placeholder retained | Exact 6 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_80005820` | `0x5820` / `0x48` | **tier B**: resident callers across memory, weather, menu, and overlays fix the count-checked object-list lookup interface; placeholder retained | Exact 18 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_800058C0` | `0x58C0` / `0x4C` | **tier D**: nested +0x40 data pointer, byte-stride status fields, and zero-valued f32 guard reconstructed from the target and object callers; placeholder retained | Exact 19 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_80007844` | `0x7844` / `0x8` | **tier D**: object-TU order and empty-leaf structure; placeholder retained | Exact 2 object words and no relocations under IDO 5.3 `-O2 -mips2 -32` |
| `func_800056F0` | `0x56F0` / `0x3C` | **tier B**: `src/main/track.c` fixes the signed index/route-list lookup interface; placeholder retained | Exact 15 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_80006EA0` | `0x6EA0` / `0x44` | **tier B**: resident callers in `src/main/anim.c` and `src/main/charControl.c` fix the object-registration interface; placeholder retained | Exact 17 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_80004340` | `0x4340` / `0x5C` | **tier D**: object-system reset role and the target's ordered word/halfword global clears; placeholder retained | Exact 23 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_8000439C` | `0x439C` / `0xB8` | **tier B**: `src/main/track.c` fixes the object teardown caller and JFG supplies the surrounding object-system order; Mickey's free-list and allocation globals remain authoritative | Exact 46 object words, all 21 relocation identities, and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32 -Wab,-r4300_mul` |
| `func_800056A4` | `0x62A4` / `0x4C` | **tier A**: Mickey-only bounds check, record base pointer, and scaled index-table read; placeholder retained | Exact 19 object and linked ROM words, frameless, and all six fallback relocation identities; reading the index table as `&D_800C9460[1]` supplies the %hi/%lo pair splat could not recover across the early-return branch |
| `func_800069E8` | `0x75E8` / `0x11C` | **tier A**: Mickey-only target-record initialization, aligned tail cursor, and renderer handoff; placeholder retained | Exact 71 object and linked ROM words, 40-byte frame, and all four relocation identities; reading `unk1C` back at its second use stops IDO reassociating the +4 into the size constant |
| `func_80005868` | `0x6468` / `0x58` | **tier A**: Mickey-only table reads, signed halfword index/result, and ROM-section DMA interface; placeholder retained | Exact 22 object and linked ROM words, 224-byte frame, and all five relocation identities; carrying the final ROM offset leaves the intermediate index temporary under unchanged TU flags |
| `func_80008A20` | `0x8A20` / `0x6C` | **tier D**: target-register ABI and +0x68 object-field dispatch into `func_8000831C`; placeholder retained | Exact 27 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |
| `func_80008A8C` | `0x8A8C` / `0x108` | **tier D**: object-type switch, trap dispatch, and +0x44/+0x88 fields reconstructed from the target; placeholder retained | Exact 66 object words, compiler-owned switch table, and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32 -Wab,-r4300_mul` |
| `func_8000486C` | `0x546C` / `0x298` | **tier A**: Mickey-only cached asset-header loading, pointer fixups, and resource dispatch; placeholder retained | Exact 166 linked C words, 72-byte frame, all twenty-three relocation identities, and full ROM; adjacent offset-table words, correct texture-loader arity, success-path control flow, and redundant-local removal |
| `func_80006C4C` | `0x784C` / `0x254` | **tier A**: Mickey-only temporary-object construction, model-loader dispatch, copy, and cleanup paths; placeholder retained | Exact 149 linked C words, 80-byte frame, all sixteen relocation identities, and full ROM; corrected post-decrement loops and call/store order, return-pointer home, typed model indexing, and destination-role reuse |
| `func_80008128` | `0x8D28` / `0x1F4` | **tier A**: Mickey-only object movement, track bounds, and segment lookup; placeholder retained | Exact 125 linked C words, 64-byte frame, all five relocation identities, and full ROM; expanded bound locals and used movement copies recover scheduling and stack homes |
| `func_80009220` | `0x9E20` / `0x1F4` | **tier A**: Mickey-only three-vector, sprite-segment layout, renderer calls, and literal-pool ownership; placeholder retained | Exact 125 linked C words, 112-byte frame, all nine relocation identities, and full ROM; real aggregates, consumed-argument reuse, dot-product grouping, and command-pointer post-increment remove the reconstruction residual |
| `func_80009F74` | `0xAB74` / `0x2D0` | **tier A**: Mickey-only distance thresholds, unsigned model index, camera calls, and renderer dispatch; placeholder retained | Exact 180 object and linked ROM words, 40-byte frame, all 15 relocation identities, and full ROM under unchanged TU flags; post-call data reloads, selector-carrier reuse, and declaration order close the residual |
| `func_8000A830` | `0xB430` / `0x208` | **tier A**: Mickey-only object-data sizing switch and caller ABI; placeholder retained | Exact 130 linked C words, frameless with no saved registers, three relocation sites/identities, 92 compiler-table destinations, and full ROM; direct switch expression removes the unnecessary allocated carrier |
| `func_8000AA38` | `0xB638` / `0x4B4` | **tier A**: Mickey-only instruction, table-destination, runtime-call and ABI proof; placeholder retained | Exact 301 linked C words, 24-byte frame, 74 relocation sites/identities, 92 compiler-table destinations, and full ROM; caller forwards the third preserve-state parameter |
| `func_8000AEEC` | `0xBAEC` / `0x4E0` | **tier A**: Mickey-only switch destinations, runtime calls and two-argument caller ABI; placeholder retained | Exact 312 linked C words, 32-byte frame, 69 relocation sites/identities, 89 compiler-table destinations and full ROM; direct selector restores allocation, and retail table evidence corrects four case labels |
| `func_80009F08` | `0x9F08` / `0x6C` | **tier D**: +0x30 f32 scale input, flag gate, 250.0/2.0 clamp, and target branch structure reconstructed from the object block; placeholder retained | Exact 27 object words and linked ROM bytes under IDO 5.3 `-O2 -mips2 -32` |

Measured plateau:

| Mickey routine | Best result | First mismatch | Remaining hypothesis |
|---|---|---:|---|
| `amTuneSetFadeScaled` | Exact 29-word instruction/opcode schedule, frame, and relocation surface; 7 register-only differences after the flag lattice and 10 source-shape attempts | function `+0x1C` | IDO 5.3 temporary-FIFO phase: the target and candidate assign the three initial address/index temporaries from different positions in the same ring. The candidate remains under `NON_MATCHING`; canonical output is still assembly-backed |
| `func_80003480` | Fresh configured V0 is exact-size at 94 instructions, with 41/94 words matching; its `0x48` frame remains larger than the target's `0x30`, and 4/5 relocation sites align by offset, type, and identity | function `+0x0` | IDO 5.3 web formation and spill placement remain unresolved in the final replacement path; old source forms, flags, and permutation remain closed, and canonical output remains assembly-backed |
| `func_80003760` | Evidence A exact C: all 25 words, frame, relocations, and linked ROM bytes match under `-Wo,-loopunroll,0` | none | Bounded permutation resolved the temp-FIFO allocation; the donor peer remains assembly-only role evidence |
| `func_8000BD0C` | Shape-exact 15-instruction candidate with 2 schedule-only differing words, identical instruction multiset, frameless, and no relocations | function `+0x2C` | Permuter-ready JFG `GetRangeSquared` adaptation; preserve the guarded candidate and resolve the late scheduling tie with the bounded search |
| `func_8000BCB0` | Shape-exact 23-instruction candidate with 2 schedule-only differing words, frameless, and exact `sqrtf` call topology | function `+0x3C` | Permuter-ready JFG `GetRange` adaptation; preserve the guarded candidate and resolve the FP scheduling tie |

PROVENANCE: TU labels, order, and semantic roles derive from JFG's permitted
public decomp/objects. C retains Mickey-owned stubs and point-disclosed adapted
bodies; every promotion remains byte-exact to Mickey.

---

### 3.17 Vehicle sounds, models and gsSnd census

These three existing 16-byte-aligned splat boundaries were moved from raw
`asm` subsegments to C translation units with one `GLOBAL_ASM` per function.
That changes ownership, not bytes. **PROVENANCE:** JFG's permitted
`src/audio_manager_36D0.c`, `audio.h`, `src/models.c`, `models.h`,
`src/camera.c`, `src/gsSnd.c` and `src/gsSnd.h` were read while identifying the
APIs and candidate names. The initial split adapts no body from them.

Every function was checked with `skeleton_scan.py similar --top 5`. ROM
`0x58E50`–`0x59B90` produced no exact JFG match. The only exact function in
ROM `0x5B300`–`0x5C310` is `camConvertMatrixList` (12 words, 4 relocation-
masked, ROM-wide unique); the loader/free pair's nearest JFG shapes are in
`models.c` but are non-exact, so their Mickey address names remain. The entire
`gsSnd` object is already a Tier-A match; its per-function scan re-confirmed
every function of at least 10 words except the ambiguous placeholder at
`0x8005CD3C` and the final `gsSndpLimitVoices`, whose standalone bound omits
the object's four padding bytes. `gsSndpGetGlobalVolume` is below the scanner
floor. The whole-object match carries all three without importing a
placeholder name or counting padding as function text.

| ROM / VRAM | Size | Function | Evidence and call-graph role |
|---|---:|---|---|
| `0x58E50` / `0x80058250` | `0x58` | `func_80058250` | D: clears four positional engine-sound slots; called from resident audio setup |
| `0x58EA8` / `0x800582A8` | `0x64` | `func_800582A8` | B: stops those four handles; called from the main state-transition path |
| `0x58F0C` / `0x8005830C` | `0xBE8` | `func_8005830C` | D: walks active racers and maintains two positional sounds from speed and listener distance; no per-symbol caller argument recorded |
| `0x59AF4` / `0x80058EF4` | `0x90` + `0x0C` padding | `func_80058EF4` | D: local logarithm-series helper used to derive Doppler pitch |
| `0x5B300` / `0x8005A700` | `0x64` | `func_8005A700` | D: allocates animation table/cache storage |
| `0x5B364` / `0x8005A764` | `0x0C` | `func_8005A764` | D: resets the pending-animation counter |
| `0x5B370` / `0x8005A770` | `0x30` | `func_8005A770` | D: flushes the pending animation table, then resets its count; no per-symbol caller argument recorded |
| `0x5B3A0` / `0x8005A7A0` | `0x1A8` | `func_8005A7A0` | B: loads a model's animation-ID table and allocates its animation pointer array; sole proven caller is `func_8001F520+0x42C`, passing the loaded model and model ID |
| `0x5B548` / `0x8005A948` | `0x178` | `func_8005A948` | B: reference-counted single-animation loader; sole proven caller is `func_8005A7A0+0x104`, passing an `lh` animation ID and consuming the returned pointer |
| `0x5B6C0` / `0x8005AAC0` | `0xB8` | `func_8005AAC0` | B: releases one reference-counted animation; direct callers are `func_80020278+0xD0` and `func_8005A7A0+0x140`, each passing an animation pointer |
| `0x5B778` / `0x8005AB78` | `0x30` | `camConvertMatrixList` | A: exact JFG `camera.c` helper, used by the matrix builder below |
| `0x5B7A8` / `0x8005ABA8` | `0x1BC` | `func_8005ABA8` | D: advances/clamps the current animation frame |
| `0x5B964` / `0x8005AD64` | `0x1B0` | `func_8005AD64` | D: selects an animation and establishes its frame/blend state; matched C, 108 exact words, no frame or relocations |
| `0x5BB14` / `0x8005AF14` | `0x730` | `func_8005AF14` | B: builds model matrices and transformed attachment points, then calls `camConvertMatrixList` to queue matrix conversion |
| `0x5C244` / `0x8005B644` | `0xCC` | `func_8005B644` | D: constructs a parented matrix list for the builder |
| `0x5C640` / `0x8005BA40` | `0x12FC` | `func_8005BA40` | A: bare `-g -mips2 -32` emits exact text and the measured `main/gsSnd` rodata split links byte-identically |

The `gsSnd` function boundaries are: `gsSndpNew` `0x268`,
`func_8005B978` `0xC8`, `func_8005BA40` `0x12FC`, `func_8005CD3C` `0x70`,
`func_8005CDAC` `0x7C`, `func_8005CE28` `0x104`,
`getSoundStateCounts` `0x104`, `func_8005D030` `0x230`,
`func_8005D260` `0x144`, `gsSndpSetPriority` `0x28`, `gsSndpGetState`
`0x30`, `ad_sndp_play` `0x2E8`, `gsSndpStop` `0x80`,
`sndp_stop_with_flags` `0xBC`, the three `gsSndpStopAll*` wrappers `0x28`
each, `gsSndpSetParam` `0x7C`, `gsSndpGetMasterVolume` `0x2C`,
`gsSndpSetMasterVolume` `0xE0`, `gsSndpSetGlobalVolume` `0x28`,
`gsSndpGetGlobalVolume` `0x1C`, and `gsSndpLimitVoices` `0x48` followed by
four bytes of TU padding. All are inside the measured Tier-A `gsSnd.c` object.

The direct strings are confined to `gsSnd`: state-count diagnostics and bad
event/play-state diagnostics in `func_8005BA40`, allocation failure in
`ad_sndp_play`, and the existing null-handle warnings in `gsSndpStop` and
`gsSndpSetParam`. The other two ranges have no direct string reference. None
of the 38 functions uses an odd single-precision FP register, so §6.2's
hand-written-assembly exclusion removes no candidate from these ranges.

**Exact C promotions:** `getSoundStateCounts`, `gsSndpSetPriority`,
`gsSndpGetState`, `gsSndpStopAll`, `gsSndpStopAllRetrigger`,
`gsSndpStopAllLooped`, `gsSndpGetMasterVolume`, `gsSndpSetGlobalVolume`,
`gsSndpGetGlobalVolume`, `gsSndpLimitVoices`, `gsSndpStop` and
`gsSndpSetParam`, together with `sndp_stop_with_flags` and
`gsSndpSetMasterVolume` (`0x524` bytes total), are adapted JFG bodies compiled
with the TU's measured bare `-g -mips2 -32` flag group. Their linked owned
ranges are instruction-word-identical and the full ROM retains the expected
hash. Mickey-derived player initializer `gsSndpNew`, callback
`func_8005B978`, `func_8005CD3C`, event-queue unlinker `func_8005CE28`, and
sound-state allocator/releaser pair `func_8005D030`/`func_8005D260` add
another exact `0x818` bytes under the same flags, bringing exact C in
`main/gsSnd` to `0xD3C` bytes; JFG retains all six functions as assembly, so
their bodies are not donor adaptations.

`ad_sndp_play` sits inside this JFG-matched TU (its name, like every other
symbol in the whole-`.text` block above, comes from JFG's built `gsSnd.c.o`,
per the tier-A whole-TU byte match), but its *body* is not a JFG adaptation:
JFG keeps this function as assembly with no C source. The C written for it is
adapted from the corresponding permitted DKR/PD sound-player sequence logic
and then proved against Mickey, adding `0x2E8` exact bytes. Its nested
play/retrigger event lifetimes are required for IDO's target delay-slot
schedule. Exact C in `main/gsSnd` therefore totals `0x1024` bytes.

The adjacent pitch-event helper `func_8005CDAC` is adapted from the permitted
BK/PD sound-player implementations identified in its source provenance note.
A 16-byte raw event footprint and integer pitch-bit copy reproduce all 31
target instructions, the `0x30` frame, call relocations, and linked owned
range under bare `-g -mips2 -32`. It adds `0x7C` exact bytes, bringing exact C
in `main/gsSnd` to `0x10A0` bytes before the dispatcher promotion.

In `main/models`, `camConvertMatrixList`, initialization helper
`func_8005A700`, and the counter reset/flush pair `func_8005A764` and
`func_8005A770` (`0xD0` bytes total) are exact under the resident
`-O2 -mips2 -32` group. The first is adapted from JFG `camera.c`, and the
initialization helper from JFG `models.c`. Their function bytes and relocation
identities match in the linked ROM.

Mickey-derived parented matrix-list builder `func_8005B644` adds `0xCC`
proven exact bytes under the TU's measured `-Wo,-loopunroll,0` override,
bringing proven exact C in `main/models_5B300` to `0x19C`. The tracked
scoreboard carries another exact `0xB8` for raw-array `func_8005AAC0`, bringing
proven exact C in this TU to `0x254`. A target object independently rebuilt
from the historical pre-promotion split is instruction-identical to the
retained configured C across all 46 words with frame `0x20`. Its seven target
tuples exactly match the candidate: `D_800D7D04` HI/LO at `+0x14/+0x28`,
`D_800D7CF4` HI/LO at `+0x38/+0x3C` and `+0x7C/+0x84`, and `mmFree`
`R_MIPS_26` at `+0x74`. Linked function/TU/resident bytes are also exact. A
fresh current-source compile through full-ROM comparison remains queued as a
contemporaneous reproof, not as missing target evidence. JFG retains both
corresponding routines as assembly, so no donor body was adapted.

`func_8005A948` now has fresh configured full-TU proof. V0 has the target's
94-word size and frame `0x38`, with 62/94 raw and relocation-normalized words
(68/94 workbench-normalized), first mismatch `+0x40`, and all 13 relocation
tuples exact. All 119 flag
identities were nonexact; seven O2/MIPS-II configurations tie V0. A
fidelity-clean proc-4 allocator trace and aligned comparison show all 34
colored-variable slots agree while the temporary FIFO first trails by one pop
at slot 4. The sole natural boolean-normalization form fixes the downstream
rotation and improves to 85/94 raw/relocation-normalized and 91/94
workbench-normalized words. The remaining
register web is `t7` versus target `t8` at `+0x40/+0x44/+0x80`; call-argument
homes at `+0xF0/+0xFC/+0x100/+0x104/+0x120/+0x12C` are each four bytes below
target. There are zero runtime records or exports, `func_8005A7A0+0x104` is the
sole caller and passes an `lh` animation ID, and no overlay or stored-pointer
inbound exists. The function owns no target padding. The bounded route is
exhausted without a historical control or generic batch; assembly remains
canonical.

`func_8005A7A0` has fresh unchanged current-HEAD configured full-TU evidence at
106 words: 96/106 raw and 99/106 relocation-normalized object words, with ten
raw sites and seven relocation-normalized positional sites. The raw sites are
`+0x000/+0x08c/+0x0a0/+0x0bc/+0x0c0/+0x0c4/+0x0c8/+0x0dc/+0x0f4/+0x1a4`.
Its `0x50` candidate frame remains larger than the target's `0x38`, and the
alignment carrier uses `v1`/`sp+0x30` instead of `s0`/`sp+0x34`. All ten
relocation identities agree as a multiset and nine tuples are exact, but the
second `piRomLoadSection` call moves from target `+0xbc` to candidate `+0xc0`.
The real-address linked V0 is 94/106 raw and 99/106 relocation-normalized
words. More than ten pointer/index, size-expression, coalescing, scope, and
mask forms plus all 119 flag groups exhausted the family. The focused reproof
is complete; park pending a new frame/carrier allocation mechanism. Assembly
remains canonical. `func_8005ABA8` emits 110 against 111 and first diverges
at `+0x38` before an FP-allocation cascade. `func_8005AF14` now has a
Mickey-derived matrix/attachment-point candidate; its source comment discloses
the pinned JFG declaration files used only to cross-check field roles, while
JFG's peer body remains assembly. The target is 460 instructions with a
`-0xF8` frame versus the candidate's 463 and `-0x110`; the workbench reports
`structure-mismatch`, 377 differing words from `+0x0`. Assembly remains
canonical. Its `next:` routing is `constant-audit` on the earliest immediate,
then structural repair and only then register work; the frame/local shape and
camera-angle block are the candidate-specific structural focus.

In `main/vehicle_sounds`, the Mickey-derived handle cleanup loop
`func_800582A8` (`0x64` bytes) is exact under `-O2 -mips2 -32`; its linked
function bytes and call relocation match.

The Mickey-derived logarithm-series helper `func_80058EF4` is exact under
`-O2 -mips2 -32 -Wab,-r4300_mul`. A named loop-invariant square reproduces
the target FP lifetime coloring, and a direct integer-constant multiplication
reproduces its return-register coalescing. All 36 executable words and the
`D_80084318` relocation pair match; the following `0x0C` bytes are TU padding
and receive no credit. Exact executable C in `main/vehicle_sounds` now totals
`0xF4` bytes.

The remaining vehicle functions plateau without exact credit.
`func_80058250` has a fresh configured full-TU reproof at 26 candidate versus
22 target words, with 3/22 positional matches and first mismatch `+0x0`. The
target owns exactly ROM `0x58E50..0x58EA8`, is frameless, and has no padding
before `func_800582A8`. The TU owns the measured four-record
`0x800D78B0`–`0x800D78F0` BSS. Its twelve stores are semantically correct, but
the candidate carries 24 relocations against 20: the target uses separate
handle and racer-object bases for each record and reuses the latter high half
for the preceding float's distinct LO16, eliminating exactly four HI16 loads.
All twelve BSS identities and the sole call from `func_80004FE0+0x54C` are
authenticated. The earlier 119-configuration lattice, array, four-slot,
volatile and split-tail families, plus fresh target-order, TU-local-scalar,
comma-expression, racer-base-pointer and volatile-tail probes are nonexact;
none strictly improves the retained named-global V0. No exact or structurally
credible permitted donor was found. Resume only with an original declaration
or TU model that naturally emits two direct bases and three separately named
LO16s per slot, or evidence that the initializer was handwritten. Assembly
stays canonical.

`func_8005830C`: 758/762 words, 699 positional differences from `+0x0`, frame `0x110`/`0x118`.
Workbench reports mixed constant/structure/register mismatch; the next lever is constant-audit.
Its DKR organization/terminology provenance remains recorded at the body; assembly stays canonical.


### `func_8003D4FC` structural reconstruction evidence

The configured full-TU baseline reproduces 1,037 raw and relocation-masked
differing words: 1,066 candidate words against 1,068 target words, an eight-byte
deficit, frame `0x138`, first mismatch `+0xC`. This is compiled nonexact C;
the guarded assembly remains canonical. The workbench compare verdict is
`structure-mismatch`; diagnose routes the mixed residual through
`constant-audit` before `structure-buckets`. Applicable levers are the
constant/type audit and removal of duplicated source loops that fight IDO's
own loop transforms. No flag or permutation search is part of this reopening.

**Tier B, Mickey call and relocation evidence:** the target has fourteen
`R_MIPS_26` calls and no global-data relocations. They name `func_800349A4`
three times (initial material setup, batch flush, final flush),
`func_8002A8BC` and `func_8002A8C0` five times each (two angle pairs for each
of the first two geometry modes, one pair for the third), and `pointListRPY`
once (the general rotation loop). The retained candidate has the same ordered
call identities and count, but none of the call-site offsets match. That is
call-graph agreement, not exact relocation-tuple agreement. Existing Mickey
prototypes supply signed sixteen-bit angles, float returns, and the
three-component input/output contract for `pointListRPY`.

**Tier D, Mickey load/store and control-flow evidence:** the entry stride is
`0x78`; the signed type discriminator is at `0x2C`, the signed alpha word at
`0x64`, and the three colors and intensity are unsigned bytes. Batch alpha is
a separate signed sixteen-bit local holding the masked high byte; vertex
alpha is the signed shift result whose low byte is stored. The render-texture
selector is an unsigned halfword at offset six, distinct from the existing
`ParticleTexture.flags` field at offset four. The general rotation input is
one contiguous three-float vector, not independent address-taken scalars.

The draft and direct assembly review expose two retained-C correctness gaps:
a flush must revisit the first live entry instead of consuming it, and the
closing pipe sync belongs inside the nonempty-entry path. The entry count is
reloaded after calls, whereas point and primitive counts are captured before
initial material setup. These are structural reconstruction inputs, not
allocator-only experiments.

The fresh masked-skeleton oracle identifies JFG's existing placeholder sibling
as nearest (4,956 bytes versus Mickey's 4,272); no new donor material or name
was adopted. The generated m2c draft and scoped context stay ignored. The
all-header context failed on unrelated incomplete types; a particle-only
context generated successfully without changing project headers.

The authorized reconstruction finished at attempt 18 with **995 of 1,068
positional words exact**, 73 raw and relocation-masked differing words,
first mismatch `+0x2D0`, exact 4,272-byte extent, exact frame `0x138`, and
all fourteen offset/type/identity relocation tuples exact. Workbench `view`
reports `register-permutation`: zero structural, schedule, or constant
differences, with a consistent exchange of two integer temporary registers.
This remains compiled `NON_MATCHING` C, with no new matching-byte credit.

The meaningful reconstruction steps were separate batch/vertex alpha and a
real three-float vector; flush-versus-consume control flow and post-call
count reloads; equality-terminated point loops whose peeling and pipelining
belong to IDO; direct signed-point conversions and the observed rotation
expression trees; native GBI command scopes and DMA fields; local declaration
layout recovered from observed stack homes; promoted texture-selector values;
and batch initialization plus material-cache argument lifetimes. No artificial
padding, inert local, flag lattice, permuter, allocator trace, or object edit
was used. The retained source was rewritten in project idiom; the generated
draft was not adopted verbatim.

The baseline and all eighteen material attempts retain source, configured
full-TU object, full comparison report, concise metrics, relocation surface,
and rationale under ignored `build/wb/t2-3/attempt-*`. Attempt 7 first aligned
all fourteen relocations; attempt 11 restored exact size and frame; attempt 12
closed every stack offset; attempt 18 closed the remaining source schedule.
The narrowed/intermediate alpha experiments and the sizeof-based DMA length
regressed and are preserved as rejected evidence. Explicit cache assignments
inside the material-call arguments reproduced the target's update schedule.

**Stopping evidence:** this is a mechanism-scope handoff, not an attempt-cap
or five-attempt-stall claim. The last attempt still improved the result, but
it completed the authorized structural reconstruction and isolated a pure
register exchange. Continuing into allocator or permutation work would replace
the assignment's explicitly exclusive mechanism. The next concrete lever is
a separately authorized `register-role-audit` of the first vertex-DMA command's
integer temporary roles and expression evaluation order. Preserve the proven
frame, field types, CFG, and all fourteen relocation tuples while investigating
that residual; do not repeat the closed reconstruction or rejected type forms.

Validation of this guarded result: the fresh configured full-TU comparison
reproduced the attempt-18 metrics after source cleanup. `gmake verify` rebuilt
the canonical tree and reproduced the expected US ROM SHA1. That ROM equality
proves the assembly fallback and unchanged matched neighbors; it does not
promote this nonexact candidate. The target's owned range remains resident
ROM `0x3E0FC` through `0x3F1AC`, with no function padding credited.

**Closed (Evidence A, exact).** The 73-word register exchange was not an
allocator problem at all. The batch flush caches the texture and its scale for
the next comparison, and the guarded source spelled both caches as assignments
embedded in the material call's own argument list -- the shape the earlier
attempt log records as "explicit cache assignments inside the material-call
arguments reproduced the target's update schedule". Hoisting them out and
writing the flush as three ordinary statements -- the call, then each carrier
assignment on its own line -- closes the function: 1,068 of 1,068 words, frame
`0x138`, all fourteen relocation tuples, and `gmake verify` reproducing
`507341c0a40ca3e9a7cee969b396ee53facfb548` with the guard removed.

An assignment folded into an argument keeps that value live across the call in
addition to the argument copy, so ugen frees the two integer temporaries in the
opposite order at the first vertex-DMA command; the ring is one position out
from `+0x2D0` to the end of the function, and every one of the 73 words is that
one exchange. The lesson generalizes past this function: an embedded assignment
is a register-lifetime edit disguised as a spelling choice, and a residual that
is a pure two-register exchange with an exact frame and exact relocations
should be read as a liveness question about the statement *before* the first
divergence, not as an allocator contest at it.

### 2026-09-13: func_80006448 indexed resource release (lane f1)

Tier A byte identity: the existing placeholder at ROM 0x7048..0x7134,
VRAM 0x80006448..0x80006534, is now canonical C in src/main/objects.c.
All 236 executable bytes (59 words), the 0x28 frame, and four relocation
offset/type/symbol tuples match. The full linked ROM rebuild passes
`tools/gates.sh verify` at the expected US SHA1.

Deleting the resource carrier together with the declared byte-offset carrier
lets IDO generate the indexed address and load value. The residual falls from
nine words to two: aligned buckets change from 50 exact, six naming and three
structural to 57 exact and two naming. Joining the index initializer and do
statement onto one physical line resolves the final entry initialization swap,
yielding 59 exact with no remaining rows or gaps. The draw sequence stays at
ten throughout; the final edit moves two emission records onto the loop line.
Thus equal draw counts do not rule out an assembler scheduling improvement.

The three release paths, dispatch call, iteration domain and access widths
are preserved. No compiler force, flag change or post-compile instruction edit
is used. Stock/instrumented full-TU text identity and the exact fallback
relocation surface were checked before promotion; linked owned bytes and the
full ROM were compared after extraction and the canonical rebuild. Diagnostic
sources, objects and census differences remain ignored under
build/f1/func_80006448. The old plateau shard is retired in Git history.
