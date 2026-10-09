# Resident donor coverage at the phase-100 baseline

Audit date: 2026-09-27. Scope is the committed ranking at
`d4f75a605837509edd4c7232e288db815d45e66e`: 108 resident guarded candidates,
126,876 executable bytes. This is donor scheduling evidence, not match proof
or authority to reopen a target. No matching source changed.

## Verdict

| Classification | Functions | Bytes |
| --- | ---: | ---: |
| actionable-donor | 0 | 0 |
| exhausted-matrix | 0 | 0 |
| bounded-negative | 50 | 61900 |
| uncovered | 58 | 64976 |

An actionable donor means an authenticated, newly applicable source input,
not a body already adapted or an allocator force. None was established here.
No complete source/version/flag/conditional/TU matrix was established for a
remaining target; zero exhausted-matrix rows is deliberate. A bounded negative
covers only its named scan or pinned counterpart. Uncovered means insufficient
target-bound coverage evidence, not that no previous donor investigation ever
occurred. Several uncovered functions already have disclosed donor-derived C;
that alone does not establish coverage of the other relevant inputs.

## Reference identities and reused evidence

The DKR, Perfect Dark, Banjo-Kazooie and Conker object surfaces reproduce their
locked counts and mining digests. JFG requires two distinct identities:

- Current committed reference source is `efd5abb1c79636e297b831f7c2d5bf47eac39c0c`.
- Its 772-object mining surface still reproduces the digest locked to
  `c82affffe8f11cb5b440cfa918f4582ad8573279`. These are the older compiled
  objects, not proof of a build from the newer source.

The aggregate verifier correctly fails JFG's checkout-revision check. A
separate direct mining digest confirms the old object surface is reusable.
No lock was updated and no reference was rebuilt. The other locked surfaces
contain 243 DKR, 2,546 Perfect Dark, 1,232 Banjo-Kazooie and 1,446 Conker
objects. Their differing historical build outcomes remain as documented in
[references.md](references.md); object identity does not prove source coverage.

The September 8 track and fx donor audits remain valid only for their pinned
source scope. The fx anchor shard was deleted when its own function matched;
its evidence was recovered from commit
`0124e56849a0eca16f5c657a6649d9fafb2b7d4f`, not another lane's files. Its old
links do not establish new C availability. Source, handoff and inspected
reference fingerprints are retained in ignored audit evidence.

The stock skeleton scanner recursively loads object files, whereas the lock
fingerprints named object roots. The two new queries deliberately restrict
the corpus to those verified roots. Historical scans that omit corpus hashes
are retained as bounded reports, not silently upgraded to authenticated
whole-farm exhaustion.

## Two named missing-input checks

Only `func_80053868` and `func_800517E0` received new object scans. Their
existing receipts lacked an unrestricted-size check. The same five verified
object surfaces supplied 20,933 function records of at least ten words;
masked four-gram similarity was compared both within the old thirty-percent
size window and without a size filter. Scores are diagnostics, not identity.

For `func_80053868`, the unrestricted leader is unrelated Conker code at
approximately 0.056 similarity. No credible source donor was established.
JFG's independently named `hitUpdate` counterpart remains GLOBAL_ASM in the
newer pinned `src/hit.c`. For `func_800517E0`, JFG's command-list counterpart
remains the clear leader at approximately 0.203, versus approximately 0.036
for the next result; the newer pinned `src/anim.c` still has no C body for it.
Neither result exhausts other source versions or compiler configurations.

## Coverage by exact target

B means bounded-negative; U means uncovered under the definition above.
Each target links to its committed per-symbol evidence, or source when no
shard exists. The family notes
state the reused scope; they do not infer identity from a shared TU name.

### `src/main/anim.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_80051364](matching-triage-handoffs/func_80051364.md) | 1148 | B |
| [func_80056DD8](matching-triage-handoffs/func_80056DD8.md) | 916 | U |
| [func_80054B3C](matching-triage-handoffs/func_80054B3C.md) | 1480 | U |
| [func_80055104](matching-triage-handoffs/func_80055104.md) | 1780 | U |
| [func_800563B4](matching-triage-handoffs/func_800563B4.md) | 2596 | U |
| [func_80053868](matching-triage-handoffs/func_80053868.md) | 4820 | B |
| [func_800517E0](matching-triage-handoffs/func_800517E0.md) | 7232 | B |

New unrestricted-size five-reference scan has no credible leader; pinned JFG hitUpdate is assembly-only. New unrestricted-size scan confirms JFG animseqProcessCommandList; pinned newer source remains assembly-only. Pinned newer JFG animseqUpdate remains assembly-only; existing point-of-use provenance already uses assembly context.

