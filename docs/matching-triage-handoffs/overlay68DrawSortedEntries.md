<!-- plateau-handoff:overlay68DrawSortedEntries:start -->
### `overlay68DrawSortedEntries` plateau handoff

- source: `src/overlays/o068/overlay68DrawSortedEntries.c`
- score: 0/213 words, promoted
- frame: 0x108
- relocations: 3
- first mismatch: none
- summary: Matched. Listing rewrite in the overlay 69 sorted-renderer shape: packet macro, counted collect loop, i reused by the sort, temp swap, order slot local.
#### 2026-10-02, lane g-ovl5: no change, 144

- Declaration-order hill climb over the 13 declarations: floor 144.
#### 2026-10-02, lane x-sort: 144 to 0, promoted

The inherited body was discarded and rewritten from the listing in the shape
the overlay 69 renderer had just reached in the same lane. Direct-compile
measurements (masked words, size delta 0 throughout):

- Natural rewrite: a counted collect loop ending in i != 4 with no limit
  local, the swap through one temporary, the submit loop indexing
  entries[order[i]], five scalars declared above order[] and three between
  the descriptor and entries[] (every array home lands on the target's):
  148.
- The bubble sort reuses i as its outer index instead of a separate pass
  local, and the environment colour is a packet macro: 33.
- One-line packet macro (as1 then stores w1 before w0, as the target does),
  entry++ before count++: 25.
- The submit loop reads order[i] into its own local before indexing
  entries[], and stores the weight before the 1.0 constant: 0.
<!-- plateau-handoff:overlay68DrawSortedEntries:end -->
