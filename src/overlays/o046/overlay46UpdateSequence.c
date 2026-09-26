#include "PR/ultratypes.h"

typedef struct Overlay46DisplayCommand {
    u32 w0;
    u32 w1;
} Overlay46DisplayCommand;

typedef struct Overlay46Progress {
    u8 pad00[3];
    u8 alpha;
} Overlay46Progress;

typedef struct Overlay46Counter {
    u8 pad00[4];
    s32 value;
} Overlay46Counter;

extern s32 gOverlay46Timer5C;
extern u8 D_60[];
extern u8 D_C0[];
extern u8 D_120[];
extern Overlay46Progress *gOverlay46Group54Render;
extern s32 gOverlay46Mode58;
extern s32 gOverlay46Value4;
extern s32 gOverlay46Value0;
extern s32 gOverlay46Value8;
extern Overlay46Counter *gOverlay46Group54Update;
extern void *gOverlay46Resource50;
extern void *gOverlay46Resource4C;
extern u8 *gOverlay46ExpectedName180;
extern s32 gOverlay46StatusFlags;
extern Overlay46DisplayCommand *gDisplayListHead;
extern Overlay46DisplayCommand *gDisplayListHeadB;
extern u8 gOverlay46MotionTarget[];
extern s32 D_80003634;
extern s32 D_80003638;

extern s32 func_80037664(void);
extern void func_80037414(s32 mode, f32 scale, f32 value, s32 arg3, s32 arg4,
                          s32 arg5, s32 arg6);
extern void func_80037658(void);
extern void func_overlay_099_F0000064_18D9614(s32 mode, void *resource,
                                               f32 scale, f32 value);
extern s32 mathRnd(s32 minimum, s32 maximum);
extern void func_80000F94(s32 soundId, void *handle);
extern void func_overlay_044_F0000294_188BAF4(void *state, s32 updateRate);
extern void *func_80058240(void);
extern void func_800291B4(void);
extern void func_8003A680(s32 value);
extern void func_80001608(void);
extern void func_80028374(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4,
                          s32 arg5);
extern void func_80028528(s32 value);
extern void func_80028D30(s32 value);
extern void func_80036F08(Overlay46DisplayCommand **commands, void *resource,
                          s32 arg2);
extern void func_overlay_044_F0000580_188BDE0(void *state, void *target,
                                              f32 scale);
extern void func_8004B0A4(s32 mode);
extern void func_8004B0DC(s32 red, s32 green, s32 blue, s32 alpha);
extern void func_8004B0B8(s32 red, s32 green, s32 blue, s32 alpha,
                          s32 intensity);
extern s32 func_overlay_046_F000069C_188EA94(void);
extern s32 func_overlay_046_F0000874_188EC6C();

#define O46_SHIFTL(value, shift, width) \
    (((u32)(value) & ((1U << (width)) - 1U)) << (shift))
#define O46_PRIM(packet, red, green, blue, alpha) { \
    Overlay46DisplayCommand *macroCommand = (Overlay46DisplayCommand *)(packet); \
    macroCommand->w0 = O46_SHIFTL(0xFA, 24, 8); \
    macroCommand->w1 = O46_SHIFTL(red, 24, 8) | \
        O46_SHIFTL(green, 16, 8) | O46_SHIFTL(blue, 8, 8) | \
        O46_SHIFTL(alpha, 0, 8); \
}