### `src/main/audio_manager_36D0.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_80003480](matching-triage-handoffs/func_80003480.md) | 376 | B |

JFG peer is assembly-only; historical near-match query also lacked an ownership size.

### `src/main/audiomgr.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_80001740](matching-triage-handoffs/func_80001740.md) | 836 | B |
| [func_80001BF4](matching-triage-handoffs/func_80001BF4.md) | 1344 | U |

JFG amCreateAudioMgr remains GLOBAL_ASM at efd5abb; published shape already used.

### `src/main/block_4F4E0.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_8004EC60](matching-triage-handoffs/func_8004EC60.md) | 328 | U |

### `src/main/camera.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_80023598](matching-triage-handoffs/func_80023598.md) | 1136 | B |
| [func_80022FD4](matching-triage-handoffs/func_80022FD4.md) | 1476 | U |

Pinned JFG camera audit supplies no new counterpart for this high-level sprite path.

### `src/main/charControl.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_8001D880](matching-triage-handoffs/func_8001D880.md) | 144 | U |
| [func_8001C4C0](matching-triage-handoffs/func_8001C4C0.md) | 1612 | U |
| [func_8001EC44](matching-triage-handoffs/func_8001EC44.md) | 952 | B |
| [func_8001DD70](matching-triage-handoffs/func_8001DD70.md) | 2132 | U |
| [func_8001E5C4](matching-triage-handoffs/func_8001E5C4.md) | 1664 | U |

Recorded five-reference size-window scan found no credible donor; not a version-matrix exhaustion.

### `src/main/diCpu.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_80046BCC](matching-triage-handoffs/func_80046BCC.md) | 424 | U |
| [render_epc_lock_up_display](matching-triage-handoffs/render_epc_lock_up_display.md) | 1376 | B |
| [func_80045D34](matching-triage-handoffs/func_80045D34.md) | 1836 | B |

Pinned JFG diCpu source audit supplies no complete C counterpart.

### `src/main/font.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_8004C690](matching-triage-handoffs/func_8004C690.md) | 584 | U |
| [func_8004B1DC](matching-triage-handoffs/func_8004B1DC.md) | 2224 | B |

Pinned JFG font body unchanged after renames; source differences already compared against Mickey.

### `src/main/frontend_37D50.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_80037C74](matching-triage-handoffs/func_80037C74.md) | 1308 | U |
| [func_80038190](matching-triage-handoffs/func_80038190.md) | 1472 | U |

### `src/main/fx.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [fxAllocateCone](matching-triage-handoffs/fxAllocateCone.md) | 440 | B |
| [func_800470B0](matching-triage-handoffs/func_800470B0.md) | 596 | B |
| [func_8004A10C](matching-triage-handoffs/func_8004A10C.md) | 628 | B |
| [func_80047304](matching-triage-handoffs/func_80047304.md) | 740 | B |
| [fxSPDPRipple](matching-triage-handoffs/fxSPDPRipple.md) | 928 | B |
| [func_80049B14](matching-triage-handoffs/func_80049B14.md) | 824 | B |
| [wakeUpdate](matching-triage-handoffs/wakeUpdate.md) | 1592 | B |
| [fxScreenEffect](matching-triage-handoffs/fxScreenEffect.md) | 588 | B |
| [wakeAllocate](matching-triage-handoffs/wakeAllocate.md) | 1404 | B |
| [fxMakeConeTextureCoords](matching-triage-handoffs/fxMakeConeTextureCoords.md) | 1004 | B |

Pinned JFG fx audit: assembly-only TU; reused deleted anchor through committed Git history.

### `src/main/joy.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [joyRead](matching-triage-handoffs/joyRead.md) | 636 | U |

### `src/main/level.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [levelInit](matching-triage-handoffs/levelInit.md) | 2064 | U |

### `src/main/lights.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_80018F08](matching-triage-handoffs/func_80018F08.md) | 820 | U |
| [lightObject](matching-triage-handoffs/lightObject.md) | 736 | B |
| [func_8001953C](matching-triage-handoffs/func_8001953C.md) | 1016 | U |

JFG lightObject remains GLOBAL_ASM at efd5abb.

### `src/main/main.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_80028FCC](matching-triage-handoffs/func_80028FCC.md) | 108 | B |
| [func_80029274](matching-triage-handoffs/func_80029274.md) | 348 | U |
| [func_80026FB4](matching-triage-handoffs/func_80026FB4.md) | 1652 | U |
| [func_80028564](matching-triage-handoffs/func_80028564.md) | 1956 | U |

JFG mainAnyoneHas remains assembly-only at efd5abb; no new C donor.

