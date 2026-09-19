#include "PR/ultratypes.h"

typedef struct O57MiddleRenderItem {
    s32 type;
    u8 value04;
    u8 value05;
    u8 value06;
    u8 value07;
} O57MiddleRenderItem;

typedef struct O57MiddleInfo {
    u8 pad00[8];
    u8 value08;
    u8 value09;
    u8 value0A;
} O57MiddleInfo;

typedef struct O57MiddleChoice {
    u8 pad00[0x28];
    s16 tableIndex;
    s8 active;
    u8 pad2B[9];
} O57MiddleChoice;

typedef struct O57MiddleOutput {
    u8 value0;
    u8 pad01[3];
    u8 controller;
    u8 pad05[0x23];
} O57MiddleOutput;

typedef union O57MiddleInput {
    s32 word;
    u32 uword;
    s16 half;
    u8 bytes[4];
} O57MiddleInput;

typedef struct O57MiddleTextureNode {
    void *texture;
    void *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} O57MiddleTextureNode;

/* Tier B: calls and globals separated by overlay 57 runtime relocations.
 * Reserved data/BSS selectors remain separate from overlay-local storage. */
extern O57MiddleInput gO57MiddleFlags;
extern s16 gO57MiddleHorizontal, gO57MiddleVertical;
extern s32 gO57MiddleButtons, gO57MiddlePlayerCount;
extern s16 gO57MiddleCourseIds[], gO57MiddleCourseNames[];
extern char **gO57MiddleText;
extern void *gO57MiddleDisplayList, *gO57MiddleVertexList;
extern void *gO57MiddleGraphics[];
extern O57MiddleChoice gO57MiddleChoices[];
extern s32 gO57MiddleData31E4, gO57MiddleData31E8, gO57MiddleData31EC;
extern s32 gO57MiddleData31F0, gO57MiddleData31F4;
extern u8 gO57MiddleData3198, gO57MiddleData31AC;
extern s8 gO57MiddleData319C, gO57MiddleData31A4, gO57MiddleData31A8;
extern u16 gO57MiddleData31B4, gO57MiddleData31B8;
extern u8 gO57MiddleData3208, gO57MiddleData3214;
extern void *gO57MiddleItems[];
extern s32 gO57MiddleAlpha, gO57MiddlePreviousAlpha;
extern s32 gO57MiddleMode, gO57MiddleNextMode, gO57MiddleMoving;
extern s32 gO57MiddleFade, gO57MiddleFadeDelay;
extern s32 gO57MiddleSelection, gO57MiddlePreviousSelection;
extern s32 gO57MiddleState188, gO57MiddleState194;
extern s32 gO57MiddleUnlocked198, gO57MiddleUnlocked19C;
extern O57MiddleInfo gO57MiddleInfo;
extern s32 gO57MiddleStopList[], gO57MiddlePathIndices[], gO57MiddlePathList[];
extern O57MiddleOutput *gO57MiddleOutput;
extern s32 gO57MiddlePanelPosition, gO57MiddleTransition, gO57MiddleCanLeave;
extern s16 gO57MiddleColumns[], gO57MiddleColumnsEnd[];
extern char gO57MiddleEmptyFormat[], gO57MiddleResultFormat[];
extern O57MiddleTextureNode gO57MiddleCaption[], gO57MiddleBadge[];
extern char *gO57MiddleLabels[];
extern s16 gO57MiddleCharacterIds[];
extern u8 gO57MiddlePathId;

typedef struct O57MiddleRenderParameters {
    u8 pad00[0x10C];
    f32 position;
    f32 scale;
} O57MiddleRenderParameters;
extern O57MiddleRenderParameters gO57MiddleRenderParameters;

extern void func_80000F94(u16 soundId, void **handle);
extern void func_80005548(u8 count);
extern void func_80022A50(void **displayList, void **matrices);
extern void func_80025444(s8 *players);
extern s32 func_80025D60(s32 course);
extern void func_80028374(s32 level, s32 character, s32 animation, s32 mode, s32 arg4, s32 arg5);
extern void func_80028528(s32 group);
extern void func_80028540(s32 cameras);
extern void func_80028D24(s32 mode);
extern void func_800291B4(void);
extern O57MiddleRenderItem * func_800291C4(void);
extern void func_8002F618(void **displayList, O57MiddleTextureNode *nodes, s32 x, s32 y, u8 r, u8 g, u8 b, u8 a);
extern void func_8002FB34(void **displayList, O57MiddleTextureNode *nodes, f32 x, f32 y, f32 sx, f32 sy, s32 mode, s32 flags);
extern void func_80039E34(s32 spacing);
extern void func_8003A680(u8 character);
extern s32 func_8003A700(u8 character);
extern s32 func_800429A4(char *buffer, const char *format, ...);
extern void func_8004B0A4(s32 font);
extern void func_8004B0B8(s32 r, s32 g, s32 b, s32 a, s32 opacity);
extern void func_8004B0F8(void **displayList, s32 x, s32 y, char *text, s32 align);
extern void func_80050688(u8 path);
extern void func_80050704(u8 path);
extern void func_overlay_045_F0000314_188C76C(void *descriptor, s32 x, s32 y, s32 flags);
extern void func_overlay_056_F00000B8_18A2E30(s32 time, s32 *first, s32 *second, s32 *third);
extern void func_overlay_057_F0001020_18A4C18(s32 updateRate);
extern void func_overlay_057_F00067DC_18AA3D4(s32 id, s32 argument, f32 value);
extern s32 func_overlay_068_F000146C_18C85CC(s32 course);
extern s32 func_overlay_084_F0000C74_18D1154(void);
extern void func_overlay_084_F0001060_18D1540(s32 state);
extern void func_overlay_084_F0001350_18D1830(void);
extern void func_overlay_084_F0001398_18D1878(void);

