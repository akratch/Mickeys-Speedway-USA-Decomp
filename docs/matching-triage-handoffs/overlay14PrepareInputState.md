<!-- plateau-handoff:overlay14PrepareInputState:start -->
### `overlay14PrepareInputState` plateau handoff

- source: `src/overlays/o014/overlay_014.c`
- score: 0/131 words
- frame: 0x20
- relocations: 29
- first mismatch: none
- summary: Resolved Tier A: 524 ROM-exact C bytes and 29 exact relocation identities after replacing four timer carriers with direct updates and retests.

#### 2026-09-13, lane `j1`: timer-carrier source closure

Fresh configured baseline reproduces 131 words, masked 65, frame 0x20,
first masked +0x34, and 66 exact plus 65 naming aligned rows. The historical
+0xC first difference was an unlinked call relocation. Procedure 0 records
15 draws and 347 emissions. Source and existing shard were read; no colour
sweep, flag lattice or permutation was repeated.

Each timer branch now updates its timer directly and tests that same memory
value. There is no intervening call, volatile access or mutation. Access
widths, branch predicates, store order, pulse values and joystick calls are
preserved, including the unusual horizontal predicates. Arithmetic formerly
computed before a horizontal guard is now performed only on its taken path;
this removes an unused calculation without changing defined observable behavior.

Removing only either vertical carrier adds two draws on its update/test lines
and improves 65 to 64 naming words, solely in the last address window. Removing
both changes the ring membership and reaches fifteen naming words, closing
the first three windows. Removing the horizontal positive carrier reaches
seven; removing the negative carrier reaches zero. Draws progress from 15 to
17, 19, 21 and 23 while emissions remain 347. Every adopted step preserves
131-word extent and frame 0x20. The complete draw order changes, and aligned
comparisons distinguish the local line changes from their wider allocation
effects. Formatting cleanup retains zero masked differences. Private source,
stock/traced objects, census pairs and aligned residuals preserve the controls.
Full-TU traced and stock instruction output agree throughout.

The unconditional C owns overlay 14 +0xB5C through +0xD68, ROM 0x1870434
through 0x1870640: 524 executable bytes, no padding credit. The existing nine
BSS definitions retain their offsets and runtime identities. The fallback
preflight resolved only three calls and left 26 data records unresolved;
post-promotion linked evidence resolves all 29 with static identity proof.
The three resident joystick calls are unchanged. No instruction or addend
normalization was added.

`promotion_proof.py` reports exact ROM comparison, 131 words, frame 0x20,
29 configured and 29 exact relocations. `tools/gates.sh verify` reproduces
SHA1 507341c0a40ca3e9a7cee969b396ee53facfb548 with this C active. The atlas,
alias list and scoreboard are regenerated, and the retired ranking row is
pruned. Commands include the assignment gate, configured stock/traced compile,
draw_census.py comparisons, residual_map.py --object --against,
function_preflight.py, overlay-atlas-write, refresh_atlas_digest.py, extract,
overlay-syms, real-core-count build, promotion_proof.py, scoreboard and
promotion gates. The lane does not merge or push.
<!-- plateau-handoff:overlay14PrepareInputState:end -->
