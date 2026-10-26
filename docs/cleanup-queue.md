# Cleanup queue: matched functions awaiting a more natural C form

Policy (user, 2026-08-27): when a match is only reachable via a non-idiomatic /
"looks fake" C form (a permuter-forced spelling — `do{}while(0)` boundaries,
`^ 0`, `if (1)`, `[x = 0]` index side-effects, redundant temps for register
pressure), **commit the working match now** with an in-source comment noting the
form is inert, and record it here. Return later to find a cleaner C that produces
the same bytes. These are byte-identical matches with **no fakes and no
ROM-derived content** — only the *spelling* is unnatural.

This exception records already-proved exact C; it never authorizes adding an
inert guard or operation to improve a non-exact candidate.
ADR 0017 separately permits defined, labelled source-shaping diagnostics inside
an authorized causal exploration packet. Those nonexact artifacts stay ignored;
neither diagnostic permission nor this cleanup policy authorizes their adoption.

A match here is DONE for scoreboard purposes; the cleanup is a quality follow-up,
not a correctness gap.

| Function | TU | Non-natural form | Notes for the cleaner pass |
|---|---|---|---|
| `func_overlay_064_F0000000_18C3B28` | overlay 64 texture generator | initialized integer identities `k OR= 0` and `lo OR= temp * 0`; obsolete unused `select` declaration | Independently reviewed under ADR 0017: both identities preserve initialized local values, with no added observable access or overflow. `select` survives from the earlier selector reconstruction and has no executed read; it is not newly introduced dummy padding. Seek natural lifetimes retaining 420 stock words, frame `0x70`, all 20 runtime relocation records, the 16-byte local data payload and linked ROM identity. |
| `func_overlay_045_F0001158_188D5B0` | overlay 45 rotated text | empty FP/integer comparisons, explicit operand carriers and packed triangle stores | Independently reviewed under ADR 0017; five former padding roles now carry actual initialized colors and indices. Seek natural lifetimes preserving 674 words, frame `0x190`, 25 relocation identities and linked ROM. Original uninitialized loop limit is separately preserved under owner-approved ADR 0019, not an inert spelling. |
| `wakeAllocate` | fx.c wake allocation | empty region, two initialized XOR-zero copies and four dead tail product assignments | Independently reviewed under ADR 0017. GLOBAL creates required expression owners before deleting the dead stores. Seek natural lifetimes preserving 351 stock words, frame `0x90`, three relocation identities and linked ROM bytes. |
| `func_80051364` | anim.c scene update | three initialized integer zero-XOR assignments; retained nested guards and do-while camera clear | Coordinator semantic review passed under ADR 0017. A dead duration redefinition blocks rate propagation before disappearing; seek natural lifetimes preserving 287 stock words, frame `0x40`, all 47 relocation identities and linked ROM bytes. |
| `func_8004B1DC` | font.c text renderer | empty duplicated nonvolatile colour-global test and redundant u8 width mask | Independently reviewed under ADR 0017; both forms are semantically inert. Removal of the test restores 25 register differences. Seek natural allocation preserving 556 stock words, frame `0x80`, exact relocation identities and linked ROM bytes. |
| `func_overlay_092_F0000308_18D6228` | overlay 92 camera update | initialized union representation self-copy `x.bits = x.bits` | Independently reviewed for IDO/o32; retains a stack-backed float lifetime without arithmetic or observable access. Seek natural spelling retaining 458 stock words, frame `0x70`, all 33 runtime relocation records and linked ROM identity. |
| `func_80011980` | track.c ray query | two initialized pointer OR-zero identities and two short-circuited zero guards, including a promoted halfword inside comparison | Independently reviewed under ADR 0017: the pointer identities preserve the configured 32-bit representation; pad stays zero, so the guarded comparison and FP copies never execute. Seek natural value lifetimes preserving 215 stock words, frame `0xC8`, all 12 relocation identities and linked ROM bytes. |
| `func_80010B4C` | track.c multi-point collision | inherited unsigned `bit = bit OR 0` in the retry loop | Initialized, nonvolatile and semantically inert; independently reviewed. Seek a natural bit/index lifetime retaining 678 stock words, frame `0x148`, all nine relocation records and linked ROM identity. |
| `func_8000E5EC` | track.c course display | dead `resultCount = segmentCount` inside the camera-segment product | `resultCount` is assigned 0 before any read. The store is what lets the product and the list cursor take the retail registers. Seek a natural live use retaining 205 words, frame `0xD8`, and the linked ROM bytes. |
| `func_8000590C` | objects.c spawn | dead `aligned` pointer reused as the unk48 nested integer | The nested add commutes object-first only when that operand is an isvar, and a new local moves the 0x90 frame. `offset` already carries the unk4C nested value; `aligned` is dead after the copy. Seek a natural integer temp that keeps 719 words, frame `0x90`, and 99 relocations. |
| `overlay19BuildAdjacency` | overlay 19 adjacency builder | fifth-argument assignment to the existing local inside the call | The local is private and unobserved by other arguments. Removing only the assignment adds a draw and breaks the selector emission order. Seek a plain argument retaining 123 words, frame `0x80`, and one relocation identity. |
| `overlay89InitializeEffect` | overlay 89 effect initializer | inert `if (1)` region around the maintain conditional | The region makes the retained state pointer stay caller-saved. Independent semantic review passed; seek natural grouping preserving 205 words and the runtime relocation surface. |
| `overlay1FindType5ByKey` | overlay 1 record lookup | single-execution `do/while(0)` predicate block and packed cursor/loop line | The unused key local was removed without changing output. Replacing the predicate scaffold with an ordinary block regressed; retain the used Boolean and bounds-before-offset ordering while seeking natural grouping. |
| `func_80046AA8` | diCpu | `do{}while(0)` around the setup block | The boundary forces IDO to close the setup as its own basic block before the blit loop. A cleaner equivalent (a real early computation dependency?) may exist. |
| `runlinkEnsureJumpIsValid` | runlink | four inert allocation aids: one `overlayCount` block and three `if (1) {}` blocks | The blocks preserve the exact long-lived-register allocation found by the bounded permuter. Seek a natural declaration/lifetime spelling with the same 101 words and 21 relocations. |
| `func_overlay_014_F00009F4_18702CC` | overlay 14 asset loader | inert `if (1) {}` around the relocation loop | The boundary preserves the exact pool allocation found by the annotated-target permuter; seek a natural loop/lifetime spelling. |
| `overlay1UpdateValueCache` | overlay 1 value cache | algebraically zero `remaining * 0` return | The live counter read preserves IDO's exact caller-saved web. Seek a natural zero return with the same 120 words and two local relocation pairs. |
| `overlay21ApplyPriorities` | overlay 21 plane priority pass | redundant `selected` comparison temporary (was `new_var`; the direct test differs in 10 words, a `continue` grows the function, lane c-5) | The temporary moves the overlay-local count reload into retail's caller-saved allocation. Seek a natural declaration/lifetime spelling with the same 114 words and nine relocation records. |
| `func_overlay_061_F0001648_18C0A10` | overlay 61 controller-pak character loader | one-iteration `do/while (0)` around the copy-size/copy block | The inert grouping preserves the exact caller-saved allocation. Seek a natural scope/lifetime spelling with the same 92 words and 11 relocation records. |
| `func_overlay_012_F00000C4_186D344` | overlay 12 release path | empty `!i` condition after fixed-count clear | The inert condition preserves exact allocation in the 60-word release routine. Seek a natural loop/epilogue spelling with the same bytes and relocation surface. |
| `overlay3FindClosestObject` | overlay 3 closest-object search | empty `!cursor` test and `if (1)` block | The two inert blocks rank the index web ahead of the cursor (s3/s4); the `&pad` probe was inert and was removed (lane c-4, 2026-10-09). Five loop shapes measured in the naturalness ledger. Seek a natural second use of the index with the same 77 words, frame and relocations. |
| `func_overlay_057_F0000954_18A454C` | overlay 57 interface update | empty read of `O57_config0954.field08` | The inert read preserves the exact register web in the 435-word body. Seek a natural use/lifetime spelling with the same bytes and relocations. |
| `func_800557F8` | anim collision callback | two empty `if (1)` blocks | The inert blocks preserve the target `v0` allocation. Seek natural call/result lifetimes with the same bytes and relocations. |
| `func_80055970` | anim collision callback | empty `if (1)` block | The inert block preserves the callback-result allocation. Seek a natural call/result lifetime with the same bytes and relocations. |
| `func_80055B24` | anim collision callback | empty `if (1)` block | The inert block preserves the post-callback allocation. Seek a natural typed-call/result lifetime with the same bytes and relocations. |
| `diRcpMoveWd` | diRcp debug decoder | duplicated empty `command` read plus the `command = index` store in the G_MW_FOG case | Together they keep the six empty cases as separate jump-table blocks; either removed merges them into the exit (-12 words). Guarding the final print on `command` (two spellings) did not reproduce (lane c-3, 2026-10-08). Seek a natural case body that is non-empty in the IR and emits nothing. |
| `__scHandleRSP` | scheduler RSP handler | empty task-flags condition | The inert condition preserves exact scheduler allocation. Seek a natural state/branch spelling with the same bytes and relocations. |
| `trackChangeFog` | track fog changer | empty `if (1)` block | The inert block preserves exact fog-distance allocation. Seek a natural scope/association spelling with the same bytes and relocations. |
| `overlay8ScaleOutputs` | overlay 8 scale outputs | empty `!index` condition | The inert condition preserves exact selector/index allocation. Seek a natural cast/lifetime spelling with the same bytes and relocations. |
| `overlay7FillValues` | overlay 7 value-table reset | empty duplicated `!value` condition | The comma operand was not required: an ordinary `*value = 0xF0` matches the previous object, including text, symbols, and relocations. Removing the condition as well keeps 11 words and both LOCAL records but swaps the two caller-saved temporaries. Seek a natural first temporary retaining 11 words and the two LOCAL HI16/LO16 records. |
| `overlay1UpdateCountdown` | overlay 1 countdown update | impossible empty pointer/mask condition plus packed statement line | Retained C is 22/22 after relocation masking with seven records. Removing only the condition (2026-10-04) keeps every symbol size and reschedules this function, so the object is not the retail one. Independently unpack the three real statements only after a spelling that keeps the current schedule. |
| `func_80035ADC` | texture sprite-frame builder | canceling triangle-pointer increment/decrement | The pair preserves the exact triangle-cursor/tile-end register allocation. Seek a natural pointer-update spelling retaining 235 words, the `0x48` frame, and all 12 relocation records. |
| `func_overlay_029_F0000EE0_187E190` | overlay 29 point projection | redundant single-precision identity multiply inside a cross product | The configured compiler removes the extra operation while retaining the exact multiply operand order. Seek a natural expression spelling retaining 121 words, the `0x68` frame, all three relocation identities, and the linked ROM bytes. |
| `overlay1AllocateRecord` | overlay 1 record allocator | redundant `0xFFFF` mask on a `u16` flags read | The width mask preserves the target temporary-FIFO phase. Seek a natural flags-read spelling retaining 40 words and all 10 runtime relocation records. |
| `overlay1AssignRecordIndex` | overlay 1 variable-record scan | redundant `newIndex` to `next` assignment | The two names preserve distinct decoded-record and candidate-maximum roles while steering IDO's exact allocation. Seek one natural carrier retaining 44 words, the `0x38` frame, and all seven runtime relocation records. |
| `overlay1FindBestRecord` | overlay 1 best-record scan | dead `value = 0` store plus the packed countdown/group statement line | The dead store reserves the `a1` pool colour and the packed line emits the group load ahead of the countdown; splitting the line costs the two-instruction swap and reversing the two statements costs two colours. Seek a natural spelling retaining 30 words, frameless, and both runtime relocation records. |
| `overlay68CheckKind` | overlay 68 probe check | `currentKind ^ 0` comparison | The algebraically inert xor keeps the copy-propagated sentinel first on the map-loop back-edge (L67). Seek a direct comparison retaining 80 words, frame `0x48`, and nine relocation identities. |
| `overlay31CreatePool` | overlay 31 pool allocator | inner-loop `zero |= 0` on a last-declared s32 | The algebraically inert OR-zero is an L109 probe that creates a phantom a0 web (save 100.5) so the record cursor is forbidden c3 and takes a1, and the outer index takes a2. Declaration order is load-bearing (`s32 i; s32 j;` first, `zero` last). Seek a natural occupant of a0 retaining 50 words, frame `0x38`, and four relocation identities. |
| `func_overlay_022_F0000D30_1878E38` | overlay 22 object removal | empty `if (i)` after the compact plus a loop-local `node` carrier | The empty test is an L100 occurrence that drops the search index save below the compact cursor so the index takes v1 and occupies v1 through compact; the node local makes ugen reuse v0 for the search load. Seek a natural live-range and load spelling retaining 91 words, frame `0x28`, and 12 relocation identities. |
| `overlay62Update` | overlay 62 fade update | `volatile` on the local `screenBase` | The qualifier is semantically inert -- the value is the same constant on all three paths -- but without it IDO folds the local away and the function loses six words. Seek a natural spelling retaining 294 words, the `0x88` frame, and all 71 relocation records. |
| `mmFreeTick` | memory delayed-free tick | empty `D_800D21B0 < 0xC000` guard inside the low-memory branch | JFG's `mmFreeTick` re-reads FreeRAM there and calls a module Mickey never links, so the block is empty; the second reference is what makes IDO materialize the global's address into a callee-saved register instead of folding the `%lo` into the load, and without it the function is one instruction short. Seek a natural spelling retaining 63 words, frame `0x30`, and all 12 relocation records. |
| `overlay99BuildHeightGrid` | overlay 99 height grid | empty `unused != 0` test on the second parameter | IDO homes an unreferenced named parameter to its incoming argument slot, which is one instruction the target does not have; the empty test removes the store and its position after the grid null check is load-bearing. Seek a natural use or signature with the same 114 words, frame `0x28`, and 29 relocation records. |
| `overlay17DrawStrip` | overlay 17 strip renderer | unsigned OR-zero reassignment of the initialized strip-packet cursor | Preserves all 32 pointer bits and every store and increment. It lowers the cursor save from 30 to 25 with no emitted instruction. Seek a natural spelling retaining 119 words, frame `0x38`, one relocation, and linked ROM identity. |
| `func_overlay_058_F000138C_18B0574` | overlay 58 results renderer | `i &= 0` after case 12's first call; `(0xFF - (i = 0))` resets in the last colour argument of cases 1, 12 and 13; an empty `!= 0` test on `columnStep` (the `columnCount` tests and the `i * 0` increment were removed byte-identically by lane c-1, 2026-10-08); unused `cursor` and `portraitX` declarations; a dead index local at two slot lookups | Each is zero instructions and load-bearing for one allocator or pass-order fact named at its point of use and in `docs/WHALE.md`. Seek natural spellings retaining 3614 words, frame `0x138` and 1253 relocation identities; try the probes first and the self-read with the guard reset last. |
| `overlay3SelectScoredObject` | overlay 3 scored-object search | unused `Overlay3Object **cursor` declaration | Holds the `0x80` frame: with one pointer-typed declaration fewer than the body declares the frame is `0x78` and 7 words differ; an unused `f32` is eliminated and does not hold it. Seek a natural used pointer local retaining 118 words, frame `0x80` and 5 relocation identities. |
| `func_overlay_035_F00001E0_1881EC0` | overlay 35 level-model loader | unused `void *unusedPad` declaration | Holds `mdl`'s home at `sp+0x38` (L99): without it 2 words differ. Seek a natural used local retaining 356 words, frame `0x40` and 63 relocation identities. |
| `func_overlay_041_F0001464_188879C` | overlay 41 process-entry | unused `Overlay41Input *pad` declared first | Occupies +0x44 so the address-taken `mode` and `child` homes sit at +0x40 and +0x38 (L99). Without it both homes are 4 bytes high and 7 immediate-only words remain. Seek a natural used pointer retaining 123 words, frame `0x48`, and 5 relocation identities. |
| `overlay89UpdateStateAndParticles` | overlay 89 state/particle update | two empty `do { } while (0)` statements | They add four blocks and no code so the particle-address web's split cost reaches its save under L56 and the web leaves `s1`. Seek a real statement (a conditional, a loop guard) that supplies the blocks, retaining 136 words, frame `0x88` and 14 relocations. Not reproduced (C3-natural, 2026-09-23; `do{}while(0)` removed in each): one empty region (+3 words); `count` assigned in the particle test; `while (count--)`; pre-decrement guard and loop; count read before the test; `(timer -= rate) <= 0`; `goto done` on frozen, on timer > 0, and both (+3 each); frozen early return with the tail duplicated (+8). `goto done` plus one empty region is byte-identical but still inert. Not reproduced (w4-o089, 2026-09-24): `while (0)`, `for (; 0;)`, `do { break; } while (1)` (including wrapped around the maintain tail), four fallthrough gotos, empty `u8 >= 0` tests, empty elses on the frozen and `timer <= 0` tests, and empty `timer == 0` / `active == 0` tests after those stores are byte-identical but still inert; `&&` of frozen with timer, `switch (frozen)`, a counted decrement loop, and proved-true ifs with the next real statement inside are not. |
| `func_80006B04` | objects slot scan | empty `do { } while (0)` before the `slots` read-back | The boundary keeps a coloured-but-never-emitted `slots` web on `t1` so the loop count takes `t2` as in the target. Seek a natural placement of that read, retaining 79 words, frameless, 0 relocations. Not reproduced (C3-natural, 2026-09-23; region removed in each): block scope, `slots` read inside the count test, `slots` declared at use, `output->unk74` read directly into the cursor, read after `var_v1`, read before `temp_t0`, unkA stored first, chained `temp_t0 = output->unkA = temp_a3`, `if (1)` for the count test; the `unk74` store hoisted above the `temp_a3 > 0` guard differs by schedule only (and stores when the count is 0). An empty `if (temp_a3 != 0) {}` is byte-identical but still inert. Reading `slots` as the first statement of the `temp_t0 > 0` body, with the empty region removed, kept 79 words but not the same bytes (2026-09-24). |
| `func_80037414` | frontend trap caller | `var_a1 \| 0` as the `TrapDanglingJump` argument | An operand of an operator is not copy-propagated, so the identity keeps `var_a1` itself in `a1`. Lane C3-natural found `TrapDanglingJump(arg0)` byte-identical only because the trampoline forwards a1/a2 as left by the preceding computation; rejected because the C would no longer state the arguments. Sixteen three-argument spellings did not reproduce the bytes. Seek a three-argument spelling retaining 145 words, frame `0x30` and 42 relocations. Not reproduced (w4-front): bare `(arg0, var_a1, var_a2)` including parenthesized, `register`, and initializer forms; call-site `(s32)(arg1 * 60.0f)` and `(s32)(arg2 * 60.0f)`; delay `sp28`; `D_8007BE94 = var_a1`; implicit conversion; a `frames` or `f32 dur` local; in-place `arg1 *= 60.0f`; an unsigned local; and a duration copied or recomputed after `func_800371BC` (13 words, or a larger size). |
| `func_8001B798` | character-plane collision | unreachable floating expression seed, initialized comparison carriers, empty guarded condition, distance clears, dead radius read and single-execution region | Independently reviewed under ADR 0017. The seed is guarded by an unused parameter assigned zero; added floating comparisons are dominated by the negative-distance test, excluding NaN operands without assuming disabled exceptions. Every retained declaration has a defined read. Seek natural expression ownership retaining 175 stock words, frame `0xE0`, all nine effective relocation identities and linked ROM bytes. |

