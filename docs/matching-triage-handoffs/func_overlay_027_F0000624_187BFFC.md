<!-- plateau-handoff:func_overlay_027_F0000624_187BFFC:start -->
### `func_overlay_027_F0000624_187BFFC` plateau handoff

- source: `src/overlays/o027/overlay_027.c`
- score: 12 differing words
- frame: 0x98
- relocations: 15
- first mismatch: +0x1FC
- summary: 12 words at delta 0: two colour decisions, the two vertex-address webs (a0/v1 here, a3/a1 in the target); forcing both scores 0.

Summary before this remeasure: Fresh V0 on b4d1624a reproduces 1016B/254w and the 15-site ambiguity; no new identity or donor evidence; body untouched.

Fresh maintenance evidence on base
`b4d1624a2b85efcd237d708762e80bf2df56e81e`:

- the assembly fallback and adjacent function boundary bind overlay 27 `.text`
  `+0x624..+0xA1C`, ROM `0x187BFFC..0x187C3F4`: `0x3F8` / 1,016
  executable bytes with no target padding before `overlay27UpdateCoordinates`;
- the ABI remains `void (O27Command **, void *, s16 *, O27Object *)`.
  ROM-table export 1189 owns `overlay:27:+0x624`; resident relocation 31 at
  ROM `0xA654` is its sole authenticated inbound call, and no overlay-local
  SYMBOL relocation targets that export;
- configured IDO 5.3 `-O2 -mips2 -32` again emits exactly 1,016 bytes / 254
  words. Target and candidate frames are both `0x98`; 59 positional words
  match, leaving 195 relocation-masked / 196 raw differences, first `+0x8`;
- workbench again classifies 46 aligned structural, three schedule, 145
  register, and three constant residuals, with 18 insertions and 18 deletions.
  The dominant early divergence remains the object/child pool-home swap;
- target runtime and candidate static surfaces each contain 15 relocations:
  seven `R_MIPS_26`, four `R_MIPS_HI16`, and four `R_MIPS_LO16`. Several
  schedules remain shifted, and candidate proxy `D_80000050` still has no
  unique runtime identity, so `function_preflight.py` correctly fails closed;
- the donor scan is unchanged and exact-negative. Its weak top results remain
  PD `filemgr_render_perfect_head_thumbnail` (`0.0675`), JFG `fxDrawCone`
  (`0.0667`), and DKR `func_80080E90` (`0.0629`); none supports adoption;
- the existing flag lattice, allocation/lifetime/order forms, and bounded
  permuter remain exhausted. This authorized pass did not mutate the body or
  run flags, source variants, or permutation.

Next lever: obtain a non-circular overlay-local identity for `D_80000050` and
the shifted relocation sites. Only a newly authenticated source or allocator
mechanism may then reopen the body; do not repeat the old register/lifetime
family or weaken relocation identity checks.
#### 2026-10-02, lane x-ovla: mode-call arguments, flat at 100 (+4)

The target reads as one variable split across two registers: the first
batch's vertex pointer is in a3, a3 is cleared right after the synchronise
command, a1 is cleared once before the pulse branch (it serves both the mode
call's second argument and the finish call's argument on the not-taken
path), and a2 = 5 sits in the branch delay slot. Passing the vertex variable
to the mode call (`(commands, 0, 5, verts)`, `(commands, verts, 5, verts)`,
`(commands, verts, 5, 0)`) is byte-identical to the literal zeros: uopt
propagates `verts = 0` into every argument, so a source form that reaches the
target has to keep that zero from being a known constant at the call.
<!-- plateau-handoff:func_overlay_027_F0000624_187BFFC:end -->
