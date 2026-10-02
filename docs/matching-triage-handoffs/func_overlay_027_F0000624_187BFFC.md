<!-- plateau-handoff:func_overlay_027_F0000624_187BFFC:start -->
### `func_overlay_027_F0000624_187BFFC` plateau handoff

- source: `src/overlays/o027/overlay_027.c`
- score: 12 differing words
- frame: 0x98
- relocations: 15
- first mismatch: +0x1FC
- summary: 12 words at delta 0: two colour decisions, the two vertex-address webs (a0/v1 here, a3/a1 in the target); forcing both scores 0.

Summary before this remeasure: Delta +4, 25 aligned rows. Open: one extra constant move at +0x284 for the mode call's zero arguments.

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
#### 2026-10-02, lane x-o101: real callee arities, 100 to 12 at delta 0

The relocation table names every call. The closing call (+0x9A0) is a
SYMBOL record for resident camPopModelMtx, which takes one argument
(camera.c, matched); the inherited body passed the second vertex pointer to
it, and that was the +4 and the "extra constant move". The a1 the shipped
call carries is a leftover. The texture-part call (+0x77C) and the mode
call (+0x8B8) are one resident routine, func_800349A4 (dlist, texture,
flags, frame), so both are now spelled as that one callee. With the real
arity: 100 to 12 at size delta 0.

tools/align_symbol.py: 242 byte-exact, 10 naming, 0 immediate, 2 really
different (was 230 exact of 255 at +4).

Decision variable, priced with the instrumented uopt (identity gate passed,
proc 2 of the TU, 21 p1 decisions): the 12 words are exactly two colour
decisions. Web 119 is the first vertex address (D_80000000, block 15) and
takes a0, the lowest free colour at cost 0; web 141 is the second
(D_80000118, block 18) and takes v1. Forcing p1:w119=c6 (a3) and
p1:w141=c4 (a1), both accepted, scores 0; w119 alone 4, w141 alone 8.
Every caller-saved colour costs 0 for both webs, so the target's choice
needs a0-a2 (and v1, a0) taken by interfering webs in those blocks that this
body does not have. Not reached by: the mode call's arguments as a
zeroed variable (16 cells, all 12), the frame parameter as s16, an untyped
callee prototype, the vertex pointers through a local or written in place,
one model-data base symbol for the four arrays (170 at +24: one held base
web), or the TU's data as one struct.

<!-- plateau-handoff:func_overlay_027_F0000624_187BFFC:end -->