### `src/main/matrix.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| MatrixMultiplyVec4 (func_8002AF6C) | 212 | U |
| [func_8002B040](matching-triage-handoffs/func_8002B040.md) | 136 | U |
| func_8002AB78 | 268 | B |
| [func_8002AA50](matching-triage-handoffs/func_8002AA50.md) | 296 | B |
| func_8002AC84 | 396 | B |
| func_8002AE10 | 348 | B |

Known JFG counterpart is handwritten assembly; stock-IDO ABI/register limitation remains, not a proved all-source matrix.

Superseded on 2026-10-07 for four of these rows: func_8002AB78,
func_8002AC84, func_8002AE10 and func_8002AF6C are byte-identical (R_MIPS_26
masked) to routines two to five of JFG's hand-written `hasm/math_matrix`
object and are now the verified-assembly subsegment `main/math_matrix`, so
they left the queue and their shards were retired. func_8002B040 moved to
`src/main/matrix_2BC40.c`.

### `src/main/menu.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [frontDrawObj](matching-triage-handoffs/frontDrawObj.md) | 1048 | U |

### `src/main/models.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [modSetTextureFrame](matching-triage-handoffs/modSetTextureFrame.md) | 192 | B |
| [func_80020B10](matching-triage-handoffs/func_80020B10.md) | 636 | B |
| [makeModelGfx](matching-triage-handoffs/makeModelGfx.md) | 1368 | B |
| [func_8001FC50](matching-triage-handoffs/func_8001FC50.md) | 1332 | U |
| [modLoadModel](matching-triage-handoffs/modLoadModel.md) | 1604 | U |

Pinned JFG models counterparts remain assembly-only; point-of-use provenance records the audit.

### `src/main/models_5B300.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_8005ABA8](matching-triage-handoffs/func_8005ABA8.md) | 444 | U |
| [func_8005AF14](matching-triage-handoffs/func_8005AF14.md) | 1840 | B |

Recorded five-reference size-window scan found no credible donor; not a version-matrix exhaustion.

### `src/main/objects.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_80007C68](matching-triage-handoffs/func_80007C68.md) | 472 | U |
| [func_80006534](matching-triage-handoffs/func_80006534.md) | 820 | B |
| [func_8000BB84](matching-triage-handoffs/func_8000BB84.md) | 260 | U |
| [func_80006FA0](matching-triage-handoffs/func_80006FA0.md) | 376 | U |
| [func_8000831C](matching-triage-handoffs/func_8000831C.md) | 424 | U |
| [func_80004FE0](matching-triage-handoffs/func_80004FE0.md) | 1384 | U |
| [func_80007E40](matching-triage-handoffs/func_80007E40.md) | 488 | B |
| [func_8000B3CC](matching-triage-handoffs/func_8000B3CC.md) | 1976 | U |
| [func_8000A39C](matching-triage-handoffs/func_8000A39C.md) | 656 | B |
| [func_80007118](matching-triage-handoffs/func_80007118.md) | 1836 | U |
| [func_800084C4](matching-triage-handoffs/func_800084C4.md) | 1372 | B |
| [func_80009414](matching-triage-handoffs/func_80009414.md) | 1684 | U |

Bounded nearest-shape/source inspection found only assembly-backed or noncredible counterparts. JFG assembly counterpart and DKR body examined; DKR differs in guards and edge/mask behavior.

### `src/main/particles.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_8003D25C](matching-triage-handoffs/func_8003D25C.md) | 672 | U |
| [func_80040B88](matching-triage-handoffs/func_80040B88.md) | 1208 | U |

### `src/main/rcpFast3d.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_8002EBE0](matching-triage-handoffs/func_8002EBE0.md) | 1020 | U |
| [func_8002FB34](matching-triage-handoffs/func_8002FB34.md) | 1436 | U |

### `src/main/runlink.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [ProcessRelocationEntry](matching-triage-handoffs/ProcessRelocationEntry.md) | 584 | U |

### `src/main/saves.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_8002CF6C](matching-triage-handoffs/func_8002CF6C.md) | 352 | U |

### `src/main/sched.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_80030610](matching-triage-handoffs/func_80030610.md) | 768 | U |

### `src/main/shadows.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_800180B4](matching-triage-handoffs/func_800180B4.md) | 824 | U |
| [func_80017BCC](matching-triage-handoffs/func_80017BCC.md) | 1256 | U |
| [func_80017140](matching-triage-handoffs/func_80017140.md) | 1312 | U |
| [shadowGenerate](matching-triage-handoffs/shadowGenerate.md) | 2040 | U |
| [func_80016890](matching-triage-handoffs/func_80016890.md) | 2224 | U |

