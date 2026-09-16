#include "PR/ultratypes.h"

/* NON_MATCHING structural reconstruction from Mickey's runtime relocations.
 * Resident identities use the ROM-table exports and reserved section bases;
 * D_o058 names identify byte offsets in overlay 58, including its BSS.
 * The guarded body retains the target call graph and typed data accesses.
 */
/*
 * Plateau (2026-09-10): 733 of 3,614 relocation-masked words differ, down from
 * 894.  Size delta 0, frame 0x138.  Under a shape-tolerant alignment 3,034
 * rows are byte-exact (up from 2,874) and 631 of 848 saved-register operand
 * slots agree (up from 434); the s7-with-s8 transposition that three grinds
 * named is GONE -- 0 transposed slots against 187.  Report all three numbers
 * for anything measured here: at 894 words the positional count is dominated
 * by displacement and does not order configurations on its own.
 *
 * The residual is p1 colour, not shape.  Every one of this procedure's 428
 * allocator decisions is phase one, so declaration order and definition
 * position are the wrong axis for a COLOUR (165 earlier forms were flat for
 * that reason).  **Statement order is not** -- see note 7 below; the phase
 * census retires L106 and nothing else, and statement order reaches as1's
 * scheduling and ugen's emission order, which the census does not touch.  It
 * is worth 733 -> 688 here on its own.  For the colour the axis is L100's
 * ratio,
 * `save = totalsave / nocs`, and there are two source dials on it:
 *
 *   1. Discarded-expression probes at loop depth 2 (three of them in case 9's
 *      grid inner loop, below).  100 of totalsave each, no instructions.
 *   2. WHICH DECLARED SCALAR PLAYS A LOOP'S INDEX.  A loop whose index is a
 *      carrier of its own takes its occurrences out of `i`'s web, and the
 *      target's own shape says it did this: it emits no `move <saved>,zero`
 *      in either the case-2 decrement loop's preheader or case 8's, which is
 *      what a loop whose index dies at strength reduction looks like.  Case 2
 *      is carried by `savedPosition` and case 8 by `opponent`; both are dead
 *      in their case, so the rename carries no meaning.  Worth 894 -> 755 and
 *      755 -> 733 respectively, and 197 saved-register slots between them.
 *
 * Still open, largest first: 119 words in case 9's grid-loop exit test (the
 * target tests `slti $at,<saved>,10` and the natural `opponent < 0xA` puts 10
 * in a saved register instead -- a second p1 contest, this one between the
 * constant's web and a global address web, and NOT reachable by respelling
 * either loop's bound: seven erase-loop spellings crossed with three grid
 * spellings are all exactly inert); 72 words in case 8; 40 in case 12; 34 in
 * case 1; 34 in case 2's draw loop; 29 in case 3; 22 at the case 1/2 boundary.
 *
 * Falsified: spelling a frame address as a cached pointer local (`char
 * *textPtr = &text[0];`, and the same for `nodes` and `character`) does not
 * reproduce the target's cached form -- all three measure one word WORSE at
 * unchanged size and frame, and caching `&character[0]` for the +0x3704 site
 * is byte-identical to not doing it.
 *
 *  7.  STATEMENT ORDER, 2026-09-12 (lane p11-big).  The claim above that
 *      statement order is "the wrong axis" is false as stated, and it had
 *      been closing the largest untried dimension on this function.  A
 *      move-one hill climb over EVERY run of three or more consecutive
 *      single-line statements in the body -- 67 such runs, each climbed to a
 *      local optimum and the whole sweep then re-run twice to a fixed point --
 *      takes 733 -> 688 masked at size delta 0, byte-exact 3120 -> 3144,
 *      register naming 379 -> 371, immediate 25 -> 24 and really different
 *      103 -> 88, with both frame ladders unchanged.  Five orders moved:
 *      `rowY = rowBase;` to the top of case 1, `i = 0;` after `x = ...` in
 *      case 1, `columnStep` after `i = 0;` in case 5, `nodes[0].texture` to
 *      the head of its store group in the portrait loop, `cursor++` after
 *      `textY += 0x1E;` in case 6, and `nodes[0].y` before `nodes[0].x` in
 *      case 12.  672 is reachable but only through three moves that are
 *      SEMANTICALLY WRONG -- two put `nodes[1].texture = 0;` after the
 *      func_8002F618 call that walks the node list to its NULL terminator,
 *      and one moves `x = -x;` in front of a call that takes `x + 0xA0` as an
 *      argument.  They are rejected; 688 is the safe move-one optimum, and
 *      re-climbing from it finds only those same three moves again.
 *
 *  8.  THE COUNTED-LOOP EXIT REWRITE, 2026-09-12 (lane p12-whale).  688 ->
 *      634 at size delta 0, byte-exact 3144 -> 3260, naming 371 -> 247,
 *      immediate 24 -> 31, really different 88 -> 89, frame 0x138 both sides.
 *      uopt rewrites a provably counted loop's `< CONST` exit test into `!=`
 *      against a constant hoisted into a register, and it does so
 *      UNCONDITIONALLY at this loop's depth -- measured at bounds 0x9, 0xA,
 *      0xB and 0x40, and still with every other `0xA` in the procedure
 *      removed.  So no spelling of `opponent < 0xA` reaches the target's
 *      `slti $at,<index>,10`: `< 0xA`, `<= 9`, `< 10`, `!(>= 0xA)`,
 *      `0xA > opponent`, `< 0x000A`, `< 5 + 5`, `< 0xAL`, `< (s32) 0xAU` and
 *      `< 0xA && 1` all compile to ONE object, and so do the erase loop's
 *      `!= 0xA`, `< 0xA` and `<= 9`.  What the rewrite needs is a known
 *      initial value, so an OPAQUE ZERO retires it at no instruction cost.
 *      The old `(opponent - 1) < (0xA - 1)` bought the same `slti` by
 *      comparing a derived value, but its `addiu` spent one temp-ring draw
 *      and rotated the whole free list for the remaining 1,196 bytes: a
 *      closed nine-cycle t1->t5->t2->t6->t7->t8->t3->t4->t9->t1 carrying 150
 *      of the 212 differing register slots after +0x3148, and none before it.
 *
 *      The statement-order climb was re-run from the new shape (L146: an
 *      order optimum belongs to the shape) and found 634 -> 623 in two
 *      moves, both in case 10's node fill: `nodes[0].texture` to the head of
 *      the store group, and `nodes[0].x` before `nodes[0].y`.  No call sits
 *      between them and the NULL terminator is still stored before
 *      func_8002F618.  An opaque zero at each of the other 17 index resets
 *      x four carriers (69 cells) is inert or worse everywhere: the lever is
 *      site-specific to this one loop.
 *
 * The `if (i != 0);` statements below are discarded-expression probes
 * (ido-5.3 L37) -- zero instructions, one web occurrence each.  They were
 * found by a two-pass climb over 11,304 variants and are a local optimum;
 * a further 144-variant sweep (one probe at the top of each case body, for
 * twelve different locals) found every size-preserving one inert.
 */
#ifdef NON_MATCHING
#include "game/anim.h"
#include "game/menu.h"
#include "game/font.h"
#include "overlays/overlay_056.h"

struct MenuCommand { u32 w0; u32 w1; };
typedef struct RcpTextureInfo RcpTextureInfo;
typedef struct RcpTextureNode {
    RcpTextureInfo *texture;
    RcpTextureInfo *alternate;
    u32 packedOffset;
    s16 x;
    s16 y;
} RcpTextureNode;

typedef struct Overlay58RaceEntry {
    u8 character;
    u8 variant;
    u8 variantCopy;
    u8 gap;
    s32 value;
    s32 lapTimes[3];
    u8 pad14[8];
    u8 counters[6];
    u16 rank;
    u8 flags[4];
} Overlay58RaceEntry;

typedef struct Overlay58RaceState {
    u8 mode;
    u8 active;
    u8 player;
    u8 countdown;
    Overlay58RaceEntry entries[6];
} Overlay58RaceState;

typedef struct SavesPackedEntry {
    s32 value;
    u8 name[3];
    u8 character;
} SavesPackedEntry;

typedef struct SavesSlot {
    SavesPackedEntry records[4];
} SavesSlot;

typedef struct ColourCycle {
    s32 frame;
    s32 time;
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
    struct ColourCycle *table;
} ColourCycle;

typedef union Overlay58AnimationState {
    s32 word;
    u8 bytes[4];
} Overlay58AnimationState;

typedef struct Overlay58LanguageText {
    char *text[182];
} Overlay58LanguageText;