/* 317 words, delta 0, frame exact, 118 relocation records aligned; 54 masked words remain and
 * the aligner puts ALL 54 in one bucket -- register naming, with zero immediate-only, zero
 * structural and zero displacement tax.  This is a pure colouring residual and nothing else.
 *
 * 2026-09-12, lane p7-ovl3, read off the instrumented allocator rather than inferred.  The
 * procedure is p1-only (331 p1cand, 24 p1dec, 21 p1color, no p2), so [L106] does not apply and
 * `save = totalsave/nocs` is the whole order.  globalcolor consumes colours 1..7 -- v0, v1, a0,
 * a1, a2, a3 and t0 -- and colour 7 is taken by exactly ONE web, web 51, the case-5 `matched`
 * flag (class 1, save 1.5, nocs 4, totalsave 6).  ugen's expression ring then starts at t1
 * because t0 is spoken for, where the target's starts at t0, and that single position is the
 * entire residual: ours reads t1/t2/t3/t4 where the target reads t0/t1/t2/t3, with five windows
 * of re-phasing after it.
 *
 * Web 51's own records say why it needs a seventh colour and what would retire it:
 *   - `forbidden0=0x7e000000` -- colours 1..6, every argument and return register -- and its
 *     `p1cost` list STARTS AT COLOUR 7, so those six are not priced for it at all.
 *   - it interferes with the webs holding all six (77 v0, 63 v1, 74 a0, 58 a1, 70 a2, 105 a3).
 *   - freeing one of them is not enough and this was measured, not assumed: forcing web 70 off
 *     a2 leaves a2 unassigned at web 51's decision and web 51 still takes t1, because its
 *     forbidden mask is 0x7f000000 by then; the object is 90 words, worse.
 * The same day the sibling residual in o086 read the same way, and there the mask cleared when a
 * call inside the web's range dropped an argument: a web live across a call is denied the
 * argument registers that call loads.  Here the spanned calls load a0..a3, which is why all six
 * are gone rather than two.
 *
 * Measured flat at 54 on this base, all against the whole 317-word target: seven `if (1) { }`
 * region placements plus eight more inside case 5 ([L97]/[L107] -- three of the sixteen are
 * worse, none is better, so a region does not reach this residual); splitting the tail alpha out
 * of `matched` (59) or into `value` (63); block-scoping `matched` (70); merging `matched` and
 * `value` (60); u8/s16/u32 spellings of the flag (55, 66, 54); inverting the flag (55); testing
 * it against 1 (149); a while-loop form of the name compare (54); hoisting the status-flag
 * address into a local (163); and 24 declaration orders, whose floor is the order already here.
 * One spelling IS adopted-in-waiting rather than adopted: writing the compare as
 * `expected != current` restores the target's own `beql a0,<current>` operand order and is flat
 * at 54 today, because the site stays a naming difference until the colour is right.  It is
 * recorded here so the next lane does not have to rediscover it after the ring lands.
 *
 * Next lever: shorten web 51's span so that it stops crossing a call which loads a2 or a3, or
 * find a source form in which the flag is not a distinct symbol at all.  Raising its save cannot
 * work; a colour absent from a web's cost list is not for sale at any ratio.
 *
 * 2026-09-12, lane p8-arity.  The arity lever named in the closure above is real here and it
 * moves the flag by exactly one colour, which is one short.  Everything below is measured on the
 * instrumented uopt, whose `.text` is byte-identical to the tree's stock object.
 *
 * Baseline reproduces: web 51 save 1.5, nocs 4, totalsave 6, forbidden0 0x7e000000, p1cost
 * starting at colour 7, decision=color, bestreg=t0.  The interference list holds webs assigned
 * every one of colours 1 to 6.
 *
 * **The a2 denial is not interference, and that is new.**  Forcing web 70 off a2 leaves a2
 * unassigned at web 51's decision and its mask still carries colour 5 (it becomes 0x7e800000,
 * colours 1 to 6 plus t1) -- so a2 is denied by the call rule, not by an incumbent.  A direct
 * `CDX_FORCE=p1:w51=c5` is declined byte-identically, and so are c1 through c4 and c6, which is
 * the L101 already-forbidden case.
 *
 * **The denial is a COUNT keyed to argument arity, and it saturates one colour above a2.**
 * Varying `func_80036F08`'s argument count alone, everything else fixed:
 *   0, 1 or 2 arguments -> forbidden 0x7c000000 (colours 1..5), web 51 takes a3;
 *   3 arguments (the tracked form) -> 0x7e000000 (colours 1..6), web 51 takes t0;
 *   4 arguments -> 0x7f000000 (colours 1..7), web 51 takes t1.
 * One more argument on a call inside the web's range denies exactly one more colour, from the
 * bottom of the table up.  `func_80028374` (six arguments) and the two `func_8004B0` calls move
 * the mask by nothing at all, so they are outside the range; `func_80036F08` and the tail's
 * `matched` reuse are inside it.  Inlining the tail use of `matched` drops totalsave 6 -> 4 and
 * the mask to 0x7c000000 on its own.
 *
 * **The floor is a3, and it is a floor.**  With EVERY tail call cut to one argument, and with
 * the tail use of `matched` inlined away as well, the mask stays at 0x7c000000 and the flag
 * takes a3, never a2.  Colours 1 to 5 survive every arity reduction available.  So the target's
 * a2 is not reachable through the call-argument rule, and this lever is closed rather than
 * unexplored.  Each of those probes also scores worse than the base (108 to 124 against 54).
 *
 * **The single-decision axis is closed too, and this is the more useful number.**  A full greedy
 * force ceiling was run: all 24 p1 decisions x colours 1..22 plus the split path, 552 cells, of
 * which 438 were accepted (433 distinct objects).  **Not one scores better than 54.**  The best
 * accepted cell ties at 54 and the next is 56.  So the residual is not one globalcolor decision:
 * it needs the web SET to change, not a recolouring, and the next lane should not re-run a force
 * sweep or a colour lattice.
 *
 * 2026-09-12, lane p11-mid.  One premise above is wrong and one argument is circular.
 *
 * **The a2 denial IS interference.**  Web 51's `intf` records name webs 77 and 111 holding colour 1,
 * 63 and 103 holding 2, 74 and 159 holding 3, 58 holding 4, 70 holding 5 and 105 holding 6 -- which is
 * `forbidden0=0x7e000000` exactly, colour for colour.  The arity reading is a second-order description
 * of the same fact: changing a call's argument count changes which webs are live across it.  Read the
 * `intf` rows against the `p1color` list before believing any statement about why a colour is denied.
 *
 * **"A colour absent from a cost list is not for sale at any ratio" is circular.**  It is absent
 * BECAUSE the web is decided late, after six colours are taken; a web decided earlier meets fewer.  So
 * the save axis was never actually retired by that argument.  It is retired now by measurement instead:
 * the [L109] probe family is inert in this function.  One and three OR-with-zero probes on `matched`
 * inside the compare loop, in the goto form and in a `while` form, leave all 21 `p1color` assignments
 * bit-identical and the score at 54.  The save is not settable from source here.
 *
 * The arithmetic that remains: globalcolor consumes colours 1..7 for us, and a target whose flag sits on
 * a2 consumes 1..5, so the web set has to lose TWO colour consumers.  Deleting `expected` is not one of
 * them -- in that build a2 is still taken and web 51 still lands on t0.
 *
 * Flat at 54 or worse, added this pass: the [L149] double-mask family at the tail alpha site, which is
 * already at its floor (dropping the macro's second mask 56, the first 87, both 85, only the `(u32)`
 * cast 87 -- the cast is load-bearing); the compare-loop carrier family (deleting `expected` three ways,
 * 70 / 65 / 65; `expected` as `s32` 54; `current` as `s32` 59; `expected != current` 54); and the
 * `while` rewrite of the compare, 54 with a bit-identical colouring.
 */

