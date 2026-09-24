#include "PR/ultratypes.h"

typedef struct Overlay20RemoveOwner {
    u8 pad0[0x84];
    void *entry;
} Overlay20RemoveOwner;

extern void *gOverlay20Entries[];
extern void *gOverlay20ShiftEntries[];
extern s32 gOverlay20EntryCount;
extern u8 gOverlay20MarkerEnd;
extern u32 gOverlay20ActiveBits;

/* PROVENANCE: indexed search and list-compaction loops adapted from Diddy Kong
 * Racing's published src/weather.c::lensflare_override_remove. Mickey's owner
 * offset, arrays, marker cleanup, relocations, and target bytes remain
 * authoritative. */
/* Bounded plateau (2026-09-08): configured C is 51/53 words, frameless, with
 * exact 0xD4 ownership and all ten relocation tuples and identities. The
 * donor-style indexed compaction removes two pool webs and makes the 12-web
 * temporary lane exact. Capturing the decremented count in new_var removes
 * four residual words; the remaining two words are a pool-color tie where
 * the target uses v0 and IDO chooses a2.
 * Fidelity-clean proc-0 tracing identifies an invisible v0 web that conflicts
 * with the end web. A diagnostic split plus forced cursor color reaches the
 * target pool assignments but introduces a stack frame, so it is not a valid
 * promotion. The bounded forced-color permuter follow-up was flat at this
 * 51/53 result. Pointer-, index-, cursor-relative-, explicit-base-, register-,
 * byte-offset-, and association variants were exhausted. IDO's trailing 0xC
 * is section alignment, not target padding. */
/* 2026-09-09: re-measured unchanged at two words, 53 instructions, frameless,
 * and the single site is `addu v0,t0,t9` against `addu a2,t0,t9` -- the
 * compaction loop's destination base. The temp and shared lanes are exact 6/6
 * and 6/6; only pool slot 9 differs, so this is uopt colouring and the ugen
 * ring model that closed overlay7DispatchSelection and the o001 middle pair
 * does not reach it. Newly eliminated: giving the count its own local instead
 * of reusing the `owner` parameter (25 words), giving the marker pointer its
 * own local (11), the natural `for` search loop in place of the m2c
 * `do/while(1)` (44 words and one instruction fewer), and dropping the
 * `new_var` bound carrier (6). The parameter reuse is load-bearing, not an m2c
 * artefact to be cleaned up. Next lever is unchanged: the invisible v0 web that
 * interferes with the compaction base, which needs the instrumented uopt
 * capture rather than a source spelling. */
/* 2026-09-09 (second pass): the instrumented uopt names the blocker exactly.
 * The compaction limit is pool web 42; when it is coloured, v0, v1, a0 and a1
 * are all forbidden (`forbidden0=0x78000000`) and a2 is the lowest colour left.
 * Its four interference partners are webs 37 (a1, the cursor), 34 (a0), 10
 * (v1, `i`) and 8 (v0) -- and web 8 is invisible: forcing it to a3 changes no
 * instruction anywhere outside the compaction loop, so it holds v0 without
 * emitting a word. `CDX_FORCE=p2:w42=c1` is declined at both the decision and
 * the colour site, which proves the interference is real and not a priority
 * choice, so no amount of source reordering that leaves web 8 where it is can
 * reach the target. Freeing v0 by forcing web 8 elsewhere is not the answer
 * either: web 37 is coloured first and takes v0, the limit takes a1, and the
 * residual grows to six words. The target therefore needs web 8 not to
 * interfere with web 42, not merely not to hold v0.
 * Newly eliminated at two words (all byte-flat): every dead-store colour
 * reservation on a spare s32/u32/pointer local at four placements and on
 * `entry` itself; a bare extra declaration of each type; `entry` typed as s32,
 * u32, u8 * and void **; four declaration orders crossed with three search-loop
 * spellings, four compaction spellings and two tail orders (120 rows, of which
 * the limit register is only ever a1 or a2); the `owner`/`new_var`/global
 * re-read bound carriers; and nesting the whole body instead of the early
 * returns. Deliberate extra pressure (keeping `entry` or `owner` live past the
 * loop) pushes the limit up to a3, never down to v0, which is the same
 * one-directional signature.
 * 2026-09-10, lane o7-tight: the instrumented uopt settles the ordering law.
 * globalcolor's caller-saved sweep colours webs in ASCENDING WEB NUMBER, taking
 * the lowest colour no already-coloured interferer holds; all twelve p2 records
 * here reproduce their forbidden sets under that order and under no other, and
 * descending `save` (which is the p1 order, measured separately on
 * overlay1ResolvePathPoint's nineteen p1 decisions) predicts web 0's set wrong.
 * Web 8 is the decremented count, not the marker loop's dead copy. The cursor
 * reaches a1 only when web 8 (v0), `i` (v1) and `new_var` (a0) are all coloured
 * in front of it, and all three also interfere with the limit, so the ordering
 * that gives the target's cursor forbids the target's limit: the residual is a
 * contradiction on this CFG, not a missing spelling. Hoisting the array base
 * into a local does invert the limit/cursor numbering (limit below cursor,
 * cursor on a1) at 5 words, which is the proof that the numbering axis is open;
 * the open question is whether the decrement can be numbered above the limit.
 * ~1,050 further candidates measured flat this pass; see the shard.
 * 2026-09-10, lane nm-ovlsmall: three new loop-shape results, from the lever
 * that closed overlay41AddSlot in the same lane. A top-tested compaction loop
 * (`while (i < new_var)`, with or without the guarding `if`, and the `for`
 * form) gives 6 words and shifts the limit and the cursor DOWN one colour each
 * -- a2/a1 becomes a1/a0. The target's v0/a1 puts the limit BELOW the cursor,
 * so it is an order inversion, not a uniform shift, and no amount of shifting
 * reaches it. A top-tested marker loop is byte-flat; a top-tested search loop
 * is 42 words and one instruction short. Most usefully, giving the marker loop
 * its own counter (declared first or last) or reusing `new_var` for it costs
 * 11 words and swaps only that loop's own counter/copy pair, leaving the
 * compaction limit at a2 -- which falsifies the reading that the marker loop's
 * dead `move v0,v1` is the invisible v0 web that blocks web 42. */