extern Overlay58RaceState *func_80028F54(void);
extern void func_80036AB0(void *cycle, s32 updateRate);
extern void func_8004B0A4(s32 font);
extern void func_8004B0F8(MenuCommand **displayList, s32 x, s32 y, char *text, s32 flags);
extern void func_8002F618(MenuCommand **displayList, RcpTextureNode *node, s32 x, s32 y, u8 red, u8 green, u8 blue, u8 alpha);
extern void func_8005055C(s32 pathIndex);
extern s32 sprintf(char *buffer, const char *format, ...);
extern void amSndPlay(u16 sound, void **handle);
extern void mainChangeCameras(s32 mode);
extern void joyCreateMap(s8 *activePlayers);
extern void mainChangeLevel(s32 nextLevel, s32 nextCharacter, s32 nextAnimGroup, s32 nextMenu, s32 arg4, s32 arg5);
extern void mainSetAnimGroup(s32 group);
extern void func_8003A754(void);
extern char *func_8003A5A0(s32 value);
extern SavesSlot *func_800291C4(void);
extern s32 levelGetBlurEffect(s32 level);
extern s32 func_8003A6B0(u8 index);
extern void func_80029120(u32 flags);
extern void func_8002917C(void);
extern s32 func_8003A700(u8 index);

extern u8 D_8007BEF8;
extern u8 D_8007BEFC_o058Reloc;
extern u8 D_8007BF0C_o058Reloc;
extern s32 D_8007BF44_o058Reloc;
extern s32 D_8007C1A0_o058Reloc;
extern Overlay58LanguageText *D_8007C0B8;
extern MenuCommand *D_800D3140;
extern s32 D_800D31B8_o058Reloc;
extern s16 D_800D31BC_o058Reloc;
extern s16 D_800D31BE_o058Reloc;
extern u8 D_800D31C4_o058Reloc[4];
extern RcpTextureInfo *D_800D31C8_o058Reloc[];
extern s16 D_8007C0C0_o058Reloc[4][4];

extern s32 D_o058_5E50[6];
extern char *D_o058_5E68[4];
extern Overlay58AnimationState D_o058_5E84;
extern Overlay58RaceEntry *D_o058_5EC8[6];
extern Overlay58RaceEntry *D_o058_5EE0[6];
extern s32 D_o058_5EF8[6];
extern s32 D_o058_5F10[6];
extern ColourCycle D_o058_5F38;
extern s32 D_o058_5B28[6];
extern char *D_o058_5C5C[3];
extern s16 D_o058_5C68[6];
extern s16 D_o058_5C74[6];
extern s16 D_o058_5C80[6];
extern s16 D_o058_5C8C[6];
extern char *D_o058_5C98[6];
extern s16 D_o058_5CB0[12];


extern u16 D_8007BF1C_o058Reloc;
extern s32 D_8007BF48_o058Reloc;
extern s32 D_8007BF4C_o058Reloc;
extern s32 D_8007BF50_o058Reloc;
extern s32 D_8007BF54_o058Reloc;
extern u8 D_8007BF74_o058Reloc;
extern s32 D_8007C1B4_o058Reloc;
extern s16 D_800D304E_o058Reloc;
/* Runtime relocation evidence, not a resident-address naming inference:
 * all six halfword reads bind the same reserved-selector identity.
 * The former D_800D3050 spelling therefore shares this import; it does
 * not denote a second object. Each of the six owned HI/LO pairs agrees
 * in selector and base, with a zero candidate addend.
 * Coalescing the two spellings preserves the compiled sections and the
 * relocation map after the proved import renames. The older inference
 * of distinct storage from compiler behavior is superseded.
 * The _o058Reloc imports keep reserved runtime identities separate from
 * resident-address names. See the handoff for the scoped proof. */

extern RcpTextureNode D_o058_5BA0[];
extern s32 D_o058_5CD8;
extern char D_o058_5D2C[];
extern char D_o058_5D30[];
extern char D_o058_5D34[];
extern char D_o058_5D38[];
extern char D_o058_5D3C[];
extern char D_o058_5D44[];
extern char D_o058_5D48[];
extern char D_o058_5D50[];
extern char D_o058_5D54[];
extern char D_o058_5D5C[];
extern char D_o058_5D60[];
extern char D_o058_5D64[];
extern char D_o058_5D68[];
extern char D_o058_5D6C[];
extern char D_o058_5D70[];
extern char D_o058_5D74[];
extern char D_o058_5D78[];
extern char D_o058_5D7C[];
extern char D_o058_5D84[];
extern char D_o058_5D88[];
extern char D_o058_5D90[];
extern char D_o058_5D94[];
extern char D_o058_5D98[];
extern char D_o058_5DA0[];
extern char D_o058_5DA4[];
extern char D_o058_5DAC[];
extern char D_o058_5DB0[];
extern char D_o058_5DB4[];
extern char D_o058_5DB8[];
extern char D_o058_5DC4[];
extern s32 D_o058_5E78;
extern s32 D_o058_5E7C;
extern s32 D_o058_5E80;
extern s32 D_o058_5E88;
extern s32 D_o058_5E8C;
extern s32 D_o058_5E90;
extern s32 D_o058_5E94;
extern s32 D_o058_5E98;
extern s32 D_o058_5E9C;
extern s32 D_o058_5EA0;
extern s32 D_o058_5EA4;
extern s32 D_o058_5EA8;
extern s32 D_o058_5EAC;
extern s32 D_o058_5EB0;
extern s32 D_o058_5EB4;
extern s32 D_o058_5EC0;
extern s32 D_o058_5EC4;
extern s32 D_o058_5F28;
extern s32 D_o058_5F2C;
extern s32 D_o058_5F30;

extern s8 D_o058_5F48[4];