Add rows as ugly-but-verified matches land. When revisiting: reproduce the match
with idiomatic C, `gmake verify`, then delete the row + the in-source comment.

## Additional bounded reconciliation

The table above retains all **51 required entries**. A committed-source check at
`3272f4ebb` also revisited the named findings in the historical
[naturalness ledger](NATURALNESS_LEDGER.md). It identified **44 additional
source-review candidates**, grouped below. This is a bounded reconciliation,
not an exhaustive current census and not a claim that 95 functions are fake.
No entry was removed and no source form was proved naturally resolved by this
check. The additional findings remain work; absence from the original table
never waives source-quality acceptance.

### Further unresolved inert-aid findings

These 29 functions retain the historical ledger's recorded form. Their prior
negative measurements remain useful; adding the disclosure does not authorize
another spelling sweep or certify every historical semantic explanation anew.
Cleanup still needs natural, exact C, or a specifically justified owner review
of the actual form. A name/type change is not presumed codegen-neutral.

| Function | Current TU | Form still present |
|---|---|---|
| `animseqFreeGroup` | `src/main/anim.c` | zero-XOR loop test and multiply-by-zero store |
| `func_80023598` | `src/main/camera.c` | single-execution region at the display-list join |
| `func_8001D2A0` | `src/main/charControl.c` | nonvolatile global self-assignment after the overlay call |
| `func_800580F0` | `src/main/flash_58C10.c` | three single-call do/while-zero wrappers |
| `func_8004C690` | `src/main/font.c` | OR-zero copy-loop increment |
| `func_80036DD0` | `src/main/frontend_37680.c` | all-ones mask on a compressed address |
| `frontSetScreenMode` | `src/main/menu.c` | zero-XOR store and address-taken local carrier |
| `func_8003A754` | `src/main/menu_3B1A0.c` | empty record/base inequality test |
| `func_8000BB84` | `src/main/objects.c` | empty constant-true region before reflection scaling |
| `effectboxControl` | `src/main/spranim.c` | one-statement region around the owner-pointer store |
| `func_8005830C` | `src/main/vehicle_sounds.c` | empty camera-pointer test |
| `overlay1StartTimerCallbacks` | `src/overlays/o001/overlay_001_tail.c` | empty post-callback mode-mask reread |
| `overlay1ConsumeNearbyPending` | `src/overlays/o001/overlay_001_tail.c` | empty object-pointer test before the state read |
| `func_overlay_008_F0001000_185ED58` | `src/overlays/o008/overlay_008.c` | empty unused-parameter test that avoids an argument home |
| `overlay59PrepareEntry` | `src/overlays/o059/overlay_059.c` | all-ones masks on the texture-load value |
| `overlay63Initialize` | `src/overlays/o063/overlay_063.c` | zero-XOR sentinel comparison |
| `func_overlay_073_F0000000_18CAAC0` | `src/overlays/o073/overlay_073.c` | multiply-by-zero state store |
| `func_overlay_079_F0001290_18CE230` | `src/overlays/o079/overlay_079.c` | retained constant-true region; earlier empty position test is resolved |
| `overlay101BuildBorder` | `src/overlays/o101/overlay_101.c` | two constant-true regions and colour lifetime constraints |
| `overlay68UpdateAnimation` | `src/overlays/o068/overlay_068.c` | narrowed index self-assignment before interpolation |
| `texDPTextureX` | `src/main/textures_354C8.c` | dead settings-word read |
| `func_80010654` | `src/main/track.c` | redundant unsigned-halfword mask |
| `func_80049B14` | `src/main/fx.c` | redundant carry clear in the case-two else arm |
| `func_80011CDC` | `src/main/track.c` | one-statement record-address region |
| `func_overlay_066_F00004E0_18C6948` | `src/overlays/o066/overlay_066.c` | two multiply-by-zero width expressions |
| `func_80038190` | `src/main/frontend_37D50.c` | cursor and loop-index OR-zero probes |
| `sprDPset` | `src/main/textures_354C8.c` | empty frame-index and settings-flag tests |
| `overlay1UpdateObjectPhysics` | `src/overlays/o001/overlay_001_tail.c` | two empty call-result tests; forced field reread is separately recorded |
| `func_overlay_008_F0001294_185EFEC` | `src/overlays/o008/overlay_008.c` | single-execution regions and dead update-parameter store |

