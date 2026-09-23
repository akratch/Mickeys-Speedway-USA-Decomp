<!-- plateau-handoff:overlay20UpdateGrid:start -->
### `overlay20UpdateGrid` plateau handoff

- source: `src/overlays/o020/overlay20UpdateGrid.c`
- score: 185 differing words
- frame: 0x188
- relocations: 6
- first mismatch: +0x0
- summary: Remeasured 2026-09-23: 185 masked at size delta +4 (216 of 215 words), frame 0x188 against 0x140; frame, registers and loop topology remain.

First structured handoff, and the first measurement of this candidate the
preflight could take: until 2026-09-23 it refused any candidate longer than
its TU owner. The plateau commit of 2026-08-25 recorded prose only, in the
source: best -O2 candidate of four variants 16 bytes short, 173 of 215 words
differing from +0x0; explicit bounds/top-load and register-local spellings
tie, scoped locals worsen it. The candidate has since grown to 4 bytes long;
one of 6 relocation identities is unresolved, so the verdict is partial.
<!-- plateau-handoff:overlay20UpdateGrid:end -->
