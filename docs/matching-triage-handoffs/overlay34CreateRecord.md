<!-- plateau-handoff:overlay34CreateRecord:start -->
### `overlay34CreateRecord` plateau handoff

- source: `src/overlays/o034/overlay34CreateRecord.c`
- score: matched (0/125 words, promoted 2026-09-17 by lane w3-o034)
- frame: 0x30
- relocations: 12
- first mismatch: none
- summary: Post-call rebind `record = candidate` makes the fill-pointer assigned tree ILOD-free, so forwarding skips the base Ulods. short16 and byte12 share one block in ROM order; height is one block and outranks the resource copy without self-defs.

#### 2026-09-17, lane w3-o034: matched on a no-ILOD pointer rebind

Baseline reproduced: 2 masked (4 raw), delta 0, aligned 123/0/0/2, first
+0xE8, frame 0x30. Instrumented `.text` is byte-identical to stock (proc 0).
Height (sym@-16) refs [10,11] colour v1; resource copy refs [10] colour a0;
literal 2 refs [10].

Named lever only. A comma of the contested pair, a global pointer-table
ILOD base, and deleting `candidate` for `gOverlay34Records[index]` stores
were 12 / 301(+788) / 214(+376). Rebinding `record = candidate` after the
resource test, filling through `record`, and emitting short16 before byte12
is 0 masked, 2 raw reloc-spelling, delta 0, aligned 125/0/0/0. Height refs
[10] only, colour v1; copy stays a0. The two height self-defs are gone.

#### 2026-09-17, lane lm-o034: block table read; seven cells, unmoved at 2

Baseline reproduced: 2 masked (4 raw), delta 0, aligned 123/0/0/2, first
+0xE8, frame 0x30. Instrumented `.text` is byte-identical to stock. The
instrumented per-web sets and stock `-Wo,-zdbug:2` `uoptlist` (listing
complete before the recomp `wrapper_ecvt` abort) place the boundary:

- Node 10 `expoccur` holds itable bits 42–70: both dimension defs and the
  field stores through `byte12 = 2`. Node 11 starts at the surviving
  height self-def (bit 71) then `short16` (bit 72).
- Height (sym@-16) refs blocks 10,11, colour v1; the resource copy refs
  10, colour a0; literal 2 refs 10 only.

uopt source names the counter: `endblock = varrefs >= curvarreflimit`
(default 20) at a statement boundary. `varrefs++` is each cfe `Ulod` of a
non-`veqv` isvar; forwarding skips it. The statement that hits 20 stays
in the block; the next opens the next one. ROM order still needs at most
17 local-loads before `short16`.

Seven cells, none below 2, none putting `short16` and `byte12` in one
block without a size or shape regression:

- Two-step `width = p->w; width = (width-1)<<5` then ROM order: 11 (2
  self-defs) / 15 (none). Literal 2's refs become [10,11]. Copy-prop does
  not cheapen later `Uistr` of that s32.
- `width = p->f = expr` 47; `p->f = width = expr` 27 (the folded-def
  family nx-b had at 27–28). Literal 2 still nocs 2.
- `*(s32*)&short0C = 0` 80, delta -4: one wider store saves a unit and
  changes the opcode.
- Repeating the ILOD expression at every dimension store 108, +44:
  candidate stores kill the resource ILOD, so each use rematerializes.

Eliminated as the way to save two units while keeping two declared s32
dimensions and the same sb/sh sequence: split-ILOD forwarding, chained
def/store, packed adjacent zeros, repeated expressions. Next is still
cfe emitting fewer `Ulod` of the record base (a non-isvar temp, or one
lod feeding two `Uistr` that still become the same two store opcodes).
Do not repeat the statement-order lattice or these seven cells.

#### 2026-09-16, lane lm-a: two cycles, no source reaches the ranking

Baseline reproduced: 6 masked at delta zero, aligned 119/4/0/2, one
`a0`/`v1` cycle at `+0x88..+0xA4` and the byte12/short16 store pair. The
records say the four naming rows are the ranking already named -- the
resource copy (web 44: save 3, `nocs` 1, block 10) is decided before
height (web 50: save 2, `nocs` 2, blocks 10 and 11) -- and height's second
block is the open question: width (web 42) has all its references in block
10, and the constant `1` web reaches block 13 through `active = 1`, so the
store sequence is not one uopt block, but nothing in the source names the
boundary. Seventeen cells, all at or below the retained form: routing the
null test, the width load or the height load through the loop's `record`
or `current` aliases (119-124 words, +4 to +24 bytes: the aliases are
separate webs and referencing them after the loop extends their ranges);
splitting or multiplying the height intermediate (16, 12); `s16`/`u16`
dimensions (50, 102); the mode multiply moved to the end (39); height
declared or computed before width (6 and 92). Next: find the block
boundary inside the store sequence from the instrumented uopt's block
table before trying any further order, because every order cell so far
has been measured blind to it.
<!-- plateau-handoff:overlay34CreateRecord:end -->
