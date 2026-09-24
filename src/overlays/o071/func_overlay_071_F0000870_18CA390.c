#include "PR/ultratypes.h"

typedef struct Overlay71Command {
    u32 w0;
    u32 w1;
} Overlay71Command;

typedef struct Overlay71DrawState {
    u8 pad00[0xCC];
    u8 active;
    u8 vertexBank;
    u16 flags;
} Overlay71DrawState;

typedef struct Overlay71DrawObject {
    u8 pad00[0x64];
    Overlay71DrawState *state;
    s32 *resourceIndex;
} Overlay71DrawObject;

extern void *gOverlay71InitialResourceReloc;
extern u8 D_80000008[];
extern u8 D_80000028[];
extern u8 D_800000D8[];

extern void func_80032BF0(void *resource, s32 mode, s32 flags);
extern void func_8002409C(Overlay71Command **commands, s32 context,
                         Overlay71DrawObject *object, f32 scale, f32 extra);
extern void func_80034554(Overlay71Command **commands, s32 resource, s32 mode,
                         s32 flags);
extern void func_800241BC(Overlay71Command **commands);

/* DKR v77/v80 and JFG contain no exact donor for this renderer. */
/*
 * Workbench: exact 182 words and -48 frame; 11 masked words, first +0x94.
 *
 * Lane c6-band-b (2026-09-10): 33 -> 11 by ONE whitespace edit.  Every
 * two-word display-list command shipped its `w1` store before its `w0` store
 * and this candidate emitted them the other way round -- ten sites, twenty
 * words.  The two long-expression commands (the vertex-bank pair) already
 * agreed.  Writing `w1` first in the source REGRESSES to 41: as1 orders two
 * ready stores by physical source line (L59, lexicographic minimum on
 * `node->lineno`), so an inversion just moves the same tie.  Folding the pair
 * onto ONE physical line makes the two line numbers equal, drops the tie
 * through to ready-list position, and that is the shipped order at all ten
 * sites at once.  Token stream unchanged after whitespace normalisation.
 *
 * The residue is one colouring fact: the shipped code reads `state->flags`
 * into `a2` at all four tests, this candidate into `a1` (the first two) and
 * `a0` (the last two).  Instrumented globalcolor says it is fully
 * colour-reachable -- forcing the two load webs to c5 gives a byte-identical
 * object, 0 differing words -- but the allocator takes the first available
 * colour at strictly-minimum cost, and reaching a2 needs interfering webs
 * already parked on a0 AND a1.  The `u16 flags` local supplies the a0 one
 * (its zero-extend is peepholed away, so the web is invisible in the object,
 * and dropping the local costs a word); no source form found so far supplies
 * an a1 one.  Flat at 11 or worse: the 16 single-local and 60 two-local
 * carrier placements over the four tests, the local's type and qualifier
 * lattice (`register` is inert at -O2, `volatile` changes the frame), six
 * uopt region boundaries (L97), `!= 0` and unsigned-literal test spellings,
 * hoisting `*object->resourceIndex`, and reloading `state` inside the block.
 * Also inert, all twenty of them byte-identical to this file: wrapping each
 * command word's constant in a nested assignment to a named local, the
 * `w0 = (x = 0xE7000000)` idiom a permuter run proposed. The pointer form of
 * that idiom is a real web on overlay101DrawTransformed because the pointer
 * escapes; a constant one is propagated away here.
 *
 * That permuter run is also why `skills/tools/permuter.md` now warns that the
 * importer reformats the TU: it undoes the fold above before generating a
 * single candidate, reported this function's unfolded score of 420 as its
 * base, and spent its whole budget improving that.
 *
 * 2026-09-11, lane p2-close: the colour table and the three decision records
 * were re-derived from this procedure's own log rather than quoted -- c1 v0,
 * c2 v1, c3 a0, c4 a1, c5 a2, c6 a3, c7 t0 through c13 t6, c14 s0 -- and they
 * reproduce exactly: web 25 save 2.0 nocs 1 totalsave 2 takes c3, web 27 save
 * 2.0 nocs 2 totalsave 4 takes c4, web 50 save 2.0 nocs 2 totalsave 4 takes
 * c3.  What the earlier passes did not say is that there are FOUR flag loads,
 * and the shipped code puts all four in a2 while this candidate splits them
 * a1/a1/a0/a0.  Because webs 27 and 50 each have two live-range components and
 * the components are separated by calls, no single web can interfere with both
 * -- so the lever has to supply an interferer per component, on c3 and on c4,
 * and it has to be a web that does not span a call, since a call-spanning web
 * is forbidden a0 through a3 intrinsically.  That is four webs to create, not
 * one.  See the handoff shard for the forms falsified this pass.
 *
 * 2026-09-12, lane p10-near: 11 -> 9, and the missing web is now built rather
 * than argued about. Giving the FIRST command site its own pointer local and
 * opening an L97 region across its advance makes that local's web span two
 * basic blocks, so `nocs` is 2 and its save is 2.0 (L147); it is numbered
 * below the `flags` carrier, so it is coloured first, and because it still
 * shares a block with `command` it is forbidden v0 and takes a0. `flags` then
 * takes a1 and the FIRST pair of flag loads takes a2, which is the shipped
 * register -- confirmed against the decision records, at size delta 0. What
 * this costs is that the site's pointer is now a0 where the ROM has v0, four
 * words; the five that remain are the second pair of flag loads, still on a0.
 * So the ROM has a non-call-spanning a0 web in that block that emits NOTHING,
 * like the `u16 flags` carrier whose zero-extend is peepholed away, and the
 * open question is what that second invisible web is. Every route that makes
 * it a pointer pays four words for the pointer's own colour.
 *
 * 2026-09-10, lane w8-tu: the instrumented allocator names the requirement.
 * The two flag-load webs are both p1 decisions at save 2.0; the first has
 * three colours plus a callee-saved one forbidden and takes the fourth, the
 * second has two plus the callee-saved one and takes the third. Both want the
 * fifth. The sweep runs in strictly descending save with ties broken by
 * ascending web number, and the invisible `flags` carrier -- the web just
 * ahead of them, also at save 2.0 -- interferes with the first pair and not
 * with the second, which is exactly why they land one colour apart. So the
 * lever must CREATE a web that interferes with both, is coloured before them,
 * and pushes the carrier up one. Every other interferer of the two is a
 * constant web at save 1.0 or below, so none of the existing ones can. See
 * the handoff shard.
 *
 * 2026-09-17, lane w4-o071: delete-flags is 16/12; block-11 discards DCE or
 * CSE. w32 is dtype 8, w57 is dtype 6. A u16 later from state->flags joins
 * w32 and steals head's a0 (16). An s32 later coalesces into w57 (9). A
 * narrowing copy tmp=flags; later=tmp creates w67 in block 11 without
 * touching w32, but it is numbered after w57 and spends a draw (37).
 */