void func_overlay_058_F000138C_18B0574(s32 arg0) {
    s32 i;
    s32 opponent;
    s32 textY;
    s32 columnX;
    s32 x;
    char **cursor;   /* one cell, exactly as the scalar it replaces; case 12's title pointer (wv-x) */
    s32 portraitX;   /* unused: the loop indices below ride on letter1 and textY (wv-v) */
    s32 columnStep;
    s32 portraitIndex;
    s32 countdownX;
    s32 seconds;
    s32 minutes;
    s32 centiseconds;
    s32 rowY;
    s32 highlighted;
    s32 rowHeight;
    s32 letter0;
    s32 letter1;
    s32 savedX;
    s32 savedPosition;
    s32 savedOffset;
    s32 rowBase;
    s32 columnCount;
    char character[2];
    char text[24];
    s32 erase;
    Overlay58RaceState *state;
    RcpTextureNode nodes[4];
    AnimPath *path;
    SavesSlot *saves;
    SavesSlot *slot;

    state = func_80028F54();
    func_80036AB0(&D_o058_5F38, arg0);
    func_8004B0A4(0);
    switch (D_o058_5E94) {
    case 1:
    case 2:
        if ((D_o058_5E50[0] == -1) && (D_8007BEF8 > 0)) {
            /* `letter1`, the same name case 2's decrement and draw loops
             * subscript with: uopt keeps one web per IR name (L131), so the
             * D_o058_5E50 cursor spans the draw loop's calls and takes s0 on
             * its own, as the target has it in all three loops.  Any other
             * index here leaves this loop's cursor in a0 (wv-r).
             * WHY letter1 (wv-v): the index's seven folded resets are dead
             * defs that uopt still colours, as one web per symbol.  A
             * symbol of its own (portraitX) takes a0, the lowest free
             * colour, and a0 here collides with D_o058_5E94's value web,
             * which cascades a1->a0/a2->a1/a3->a2 through the function.
             * Written on a local that is dead in cases 1/2 and already
             * coloured s1 by case 10 (letter1, save 10.0), the resets join
             * that web and take s1, which nothing at blocks 9/17/85/93
             * wants.  The case-1 draw loop must share the name (the
             * D_o058_5EF8 address web spans entry + both draw loops), and
             * letter1 is dead throughout it; textY is not (it is case 1's
             * text y), so cases 3/13 ride on textY and cases 1/2 on letter1.
             * s0 carriers (letter0, portraitIndex, countdownX) flip their
             * own case; savedPosition and erase cost width. */
            letter1 = 0;
            do {
                D_o058_5E50[letter1] = D_o058_5B28[D_o058_5EF8[letter1]];
                letter1++;
            } while (letter1 < D_8007BEF8);
        }
        break;
    }

    rowHeight = D_o058_5C74[D_8007BEF8 - 1];
    rowBase = D_o058_5C68[D_8007BEF8 - 1];
    switch (D_o058_5E94) {
    case 1:
        fontColour(0xFF, 0x80, 0, 0xFF, (0xFF - (i = 0))); /* Keep reset in the last argument value. */

        func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5EA4 + 0xA0, 0x1E, D_8007C0B8->text[0x27], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        x = D_o058_5E98 + D_o058_5EA8;
        letter1 = 0; rowY = rowBase;
        if ((s32) D_8007BEF8 > 0) {
            do {
                x = -x;
                nodes[0].texture = D_800D31C8_o058Reloc[0x51 + D_o058_5EC8[letter1]->character];
                nodes[0].alternate = NULL;
                nodes[0].x = x + 0x4E;
                nodes[0].y = rowY - 4;
                nodes[0].packedOffset = 0;
                nodes[1].texture = 0;
                func_8002F618(&D_800D3140, &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                func_8004B0F8(&D_800D3140, x + 0x28, rowY, D_o058_5C98[D_o058_5EF8[letter1]], 0);
                if ((D_8007C1A0_o058Reloc == 1) && (state->entries == D_o058_5EC8[letter1])) {
                    fontColour((s32) D_o058_5F38.red, (s32) D_o058_5F38.green, (s32) D_o058_5F38.blue, 0xFF, 0xFF);
                } else {
                    fontColour(0, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                func_8004B0F8(&D_800D3140, x + 0x71, rowY, D_8007C0B8->text[D_o058_5EC8[letter1]->character + 0x1A], 0);
                if (state->mode == 5) {
                    if (i == 0) {
                        overlay56SplitTime(D_o058_5EC8[letter1]->value, &minutes, &seconds, &centiseconds);
                    } else {
                        overlay56SplitTime(D_o058_5EC8[0]->value - D_o058_5EC8[letter1]->value, &minutes, &seconds, &centiseconds);
                    }
                } else if (i == 0) {
                    overlay56SplitTime(D_o058_5EC8[letter1]->value, &minutes, &seconds, &centiseconds);
                } else {
                    overlay56SplitTime(D_o058_5EC8[letter1]->value - D_o058_5EC8[0]->value, &minutes, &seconds, &centiseconds);
                }
                fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
                textY = rowY + D_o058_5EAC;
                if ((i == 0) || (minutes != 0)) {
                    sprintf(&text[0], D_o058_5D2C, minutes);
                    func_8004B0F8(&D_800D3140, x + 0xDC, textY, &text[0], 1);
                    func_8004B0F8(&D_800D3140, x + 0xDF, textY, D_o058_5D30, 0);
                }
                if (i != 0) {
                    if (state->mode == 5) {
                        func_8004B0F8(&D_800D3140, x + 0xBA, textY, D_o058_5D34, 0);
                    } else {
                        func_8004B0F8(&D_800D3140, x + 0xBA, textY, D_o058_5D38, 0);
                    }
                }
                sprintf(&text[0], D_o058_5D3C, seconds);
                func_8004B0F8(&D_800D3140, x + 0xE6, textY, &text[0], 0);
                func_8004B0F8(&D_800D3140, x + 0xFD, textY, D_o058_5D44, 0);
                sprintf(&text[0], D_o058_5D48, centiseconds);
                func_8004B0F8(&D_800D3140, x + 0x104, textY, &text[0], 0);
                letter1 += 1; i += 1;
                rowY += rowHeight;
            } while (i < (s32) D_8007BEF8);
        }
        D_o058_5E98 -= arg0 * 0xF;
        if (D_o058_5E98 < 0) {
            D_o058_5E98 = 0;
            if ((D_o058_5EB0 == 0) && (D_800D31B8_o058Reloc & 0x9000)) {
                if ((D_o058_5EC4 >= 4) && (D_8007BF0C_o058Reloc == 0)) {
                    if (state->countdown == 0) {
                        D_o058_5EB0 = 5;
                    } else {
                        D_o058_5EB0 = 6;
                    }
                } else {
                    *D_o058_5E50 = -1;
                    D_o058_5EB0 = 2;
                }
                amSndPlay(0xCU, NULL);
            }
            if (D_o058_5EB0 != 0) {
                D_o058_5EA4 += arg0 * 0xF;
                if ((D_o058_5EB0 == 5) || (D_o058_5EB0 == 6)) {
                    D_o058_5EA8 += arg0 * 0xF;
                    if (D_o058_5EA8 >= 0x141) {
                        D_o058_5E94 = D_o058_5EB0;
                        amSndPlay(0x1FAU, NULL);
                        return;
                    }
                } else {
                    D_o058_5EAC += arg0 * 0xF;

                    if ((D_o058_5EAC >= 0xB5) && (D_o058_5EA4 >= 0x141)) {
                        D_o058_5E9C = 0x6E;
                        D_o058_5E94 = D_o058_5EB0;
                        amSndPlay(0x1FAU, NULL);
                        D_o058_5EB0 = 0;
                        D_o058_5EB4 = 0x4B;
                        D_o058_5EA0 = 0;
                        return;
                    }
                }
            }
        }
        break;
    case 2:
        i = 0;
        if (*D_o058_5E50 > 0) {
            D_o058_5EB4 = D_o058_5EB4 - arg0;
            if (D_o058_5EB4 < 0) {
                D_o058_5EB4 = 0xF;
                amSndPlay(0x1BU, NULL);
                if ((s32) D_8007BEF8 > 0) {
                    /* Two inductions on purpose.  `letter1` subscripts and
                     * tests, so `D_o058_5E50[letter1]` is the entry loop's
                     * and the draw loop's IR name (one web, callee-saved, s0);
                     * `i` is counted beside it so the post-loop `i = 0` is a
                     * real reset and survives.  Indexing by `i` alone makes
                     * the draw loop initialise its cursors from `i * 4` (two
                     * reaching definitions, no constant folding); any other
                     * lone index lets uopt delete the reset.  See
                     * docs/whale-role-carriers.md. */
                    i = 0;
                    letter1 = 0;
                    do {
                        if (D_o058_5E50[letter1] > 0) {
                            D_o058_5E50[letter1] -= 1;
                        }
                        letter1++;
                        i++;
                    } while (letter1 < D_8007BEF8);
                    i = 0;
                }
            }
        }
        fontColour(0xFF, 0x80, 0, 0xFF, 0xFF);
        func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5EA0 + D_o058_5EA4 + 0xA0, 0x1E, D_8007C0B8->text[0x28], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        savedX = D_o058_5E98;
        savedOffset = D_o058_5EA0;
        letter1 = 0; /* Independent array induction; visible i stays early. */
        rowY = rowBase;
        if ((s32) D_8007BEF8 > 0) {
            do {
                D_o058_5E98 = -D_o058_5E98;
                D_o058_5EA0 = -D_o058_5EA0;

                nodes[0].texture = D_800D31C8_o058Reloc[0x51 + D_o058_5EC8[letter1]->character];
                nodes[0].alternate = NULL;
                nodes[0].x = D_o058_5E98 + D_o058_5EA0 + 0x4E;
                nodes[0].y = rowY - 4;
                nodes[0].packedOffset = 0;
                nodes[1].texture = 0;
                func_8002F618(&D_800D3140, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5EA0 + 0x28, rowY, D_o058_5C98[D_o058_5EF8[letter1]], 0);
                if ((D_8007C1A0_o058Reloc == 1) && (state->entries == D_o058_5EC8[letter1])) {
                    fontColour((s32) D_o058_5F38.red, (s32) D_o058_5F38.green, (s32) D_o058_5F38.blue, 0xFF, 0xFF);
                } else {
                    fontColour(0, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5EA0 + 0x71, rowY, D_8007C0B8->text[D_o058_5EC8[letter1]->character + 0x1A], 0);
                /* `columnX`, not `textY`: uopt colours a symbol as ONE web across
                 * every case (textY is 29 blocks over cases 1/2/8/9/10 and takes
                 * s4 early), and the target's rank difference here sits in s1 with
                 * the format-string address in s4 -- the colours of a web decided
                 * late, i.e. the variable that also carries column X in cases 9/12/13.
                 * Fourteen other carriers measured 151-3209; this one is -10 at delta 0. */
                columnX = D_o058_5EC8[letter1]->rank - D_o058_5E50[letter1];
                fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
                sprintf(&text[0], D_o058_5D50, columnX);
                func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5E9C + D_o058_5EA0 + 0xBE, rowY, &text[0], 0);
                sprintf(&text[0], D_o058_5D54, D_o058_5E50[letter1]);
                func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5E9C + D_o058_5EA0 + 0xEB, rowY, &text[0], 0);
                func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5E9C + D_o058_5EA0 + 0x113, rowY, D_o058_5D5C, 0);
                letter1 += 1; i += 1;
                rowY += rowHeight;
            } while (i < (s32) D_8007BEF8);
        }
        D_o058_5E98 = savedX;
        D_o058_5EA0 = savedOffset;
        D_o058_5E9C -= arg0 * 0xF;
        D_o058_5EA0 -= arg0 * 0xF;
        D_o058_5EA4 -= arg0 * 0xF;

        if ((D_o058_5E9C < 0) && (D_o058_5EA0 < 0) && (D_o058_5EA4 < 0)) {
            if (D_o058_5EB0 == 0) {
                if (D_800D31B8_o058Reloc & 0x9000) {
                    D_o058_5EB0 = 3;
                    amSndPlay(0xCU, NULL);
                } else if ((D_800D31B8_o058Reloc & 0x4000) && (state->mode != 5)) {
                    D_o058_5EB0 = 1;
                    amSndPlay(0xDU, NULL);
                }
            } else {
                D_o058_5E98 += arg0 * 0xF;
                if (D_o058_5E98 >= 0x141) {
                    D_o058_5EAC = 0;
                    D_o058_5E94 = D_o058_5EB0;
                    amSndPlay(0x1FAU, NULL);
                    D_o058_5EB0 = 0;
                    D_o058_5EA4 = 0;
                }
            }
        }
        if (D_o058_5E9C < 0) {
            D_o058_5E9C = 0;
        }
        if (D_o058_5EA0 < 0) {
            D_o058_5EA0 = 0;
        }
        if (D_o058_5EA4 < 0) {
            D_o058_5EA4 = 0;
            return;
        }
        break;
    case 3:
        fontColour(0xFF, 0x80, 0, 0xFF, (0xFF - (i = 0)));
        func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5E9C + 0xA0, 0x1E, D_8007C0B8->text[0x29], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        savedX = D_o058_5E98;

        savedPosition = D_o058_5E9C;
        textY = 0; /* the case-3/13 row index rides on textY (s4, dead here): see case 13 */
        /* Keep the row home in the nonempty block, beside its first use. */
        if ((s32) D_8007BEF8 > 0) {
            rowY = rowBase; do {
                D_o058_5E98 = -D_o058_5E98;
                D_o058_5E9C = -D_o058_5E9C;

                nodes[0].texture = D_800D31C8_o058Reloc[0x51 + D_o058_5EE0[textY]->character];
                nodes[0].alternate = NULL;
                nodes[0].x = D_o058_5E98 + D_o058_5E9C + 0x4E;
                nodes[0].y = rowY - 4;
                nodes[0].packedOffset = 0;
                nodes[1].texture = 0;
                func_8002F618(&D_800D3140, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5E9C + 0x28, rowY, D_o058_5C98[D_o058_5F10[textY]], 0);
                if ((D_8007C1A0_o058Reloc == 1) && (state->entries == D_o058_5EE0[textY])) {
                    fontColour((s32) D_o058_5F38.red, (s32) D_o058_5F38.green, (s32) D_o058_5F38.blue, 0xFF, 0xFF);
                } else {
                    fontColour(0, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5E9C + 0x71, rowY, D_8007C0B8->text[D_o058_5EE0[textY]->character + 0x1A], 0);
                sprintf(&text[0], D_o058_5D60, D_o058_5EE0[textY]->rank);
                fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
                func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5E9C + 0xD2, rowY, &text[0], 0);
                if (D_o058_5EE0[textY]->rank == 1) {
                    func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5E9C + 0x104, rowY, D_o058_5D64, 0);
                } else {
                    func_8004B0F8(&D_800D3140, D_o058_5E98 + D_o058_5E9C + 0x104, rowY, D_o058_5D68, 0);
                }
                textY += 1; i += 1;
                rowY += rowHeight;
            } while (i < (s32) D_8007BEF8);
        }
        D_o058_5E98 = savedX;
        D_o058_5E9C = savedPosition;

        D_o058_5E98 -= arg0 * 0xF;
        if (D_o058_5E98 < 0) {
            D_o058_5E98 = 0;
            if (D_o058_5EB0 == 0) {
                if (D_800D31B8_o058Reloc & 0x9000) {
                    if (state->mode == 0) {
                        D_o058_5EB0 = 4;
                    } else {
                        D_o058_5EB0 = 0xC;
                    }
                    amSndPlay(0xCU, NULL);
                    return;
                }
                if (D_800D31B8_o058Reloc & 0x4000) {
                    D_o058_5EB0 = 2;
                    *D_o058_5E50 = -1;
                    amSndPlay(0xDU, NULL);
                    return;
                }
            } else {
                D_o058_5E9C += arg0 * 0xF;
                if (D_o058_5E9C >= 0x141) {
                    if (D_o058_5EB0 == 4) {
                        animseqStopPath(D_o058_5E84.bytes[3]);
                        D_o058_5E84.word = 1;
                        path = func_800508B4(D_o058_5E84.bytes[3]);
                        if (path != NULL) {
                            func_8005055C(D_o058_5E84.bytes[3]);
                            animseqStartPath(D_o058_5E84.bytes[3]);
                            path->flags |= 2;
                        }
                        D_o058_5E80 = 2;
                        D_o058_5E88 = 0x50;
                        return;
                    }
                    D_o058_5E94 = D_o058_5EB0;
                    amSndPlay(0x1FAU, NULL);
                    D_o058_5EB0 = 0;
                    D_o058_5E9C = 0;
                    D_o058_5EA0 = 0x140;
                    return;
                }
            }
        }
        break;
    case 12:
        /* The visible index is reset through the last title-colour argument
         * (the value-producing spelling wv-i found: the reset then lands in
         * the call's delay slot), as case 13 already does.  The title loop
         * below walks a pointer, not a subscript, so the reset can live
         * here: with `D_o058_5C98[i]` uopt folds the known-zero index into
         * the cursor's constant init only when no call separates the reset
         * from the loop (measured on mini TUs and here, wv-x).  See
         * docs/whale-split-tokens.md and docs/whale-pointer-walk.md. */
        fontColour(0xFF, 0x80, 0, 0xFF, (0xFF - (i = 0)));
        func_8004B0F8(&D_800D3140, D_o058_5E9C + D_o058_5EA0 + 0xA0, 0x1E, D_8007C0B8->text[0x6E], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);


        /* Roles, not colours: `opponent` is s0 and `columnX` s1 in cases
         * 8-10, and the target keeps column X in s0 and the inner index in
         * s1 here and in case 13.  So `opponent` carries column X and
         * `columnX` the inner index; neither web changes colour (wv-r). */
        opponent = D_o058_5C80[D_8007BEF8 - 1] + D_o058_5E9C + D_o058_5EA0;
        columnStep = D_o058_5C8C[D_8007BEF8 - 1];
        if ((s32) D_8007BEF8 > 0) {
            /* The title array is walked by `cursor`, initialised inside the
             * guard, so no index web touches blocks 183/184/190: W (the
             * `&D_o058_5EA0` case-12 piece) then accepts the call block 185
             * with margin +2, rejects 191, keeps 202 at margin 0 and colours
             * t0, while the `&D_o058_5E9C` piece accepts 191 at margin 0 and
             * rejects 202, exactly the 48 body's growth (wv-x, from wv-t's
             * pointer cell).  Any dead carrier at 183 or 184 is one web too
             * many for both tests, and a known-zero `i` does not survive the
             * three calls into the cursor init.  The init is spelled from
             * `D_o058_5C80`, whose lineage already spans the window, because
             * `&D_o058_5C98` is used in cases 1/2/3/10 and its remainder
             * would count at 184 (+4: the 5E9C piece steals 202).  The
             * `cursor` cell is the frame slot the scalar used to hold.
             * Residue: the load is `lw a3,0(s1)` first in the block where the
             * target has it in the call's delay slot, because a load through
             * a pointer variable gets no `.noalias` fact (L95; the alias
             * profile reads its base as isvar/may-alias) and as1 cannot sink
             * it past `sw t8,16(sp)`; an indexed named array gets the fact
             * but needs the index known zero here.  Six words. */
            cursor = (char **) (D_o058_5C80 + 12);
            do {
                func_8004B0F8(&D_800D3140, opponent, 0x37, *cursor, 4);
                i += 1; cursor += 1;
                opponent += columnStep;
                if (columnStep != 0); /* +10 to the stride web: decided before the count (see case 13). */
            } while (i < (s32) D_8007BEF8);
        }
        savedPosition = D_o058_5E9C;
        savedOffset = D_o058_5EA0;
        i = 0;
        rowY = rowBase;
        if ((s32) D_8007BEF8 > 0) {
            do {
                D_o058_5E9C = -D_o058_5E9C;
                D_o058_5EA0 = -D_o058_5EA0;

                nodes[0].texture = D_800D31C8_o058Reloc[0x51 + D_o058_5EE0[(u32)i]->character];
                nodes[0].alternate = NULL;
                nodes[0].x = D_o058_5E9C + D_o058_5EA0 + 0x28;
                nodes[0].y = rowY + 0x12;
                nodes[0].packedOffset = 0;
                nodes[1].texture = 0;
                func_8002F618(&D_800D3140, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
                columnX = 0;
                fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);

                opponent = D_o058_5C80[D_8007BEF8 - 1] + D_o058_5E9C + D_o058_5EA0;
                if ((s32) D_8007BEF8 > 0) {
                    do {
                        sprintf(&text[0], D_o058_5D6C, D_o058_5EE0[(u32)i]->counters[columnX]);
                        func_8004B0F8(&D_800D3140, opponent, rowY + 0x16, &text[0], 4);
                        columnX += 1;
                        opponent += columnStep;
                    } while (columnX < (s32) D_8007BEF8);
                }
                i += 1;
                rowY += rowHeight;
            } while (i < (s32) D_8007BEF8);
        }
        D_o058_5E9C = savedPosition;
        D_o058_5EA0 = savedOffset;

        D_o058_5EA0 -= arg0 * 0xF;
        if (D_o058_5EA0 < 0) {
            /* Interference token, zero width: a discarded read of a local
             * that is dead here keeps the `&D_o058_5EA0` fragment from
             * adjoining this block, so the zero store below goes through
             * `at` as the target's does instead of sharing the fragment's
             * register (-4).  `columnCount` and `erase` both work; a live
             * local or a fresh declaration does not. */
            if (columnCount != 0);
            D_o058_5EA0 = 0;
            if (D_o058_5EB0 == 0) {
                if (D_800D31B8_o058Reloc & 0x9000) {
                    if (state->mode == 3) {
                        D_o058_5EB0 = 4;
                    } else {
                        D_o058_5EB0 = 7;
                    }
                    amSndPlay(0xCU, NULL);
                    return;
                }
                if (D_800D31B8_o058Reloc & 0x4000) {
                    D_o058_5EB0 = 3;
                    *D_o058_5E50 = -1;
                    amSndPlay(0xDU, NULL);
                    return;
                }
            } else {
                D_o058_5E9C += arg0 * 0xF;
                if (D_o058_5E9C >= 0x141) {
                    if (D_o058_5EB0 == 4) {
                        animseqStopPath(D_o058_5E84.bytes[3]);
                        D_o058_5E84.word = 1;
                        path = func_800508B4(D_o058_5E84.bytes[3]);
                        if (path != NULL) {
                            func_8005055C(D_o058_5E84.bytes[3]);
                            animseqStartPath(D_o058_5E84.bytes[3]);
                            path->flags |= 2;
                        }
                        D_o058_5E80 = 2;
                        D_o058_5E88 = 0x50;
                        return;
                    }
                    D_o058_5E94 = D_o058_5EB0;
                    amSndPlay(0x1FAU, NULL);
                    D_o058_5EB0 = 0;
                    D_o058_5E98 = 0x140;
                    D_o058_5E9C = 0;
                    D_o058_5EA0 = 0x140;
                    return;
                }
            }
        }
        break;
    case 13:
        /* Initialize the visible index through the last title-colour argument. */
        fontColour(0xFF, 0x80, 0, 0xFF, (0xFF - (i = 0)));
        func_8004B0F8(&D_800D3140, D_o058_5E9C + D_o058_5EA0 + 0xA0, 0x1E, D_8007C0B8->text[0x6F], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

        if (D_8007BEF8 == 4) {
            columnCount = 4;
        } else {
            columnCount = D_8007BEF8 + 1;
        }

        opponent = D_o058_5C80[D_8007BEF8 - 1] + D_o058_5E9C + D_o058_5EA0;
        columnStep = D_o058_5C8C[columnCount - 1];
        if (columnCount > 0) {
            /* Loop 1 is indexed by the same variable that indexes case 3
             * and the row loop below, with the reset folded here in the
             * preheader (wv-u): that keeps the `&D_o058_5E9C` a2 piece at 28
             * interferers at its 254 step, so the head lui/lw pair and the
             * transition keep their shared register.  `opponent` is the
             * coordinate (only its 99.4 beats the cursor's 15.5, so the
             * cursor lands s1 and the coordinate s0).  The variable is
             * `textY` because the reset web must not be a0/a1/a2: textY is
             * dead in cases 3 and 13, its web is decided early into s4 by
             * case 1, and s4 is harmless at 136/235/241 (wv-v; the forced
             * witnesses a3/t0/s4 all measured the same 11).  `cursor` stays
             * declared for its frame cell.  The subscript keeps the 0x51 so
             * the cursor register holds the bare symbol and the load spends
             * the 0x144 in its displacement, as the target does. */
            textY = 0;
            do {
                nodes[0].texture = D_800D31C8_o058Reloc[0x51 + textY];
                nodes[0].alternate = NULL;
                nodes[0].x = opponent;
                /* Stride in `columnStep` (shared with case 12, s5) and count
                 * in `columnCount` (s6), as the target colours them.  The
                 * count alone measures 124/6 = 20.7, below `i`'s 21.9, so
                 * this +10 lifts it to 22.3; the stride probe in case 12
                 * keeps the stride above it (23.2).  Order: stride, count,
                 * i.  Without the pair the exchange costs 100 rows. */
                if (columnCount != 0);
                nodes[0].y = 0x37;
                nodes[0].packedOffset = 0;
                nodes[1].texture = 0;
                func_8002F618(&D_800D3140, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
                i += 1;
                textY += 1;
                opponent += columnStep;
            /* `!=`, not `<`: with the cursor carrying the subscript, uopt
             * no longer normalises `i < columnCount` into the target's
             * `bne`, and the `<` spelling costs an extra slt.  The converse
             * of what the indexed form wanted; see the handoff. */
            } while (i != columnCount);
        }
        savedPosition = D_o058_5E9C;
        savedOffset = D_o058_5EA0;
        textY = 0; i = 0;
        rowY = rowBase;
        if ((s32) D_8007BEF8 > 0) {
            do {
                D_o058_5E9C = -D_o058_5E9C;
                D_o058_5EA0 = -D_o058_5EA0;

                nodes[0].texture = D_800D31C8_o058Reloc[0x51 + D_o058_5EE0[textY]->character];
                nodes[0].alternate = NULL;
                nodes[0].x = D_o058_5E9C + D_o058_5EA0 + 0x28;
                nodes[0].y = rowY + 0x12;
                nodes[0].packedOffset = 0;
                nodes[1].texture = 0;
                func_8002F618(&D_800D3140, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
                columnX = 0;
                fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);

                opponent = D_o058_5C80[D_8007BEF8 - 1] + D_o058_5E9C + D_o058_5EA0;
                if (columnCount > 0) {
                    do {
                        sprintf(&text[0], D_o058_5D70, D_o058_5EE0[textY]->flags[columnX]);
                        func_8004B0F8(&D_800D3140, opponent + 8, rowY + 0x16, &text[0], 4);
                        columnX += 1;
                        opponent += columnStep;
                    } while (columnX != columnCount);
                }
                textY += 1; i += 1;
                rowY += rowHeight;
            } while (i < (s32) D_8007BEF8);
        }
        D_o058_5E9C = savedPosition;
        D_o058_5EA0 = savedOffset;

        D_o058_5EA0 -= arg0 * 0xF;
        if (D_o058_5EA0 < 0) {
            D_o058_5EA0 = 0;
            if (D_o058_5EB0 == 0) {
                if (D_800D31B8_o058Reloc & 0x9000) {
                    if (state->mode == 3) {
                        D_o058_5EB0 = 4;
                    } else {
                        D_o058_5EB0 = 7;
                    }
                    amSndPlay(0xCU, NULL);
                    return;
                }
                if (D_800D31B8_o058Reloc & 0x4000) {
                    D_o058_5EB0 = 0xC;
                    *D_o058_5E50 = -1;
                    amSndPlay(0xDU, NULL);
                    return;
                }
            } else {
                D_o058_5E9C += arg0 * 0xF;
                if (D_o058_5E9C >= 0x141) {
                    if (D_o058_5EB0 == 4) {
                        animseqStopPath(D_o058_5E84.bytes[3]);
                        D_o058_5E84.word = 1;
                        path = func_800508B4(D_o058_5E84.bytes[3]);
                        if (path != NULL) {
                            func_8005055C(D_o058_5E84.bytes[3]);
                            animseqStartPath(D_o058_5E84.bytes[3]);
                            path->flags |= 2;
                        }
                        D_o058_5E80 = 2;
                        D_o058_5E88 = 0x50;
                        return;
                    }
                    D_o058_5E94 = D_o058_5EB0;
                    amSndPlay(0x1FAU, NULL);
                    D_o058_5EB0 = 0;
                    D_o058_5E9C = 0;
                    D_o058_5EA0 = 0x140;
                    return;
                }
            }
        }
        break;
    case 7:
    case 11:
        i = 0;
        if (D_o058_5EB0 == 0) {
            if ((D_800D31BE_o058Reloc >= 0x11) && (D_o058_5F28 > 0)) {
                D_o058_5F28 -= 1;
                amSndPlay(0xFU, NULL);
            } else if ((D_800D31BE_o058Reloc < -0x10) && (D_o058_5F28 < 3)) {
                D_o058_5F28 += 1;
                amSndPlay(0xFU, NULL);
            }
        }
        x = D_o058_5E9C + D_o058_5EA0;
        fontColour(0xFF, 0x80, 0, 0xFF, 0xFF);
        if (D_o058_5E94 == 7) {
            if (D_8007C1A0_o058Reloc == 1) {
                func_8004B0F8(&D_800D3140, x + 0xA0, 0x1E, D_8007C0B8->text[0xB5], 4);
            } else {
                func_8004B0F8(&D_800D3140, x + 0xA0, 0x1E, D_8007C0B8->text[0x2A], 4);
            }
        } else {
            func_8004B0F8(&D_800D3140, x + 0xA0, 0x1E, D_8007C0B8->text[0x2B], 4);
        }
        /* Indexed Y and text let strength reduction supply both carriers.
         * Preserve the early visible index across the menu input calls. */
        rowBase = 0;

        do {
            if (i == D_o058_5F28) {
                fontColour((s32) D_o058_5F38.red, (s32) D_o058_5F38.green, (s32) D_o058_5F38.blue, 0xFF, 0xFF);
            } else {
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            func_8004B0F8(&D_800D3140, 0xA0 - x, 0x50 + rowBase * 0x1E, D_o058_5E68[rowBase], 4);
            i += 1;
            rowBase++;

            x = -x;
        } while (i < 4);
        D_o058_5EA0 -= arg0 * 0xF;

        if (D_o058_5EA0 < 0) {
            if (D_o058_5EB0 == 0) {
                if (D_800D31B8_o058Reloc & 0x9000) {
                    D_o058_5EB0 = 4;
                    amSndPlay(0xCU, NULL);

                } else if (D_800D31B8_o058Reloc & 0x4000) {
                    if (D_o058_5E94 == 7) {
                        D_o058_5EB0 = 0xC;
                    } else {
                        D_o058_5EB0 = 0xA;
                    }
                    amSndPlay(0xDU, NULL);

                }
            } else {
                D_o058_5E9C += arg0 * 0xF;
                if (D_o058_5E9C >= 0x141) {
                    if (D_o058_5EB0 == 4) {
                        switch (D_o058_5F28) {
                        case 0:
                            mainChangeCameras(D_8007C1A0_o058Reloc);
                            joyCreateMap(D_o058_5F48);
                            if (((D_8007C1A0_o058Reloc == 2) || (D_8007C1A0_o058Reloc == 3)) && (D_8007BF74_o058Reloc != 0)) {
                                D_8007BEFC_o058Reloc = 4 - D_8007C1A0_o058Reloc;
                                D_8007BEF8 = 4;
                            } else if ((D_8007C1A0_o058Reloc == 1) && (state->mode == 5)) {
                                D_8007BEFC_o058Reloc = 3;
                                D_8007BEF8 = 4;
                            } else {
                                D_8007BEFC_o058Reloc = 0;
                                D_8007BEF8 = (u8) D_8007C1A0_o058Reloc;
                            }
                            for (i = 0; i < 6; i++) {
                                state->entries[i].value = 0;
                            }
                            if (D_o058_5F2C != 0) {
                                mainChangeLevel((s32) D_800D304E_o058Reloc, 0, 0, 5, 1, 0);
                                D_o058_5F2C = 0;
                            }

                            break;
                        case 1:
                            if (D_o058_5F2C != 0) {
                                mainChangeLevel(0x1D, 0, 0, 0xB, 1, 0);
                                D_o058_5F2C = 0;

                            }
                            break;
                        case 2:
                            if (D_o058_5F2C != 0) {
                                if (state->mode == 5) {
                                    mainChangeLevel(0xC, 0, 0, 0x12, 1, 0);
                                } else {
                                    mainChangeLevel(0xC, 0, 0, 0x11, 1, 0);
                                }
                                D_o058_5F2C = 0;

                            }
                            break;
                        case 3:
                            if (D_o058_5F2C != 0) {
                                mainChangeLevel(0xC, 0, 0, 0xC, 1, 0);
                                D_o058_5F2C = 0;

                            }
                            break;
                        }
                    } else {
                        D_o058_5EAC = 0;
                        D_o058_5E94 = D_o058_5EB0;
                        amSndPlay(0x1FAU, NULL);
                        D_o058_5EB0 = 0;
                        D_o058_5E98 = 0x140;
                        D_o058_5E9C = 0;
                        D_o058_5EA0 = 0x140;

                        D_o058_5EA8 = 0;
                    }
                }
            }
        }
        if (D_o058_5EA0 < 0) {
            D_o058_5EA0 = 0;
            return;
        }
        break;
    case 5:
        fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
        func_8004B0F8(&D_800D3140, D_o058_5EA8 + 0xA0, 0x50, D_8007C0B8->text[0x2C], 4);
        D_o058_5EA8 = -D_o058_5EA8;
        fontColour(0xFF, 0, 0, 0xFF, 0xFF);
        func_8004B0F8(&D_800D3140, D_o058_5EA8 + 0xA0, 0x8C, D_8007C0B8->text[0x2D], 4);
        D_o058_5EA8 = -D_o058_5EA8;

        D_o058_5EA8 = D_o058_5EA8 - (arg0 * 0xF);
        if (D_o058_5EA8 < 0) {
            if (D_800D31B8_o058Reloc & 0x9000) {
                func_8003A754();
                if ((D_8007BF44_o058Reloc > 0) && (state->mode == 0)) {
                    D_8007BF48_o058Reloc = -1;
                    if (D_o058_5F2C != 0) {
                        mainChangeLevel(0x12, 0, 0, 0xF, 1, 0);
                        D_o058_5F2C = 0;
                    }
                    mainSetAnimGroup(1);
                } else if (D_o058_5F2C != 0) {
                    mainChangeLevel(0xC, 0, 0, 0xC, 1, 0);
                    D_o058_5F2C = 0;
                }
            }
            D_o058_5EA8 = 0;
            return;
        }
        break;
    case 6:
        fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
        func_8004B0F8(&D_800D3140, D_o058_5EA8 + 0xA0, 0x50, D_8007C0B8->text[0x2C], 4);
        D_o058_5EA8 = -D_o058_5EA8;
        if (state->active == 0) {
            fontColour(0, 0xFF, 0, 0xFF, 0xFF);
            func_8004B0F8(&D_800D3140, D_o058_5EA8 + 0xA0, 0x8C, D_8007C0B8->text[0x6C], 4);
            D_o058_5EA8 = -D_o058_5EA8;
        } else {
            i = 0;
            if (D_8007BF1C_o058Reloc & 0x100) {
                sprintf(&text[0], D_8007C0B8->text[0xB3]);
                countdownX = 0x6C;
            } else {

                if (state->countdown == 1) {
                    sprintf(&text[0], D_8007C0B8->text[0x2E], state->countdown);
                    countdownX = 0x94;
                } else {
                    sprintf(&text[0], D_8007C0B8->text[0x2F], state->countdown);
                    countdownX = 0x80;
                }
            }
            fontColour(0, 0xFF, 0, 0xFF, 0xFF);
            func_8004B0F8(&D_800D3140, D_o058_5EA8 + 0xA0, 0x8C, &text[0], 4);
            D_o058_5EA8 = -D_o058_5EA8;

            if ((s32) state->countdown > 0) {
                /* Indexed X: strength reduction generates the walker (L160). */
                /* Adjacent index def; see the note in case 7/11. */
                i = 0;
                do {
                    nodes[i].texture = (RcpTextureInfo *) D_800D31C8_o058Reloc[0x51 + state->entries[0].character];
                    nodes[i].alternate = 0;
                    nodes[i].x = countdownX + D_o058_5EA8 + i * 0x28;
                    nodes[i].y = 0xAA;
                    nodes[i].packedOffset = 0;
                    /* No declared walker: it kept portraitX out of s0 here. */
                    i += 1;
                } while (i < (s32) state->countdown);
            }
            nodes[i].texture = 0;
            func_8002F618(&D_800D3140, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
        }
        D_o058_5EA8 = D_o058_5EA8 - (arg0 * 0xF);
        if (D_o058_5EA8 < 0) {
            if (D_800D31B8_o058Reloc & 0x9000) {
                if ((D_8007BF44_o058Reloc > 0) && (state->mode == 0)) {
                    D_8007BEF8 = 6;
                    D_8007BEFC_o058Reloc = 5;
                    joyCreateMap(D_o058_5F48);
                    D_8007BF48_o058Reloc = (s32) D_8007C0C0_o058Reloc[state->player][state->active];
                    D_8007BF4C_o058Reloc = (s32) state->entries[0].character;
                    D_8007BF50_o058Reloc = 5;
                    D_8007BF54_o058Reloc = 0;
                    mainChangeLevel(0x12, 0, 0, 0xF, 1, 0);
                    mainSetAnimGroup(1);
                } else {
                    D_o058_5EC0 = 1;
                }
            }
            D_o058_5EA8 = 0;
            return;
        }
        break;
    case 8:
        x = D_o058_5E98 + D_o058_5EA8;

        fontColour(0xFF, 0x80, 0, 0xFF, (0xFF - (i = 0)));
        func_8004B0F8(&D_800D3140, x + 0xA0, 0x1E, func_8003A5A0((s32) D_800D304E_o058Reloc), 4);
        x = -x;
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        func_8004B0F8(&D_800D3140, x + 0xA0, 0x39, D_8007C0B8->text[0x27], 4);

        if (D_o058_5CD8 != 0) {
            if ((D_o058_5CD8 >= 2) && (x == 0)) {
                amSndPlay(0x27CU, NULL);
                D_o058_5CD8 = 1;
            }
            if (D_o058_5CD8 == 1) {
                func_8002F618(&D_800D3140, D_o058_5BA0, x + 0x30, 0x24, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
            }
        }
        /* The visible row index starts with the title font call.
         * This separate induction starts beside the loop so uopt can fold
         * both the lap-array address and the row-coordinate stride.
         * Both indices visit zero through two once; `opponent` is otherwise
         * dead here.  Splitting only one induction leaves the other blocked.
         * This split increases `i`'s occurrence-block count; the fourth
         * grid probe below restores its saved-register priority.
         * The visible increment goes first to match the loop-tail schedule. */
        opponent = 0;
        do {
            x = -x;
            textY = 0x5B + D_o058_5EAC + opponent * 0x1B; /* This order puts the global first in the add. */
            if (i == D_o058_5E8C) {
                fontColour((s32) D_o058_5F38.red, (s32) D_o058_5F38.green, (s32) D_o058_5F38.blue, 0xFF, 0xFF);
            } else {
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            sprintf(&text[0], D_8007C0B8->text[0x30], i + 1);
            func_8004B0F8(&D_800D3140, x + 0x82, textY, &text[0], 1);
            overlay56SplitTime(state->entries[0].lapTimes[opponent], &minutes, &seconds, &centiseconds);
            if (i != D_o058_5E8C) {
                fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
            }
            sprintf(&text[0], D_o058_5D74, minutes);
            func_8004B0F8(&D_800D3140, x + 0xB4, textY, &text[0], 1);
            func_8004B0F8(&D_800D3140, x + 0xB7, textY, D_o058_5D78, 0);
            sprintf(&text[0], D_o058_5D7C, seconds);
            func_8004B0F8(&D_800D3140, x + 0xBE, textY, &text[0], 0);
            func_8004B0F8(&D_800D3140, x + 0xD5, textY, D_o058_5D84, 0);
            sprintf(&text[0], D_o058_5D88, centiseconds);
            func_8004B0F8(&D_800D3140, x + 0xDC, textY, &text[0], 0);
            i += 1;
            opponent += 1;
        } while (i != 3);
        x = -x;
        textY = textY + 0x2C;
        if (D_o058_5E90 != -1) {
            fontColour((s32) D_o058_5F38.red, (s32) D_o058_5F38.green, (s32) D_o058_5F38.blue, 0xFF, 0xFF);
        } else {
            fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        }
        func_8004B0F8(&D_800D3140, x + 0x82, textY, D_8007C0B8->text[0x31], 1);
        overlay56SplitTime(state->entries[0].value, &minutes, &seconds, &centiseconds);
        if (D_o058_5E90 == -1) {
            fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
        }
        sprintf(&text[0], D_o058_5D90, minutes);
        func_8004B0F8(&D_800D3140, x + 0xB4, textY, &text[0], 1);
        func_8004B0F8(&D_800D3140, x + 0xB7, textY, D_o058_5D94, 0);
        sprintf(&text[0], D_o058_5D98, seconds);
        func_8004B0F8(&D_800D3140, x + 0xBE, textY, &text[0], 0);
        func_8004B0F8(&D_800D3140, x + 0xD5, textY, D_o058_5DA0, 0);
        sprintf(&text[0], D_o058_5DA4, centiseconds);
        func_8004B0F8(&D_800D3140, x + 0xDC, textY, &text[0], 0);
        D_o058_5E98 -= arg0 * 0xF;
        if (D_o058_5E98 < 0) {
            D_o058_5E98 = 0;
            if ((D_o058_5EB0 == 0) && (D_800D31B8_o058Reloc & 0x9000)) {
                if ((D_o058_5E8C != -1) || (D_o058_5E90 != -1)) {
                    D_o058_5EB0 = 9;
                } else {
                    D_o058_5EB0 = 0xA;
                }
                amSndPlay(0xCU, NULL);
            }
            if (D_o058_5EB0 != 0) {
                D_o058_5EA8 += arg0 * 0xF;
                if (D_o058_5EA8 >= 0x141) {
                    D_o058_5EA8 = 0;
                    D_o058_5E98 = 0x140;
                    D_o058_5E94 = D_o058_5EB0;
                    D_o058_5EB0 = 0;
                    amSndPlay(0x1FAU, NULL);
                    return;
                }
            }
        }
        break;
    case 9:
        fontColour(0xFF, 0x80, 0, 0xFF, 0xFF);
        x = D_o058_5E98 + D_o058_5EA8;
        func_8004B0F8(&D_800D3140, x + 0xA0, 0x1E, D_8007C0B8->text[0x32], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        D_o058_5E98 -= arg0 * 0xF;
        if (D_o058_5E98 < 0) {
            D_o058_5E98 = 0;
            if (D_o058_5EB0 == 0) {
                erase = 0;
                if (D_800D31B8_o058Reloc & 0x4000) {
                    erase = 1;
                } else if (D_800D31B8_o058Reloc & 0x9000) {
                    if ((D_o058_5E78 == 2) && (D_o058_5E7C == 9)) {
                        erase = 1;
                    } else if ((D_8007C1B4_o058Reloc == 3) || (D_o058_5E78 == 3)) {
                        if (D_o058_5E78 == 3) {
                            amSndPlay(0xCU, NULL);
                            D_o058_5EB0 = 0xA;
                            saves = func_800291C4();
                            /* Slot address as overlay60Prefix spells it: the index
                             * through a dead local AND byte arithmetic.  Either alone is
                             * byte-identical to `&saves[f()]` (wv-v cycle 10); together
                             * they put `saves` first in the addu, as the target has it
                             * (wv-w cycle 1: the operand order is the tree's evaluation
                             * order, not a canonical sum). */
                            portraitIndex = levelGetBlurEffect(D_800D304E_o058Reloc); slot = (SavesSlot *) ((u8 *) saves + portraitIndex * 32);
                            if (D_o058_5E8C != -1) {
                                slot->records[3].name[0] = func_8003A6B0(D_800D31C4_o058Reloc[0]);
                                slot->records[3].name[1] = func_8003A6B0(D_800D31C4_o058Reloc[1]);
                                slot->records[3].name[2] = func_8003A6B0(D_800D31C4_o058Reloc[2]);
                            }
                            if (D_o058_5E90 != -1) {
                                slot->records[D_o058_5E90].name[0] = func_8003A6B0(D_800D31C4_o058Reloc[0]);
                                slot->records[D_o058_5E90].name[1] = func_8003A6B0(D_800D31C4_o058Reloc[1]);
                                slot->records[D_o058_5E90].name[2] = func_8003A6B0(D_800D31C4_o058Reloc[2]);
                            }
                            func_80029120(levelGetBlurEffect(D_800D304E_o058Reloc));
                            if (D_o058_5F30 != 0) {
                                func_8002917C();
                            }
                        } else {
                            amSndPlay(0xEU, NULL);
                        }
                    } else {
                        D_800D31C4_o058Reloc[D_8007C1B4_o058Reloc] = (u8) D_o058_5C5C[D_o058_5E78][D_o058_5E7C];
                        if (D_8007C1B4_o058Reloc == 2) {
                            D_o058_5E78 = 3;
                        }
                        D_8007C1B4_o058Reloc += 1;
                        amSndPlay(0xCU, NULL);
                    }
                }
                if (erase != 0) {
                    if (D_8007C1B4_o058Reloc == 0) {
                        amSndPlay(0xEU, NULL);
                    } else {
                        D_800D31C4_o058Reloc[D_8007C1B4_o058Reloc] = 0x20;
                        D_8007C1B4_o058Reloc -= 1;
                        columnX = 0; do { /* Keep the preheader scheduling tie. */

                            /* The outer row uses columnX and the inner
                             * character index uses i, as in the grid below.
                             * Both counters are reinitialized before each
                             * scan and retain the original bounds. */
                            i = 0;
                            do {
                                if ((u8) D_o058_5C5C[columnX][i] == D_800D31C4_o058Reloc[D_8007C1B4_o058Reloc]) {
                                    D_o058_5E78 = columnX;
                                    D_o058_5E7C = i;
                                }
                                i += 1;
                            } while (i != 0xA);
                            columnX += 1;
                        } while (columnX != 3);
                        amSndPlay(0xDU, NULL);
                    }
                }

                if (D_800D31BE_o058Reloc >= 0x11) {
                    if (D_o058_5E78 == 0) {
                        amSndPlay(0xEU, NULL);
                    } else {
                        amSndPlay(0xFU, NULL);
                        D_o058_5E78 -= 1;
                    }

                }
                if (D_800D31BE_o058Reloc < -0x10) {
                    if ((D_o058_5E78 == 3) || (D_o058_5E78 == 3)) {
                        amSndPlay(0xEU, NULL);
                    } else {
                        amSndPlay(0xFU, NULL);
                        D_o058_5E78 += 1;
                    }
                }

                if (D_800D31BC_o058Reloc < -0x10) {
                    if ((D_o058_5E7C == 0) || (D_o058_5E78 == 3)) {
                        amSndPlay(0xEU, NULL);
                    } else {
                        amSndPlay(0xFU, NULL);
                        D_o058_5E7C -= 1;
                    }

                }
                if (D_800D31BC_o058Reloc >= 0x11) {
                    if ((D_o058_5E7C == 9) || (D_o058_5E78 == 3)) {
                        amSndPlay(0xEU, NULL);
                    } else {
                        amSndPlay(0xFU, NULL);
                        D_o058_5E7C += 1;
                    }
                }
            }
            if (D_o058_5EB0 != 0) {
                D_o058_5EA8 += arg0 * 0xF;
                if (D_o058_5EA8 >= 0x141) {
                    D_o058_5EA8 = 0;
                    D_o058_5E98 = 0x140;
                    D_o058_5E94 = D_o058_5EB0;
                    D_o058_5EB0 = 0;
                    amSndPlay(0x1FAU, NULL);
                }
            }
        }
        textY = 0x78;
        columnX = 0; do { /* One physical line: as1 then emits the hoisted addresses first (L59). */
            /* Keep the increment opaque to the counted-loop proof.  The
             * discarded product below is defined and zero on every pass;
             * the index still starts at zero and advances by exactly one.
             * uopt therefore retains the signed comparison without making
             * its bound a saved-register web, while ugen folds the product.
             * Unlike the former opaque initial value, this does not need a
             * temporary stack slot or an extra executable instruction.
             * The record and character index choices in case 10 complete
             * the paired shape correction; either end alone misprices the
             * intervening displacement.  See the current handoff for the
             * controlled comparisons and the remaining local residual. */
            i = 0;
            x = -x;
            opponent = 0x34 + x;
            do {
                /* The erase scan and this grid share their row induction.
                 * columnX selects the row, i selects its character, and
                 * opponent carries the drawing coordinate.  Each role has
                 * its own initialization and visits the original bounds.
                 * The matching row carriers retain the shared table cursor.
                 * Together with the count/stride carrier exchange in case
                 * 13, this keeps the visible index in its target colour.
                 * The former four discarded index probes are unnecessary
                 * on this source shape and have been removed.  The opaque
                 * unit increment below still prevents the counted rewrite.
                 */
                sprintf(&text[0], D_o058_5DAC, (u8) D_o058_5C5C[columnX][i]);
                if ((i == D_o058_5E7C) && (columnX == D_o058_5E78)) {
                    fontColour((s32) D_o058_5F38.red, (s32) D_o058_5F38.green, (s32) D_o058_5F38.blue, 0xFF, 0xFF);
                } else {
                    fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                }
                func_8004B0F8(&D_800D3140, opponent, textY, &text[0], 4);
                i += (i * 0) + 1;
                opponent += 0x18;
            } while (i < 0xA);
            columnX += 1;
            textY += 0x1B;
        } while (columnX < 3);
        x = -x;
        if (D_o058_5E78 == 3) {
            fontColour((s32) D_o058_5F38.red, (s32) D_o058_5F38.green, (s32) D_o058_5F38.blue, 0xFF, 0xFF);
        } else {
            fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        }
        func_8004B0F8(&D_800D3140, x + 0xA0, textY, D_8007C0B8->text[0x33], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

        i = 0;
        columnX = x + 0x78;
        if (D_8007C1B4_o058Reloc > 0) {
            do {
                sprintf(&text[0], D_o058_5DB0, D_800D31C4_o058Reloc[i]);
                func_8004B0F8(&D_800D3140, columnX, 0x50, &text[0], 4);
                i += 1;
                columnX += 0x1E;
            } while (i < D_8007C1B4_o058Reloc);
        }
        if ((D_8007C1B4_o058Reloc != 3) && (D_o058_5E78 != 3)) {
            sprintf(&text[0], D_o058_5DB4, (u8) D_o058_5C5C[D_o058_5E78][D_o058_5E7C]);
            fontColour((s32) D_o058_5F38.red, (s32) D_o058_5F38.green, (s32) D_o058_5F38.blue, 0xFF, 0xFF);
            func_8004B0F8(&D_800D3140, columnX, 0x50, &text[0], 4);
            return;
        }
        break;
    case 10:
        x = D_o058_5E98 + D_o058_5EA8;
        fontColour(0xFF, 0x80, 0, 0xFF, (0xFF - (i = 0)));

        func_8004B0F8(&D_800D3140, x + 0xA0, 0x1E, func_8003A5A0((s32) D_800D304E_o058Reloc), 4);
        x = -x;
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        func_8004B0F8(&D_800D3140, x + 0xA0, 0x39, D_8007C0B8->text[0x34], 4);
        fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
        saves = func_800291C4();

        erase = levelGetBlurEffect(D_800D304E_o058Reloc); slot = (SavesSlot *) ((u8 *) saves + erase * 32); /* see case 9 (wv-w) */

        if (D_o058_5CD8 != 0) {
            if ((D_o058_5CD8 >= 2) && (x == 0)) {
                amSndPlay(0x27CU, NULL);
                D_o058_5CD8 = 1;
            }
            if (D_o058_5CD8 == 1) {
                func_8002F618(&D_800D3140, D_o058_5BA0, x + 0x30, 0x24, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
            }
        }
        textY = 0x5B;
        /* Separate record induction. */
        opponent = 0;
        do {
            x = -x;
            overlay56SplitTime(slot->records[opponent].value, &minutes, &seconds, &centiseconds);
            if (slot->records[opponent].value == 0) {
                sprintf(&text[0], D_o058_5DB8);
                portraitIndex = 0x4A;
            } else {
                letter0 = func_8003A700(slot->records[opponent].name[0]) & 0xFF;
                letter1 = func_8003A700(slot->records[opponent].name[1]) & 0xFF;
                sprintf(&text[0], D_o058_5DC4, letter0, letter1, func_8003A700(slot->records[opponent].name[2]), minutes, seconds, centiseconds);
                portraitIndex = slot->records[opponent].character + 0x51;
            }
            highlighted = (i == D_o058_5E90) ||
                       ((i == 3) && (D_o058_5E8C != -1));
            if (highlighted != 0) {
                fontColour((s32) D_o058_5F38.red, (s32) D_o058_5F38.green, (s32) D_o058_5F38.blue, 0xFF, 0xFF);
            } else {
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            if (i < 3) {
                func_8004B0F8(&D_800D3140, x + 0x28, textY, D_o058_5C98[i], 0);
            }
            nodes[0].texture = D_800D31C8_o058Reloc[portraitIndex];
            nodes[0].alternate = NULL;
            nodes[0].x = x + 0x58;
            nodes[0].y = textY - 4;
            nodes[0].packedOffset = 0;
            nodes[1].texture = 0;
            func_8002F618(&D_800D3140, (RcpTextureNode *) &nodes[0], 0, 0, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF, (u8) 0xFF);
            /* `rowY` is dead on entry to case 10; this independent index
             * avoids the former carrier's interference with character. */
            rowY = 0;
            do {
                if (highlighted == 0) {
                    if (rowY == 0) {
                        fontColour(0, 0xFF, 0xFF, 0xFF, 0xFF);
                    }
                    if (rowY == 3) {
                        fontColour(0xFF, 0xFF, 0, 0xFF, 0xFF);
                    }
                }
                character[0] = text[rowY];
                character[1] = 0;
                func_8004B0F8(&D_800D3140, D_o058_5CB0[rowY] + x, textY, &character[0], 0);
                rowY += 1;
            } while (rowY != 11);
            if (i == 2) {
                textY += 0x1B;
                fontColour(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
                func_8004B0F8(&D_800D3140, x + 0xA0, textY, D_8007C0B8->text[0x35], 4);
            }
            opponent += 1;
            i += 1;
            textY += 0x1B;
        } while (i != 4);
        D_o058_5E98 -= arg0 * 0xF;
        if (D_o058_5E98 < 0) {
            D_o058_5E98 = 0;
            if ((D_o058_5EB0 == 0) && (D_800D31B8_o058Reloc & 0x9000)) {
                D_o058_5EB0 = 0xB;
                amSndPlay(0xCU, NULL);
            }
            if (D_o058_5EB0 != 0) {
                D_o058_5EA8 += arg0 * 0xF;
                if (D_o058_5EA8 >= 0x141) {
                    D_o058_5E94 = D_o058_5EB0;
                    D_o058_5E9C = 0;
                    D_o058_5EA0 = 0x140;
                    D_o058_5EB0 = 0;
                    amSndPlay(0x1FAU, NULL);
                }
            }
        }
        break;
    default:
        break;
    }
}


#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o058/func_overlay_058_F000138C_18B0574/func_overlay_058_F000138C_18B0574.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_058_F000138C_18B0574:start
 * symbol: func_overlay_058_F000138C_18B0574
 * score: 6/3614 words
 * frame: 0x138
 * relocations: 1253
 * first-mismatch: +0x1314
 * summary: 6 retained: case 12's title loop walks a pointer initialised inside the guard from the D_o058_5C80 lineage, so no index web touches W's blocks and W colours t0 unforced; the residue is one load scheduled first in the title-loop block instead of in the call's delay slot, for want of a .noalias fact on a pointer-variable base.
 * PLATEAU-HANDOFF:func_overlay_058_F000138C_18B0574:end
 */