### Inherited conditions requiring source review

The following two functions disclose an empty condition inherited from a
permitted donor. That is lineage evidence, not proof of an original debug body,
not a new correctness bug, and not automatic naturalness closure. Preserve
Mickey's behavior and the recorded failed removals while reviewing them.

| Function | Current TU | Form |
|---|---|---|
| `func_800180B4` | `src/main/shadows.c` | empty face-mask test; disclosed inherited form |
| `func_80035E88` | `src/main/textures_354C8.c` | empty texture-pointer test; disclosed inherited form |

### Resolved patterns with other work remaining

Resolving a historical pattern did not necessarily make the complete function
natural. These two findings are distinct from the resolved ones.

| Function | Current TU | Remaining form |
|---|---|---|
| `func_80037AEC` | `src/main/frontend_37D50.c` | Dead `row = 0` declaration initializer before the later loop initializer; the ledger records its allocation-numbering role. The old mask chain is gone. |
| `overlay82Update` | `src/overlays/o082/overlay_082.c` | Unused `currentValues++` and the documented packed store line remain; the old empty test and cursor-based smoothing loop are resolved. |

### Explicit region and frame-home families

These eleven functions were classified B in the historical ledger. They are
additional review work, not eleven newly established semantic defects.
Do not replace an unused home with a dummy object, invent a live use, or change
an ABI to preserve output.