#ifdef NON_MATCHING
void func_overlay_071_F0000870_18CA390(Overlay71Command **commands,
                                       s32 context,
                                       Overlay71DrawObject *object) {
    Overlay71DrawState *state;
    Overlay71Command *command;
    Overlay71Command *head;
    u16 flags;
    func_80032BF0(gOverlay71InitialResourceReloc, 2, 2);
    state = object->state;
    if (state->active != 0) {
        func_8002409C(commands, context, object, 1.0f, 0.0f);

        head = *commands;
        if (1) { *commands = head + 1; }
        head->w0 = 0xE7000000; head->w1 = 0;
        command = *commands;
        *commands = command + 1;
        command->w0 = 0xFB000000; command->w1 = 0xFFFFFFFF;

        flags = state->flags;
        if (flags & 1) {
            func_80034554(commands, 0, 0x17, 0);
            command = *commands;
            *commands = command + 1;
            command->w0 = 0xFA000000; command->w1 = 0xFFFFFFD0;
            command = *commands;
            *commands = command + 1;
            command->w0 =
                ((((((((u32)state + state->vertexBank * 0x50) +
                         0x80000000) & 6) | 0x20) & 0xFF) << 16) |
                 0x04000000) | 0x30;
            command->w1 =
                (u32)state + state->vertexBank * 0x50 + 0x80000000;
            command = *commands;
            *commands = command + 1;
            command->w0 = 0x05100020; command->w1 = (u32)D_80000008;
            command = *commands;
            *commands = command + 1;
            command->w0 = 0xE7000000; command->w1 = 0;
        }

        if (state->flags & 6) {
            func_80034554(commands, *object->resourceIndex, 0x17, 0);
            command = *commands;
            *commands = command + 1;
            command->w0 = 0xFA000000; command->w1 = 0xFFFFFFFF;
            command = *commands;
            *commands = command + 1;
            command->w0 =
                ((((((((u32)state + state->vertexBank * 0x50) +
                         0x80000000) & 6) | 0x40) & 0xFF) << 16) |
                 0x04000000) | 0x58;
            command->w1 =
                (u32)state + state->vertexBank * 0x50 + 0x80000000;
            if (state->flags & 4) {
                command = *commands;
                *commands = command + 1;
                command->w0 = 0x05710080; command->w1 = (u32)D_800000D8;
            }
            if (state->flags & 2) {
                command = *commands;
                *commands = command + 1;
                command->w0 = 0x05710080; command->w1 = (u32)D_80000028;
            }
            command = *commands;
            *commands = command + 1;
            command->w0 = 0xE7000000; command->w1 = 0;
        }
        command = *commands;
        *commands = command + 1;
        command->w0 = 0xFA000000; command->w1 = 0xFFFFFFFF;
        func_800241BC(commands);
    }
}
#else
#pragma GLOBAL_ASM("asm/nonmatchings/overlays/o071/func_overlay_071_F0000870_18CA390/func_overlay_071_F0000870_18CA390.s")
#endif

/* PLATEAU-HANDOFF:func_overlay_071_F0000870_18CA390:start
 * symbol: func_overlay_071_F0000870_18CA390
 * score: 9 differing words
 * frame: 0x30
 * relocations: 13
 * first-mismatch: 0x5c
 * summary: hypothesis=second simultaneous occupier of the type-3 AND in blocks 9, 11 and 13; spellings=s16 cast merged at delta -4, pred!=0 emitted +20, flags&0xFFFF absorbed the type-4 convert at delta -4; stall=no second web at delta 0
 * PLATEAU-HANDOFF:func_overlay_071_F0000870_18CA390:end
 */