/* Typed reconstruction candidate; control-flow reconstruction remains in
 * progress. The frame is exact at 0x140: renderState is a 24-byte buffer, not
 * 32 -- 24 is the only size in 21..24 that closes the frame, and every other
 * homed object's size was already pinned by its fake name (stack5C, stack64,
 * stack78/7C/80, stackB0 are the target's own offsets). Declaration order
 * still places the homed block 16..40 bytes off those offsets.
 *
 * 494 -> 272 masked words by three source edits, two of which are a pair that
 * only works together:
 *
 *  1+2. The two path-list walks. The first was spelled with a named `index`
 *       carrying the loop value and a named `currentGroup` carrying the
 *       re-read; the target names NEITHER. Writing both walks as
 *       `while (*list != -1) { call((u8)*list); call(*list, map[*list], k); }`
 *       reproduces the target's loop instruction-for-instruction: one web for
 *       the list value, re-read after the call because the call may write it,
 *       and the `& 0xFF` mask folded into the argument as a single
 *       `andi a0,<value>,0xff` instead of `move`/`andi`/`move`. The `(u8)`
 *       cast and the `while` shape are ONE edit each and each alone moves the
 *       size (+8 and -8); together they are delta 0 and worth 36 words.
 *       Naming a re-read value is what costs the extra web -- this is the
 *       drop-a-declared-local lever the workbench names for a longer pool
 *       lane, applied to a loop body.
 *
 *  3.   `row = 0x51` was the first statement of the panel block, so as1 put
 *       `li s3,81` in the delay slot of the block's guard branch; the target
 *       leaves that slot a nop and materialises the constant 27 instructions
 *       later, inside the basic block that ends at the func_80022A50 call.
 *       Moving the assignment after `func_8004B0A4(0)` reproduces the target
 *       from `bnez at` through `swc1 $f8` exactly and is worth 186 words --
 *       far more than the one slot, because it also re-colours s3 and the
 *       whole run of calls that follow it.
 *
 *  4.   THE SURPLUS INSTRUCTION, closed 2026-09-11 (lane/p9-oneoff). The
 *       diagnosis above was right about the web and wrong about where it is
 *       anchored. It is not the two tail reads that hold
 *       `&gO57MiddleChoices` alive -- it is the choice loop's own BOUND.
 *       `do { ... } while (choice < &gO57MiddleChoices[4])` makes the array's
 *       end address a loop invariant, uopt hoists it into a callee-saved
 *       register, and from there it reaches the two
 *       `gO57MiddleCharacterIds[gO57MiddleChoices[0].tableIndex]` reads past
 *       the calls between them, which is why a region marker around the
 *       reads could never close it: the marker was on the wrong end of the
 *       web. Bounding the walk on the OTHER pointer the loop already steps,
 *       `while (source < &sourceState[4])`, names no address of the global at
 *       all. Both pointers advance in lockstep from the same do/while, so the
 *       trip count is identical and the semantics are unchanged.
 *
 *       Measured, with tools/align_symbol.py: size delta +4 -> 0 (1209 words
 *       -> 1208, exact), positional masked 272 -> 231, displacement tax
 *       59 -> 26, and the aligned split 1002/113/100 -> 1012/109/96. The first
 *       STRUCTURAL difference moves from +0x34 to +0xBC4 -- 755 words in
 *       which nothing but register naming now differs.
 *
 *       `!=` instead of `<` on either pointer overshoots to delta -4
 *       (269 and 266 words); an explicit counter in `rank` or `valueA`
 *       is +8 and +260; `for (choice = ...; choice < &gO57MiddleChoices[4];
 *       choice++, source++)` is +24. `<` on `sourceState` is the only
 *       spelling in the set that is exact-sized.
 *
 * What is left, measured: the size is now exact and the residual is
 * allocation. The first structural difference at +0xBC4 is a STACK HOME
 * (`sw a1,84(sp)` against the target's `sw a1,100(sp)`), and the whole homed
 * block is displaced: stack5C/stack64 sit at 80/84 against the target's
 * 92/100, stack78/7C/80 at 160/164/168 against 120/124/128, the
 * address-taken array block starts at 172 against 144, and activePlayers is
 * at 232 against 272. The frame total is exact at 0x140 on both sides, so
 * this is an ordering/padding question, not a size one.
 *
 * L112 was tested directly and does NOT apply here. Every unobservable
 * dimension was swept: sourceState[6/8/10], activePlayers[12/16],
 * renderState[32/40], stackB0[4/8], textureNodes[3]. Every enlargement that
 * moves anything costs exactly the same 13 words (231 -> 244) and none gains,
 * so no array count solves the displacement -- the target's gaps at
 * 132..143, 168..175 and 216..271 are not a bigger array.
 *
 * Re-tested 2026-09-11 and still rejected, now that the size is exact:
 * caching `choice->active` in a local is 527 words and delta -8; the
 * activePlayers countdown pointer walk (both the `*active-- = 1` and the
 * split `*active = 1; active--;` spellings) is 282 words and delta +4.
 *
 * Two target structures are read but NOT adoptable yet, both because they cost
 * more than they buy at this frame layout:
 *   - the choice loop caches `choice->active` in ONE load: target is
 *     `lb v1,42(v0); beqz v1; sb v1,0(a0)`, the candidate reloads because the
 *     store through `source` may alias the global. Caching it in a local
 *     reproduces the target's three instructions exactly and takes the size to
 *     delta -4, but costs 290 words: it consumes the callee-saved register the
 *     loop's `outputIndex` holds in the target. Measured identical at 562 for
 *     `choiceActive`, `rank`, `nextValue` and `activeCount` as the carrier, so
 *     the carrier is not the variable; it is the register pressure.
 *   - the activePlayers fill is a countdown pointer walk with a dead
 *     post-decrement copy (`li v1,9`, `move v0,v1`, `sb`, `addiu a0,a0,-1`,
 *     `bnez v1`, `addiu v1,v1,-1`), not the `for (i = 9; i >= 0; i--)` index
 *     form here. Adopting it alone costs 301 words and +8 bytes.
 *  5.  THE STACK-HOME FIX, 2026-09-11 (lane/o11-homes).  The gaps the note
 *      above could not account for were not gaps: they are register-class
 *      locals.  Every declared local reserves a home in declaration order,
 *      descending from the frame top and aligned to its own type, whether or
 *      not it ever reaches memory, so the target's "132..143, 168..175,
 *      216..271" are three, four and eight scalars that the target declares
 *      BETWEEN the homed objects.  tools/frame_census.py gives the object
 *      sizes and offsets on both sides, and the gaps between them divide by
 *      four exactly, which is what makes the list solvable rather than swept:
 *
 *        8 scalars, sourceState[4], activePlayers[10], 14 scalars,
 *        textureNodes[2], 1 scalar, stackB0[2], 2 scalars, renderState[24],
 *        3 scalars, stack80, stack7C, stack78, 4 scalars.
 *
 *      That is 35 four-byte cells against this candidate's 36, and the one to
 *      drop is `activeInit`, which was declared and never used -- an unused
 *      POINTER still reserves its home (L99), so it was costing a cell and
 *      nothing else.  Reordering to that list, keeping the relative order of
 *      the scalars otherwise unchanged, makes the ladder exact from +0x11C
 *      down to +0x78.
 *
 *      Measured with tools/align_symbol.py: 231 -> 217 masked words at size
 *      delta 0, byte-exact 1012 -> 1028 of 1208, really different 96 -> 68,
 *      register naming 109 -> 112, first structural difference +0xBC4 ->
 *      +0xCB8.  L112 was correctly rejected above: no array dimension solves
 *      this, because the free parameter was the scalar COUNT between the
 *      arrays, not any array's length.
 *
 *  6.  READ THE TARGET'S BOUND, 2026-09-12 (lane p11-big).  Note 4 above is
 *      wrong about what the target does, and the correction matters because
 *      the whole "next lever" was built on it.  The target's choice loop IS
 *      bounded on `&gO57MiddleChoices[4]`: it materialises that global address
 *      into `a3` before the loop and closes with `addiu v0,v0,52; sltu
 *      at,v0,a3; bnez at` at the BOTTOM.  It is not an invariant parked in a
 *      callee-saved register there -- `a3` is caller-saved, and the loop makes
 *      no call.  What the target does with its callee-saved registers instead
 *      is hold `outputIndex` in s2, which is exactly the register the cached
 *      `choice->active` form was measured to want.
 *
 *      So the `&sourceState[4]` bound adopted in note 4 is a spelling that
 *      happened to be exact-sized, not the target's shape, and the residual it
 *      leaves is structural: the candidate advances `source` at the TOP of the
 *      loop and stores through `-1(source)`, the target advances it at the
 *      bottom in the branch delay slot; and the candidate loads
 *      `choice->active` twice where the target loads it once into `v1` and
 *      uses the same register for the test and the store.
 *
 *      Re-measured at THIS frame layout, since note 4's numbers predate the
 *      home fix in note 5 and L146 voids them: bounding on
 *      `&gO57MiddleChoices[4]` is 259 at delta +4 (was "the surplus
 *      instruction"); `!=` on it is 256 at delta -4; caching `choice->active`
 *      is 516 at delta -8 alone, 552 with the global bound, and identical at
 *      552 for `choiceActive`, `rank` and `input` as the carrier, so the
 *      carrier is still not the variable and the register pressure is still
 *      real.  Spelling the two tail `gO57MiddleChoices[0]` reads as
 *      `(*gO57MiddleChoices)` or through a reset `choice` cursor is
 *      byte-identical, so L131 does not split that address web.
 *
 *      What DID move: the activePlayers fill is now the target's countdown
 *      pointer walk (`active = &activePlayers[9]; for (i = 9; i >= 0; i--)
 *      *active-- = 1;`), which note 4 rejected at 282 words and +4 bytes and
 *      which at this layout is 217 at delta 0, byte-exact 1028 -> 1029, really
 *      different 68 -> 66.  It is adopted above.  The split
 *      `*active = 1; active--;` spelling measures identically.
 *
 *  7.  STATEMENT ORDER, same lane.  A move-one hill climb over every run of
 *      three or more consecutive single-line non-call statements in the body,
 *      re-run to a fixed point (three passes), is worth 217 -> 205 at delta 0,
 *      byte-exact 1029 -> 1038, register naming 114 -> 102.  Four orders move,
 *      three of them inside one store group: `textureNodes[0].texture` ahead
 *      of `.alternate`, `.x` ahead of `.y`, `gO57MiddleData31A8 = 0;` ahead of
 *      `gO57MiddleData31B8 = ...`, and `gO57MiddleData31E8 = ...` after the
 *      three stores that follow it.  A fourth candidate the sweep offered --
 *      swapping `func_80028D24(0);` with `func_80028540(...)` -- is worth one
 *      more word and is a REORDERING OF TWO CALLS, so it is rejected; the
 *      sweep tool now refuses to move any statement containing a call.
 *
 *  8.  THE CHOICE LOOP'S OUTPUT CURSOR IS A CARRIER DECISION AFTER ALL,
 *      2026-09-12 (lane p12-o57).  203 -> 132 masked at the same 0x140 frame,
 *      byte-exact 1040 -> 1105 of 1208, naming 100 -> 53, immediate 11 -> 14,
 *      really different 68 -> 43, displacement tax 24 -> 22.
 *
 *      Note 7's packet closed "which register holds outputIndex" as inert on
 *      the strength of a 1,131-form L115 reuse lattice.  That lattice varied
 *      the countdown fill's counter, the second fill loop's index and the
 *      activePlayers scan cursor and held the CHOICE LOOP's cursor fixed at
 *      `outputIndex` throughout, so it never tested the one carrier that
 *      moves.  Carrying the choice loop's output cursor in `i` instead is
 *      worth 248 -> 230 on its own and is what puts the value in a
 *      callee-saved register: under L115 a live range is formed per symbol,
 *      `i` is referenced across calls elsewhere in this function and
 *      `outputIndex` is not, so `i`'s web is offered only the callee-saved
 *      colours while `outputIndex` is offered the argument registers first.
 *      The instrumented records name the web: forcing it onto any callee-saved
 *      colour scores 246 against 554 unforced, and s2 itself is declined with
 *      a forbidden mask, so the source form reaches what the force reaches
 *      without reproducing the exact register.
 *
 *      The rest of the gain is three composed edits on the shape that opens.
 *      (a) The loop bounded on `&gO57MiddleChoices[4]` with BOTH cursors
 *      stepped at the bottom and `choice->active` read ONCE into `rank`, which
 *      is the target's loop instruction for instruction; alone it is 554,
 *      because it takes one more caller-saved colour than the old shape and
 *      pushes a web onto t0, which removes t0 from ugen's ring free list and
 *      rotates every ring temporary in the whole function -- 315 extra naming
 *      rows from one colour.  (b) Naming `gO57MiddleChoices[0].tableIndex` in
 *      a local at BOTH tail reads, 240 -> 152, measured identical for eleven
 *      carriers and worse for `limit`, `previousGroup`, `row`, `state`,
 *      `valueA`, `valueB`, `valueC` and `currentGroup`.  (c) A re-climbed
 *      statement order, 152 -> 132 in two moves; L146 voids note 7's fixed
 *      point the moment the shape changes.
 *
 *      Flat or rejected this lane, on the pre-adoption shape unless said
 *      otherwise: all 231 spans of an `if (1)` region boundary over the
 *      confirm block's 22 statements, best 203 and worst 246, so L97 and L136
 *      are inert here exactly as L136's scope note predicts for a callee-saved
 *      span; hoisting `outputIndex = 0` above the func_80000F94 call, which
 *      does reach a callee-saved colour but only at 261; sixteen other
 *      carriers for the choice loop's cursor; ten spellings of the countdown
 *      fill; and note 6's tail-read respellings, still byte-identical.
 *
 *      What is left, measured: size delta -4.  The target materialises
 *      `&gO57MiddleChoices` afresh at each tail read where this candidate
 *      keeps it in s5 across the whole tail, which is one instruction fewer
 *      here and the whole of the deficit; the two compiler temps at sp+0x54
 *      and sp+0x58 against the target's sp+0x5C and sp+0x64 are unchanged
 *      from note 7.
 *
 *  9.  DELETE THE WALKING CHOICE POINTER, 2026-09-17 (lane w11-o057).
 *      L145-L154 / L160: the declared `choice` cursor was minting a source
 *      symbol for the array base. Subscripting `gO57MiddleChoices[index]` in
 *      a bottom-tested do-while bounded by
 *      `&gO57MiddleChoices[index] < &gO57MiddleChoices[4]`, with `i` still
 *      the output cursor, lets IDO strength-reduce the cursor into v0 and
 *      the bound into a3. Size delta -4 -> 0 (1208 words both sides).
 *      Positional 132 -> 209 is L155 shadow: aligned 1105/53/14/43 ->
 *      1106/55/10/43, first mismatch +0x34 -> +0x100, frame still 0x140.
 *      The unused `choice` declaration stays -- dropping it shrinks the
 *      frame to 0x138. `while (index < 4)` unrolls (+76). Carrying the
 *      output cursor in `outputIndex` with `i` as the index rotates the
 *      ring (488). `nextSelection` as the index is byte-identical.
 *      blockclimb 209 -> 209, 1617 compiles, move-one fixed point.
 *      Identity gate PASS, CDX_PROC=0, 122 p1 decisions. Compiler temps at
 *      sp+0x54/sp+0x58 against the target's sp+0x5C/sp+0x64 are unchanged.
 *
 * 10.  THREE IDENTITIES CLOSE EVERY INSERTION, 2026-09-19 (lane w26-o057).
 *      209 -> 86 masked at delta 0, byte-exact 1106 -> 1122, naming 55 -> 56,
 *      immediate 10 -> 6, really different 43 -> 24, displacement tax 101 -> 0.
 *      No candidate-only or target-only words remain. Frame still 0x140,
 *      compiler temps still 0x54/0x58 against 0x5C/0x64.
 *
 *      (a) `stackB0[0] = *active` ahead of `stackB0[1] = 0`. blockclimb's
 *      3+ filter skipped this two-store prefix of the columns call. Alone
 *      it is 209 -> 208 and drops the +0xDE0 insertion pair.
 *      (b) `(u8)((state < 2) ^ 1)` to `(u8)(state >= 2)`. The xor-with-1
 *      commons a literal 1 with the countdown fill and the tail. Direct
 *      compare is 208 -> 118 alone, additive with (a) at 117.
 *      (c) `gO57MiddleData31A4 = 2` AFTER the 31AC store. Rejected on the
 *      xor-1 shape; on (b) it closes every remaining insertion (L146).
 *      117 -> 86, 0 ins / 0 del.
 *
 *      L99/L112 cannot move the temps up: unused f32/pointer at the end
 *      shifts them down 4 at an unchanged 209; +8 bytes at the end grows
 *      the frame to 0x148; dropping any unused 4-byte local shrinks it to
 *      0x138. Unused s32 is NOT eliminated here. sourceState[2]/[4],
 *      activePlayers[10]/[12] and stackB0[2]/[4] are byte-identical. */