/*
 * 2026-09-12, lane p10-tight re-established the force results with CDX_PROC
 * set, because CDX_FORCE is silently ignored without it and a dropped force
 * looks exactly like a legitimate decline. The closure survives and now has a
 * positive control: forcing the compaction limit onto the first colour records
 * the never-applied value and changes nothing, while in the same harness
 * forcing the decremented count onto the sixth colour IS accepted and moves the
 * whole ladder, so the tool was demonstrably driving. The promotion chain is
 * measured over three configurations rather than one. The requirement is
 * three-sided: the decremented count gone, the limit numbered below both the
 * bound carrier and the compaction cursor, and exactly one interferer holding
 * the third colour in front of the cursor. The recorded array-base hoist buys
 * only the second and costs three words in the two index shifts.
 */
/* 2026-09-12, lane p21-two: fresh configured baseline and L154 probes preserve
 * 51/53 words, frameless, with two coherent colour-only differences. A global
 * pre-decrement regresses to 52/53 with a four-byte size excess; computing
 * owner - 1 into new_var before the global store regresses to 34/53 exact;
 * unsigned-literal and addition-negative-one spellings remain 51/53. The
 * instrumented compiler's text is byte-identical to stock. With CDX_PROC=0,
 * forcing web 42 to c1 is declined (forced=-2) and scores 2 directly, while
 * forcing web 8 to c6 is accepted and scores 6, confirming the promotion
 * chain. The remaining source decision is web creation order, not a dropped
 * force or an unpriced register phase.
 */
#ifdef NON_MATCHING
void overlay20RemoveEntry(s32 owner) {
    void *entry;
    s32 new_var;
    s32 i;

    entry = ((Overlay20RemoveOwner *)owner)->entry;
    if (entry == NULL) {
        return;
    }
    owner = gOverlay20EntryCount;
    i = 0;
    if (owner > 0) {
        do {
            if (entry == gOverlay20Entries[i]) {
                break;
            }
            i++;
            if (i < owner) {
                continue;
            }
            break;
        } while (1);
    }
    if (i >= owner) {
        return;
    }
    gOverlay20EntryCount = owner - 1;
    if (i < gOverlay20EntryCount) {
        new_var = gOverlay20EntryCount;
        do {
            gOverlay20ShiftEntries[i] = gOverlay20ShiftEntries[i + 1];
            i++;
        } while (i < new_var);
    }

    owner = (s32)&gOverlay20MarkerEnd;
    i = 31;
    do {
        if (owner != 0) {
            gOverlay20ActiveBits &= ~(1U << i);
            return;
        }
        owner -= 0x24;
    } while (i--);
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o020/overlay20RemoveEntry/func_overlay_020_F0001018_18775F0.s")
#endif

/* PLATEAU-HANDOFF:overlay20RemoveEntry:start
 * symbol: overlay20RemoveEntry
 * score: 2/53 words
 * frame: frameless
 * relocations: 10
 * first-mismatch: +0x6C
 * summary: Retype or move first use of web 8, the type-4 count load in blocks 12-13. Early/late u32: 8 to 32; or-0 folds. Still under the limit on colour 1. Stall 2/53.
 * PLATEAU-HANDOFF:overlay20RemoveEntry:end
 */