| Function | Current TU | Form |
|---|---|---|
| `overlay13DrawActive` | `src/overlays/o013/overlay_013.c` | Constant-true region with a documented propagation/allocation role. |
| `overlay33InitializeBuffers` | `src/overlays/o033/overlay_033.c` | Constant-true region with a documented propagation/allocation role. |
| `func_overlay_038_F0000000_1885D10` | `src/overlays/o038/overlay_038.c` | Constant-true region with a documented propagation/allocation role. |
| `overlay101ScheduleLinkedPair` | `src/overlays/o101/overlay_101.c` | Unused `volatile s16 reservation` declaration. |
| `overlay101ScheduleLinkedPair2` | `src/overlays/o101/overlay_101.c` | Unused `volatile s16 reservation` declaration. |
| `overlay101ScheduleLinkedFloat` | `src/overlays/o101/overlay_101.c` | Unused `volatile s16 reservation` declaration. |
| `overlay101ScheduleLinkedScaled` | `src/overlays/o101/overlay_101.c` | Unused `volatile s16 reservation` declaration. |
| `overlay101ScheduleLinkedByte` | `src/overlays/o101/overlay_101.c` | Unused `volatile s16 reservation` declaration. |
| `overlay101ScheduleLinkedPair3` | `src/overlays/o101/overlay_101.c` | Unused `volatile s16 reservation` declaration. |
| `overlay101ScheduleLinkedColor` | `src/overlays/o101/overlay_101.c` | Unused `volatile s16 reservation` declaration. |
| `overlay101ScheduleFrames` | `src/overlays/o101/overlay_101.c` | Unused `volatile s16 reservation` declaration. |

### Reconciliation limits and useful next evidence

The historical A-name set has 75 functions: 23 overlap the original table,
21 have their previously identified A construct resolved, and 31 still have
that construct outside the table. The two residual forms and eleven B-family
functions above are separate; this gives a 95-name bounded review union.
Historical site totals and the wider B/C findings have not been regenerated.

Current `overlay1ActionModes.c` provides actual callback signatures and record
fields for reviewing the old O1 shifted callback/mask views. A shared type and
compatible reader declarations can be investigated under ADR 0020, preserving
all bytes and runtime identities. This is not proof that the post-callback
empty test can disappear, nor proof of a reader's dynamic index bound or the
original table capacity. Keep those questions separate.

Group future causal work by the measured compiler mechanism: region boundaries
and propagation, induction/operand materialization, frame homes/parameter uses,
and FP representation/reload lifetimes. Do not repeat previously eliminated
bare-block, direct-comparison, literal-zero, signature or statement-order
sweeps without new source or compiler evidence. SDK idioms, real debug-only
code, `goto`, address encoding and pads are not automatically inert aids.