### `src/main/spranim.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [effectboxControl](matching-triage-handoffs/effectboxControl.md) | 772 | U |
| [func_8001B798](matching-triage-handoffs/func_8001B798.md) | 700 | U |

### `src/main/textures_354C8.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [texLoadSprite](matching-triage-handoffs/texLoadSprite.md) | 1076 | U |
| [texDPTextureX](matching-triage-handoffs/texDPTextureX.md) | 1088 | U |
| [texAnimateSprite](../src/main/textures_354C8.c) | 608 | U |
| [func_80035F48](matching-triage-handoffs/func_80035F48.md) | 1532 | U |
| [sprDPset](matching-triage-handoffs/sprDPset.md) | 1868 | U |

### `src/main/track.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_80010654](matching-triage-handoffs/func_80010654.md) | 684 | B |
| [func_8001398C](matching-triage-handoffs/func_8001398C.md) | 1320 | B |
| [func_8000F198](matching-triage-handoffs/func_8000F198.md) | 996 | B |
| [func_800140CC](matching-triage-handoffs/func_800140CC.md) | 868 | B |
| [func_8001357C](matching-triage-handoffs/func_8001357C.md) | 1040 | B |
| [func_800103D4](matching-triage-handoffs/func_800103D4.md) | 640 | B |
| [func_8000E5EC](matching-triage-handoffs/func_8000E5EC.md) | 820 | B |
| [func_80011980](matching-triage-handoffs/func_80011980.md) | 860 | B |
| [func_800115E4](matching-triage-handoffs/func_800115E4.md) | 924 | B |
| [func_8000DFBC](matching-triage-handoffs/func_8000DFBC.md) | 1584 | B |
| [func_80011CDC](matching-triage-handoffs/func_80011CDC.md) | 1368 | B |
| [func_8000E920](matching-triage-handoffs/func_8000E920.md) | 2168 | B |
| [func_8001291C](matching-triage-handoffs/func_8001291C.md) | 2192 | B |
| [func_80010B4C](matching-triage-handoffs/func_80010B4C.md) | 2712 | B |

Pinned JFG track audit: implemented routines do not supply this target; ordered assembly counterpart only.

### `src/main/vehicle_sounds.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_80058250](matching-triage-handoffs/func_80058250.md) | 88 | U |
| [func_8005830C](matching-triage-handoffs/func_8005830C.md) | 3048 | B |

Recorded DKR vehicle-audio comparison supplies organization only, no credible new body.

### `src/main/weather.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [rain_render_splashes](matching-triage-handoffs/rain_render_splashes.md) | 1616 | U |

### `src/main/weather_tail.c`

| Target | Bytes | Coverage |
| --- | ---: | --- |
| [func_8003C80C](matching-triage-handoffs/func_8003C80C.md) | 472 | B |

Recorded five-reference size-window scan found no credible donor; not a version-matrix exhaustion.

## Next animation packet prerequisites

The donor audit supports returning to Mickey-authenticated compiler structure
for `func_80053868`; it does not reopen the source by itself. Preserve the
latest source/handoff pins and replace the old generic authorization reason
with the concrete callback-constant block-membership question, then require
a zero-exit base-only assignment verdict before source edits.

Reproduce the configured complete `src/main/anim.c` baseline and actual
preprocessed compiler-input self-comparison. Map the function through retained
Ucode, rather than reusing the historical procedure ordinal. Re-establish
stock/instrumented fidelity and current relocation identities. The historical
receipt priced three callback constants across 58 blocks; its force recovered
48 words but was not a source solution. The first new evidence must identify
which source definitions and joins contribute those blocks. A source-line
coincidence or another force is not that attribution.

Preserve the proved three-float buffer, two-output-pointer callee ABI, exact
frame size, FP association, callbacks and list order. Do not repeat the
byte-identical callback wrapper, preincrement join or address-taking reload,
or the already-regressing simple-loop and generic unroll alternatives.
If block membership cannot be independently mapped, report that specific
producer prerequisite before spending source attempts.

## Validation and cost

The search and reference-verification phase compiled zero game or reference
objects. Initial lane preparation separately compiled 851 canonical game
objects; its elapsed bootstrap time was not instrumented. That setup cost is
not included in the search measurements below. Reference verification
took approximately 1.6 seconds; the two named scans shared one corpus load and
took approximately 2.3 seconds. These are measured subprocess costs, not total
worker effort or expected matching throughput. No whole-queue scan, reference
rebuild, flag matrix or source permutation was run. Detailed target records,
reference/source fingerprints, timings and scan results stay ignored under
`build/donor-audit/`. Documentation and cleanroom validation accompanies this
report-only commit; no candidate or ROM match claim is made.