/* Ownership trial note kept from 2026-08-28 is below. */
/* Ownership trial (2026-08-28): fixed the TU's +0x364..+0x378 .rodata range;
 * linked promotion is text-differs with 315 in-range words, first at +0x0.
 * Module growth is cleared; the remaining gap is codegen/register allocation. */
#ifdef NON_MATCHING
void func_overlay_046_F0000120_188E518(s32 amount) {
    s32 matched;
    s32 value;
    u8 *left;
    u8 *right;
    u8 current;
    u8 expected;
    Overlay46Counter *counter;

    switch (gOverlay46Mode58) {
    case 1:
        if (gOverlay46Timer5C == 0) {
            if (func_80037664() != 1) {
                gOverlay46Value0 = 0;
                gOverlay46Mode58 = 2;
                gOverlay46Timer5C = 100;
                gOverlay46Value4 = 255;
                func_80037414(1, 2.5f, 0.0f, 0, 0, 0, 0);
            }
        } else if (func_80037664() != 1) {
            value = (gOverlay46Timer5C -= amount);
            if (value <= 0) {
                gOverlay46Timer5C = 0;
                func_overlay_099_F0000064_18D9614(2, D_60, 2.5f, -1.0f);
                value = mathRnd(0x203, 0x204);
                func_80000F94(value & 0xFFFF, 0);
            }
        }
        break;
    case 2:
        if (gOverlay46Timer5C == 0) {
            if (func_80037664() != 1) {
                func_80037658();
                gOverlay46Value4 = 0;
                gOverlay46Mode58 = 3;
                gOverlay46Timer5C = 0;
            }
        } else if (func_80037664() != 1) {
            value = (gOverlay46Timer5C -= amount);
            if (value <= 0) {
                gOverlay46Timer5C = 0;
                func_overlay_099_F0000064_18D9614(2, D_C0, 2.5f, -1.0f);
                value = mathRnd(0x203, 0x204);
                func_80000F94(value & 0xFFFF, 0);
            }
        }
        break;
    case 3:
        func_overlay_044_F0000294_188BAF4(gOverlay46Group54Update, amount);
        counter = gOverlay46Group54Update;
        if (counter->value < 0x200) {
            gOverlay46Value8 = counter->value >> 1;
        } else {
            gOverlay46Value8 = 255;
        }
        if (counter->value >= 0x1E01) {
            if (gOverlay46Timer5C == 0) {
                func_overlay_099_F0000064_18D9614(2, D_120, 2.5f, -1.0f);
                value = mathRnd(0x203, 0x204);
                func_80000F94(value & 0xFFFF, 0);
                gOverlay46Timer5C = 1;
            } else if (func_80037664() != 1) {
                gOverlay46Value8 = 0;
                gOverlay46Mode58 = 4;
                func_overlay_046_F000069C_188EA94();
            }
        }
        break;
    case 4:
        if (func_overlay_046_F0000874_188EC6C(amount) == 0) {
            gOverlay46Mode58 = 5;
        }
        break;
    case 5:
        matched = 0;
        if ((gOverlay46StatusFlags << 4) >= 0) {
            right = func_80058240();
            if (right != 0) {
                left = gOverlay46ExpectedName180;
                right += 0x34;
                matched = 1;
                current = *left;
                if (current != 0) {
compare_name:
                    expected = *right;
                    left++;
                    right++;
                    if (current != expected) {
                        matched = 0;
                    } else {
                        current = *left;
                        if (current != 0) {
                            goto compare_name;
                        }
                    }
                }
            }
        }
        if (matched != 0) {
            *(u8 *)&gOverlay46StatusFlags |= 8;
            func_800291B4();
            D_80003634 = 0;
            func_8003A680(10);
            D_80003638 = -1;
            func_80001608();
            func_80028374(0x12, 0, 0, 0xF, 1, 0);
            func_80028528(1);
        } else {
            func_80028D30(0);
        }
        gOverlay46Mode58 = 6;
        break;
    }

    if (gOverlay46Value4 != 0) {
        Overlay46DisplayCommand *macroCommand;

        macroCommand = gDisplayListHead++;
        macroCommand->w0 = O46_SHIFTL(0xFA, 24, 8);
        matched = (u32)gOverlay46Value4 & 0xFF;
        macroCommand->w1 = O46_SHIFTL(matched, 24, 8) |
            O46_SHIFTL(gOverlay46Value4, 16, 8) |
            O46_SHIFTL(gOverlay46Value4, 8, 8) | 0xFF;
        func_80036F08(&gDisplayListHead, gOverlay46Resource50, 0);
    }

    if (gOverlay46Value0 != 0) {
        O46_PRIM(gDisplayListHeadB++, gOverlay46Value0, gOverlay46Value0,
                 gOverlay46Value0, 0xFF);
        func_80036F08(&gDisplayListHeadB, gOverlay46Resource4C, 0);
    }

    value = gOverlay46Value8;
    if (value != 0) {
        gOverlay46Group54Render->alpha = value;
        func_overlay_044_F0000580_188BDE0(gOverlay46Group54Render,
                                          gOverlay46MotionTarget, 1.0f);
        func_8004B0A4(2);
        func_8004B0DC(0, 0, 0, 0);
        func_8004B0B8(255, 255, 255, 255, gOverlay46Value8);
    }
}

#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o046/overlay46UpdateSequence/func_overlay_046_F0000120_188E518.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_046_F0000120_188E518:start
 * symbol: func_overlay_046_F0000120_188E518
 * score: 54 differing words
 * frame: 0x30
 * relocations: 118
 * first-mismatch: +0x78
 * summary: Pointer walk and unscaled index miss a zero-size partition. Post-increment walk 70 and do-while 75 at delta 0; byte-offset index +12 bytes, 160 masked. Stall.
 * PLATEAU-HANDOFF:func_overlay_046_F0000120_188E518:end
 */
