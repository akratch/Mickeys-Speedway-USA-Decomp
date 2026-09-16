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
| `overlay19BuildAdjacency` | overlay 19 adjacency builder | fifth-argument assignment to the existing local inside the call | The local is private and unobserved by other arguments. Removing only the assignment adds a draw and breaks the selector emission order. Seek a plain argument retaining 123 words, frame `0x80`, and one relocation identity. |
| `overlay89InitializeEffect` | overlay 89 effect initializer | inert `if (1)` region around the maintain conditional | The region makes the retained state pointer stay caller-saved. Independent semantic review passed; seek natural grouping preserving 205 words and the runtime relocation surface. |
| `overlay1FindType5ByKey` | overlay 1 record lookup | single-execution `do/while(0)` predicate block and packed cursor/loop line | The unused key local was removed without changing output. Replacing the predicate scaffold with an ordinary block regressed; retain the used Boolean and bounds-before-offset ordering while seeking natural grouping. |
| `func_80046AA8` | diCpu | `do{}while(0)` around the setup block | The boundary forces IDO to close the setup as its own basic block before the blit loop. A cleaner equivalent (a real early computation dependency?) may exist. |
| `func_800320F0` | runlink | four inert allocation aids: one `overlayCount` block and three `if (1) {}` blocks | The blocks preserve the exact long-lived-register allocation found by the bounded permuter. Seek a natural declaration/lifetime spelling with the same 101 words and 21 relocations. |
| `runlinkUnloadOverlay` | runlink | `patchOperation ^ 0` comparison | The algebraically inert xor retains exact comparison operand allocation. Seek a direct comparison with the same bytes and relocation surface. |
| `func_8002C94C` | saves | `if (1)` around entry initialization | The block preserves the exact callee-saved slot/counter tie-break. Seek a natural declaration or loop-scope spelling retaining exact bytes and relocations. |
| `piRomLoadCompressed` | pi | empty `if (1)` before the ROM copy | The empty boundary preserves the exact tail-call parameter lifetime. Seek a natural gzip/output-pointer scope retaining exact bytes and relocations. |
| `func_80003760` | audio manager | canceling uninitialized pointer increment/decrement plus one-iteration `do/while` | The legacy operations preserve the exact 25-word temp/FIFO allocation. Replace them with defined, natural initialization and grouping while retaining frame, relocations, and linked bytes. |
| `overlay40AddEntry` | overlay 40 entry allocator | repeated masks of known-zero `new_var` added to `green` | The algebraically zero expression preserves the exact entry/scales register allocation. Seek a natural declaration/lifetime spelling retaining 33 words and four relocations. |
| `func_overlay_014_F00009F4_18702CC` | overlay 14 asset loader | inert `if (1) {}` around the relocation loop | The boundary preserves the exact pool allocation found by the annotated-target permuter; seek a natural loop/lifetime spelling. |
| `overlay1UpdateValueCache` | overlay 1 value cache | algebraically zero `remaining * 0` return | The live counter read preserves IDO's exact caller-saved web. Seek a natural zero return with the same 120 words and two local relocation pairs. |
| `overlay21ApplyPriorities` | overlay 21 plane priority pass | redundant `new_var` comparison temporary | The temporary moves the overlay-local count reload into retail's caller-saved allocation. Seek a natural declaration/lifetime spelling with the same 114 words and nine relocation records. |
| `func_overlay_061_F0001648_18C0A10` | overlay 61 controller-pak character loader | one-iteration `do/while (0)` around the copy-size/copy block | The inert grouping preserves the exact caller-saved allocation. Seek a natural scope/lifetime spelling with the same 92 words and 11 relocation records. |
| `overlay59DrawFrame` | overlay 59 frame renderer | empty duplicated `!index` condition | The inert condition preserves the exact entry/owner register order. Seek a natural declaration or lifetime spelling with the same 76 words and relocation surface. |
| `func_overlay_012_F00000C4_186D344` | overlay 12 release path | empty `!i` condition after fixed-count clear | The inert condition preserves exact allocation in the 60-word release routine. Seek a natural loop/epilogue spelling with the same bytes and relocation surface. |
| `overlay12Initialize` | overlay 12 resource initializer | empty `!remaining` condition after fixed-count clear | The inert condition preserves exact allocation in the 49-word initializer. Seek a natural loop/epilogue spelling with the same bytes and relocation surface. |
| `overlay3FindClosestObject` | overlay 3 closest-object search | empty `&pad`, `!cursor`, and `if (1)` blocks | The three inert blocks preserve the exact 77-word allocation/control-flow form. Seek natural declarations/scopes with the same frame and relocations. |
| `func_overlay_057_F0000954_18A454C` | overlay 57 interface update | empty read of `O57_config0954.field08` | The inert read preserves the exact register web in the 435-word body. Seek a natural use/lifetime spelling with the same bytes and relocations. |
| `overlay82Update` | overlay 82 update | empty `currentValues` read | The inert read preserves IDO's exact web priority in the 291-word body. Seek a natural declaration/lifetime spelling with the same bytes and relocations. |
| `func_800557F8` | anim collision callback | two empty `if (1)` blocks | The inert blocks preserve the target `v0` allocation. Seek natural call/result lifetimes with the same bytes and relocations. |
| `func_80055970` | anim collision callback | empty `if (1)` block | The inert block preserves the callback-result allocation. Seek a natural call/result lifetime with the same bytes and relocations. |
| `func_80055B24` | anim collision callback | empty `if (1)` block | The inert block preserves the post-callback allocation. Seek a natural typed-call/result lifetime with the same bytes and relocations. |
| `diRcpDmaOffsets` | diRcp debug decoder | empty `dList` read | The inert read preserves exact debug-call argument allocation. Seek a natural formatting expression with the same bytes and relocations. |
| `diRcpMoveWd` | diRcp debug decoder | duplicated empty `command` read | The inert read preserves exact switch/formatting allocation. Seek a natural command lifetime with the same bytes and relocations. |
| `partObjFreeTriggers` | particle trigger cleanup | empty `offset` read | The inert read preserves exact loop allocation. Seek a natural pointer/index spelling with the same bytes and relocations. |
| `__scHandleRSP` | scheduler RSP handler | empty task-flags condition | The inert condition preserves exact scheduler allocation. Seek a natural state/branch spelling with the same bytes and relocations. |
| `func_800148E0` | track fog changer | empty `if (1)` block | The inert block preserves exact fog-distance allocation. Seek a natural scope/association spelling with the same bytes and relocations. |
| `overlay8ScaleOutputs` | overlay 8 scale outputs | empty `!index` condition | The inert condition preserves exact selector/index allocation. Seek a natural cast/lifetime spelling with the same bytes and relocations. |
| `overlay27Activate` | overlay 27 state transition | duplicated empty state-null condition | The inert condition preserves exact saved-object allocation. Seek a natural assignment/lifetime spelling with the same bytes and relocations. |
| `overlay7FillValues` | overlay 7 value-table reset | empty duplicated `!value` condition and dummy comma-expression operand | Both inert forms preserve the exact 11-word allocation. Seek a natural spelling retaining 11 words and the two LOCAL HI16/LO16 records. |
| `overlay1UpdateCountdown` | overlay 1 countdown update | impossible empty pointer/mask condition plus packed statement line | Retained C is 22/22 after relocation masking with seven records. First remove only the condition; independently unpack the three real statements, retaining exact bytes. |
| `overlay80UpdateContact` | overlay 80 contact updater | redundant `new_var`, `new_var2`, and `new_var3` aliases | The aliases preserve IDO's shipped 180-word allocation and `0x80` frame. Seek natural declaration/lifetime spelling retaining all 20 relocation records. |
| `func_80035ADC` | texture sprite-frame builder | canceling triangle-pointer increment/decrement | The pair preserves the exact triangle-cursor/tile-end register allocation. Seek a natural pointer-update spelling retaining 235 words, the `0x48` frame, and all 12 relocation records. |
| `func_overlay_029_F0000EE0_187E190` | overlay 29 point projection | redundant single-precision identity multiply inside a cross product | The configured compiler removes the extra operation while retaining the exact multiply operand order. Seek a natural expression spelling retaining 121 words, the `0x68` frame, all three relocation identities, and the linked ROM bytes. |
| `overlay1AllocateRecord` | overlay 1 record allocator | redundant `0xFFFF` mask on a `u16` flags read | The width mask preserves the target temporary-FIFO phase. Seek a natural flags-read spelling retaining 40 words and all 10 runtime relocation records. |
| `overlay1AssignRecordIndex` | overlay 1 variable-record scan | redundant `newIndex` to `next` assignment | The two names preserve distinct decoded-record and candidate-maximum roles while steering IDO's exact allocation. Seek one natural carrier retaining 44 words, the `0x38` frame, and all seven runtime relocation records. |
| `func_800336A8` | gameVi mode changer | donor empty `if (1) {}` before the non-widescreen framebuffer assignment | The constant-only empty statement has no side effects and preserves exact IDO allocation with direct triple-buffer global reads. Seek natural grouping retaining 195 words, the `0x28` frame, all 77 relocation identities, and linked ROM bytes. |
| `overlay1FindBestRecord` | overlay 1 best-record scan | dead `value = 0` store plus the packed countdown/group statement line | The dead store reserves the `a1` pool colour and the packed line emits the group load ahead of the countdown; splitting the line costs the two-instruction swap and reversing the two statements costs two colours. Seek a natural spelling retaining 30 words, frameless, and both runtime relocation records. |

