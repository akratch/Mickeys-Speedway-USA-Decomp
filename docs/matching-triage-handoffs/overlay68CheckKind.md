<!-- plateau-handoff:overlay68CheckKind:start -->
### `overlay68CheckKind` plateau handoff

- source: `src/overlays/o068/overlay68CheckKind.c`
- score: 0/80 words
- frame: 0x48
- relocations: 9
- first mismatch: none
- summary: ROM-exact and promoted. Deleting the walking cursor and subscripting the value array from a while-index lets IDO generate the scaled-zero init; an inert xor at the map-loop test keeps the copy-propagated sentinel first.
<!-- plateau-handoff:overlay68CheckKind:end -->