#ifdef NON_MATCHING
void func_overlay_057_F0004E18_18A8A10(s32 updateRate) {
    s32 i;
    s32 index;
    s32 limit;
    s16 input;
    s32 remainder;
    s32 previousGroup;
    s32 currentGroup;
    s32 oldValue;
    s8 sourceState[4];
    u8 activePlayers[10];
    s32 nextSelection;
    s32 panelX;
    s32 characterId;
    s32 nextValue;
    s32 cursorValue;
    s32 outputIndex;
    s32 state;
    s32 valueA;
    s32 valueB;
    s32 valueC;
    s32 stack64;
    s32 stack5C;
    O57MiddleRenderItem *renderItems;
    O57MiddleRenderItem *renderItem;
    O57MiddleTextureNode textureNodes[2];
    O57MiddleChoice *choice;
    char stackB0[2];
    s16 *color;
    char *palette;
    char renderState[24];
    u8 *active;
    s8 *source;
    O57MiddleOutput *output;
    s32 stack80;
    s32 stack7C;
    s32 stack78;
    s32 *list;
    s32 activeCount;
    s8 rank;
    s8 choiceActive;
    s32 row;

    if (func_overlay_084_F0000C74_18D1154() == 0) {
        if (gO57MiddleMode != 7) {
            gO57MiddleMode = 2;
        }

        if ((gO57MiddleFlags.uword << 13) >> 31) {
            limit = 0x14;
            state = 3;
        } else if (gO57MiddleUnlocked19C != 0) {
            limit = 0x13;
            state = 3;
        } else if (gO57MiddleUnlocked198 != 0) {
            limit = 0xF;
            state = 2;
        } else {
            state = 1;
            limit = 0xB;
        }

        if (gO57MiddleState194 == 0) {
            currentGroup = gO57MiddleSelection / 6;
            if ((gO57MiddleHorizontal < -16) && (gO57MiddleSelection > 0) && (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                func_overlay_045_F0000314_188C76C(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                gO57MiddleSelection -= 1;
                func_overlay_045_F0000314_188C76C(
                    gO57MiddleItems[gO57MiddleSelection], -0xA0, 0xBE, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                if ((gO57MiddleSelection % 6) == 5) {
                    gO57MiddleFade = 0;
                    gO57MiddleFadeDelay = 0x5A;
                    func_overlay_057_F00067DC_18AA3D4(
                        0x2F, (gO57MiddleSelection / 6) + 5, 0.012f);
                }
            } else if ((gO57MiddleHorizontal >= 17) && (gO57MiddleSelection < limit) &&
                       (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                func_overlay_045_F0000314_188C76C(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                gO57MiddleSelection += 1;
                func_overlay_045_F0000314_188C76C(
                    gO57MiddleItems[gO57MiddleSelection], 0x1E0, 0xBE, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                if ((gO57MiddleSelection % 6) == 0) {
                    gO57MiddleFade = 0;
                    gO57MiddleFadeDelay = 0x3C;
                    func_overlay_057_F00067DC_18AA3D4(
                        0x2F, gO57MiddleSelection / 6, 0.012f);
                }
            } else if ((gO57MiddleVertical < -16) && (gO57MiddleSelection < (limit - 2)) &&
                       (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                func_overlay_045_F0000314_188C76C(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                gO57MiddlePreviousSelection = gO57MiddleSelection;
                previousGroup = gO57MiddleSelection / 6;
                gO57MiddleSelection += 3;
                func_overlay_045_F0000314_188C76C(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                currentGroup = gO57MiddleSelection / 6;
                if (previousGroup != currentGroup) {
                    gO57MiddleFade = 0;
                    gO57MiddleFadeDelay = 0x3C;
                    func_overlay_057_F00067DC_18AA3D4(
                        0x2F, currentGroup, 0.012f);
                }
            } else if ((gO57MiddleVertical >= 17) && (gO57MiddleSelection >= 3) &&
                    (gO57MiddleTransition == 0)) {
                    gO57MiddleMoving = 1;
                    func_overlay_045_F0000314_188C76C(
                        gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                    gO57MiddlePreviousSelection = gO57MiddleSelection;
                    previousGroup = gO57MiddleSelection / 6;
                    gO57MiddleSelection -= 3;
                    func_overlay_045_F0000314_188C76C(
                        gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 4);
                    gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                    gO57MiddleAlpha = 0xFF;
                    currentGroup = gO57MiddleSelection / 6;
                    if (previousGroup != currentGroup) {
                        gO57MiddleFade = 0;
                        gO57MiddleFadeDelay = 0x3C;
                        func_overlay_057_F00067DC_18AA3D4(
                            0x2F, currentGroup + 5, 0.012f);
                    }
                } else if ((gO57MiddleButtons & 0x2020) && (gO57MiddleSelection >= 6) &&
                           (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                func_overlay_045_F0000314_188C76C(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                oldValue = gO57MiddleSelection;
                gO57MiddlePreviousSelection = oldValue;
                gO57MiddleSelection = oldValue - 6;
                func_overlay_045_F0000314_188C76C(
                    gO57MiddleItems[gO57MiddleSelection], -0xA0, 0xBE, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                gO57MiddleFade = 0;
                gO57MiddleFadeDelay = 0x5A;
                func_overlay_057_F00067DC_18AA3D4(
                    0x2F, (gO57MiddleSelection / 6) + 5, 0.012f);
                } else if ((gO57MiddleButtons & 0x10) && (currentGroup < state) &&
                           (gO57MiddleTransition == 0)) {
                gO57MiddleMoving = 1;
                func_overlay_045_F0000314_188C76C(
                    gO57MiddleItems[gO57MiddleSelection], 0xA0, 0x104, 0x104);
                oldValue = gO57MiddleSelection;
                gO57MiddlePreviousSelection = oldValue;
                gO57MiddleSelection = oldValue + 6;
                func_overlay_045_F0000314_188C76C(
                    gO57MiddleItems[gO57MiddleSelection], 0x1E0, 0xBE, 4);
                gO57MiddlePreviousAlpha = gO57MiddleAlpha;
                gO57MiddleAlpha = 0xFF;
                gO57MiddleFade = 0;
                gO57MiddleFadeDelay = 0x5A;
                func_overlay_057_F00067DC_18AA3D4(
                    0x2F, gO57MiddleSelection / 6, 0.012f);
                if (gO57MiddleSelection > limit) {
                    gO57MiddleSelection = limit;
                }
                }

            if ((gO57MiddleButtons & 0x4000) && (gO57MiddleTransition == 0) &&
                (gO57MiddleCanLeave != 0)) {
                func_80000F94(0xD, 0);
                func_overlay_084_F0001350_18D1830();
                func_80050704(gO57MiddlePathId);
                gO57MiddleNextMode = 7;
                func_overlay_057_F00067DC_18AA3D4(
                    0x2F, 0, -0.012f);
                gO57MiddleState188 = 0;
                gO57MiddleMode = 0;
                func_overlay_084_F0001060_18D1540(1);

                list = gO57MiddlePathList;
                while (*list != -1) {
                    func_80050688((u8)*list);
                    func_overlay_057_F00067DC_18AA3D4(
                        *list, gO57MiddlePathIndices[*list], 0.007f);
                    list++;
                }
                list = gO57MiddleStopList;
                while (*list != -1) {
                    func_80050704((u8)*list);
                    list++;
                }
            }

            for (i = 0; i < updateRate; i++) {
                gO57MiddlePanelPosition += (-0x1400 - gO57MiddlePanelPosition) >> 2;
            }
        } else {
            if (gO57MiddleState194 == 1) {
                for (i = 0; i < updateRate; i++) {
                    gO57MiddlePanelPosition += (-gO57MiddlePanelPosition) >> 2;
                }
            }
            if ((gO57MiddleButtons & 0x4000) && (gO57MiddleTransition == 0)) {
                func_80000F94(0xD, 0);
                gO57MiddleState194 = 0;
                gO57MiddleMode = 2;
            }
        }

        func_overlay_057_F0001020_18A4C18(updateRate);
        if ((gO57MiddlePanelPosition >> 4) >= -0x135) {
            renderItems = (O57MiddleRenderItem *)
                ((u8 *) func_800291C4() +
                 ((s32) func_80025D60(
                      gO57MiddleCourseIds[gO57MiddleSelection]) << 5));
            func_8004B0A4(0);
            row = 0x51;
            panelX = gO57MiddlePanelPosition >> 4;
            gO57MiddleRenderParameters.position = (f32) panelX;
            gO57MiddleRenderParameters.scale = 7.0f;
            func_80022A50(&gO57MiddleDisplayList, &gO57MiddleVertexList);
            func_80039E34(8);
            func_8004B0B8(
                gO57MiddleInfo.value08, gO57MiddleInfo.value09, gO57MiddleInfo.value0A,
                0xFF, 0xFF);
            valueA = panelX + 0xA0;
            stack64 = valueA;
            func_8004B0F8(
                &gO57MiddleDisplayList, valueA, 0x23,
                gO57MiddleText[gO57MiddleCourseNames[gO57MiddleSelection]], 4);
            func_8004B0B8(0xFF, 0x80, 0, 0xFF, 0xFF);
            func_8004B0F8(
                &gO57MiddleDisplayList, valueA, 0x37,
                gO57MiddleText[0xD0 / 4], 4);
            func_8004B0B8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
            if (func_overlay_068_F000146C_18C85CC(
                    gO57MiddleCourseIds[gO57MiddleSelection]) != 0) {
                    func_8002F618(
                        &gO57MiddleDisplayList, gO57MiddleBadge, panelX + 0x30,
                        0x26, 0xFF, 0xFF, 0xFF, 0xFF);
            }

            renderItem = renderItems;
            stack5C = panelX + 0x56;
            i = 0;
            do {
                func_overlay_056_F00000B8_18A2E30(
                    renderItem->type, &stack80, &stack7C, &stack78);
                palette = gO57MiddleEmptyFormat;
                if (renderItem->type == 0) {
                    valueA = 0x4A;
                    func_800429A4(
                        renderState, palette);
                } else {
                    valueB = func_8003A700(
                                 renderItem->value04) & 0xFF;
                    valueC = func_8003A700(
                                 renderItem->value05) & 0xFF;
                    func_800429A4(
                        renderState, gO57MiddleResultFormat, valueB, valueC,
                        func_8003A700(
                            renderItem->value06),
                        stack80, stack7C, stack78);
                    valueA = renderItem->value07 + 0x51;
                }
                if (i < 3) {
                    func_8004B0F8(
                        &gO57MiddleDisplayList, panelX + 0x2E, row,
                        gO57MiddleLabels[i], 0);
                }
                textureNodes[0].texture = gO57MiddleGraphics[valueA];
                textureNodes[0].alternate = NULL;
                textureNodes[0].x = (s16)stack5C;
                textureNodes[0].y = (s16)(row - 4);
                textureNodes[0].packedOffset = 0;
                textureNodes[1].texture = NULL;
                func_8002F618(
                    &gO57MiddleDisplayList, textureNodes, 0, 0,
                    0xFF, 0xFF, 0xFF, 0xFF);
                color = gO57MiddleColumns;
                active = (u8 *)renderState;
                do {
                    stackB0[0] = *active;
                    stackB0[1] = 0;
                    func_8004B0F8(
                        &gO57MiddleDisplayList, *color + panelX, row, stackB0, 0);
                    color++;
                    active++;
                } while (color != gO57MiddleColumnsEnd);
                if (i == 2) {
                    row += 0x1B;
                    func_8004B0B8(
                        0xFF, 0x80, 0, 0xFF, 0xFF);
                    func_8004B0F8(
                        &gO57MiddleDisplayList, stack64, row,
                        gO57MiddleText[0xD4 / 4], 4);
                    func_8004B0B8(
                        0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                i++;
                renderItem++;
                row += 0x1B;
            } while (i != 4);
            func_8002FB34(
                &gO57MiddleDisplayList, gO57MiddleCaption, (f32)(panelX + 0x2D),
                184.0f, 1.0f, 1.0f, -2, 3);
        }

        if (gO57MiddleFadeDelay > 0) {
            gO57MiddleFadeDelay -= updateRate;
        } else {
            gO57MiddleFade += updateRate * 4;
            if (gO57MiddleFade >= 0x100) {
                gO57MiddleFade = 0xFF;
            }
        }

        if ((gO57MiddleButtons & 0x9000) && (gO57MiddleTransition == 0)) {
            func_80000F94(0xC, 0);
            if (gO57MiddlePlayerCount >= 2 || gO57MiddleState194 == 1) {
                active = &activePlayers[9];
                for (outputIndex = 9; outputIndex >= 0; outputIndex--) {
                    *active-- = 1;
                }
                i = 0;
                index = 0;
                source = sourceState;
                do {
                    rank = gO57MiddleChoices[index].active;
                    *source = rank;
                    if (rank != 0) {
                        gO57MiddleOutput[i].controller =
                            gO57MiddleCharacterIds[gO57MiddleChoices[index].tableIndex];
                        activePlayers[gO57MiddleCharacterIds[gO57MiddleChoices[index].tableIndex]] = 0;
                        i++;
                    }
                    source++;
                    index++;
                } while (&gO57MiddleChoices[index] < &gO57MiddleChoices[4]);
                i = 0;
                for (outputIndex = gO57MiddlePlayerCount; outputIndex < 6; outputIndex++) {
                    while (activePlayers[i] == 0) {
                        i++;
                    }
                    gO57MiddleOutput[outputIndex].controller = i;
                    i++;
                }
                func_80025444(
                    sourceState);
                func_80028D24(0);
                func_80028540(gO57MiddlePlayerCount);
                gO57MiddleData31E4 = 0;
                if (gO57MiddlePlayerCount == 1) {
                    gO57MiddleOutput[0].value0 = 1;
                    if ((gO57MiddleFlags.half & 1) != 0) {
                        gO57MiddleFlags.bytes[1] &= 0xFE;
                        func_800291B4();
                        func_8003A680(0x14);
                    }
                } else {
                    gO57MiddleOutput[0].value0 = 4;
                }
                state = gO57MiddlePlayerCount;
                if (((state == 2) || (state == 3)) &&
                    (gO57MiddleData3214 != 0)) {
                    gO57MiddleData319C = (s8)(4 - state);
                    gO57MiddleData3198 = 4;
                } else {
                    gO57MiddleData319C = 0;
                    gO57MiddleData3198 = (u8)state;
                }
                gO57MiddleData31AC = (u8)(state >= 2);
                gO57MiddleData31A4 = 2;
                gO57MiddleData31B8 = gO57MiddleData31B4;
                gO57MiddleData31A8 = 0;
                if (gO57MiddleData31E4 > 0) {
                    characterId = gO57MiddleChoices[0].tableIndex;
                    gO57MiddleData31E8 = gO57MiddleCourseIds[gO57MiddleSelection];
                    gO57MiddleData31EC = gO57MiddleCharacterIds[characterId];
                    gO57MiddleData31F0 = 5;
                    gO57MiddleData31F4 = 0;
                    func_80028374(0x12, 0, 0, 0xF, 1, 0);
                    func_80028528(1);
                } else {
                    characterId = gO57MiddleChoices[0].tableIndex;
                    func_80028374(
                        gO57MiddleCourseIds[gO57MiddleSelection],
                        gO57MiddleCharacterIds[characterId],
                        0, 5, 1, 0);
                }
                gO57MiddleTransition = 1;
                func_overlay_084_F0001398_18D1878();
                if ((gO57MiddleData31AC == 0) ||
                    ((gO57MiddleData31AC != 0) && (gO57MiddleData3208 != 0))) {
                    func_80005548(gO57MiddleData3198);
                    if (gO57MiddleData3208 != 0) {
                        gO57MiddleData3208 = 0;
                    }
                }
            } else {
                gO57MiddleState194 = 1;
            }
        }
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o057/func_overlay_057_F0004E18_18A8A10/func_overlay_057_F0004E18_18A8A10.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_057_F0004E18_18A8A10:start
 * symbol: func_overlay_057_F0004E18_18A8A10
 * score: 86/1208 words
 * frame: 0x140
 * relocations: 375
 * first-mismatch: +0x100
 * summary: Three identities closed every insertion (209 -> 86, tax 101 -> 0). Remaining is 24 aligned structural plus compiler temps at 0x54/0x58 against 0x5C/0x64. L99/L112 cannot raise those temps without shrinking the frame.
 * PLATEAU-HANDOFF:func_overlay_057_F0004E18_18A8A10:end
 */