| `overlay62Update` | overlay 62 fade update | `volatile` on the local `screenBase` | The qualifier is semantically inert -- the value is the same constant on all three paths -- but without it IDO folds the local away and the function loses six words. Seek a natural spelling retaining 294 words, the `0x88` frame, and all 71 relocation records. |

| `func_8002B7AC` | memory delayed-free tick | empty `D_800D21B0 < 0xC000` guard inside the low-memory branch | JFG's `mmFreeTick` re-reads FreeRAM there and calls a module Mickey never links, so the block is empty; the second reference is what makes IDO materialize the global's address into a callee-saved register instead of folding the `%lo` into the load, and without it the function is one instruction short. Seek a natural spelling retaining 63 words, frame `0x30`, and all 12 relocation records. |

| `overlay99BuildHeightGrid` | overlay 99 height grid | empty `unused != 0` test on the second parameter | IDO homes an unreferenced named parameter to its incoming argument slot, which is one instruction the target does not have; the empty test removes the store and its position after the grid null check is load-bearing. Seek a natural use or signature with the same 114 words, frame `0x28`, and 29 relocation records. |

| `overlay17DrawStrip` | overlay 17 strip renderer | unsigned OR-zero reassignment of the initialized strip-packet cursor | Preserves all 32 pointer bits and every store and increment. It lowers the cursor save from 30 to 25 with no emitted instruction. Seek a natural spelling retaining 119 words, frame `0x38`, one relocation, and linked ROM identity. |
| `func_overlay_058_F000138C_18B0574` | overlay 58 results renderer | `i &= 0` after case 12's first call; `(0xFF - (i = 0))` resets in the last colour argument of cases 1, 12 and 13; empty `!= 0` tests on `columnStep` and `columnCount`; unused `cursor` and `portraitX` declarations; a dead index local at two slot lookups | Each is zero instructions and load-bearing for one allocator or pass-order fact named at its point of use and in `docs/WHALE.md`. Seek natural spellings retaining 3614 words, frame `0x138` and 1253 relocation identities; try the probes first and the self-read with the guard reset last. |

Add rows as ugly-but-verified matches land. When revisiting: reproduce the match
with idiomatic C, `gmake verify`, then delete the row + the in-source comment.
