<!-- plateau-handoff:overlay14LoadRelocatedValue:start -->
### `overlay14LoadRelocatedValue` plateau handoff

- source: `src/overlays/o014/overlay_014.c`
- score: 0/94 words
- frame: 0x28
- relocations: 6
- first mismatch: none
- summary: ROM-exact 376-byte C; named relocation intermediate, cursor-side reads and spilled-locals-first declaration order close the source.

#### 2026-09-13, lane `j1`: source closure and canonical proof

The initial configured source had 94 words, masked 52, frame 0x30 versus
0x28, aligned 42 exact, 48 naming and four immediate rows. Its procedure-0
census held 21 draws and 162 emissions. The existing source/shard was read;
no colour sweep or force was used.

The adopted reconstruction names the masked-and-tagged relocation value,
then adds the buffer base into that same value before storing it. This
removes eight temporary draws over the four command arms. Reading the
original word through cursor before advancing cursor supplies its proper
allocation weight while command remains the store address. The resulting
13-draw, 170-emission candidate has no naming or structural differences;
only eight stack displacements remain. Moving start and size ahead of the
coloured pointer/value declarations removes all eight without changing any
per-line counts or the complete draw sequence. The two spilled locals now
bound the local block and produce the target frame. Readable statement
formatting preserves the zero-masked result.

Retained rejected controls include command-carrier deletion, input-index
reuse for the starting offset or computed value, and source-parameter reuse.
They changed width, frame or live ranges and were not adopted. Private
source/object/census pairs and aligned deltas preserve each material attempt.
The intermediate named-value candidates scored 32 and 33 before the
cursor-side read and frame corrections reached eight and then zero.

Canonical ownership is overlay 14 text +0x87C through +0x9F4, ROM
0x1870154 through 0x18702CC: 94 executable words, 376 bytes, no new padding
credit. Both resident calls are named from Mickey's runtime table and existing
symbol map. The unmodified compiler's seven jump-label offsets, rebased to
the owned function, agree with the retained initialized table at +0x158.
Its runtime LOCAL base is +0x1D60 with addend +0x38. A dedicated relocation
anchor binds the two text records to that table; a digest-checked metadata
step removes only the duplicate private table and its relocation section.
No instruction word or compiler instruction addend is edited after compilation.
The atlas explicitly records externalized table ownership. No data bytes are
credited as new executable C.

`promotion_proof.py` reports post-promotion ROM comparison exact: 94 words,
frame 0x28, six configured and six exact relocations, static identity proof.
`tools/gates.sh verify` prints expected and built SHA1
507341c0a40ca3e9a7cee969b396ee53facfb548. Source and relocation proofs therefore
cover the active C, not an assembly fallback. The lane does not merge or push.

Commands: assignment gate; configured stock/traced compilation and full-TU
text identity; draw_census.py profiles/comparisons; residual_map.py aligned
comparisons; allocator record capture without forces; overlay-atlas-write;
refresh_atlas_digest.py; extract; overlay-syms; real-core-count build;
promotion_proof.py; scoreboard; and tools/gates.sh promotion checks.
<!-- plateau-handoff:overlay14LoadRelocatedValue:end -->
