# Final Jet Force Gemini wring-out — 2026-08-27

## Outcome

This pass converted **8,460 executable resident bytes** from generated
assembly to compiler-exact C across **23 target function ranges**. The donor
source contains 24 C routines because two static `xldtob` helpers occupy one
anonymous pre-`_Ldtob` target range. One already-matched overlay function also
received its exact JFG identity, with no new byte credit.

The final Mickey build has SHA1
`507341c0a40ca3e9a7cee969b396ee53facfb548`, exactly the expected US ROM.
No instruction word was edited after compilation. Function boundaries,
relocations, linked ownership, and the full ROM were checked for every source
promotion.

The work was done in
`.-lane-jfg-final-wringout` on branch
`lane/jfg-final-wringout`, based on `campaign/unchain` commit
`e981f49097c19522917dff08b2152db448b58505`.

## Reference freshness

The JFG checkout was fetched and advanced to upstream `master` commit
`2f49ab3f7437f0b7ea8ed8da83e9f765b95fe4ff` dated 2026-08-25. The three
new upstream changes since the previous observed revision were the completed
`camera.c`, `prnBorder`, and `diRcpTrace` matches. The checkout is clean and
its US baserom SHA1 is the published
`493ced9008dbe932d6e91179b68e8630cf23a023`.

The latest checkout produced 1,014 object files, and the objects used by this
pass were rebuilt when their source freshness was material. A fresh final link
of current upstream does not complete: asm-processor-generated overlay
objects contain unresolved shortened symbols such as `func_0`, `func_8`, and
`D_4`, alongside relocation-overflow errors in non-matching animation code.
That is an upstream fresh-build/link defect, not a Mickey mismatch. Therefore
the reproducible reference lock in `docs/references.md` deliberately remains
the prior fully verified `c82affffe8f11cb5b440cfa918f4582ad8573279`
surface. The current 1,014-object surface was used for discovery and each
promotion was independently proved against Mickey's own linked ROM.

## Exact source promotions

Executable bytes exclude compiler alignment padding.

| Commit | Mickey target(s) | JFG source role | Bytes |
|---|---|---|---:|
| `73cc0aa3` | `diRcpTraceGetInfo` | `diRcpTraceGetInfo` | 300 |
| `16a48fa4` | `camReversePoint` | `camReversePoint` | 260 |
| `054ff552` | `atan2f` | maths source | 500 |
| `05be7496` | `acosf` | maths source | 220 |
| `2160316a` | `tableval`, `acoss`, `asins` | fixed-point arc helpers | 516 |
| `2c314b31` | `initColourCycle` | texture colour-cycle initializer | 48 |
| `9d65bb83` | `osFlashClearStatus` | libultra flash helper | 76 |
| `213f7686` | `amAudioMgrSetScheduleMode` | audio scheduler setter | 36 |
| `38899115` | `texEnableModes` | texture mode helper | 28 |
| `92f740f3` | `texLoadTextureAddr` | texture address helper | 40 |
| `593a5982` | `_Putfld`, `_Printf` | complete `xprintf` text/data/rodata | 3,252 |
| `b69a4b9b` | `_Ldunscale`, `_Genld`, `_Ldtob` | complete `xldtob` text/rodata | 2,748 |
| `37736a51` | sync-output helper, `rmonPrintf` | JFG `syncprintf` body under Mickey's established SDK name | 84 |
| `163a8b4e` | `mainproc`, `thread1_main` | boot/thread entry source | 240 |
| `8c2b3bcc` | `rsp_segment` | `segSetBase` | 56 |
| `dbd72d89` | `byteswap32` | `rzipUncompressSize` | 48 |
| `f390cc43` | `stack_pointer` | `diCpuTraceCurrentStack` using IDO `-dollar` | 8 |
| **Total** | **23 target ranges / 24 C routines** | | **8,460** |

Commit `9d4c354d` additionally identifies Mickey overlay 106's existing exact
8-byte C leaf as JFG `osRamTest3_6105`. It adds provenance and a name but no
new matched bytes.

The three large library promotions deserve a padding note. `xprintf` owns a
3,264-byte text section with 12 alignment bytes; `xldtob` owns 2,752 bytes
with four alignment bytes; the 96-byte `rmonprintf` section has 84 executable
bytes and 12 alignment bytes. Padding is linked and verified but never counted
as new executable credit.

## Exhaustion method and residuals

The pass first compared all 1,014 latest JFG objects against Mickey with exact
word comparisons masked only at the donor object's own relocation sites. It
then checked the start and exact size of every remaining raw resident function
range, including short functions omitted by the normal six-unmasked-word
adoption threshold. The broad pass covered 828 raw starts. A final exact-size
closure pass folded JFG's duplicate `.NON_MATCHING` symbols and classified
every surviving hit by the donor source that actually built it.

After the promotions above, **no remaining exact hit has a compiled JFG C
body**. The residual exact hits fall into these closed categories:

- original hand-written assembly: entrypoint, `lights2`, `math_util`, inflate,
  `exceptasm`, and the remaining libultra assembly leaves;
- JFG `GLOBAL_ASM` or disabled-C fallbacks: `GetRomlistInfo`, two shadow
  helpers, the menu peer of `func_8003968C`, and
  `refractOutputAssembler`;
- short generic wrappers whose bytes do not select one unique donor name or
  do not clear the unmasked/ROM-uniqueness floor.

The newly matched JFG `prnBorder` source has no plausible Mickey peer. A
masked four-instruction-gram comparison topped out at 0.0683 Jaccard in the
resident segment and 0.0498 in overlays. The completed camera source yielded
the exact 260-byte reverse-point helper above, but did not solve the residual
Mickey camera ranges at `func_80022FD4`, `func_80023598`, or
`camCopyOrthoMatrix`. The new `diRcpTrace` source yielded the exact 300-byte helper;
its large crash-controller peer remains assembly-only in JFG. These are
negative closure results, not partial matches.

## Recomputed campaign effect

The scoreboard was regenerated from the final ELF and assembly tree:

- matched C functions: 1,004 → 1,024;
- resident matched bytes represented by ELF function symbols:
  222,796 → 228,196;
- whole-program resolved bytes: 349,260 → 354,660;
- `GLOBAL_ASM` remaining: 284,524 → 276,064, the full **8,460-byte** gain;
- matched-function percentage: 68.58% → 70.09%;
- whole-program resolved percentage: 36.85% → 37.54%.

The ELF-function denominator shrank by three symbols and 3,060 bytes because
IDO does not retain function symbols for the now-static helpers in `xprintf`,
`xldtob`, and `rmonprintf`. Consequently the scoreboard's matched-byte delta is
5,400 while the generated-assembly reduction is 8,460. The latter is the
correct executable-byte credit; no code disappeared from the ROM.

## Validation record

Every source commit ran the worktree clean-room and documentation gates and a
single-job, low-priority full `gmake verify`. The final state also runs
`gmake check-scoreboard`, `gmake check-docs`, and `gmake cleanroom`. No ROM,
generated binary, emulator, GUI, browser, or test executable was run.
