<!-- plateau-handoff:overlay63UpdateEffects:start -->
### `overlay63UpdateEffects` plateau handoff

- source: `src/overlays/o063/overlay63UpdateEffects.c`
- score: 137/350 words
- frame: 0xF8
- relocations: 71
- first mismatch: +0x14
- summary: Fade carrier local removed (reads gO63Fade at each use): 139 to 137. Particle loop forms indexed/ascending are 333+.

Authorized maintenance reproof on base
`4b9fafd36510efce3321060bf07686a2c40dfac3`:

- fallback and adjacent-owner evidence still uniquely bind overlay 63 `.text`
  `+0x1D4..+0x74C`, ROM `0x18C2D5C..0x18C32D4`: `0x578` / 1,400
  executable bytes, no target padding, ABI `void (s32)`;
- configured IDO 5.3 `-O2 -mips2 -32` reproduces the exact 350-word
  geometry, 211 positional matches, 139 relocation-masked / 142 raw
  differences, first masked mismatch `+0x16C`, and equal target/candidate
  frames of `0xF8`;
- workbench again reports 40 aligned structural, 12 schedule, 104 register,
  and three constant residuals with 14 insertions and 14 deletions. No body,
  flag, historical source form, or permutation was attempted;
- the target runtime surface contains 71 records; the candidate static surface
  contains 72. Around the unresolved argument, the shipped target has a local
  HI16/LO16 pair at `+0x3A4/+0x3B4` and a reserved-selector symbol pair at
  `+0x3A8/+0x3B8`. The candidate names `gO63RenderMatrixReloc` at
  `+0x3A8/+0x3BC`;
- no tracked alias, unique candidate definition, overlay owner, or independent
  stable-base witness binds `gO63RenderMatrixReloc` to that shipped symbol
  identity. Inferring it solely from the target site or target identity would
  be circular and could cross overlays sharing synthetic VMA;
- `function_preflight.py` therefore exits fail-closed on the ambiguous runtime
  identity. The scalar workbench comparison is valid for geometry and source
  shape only; it is not exact relocation evidence.

Next lever: add a Mickey-authenticated alias/owner witness or a generic
fail-closed resolver backed by independent candidate evidence. Until then,
leave the C body unchanged and do not reopen this plateau.
#### 2026-10-01, lane d-ovl1: 139 to 137

The `fade` carrier local in the fade-direction arms removed, each use reading and writing gO63Fade directly: 139 to 137 at delta 0 (first mismatch moves to +0x14). Particle loop as an indexed descending, postfix and ascending loop over gO63Particles: 333, 334, worse. Frame-order of the locals and the gfx packet store order: flat. The saved-register rotation (target s0 holds the fade address that later becomes the particle cursor, s5 the draw context; ours s1 and s4) is the remaining mass and no spelling tried moves it.
<!-- plateau-handoff:overlay63UpdateEffects:end -->
