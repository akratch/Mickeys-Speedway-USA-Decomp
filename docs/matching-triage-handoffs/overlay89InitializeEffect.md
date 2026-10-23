<!-- plateau-handoff:overlay89InitializeEffect:start -->
### `overlay89InitializeEffect` plateau handoff

- source: `src/overlays/o089/overlay_089.c`
- score: 205/205 words
- frame: 0x58
- relocations: 5
- first mismatch: none
- summary: ROM-exact: 205 words and five static relocation identities; member-scale access and a reviewed inert maintain region close the source.

#### 2026-09-12, lane `p8-close`: the colour half is bounded at 17 and the blocker is renamed

Baseline reproduces: 820 bytes, 205 of 205 instructions, delta 0, frame 0x58,
21 relocation-masked words, first mismatch +0x40. Aligner: 185 byte-exact, 18
register naming, 1 immediate-only, 2 really-different. `register_census` reads
two integer substitution sites and a float mapping over three windows at 91 per
cent with one closed five-cycle, which is the ring phase the record describes.

**Colour is bounded harder than the record says, and still cannot close it.**
The earlier entry priced colour from a one-web sweep. The JOINT grid -- both
class-2 webs against all six offered colours, 36 cells, plus both split paths
-- floors at **17**, at web 42 on colour 24 with web 16 on colour 28. So the
colour axis is worth at most 4 of the 21, not 2. It cannot close the function
for a reason the grid makes plain: both class-2 webs take the same colour, and
the ugen float ring only shifts when NO web holds that register.

**The ring fact, read from the object rather than the record.** In the size
block the target draws five scratch float registers and we draw four, because
our conversion result lands in the coloured web instead of the ring; every
later float register in the procedure is one ring position behind as a result.
Deleting the `size` local is the whole 17 words, which is what the record
already said, now with the count on both sides.

**The blocker is an evaluation order, not a colour and not a hoist.** The
inline form already has the right ring -- five scratch draws, and the
multiply's destination register is the target's -- and loses on exactly one
thing: it evaluates the scale load before the conversion, so the load takes the
first ring slot and everything else moves up one. The two source operand orders
compile BYTE-IDENTICALLY, so IDO canonicalises the commutative multiply and the
source cannot choose the order by writing it.

**Flat at 26, on top of the nineteen forms already recorded:** `const` and
sized array declarations of the scale symbol, a pointer declaration of it
(which is worse), `*(g + 1)`, `(g + 1)[0]`, an extra parenthesisation, an
or-with-zero and an add-with-zero folded into the conversion, and `if (1) { }`, `do { } while (0)` and
a bare brace before or around the statement.

**Also refuted this pass, none at 21 or better:** the assignment-in-expression
and comma forms, 55 each; `object->size` as its own intermediate, 187; a
ternary, 200; a second use of `size`, 197; a signed-conversion cast, 194 at
delta -20. Reusing the existing `range` local as the intermediate, and an
initialiser-style declaration, are byte-identical to the incumbent, so the
identity of the local does not matter -- only whether one exists.

**Decision variable for the next lane:** a source form in which the multiply's
conversion operand is evaluated before its memory operand while both remain in
one statement. Nothing measured moves IDO's canonicalisation, so this is a
weight question inside ugen and not a spelling question.

**Force hygiene for the 36-cell grid.** Every cell was run with `CDX_PROC=0`
and checked on the record's `forced` field; the best cell records both webs as
`decision=color` at the requested colours with no `force_declined` line, and
the same force without `CDX_PROC` returns the unforced 21-word object with every
record at `forced=-2`. Both split paths are accepted rather than declined and
are far worse -- 165 and 187 words at delta +8 -- so the floor of 17 is a
measured bound on the colour axis and not an artefact of dropped forces.
#### 2026-09-12, lane p23-lastmile3: exhaustive colour floor

Fresh residual map: 185 byte-exact, 18 naming, 1 immediate, 0 structural,
with one candidate-only and one target-only offset. The every-colour footprint
sampled 88 probes over 11 coloured webs. Its sole winner was web 16 colour 28,
at 17 masked words; the measured force lattice floor is therefore 17. The
source question is the evaluation order that makes the conversion operand draw
before the scale memory operand in one statement.

#### 2026-09-12, lane p24-second-mid: source closure after a diagnostic exact force

The base-only assignment gate passes. The configured starting point is
820 bytes, zero size delta, 21 raw and masked differences: 185 aligned exact,
18 naming, one immediate, no paired structural rows, and one candidate-only
plus one target-only word. Its previous colour landscape was not repeated.

L92 reaches the source question directly. Inlining the unsigned size conversion
and reading the scale at offset four through a two-float struct member gives
four raw and masked differences at unchanged size. An explicit float cast on
the array operand instead gives 26; a redundant byte mask gives 132. The
member form removes seventeen aligned naming rows without regression in any
window. Its fresh stock/instrumented full-TU identity check passes; all ten
coloured webs are covered by 83 single forces, with no improving winner.

The remaining four words are the maintain call's retained state argument.
Passing state directly makes its web choose a saved register and removes
sixteen bytes, scoring 195. Address-taking costs 96 bytes and scores 220;
reusing the source pointer as a carrier again removes sixteen bytes and
scores 195. A fresh 166-force cross-kind landscape of the direct-state form
finds an accepted diagnostic exact result: p1:w8=c5 gives zero raw and masked
words at zero size delta. The complete function is solved by that allocation;
the force itself is not promotable.

The source decision is now priced. Without a region, the state web's caller
cost is 6.2 against the first saved colour's 6.05. A nested spelling of the
existing two conditions is identical. An inert if-one region around the
maintain conditional increases the component count from six to seven and
makes the caller register win naturally. Untouched configured IDO then emits
205 of 205 exact words. Removing the unused size local remains exact, so
no dummy frame declaration is retained. Independent read-only semantic review
approves the member access, u8 conversion, target-supported retained pointer,
and inert region after that removal. The region requires cleanup-queue coverage.

Private evidence under build/p24/overlay89InitializeEffect/ retains every
source, configured object, score and aligned comparison. The two remeasured
landscapes live under build/p24/o89-member-landscape-v2/ and
build/p24/o89-call-state-landscape/. Stock/instrumented identity is verified
for both. Five compiler relocation sites versus three literal-bearing target
assembly sites is not a complete runtime relocation proof; linked promotion
must authenticate the actual overlay records. Commands include lane_status,
residual_map with --against, web_footprint --every-colour, configured stock
and instrumented compiles, finalize_plateau, and the ordered overlay gates.

Promotion completed in the prescribed order, including the second
alias generation and rebuild. Promotion proof passes at 205 words, frame
0x58, and five of five static relocation identities. Full-ROM verification
reproduces the required US hash. The function owns exactly 820 executable
bytes; no boundary padding is credited. Scoreboard regeneration adds exactly
820 overlay C bytes. The obsolete ranking row is pruned because check-docs
correctly rejects retired rows. The inert region is disclosed in source and
recorded in docs/cleanup-queue.md; no unused scalar padding remains.

<!-- plateau-handoff:overlay89InitializeEffect:end -->
