#include "PR/ultratypes.h"

extern u8 gOverlay33Initialized[];
extern s32 D_8007A3EC;
extern s32 D_800D1C38;
extern s32 D_800D1C34;
extern volatile s32 gOverlay33Allocation;
extern s32 D_800D2CD0;
extern s32 D_800D1C28[];
extern s32 gOverlay33ActiveBuffer;

extern void viGetCurrentSize(s32 *width, s32 *height);
extern s32 mmAlloc(s32 size, s32 tag);
extern s32 osRecvMesg(s32 *context, s32 *status, s32 mode);
extern void overlay33ReleaseGlobal(void);

/*
 * Plateau (2026-09-04): the exact-sized 81-word, 0x38-frame candidate is
 * 75/81 words with six relocation-masked differences. A fidelity-clean as1
 * trace finds two line-key decisions, both in already-exact blocks; the
 * store/branch/copy cluster is instead decided by besttime. Both relevant
 * line joins and the addition commutation are byte-flat; declaration changes
 * are flat/regressing, and all 25 relocation offsets/types align.
 *
 * 2026-09-10, lane nm-ovlsmall: the sixth word, the `addu` at +0xDC, is
 * closed. It was never a schedule tie: IDO canonicalizes a commutative
 * integer add so that the heavier operand becomes the left one, which is why
 * `allocation + (width * height * 2)` and `(width * height * 2) + allocation`
 * both emit `addu t6,t5,v0`. An explicit `(s32)` cast on the local makes it
 * the heavier operand, the written order survives, and the site becomes
 * `addu t6,v0,t5`. (The same rule closed the float pair in
 * overlay41AddSlot, where a struct-member reference replaced an indexed
 * array reference.) Shifting `<< 1`, reassociating the product, and
 * commuting in source are all byte-flat; `width * (height * 2)` regresses.
 * The residual is now the five-word delay-slot cluster alone, and the
 * following are newly falsified against it, all byte-flat: every source-line
 * layout of the store and the test (same line, blank lines between,
 * expressions split across lines -- so this is not a `.loc` boundary),
 * `if (allocation)` and `if (!(allocation == 0))`, casting the copy,
 * testing `allocation` instead of `original` in the alignment test, and both
 * declaration orders of the pair. Inverting the if/else costs an
 * instruction (82); driving the then-path from `allocation` costs nine more
 * words.
 *
 * The five-word cluster is now read directly off an as1 schedule trace
 * (`cc -Wa,-R`, byte-inert on .text here: the traced object's `.text` sha1 is
 * the untraced one's). The whole cascade is ONE pick in a two-node basic
 * block. as1 holds `sw v0,0(s0)` (the allocation store) and `beqz v0` in a
 * block of exactly two independent nodes, and picks by highest `aftercycles`,
 * breaking ties on the lower ugen emission index. The store's afternode list
 * is empty, so its aftercycles is 0; a branch is seeded at 1. The branch
 * therefore always wins, the store sinks into its delay slot, and the `move`
 * and `li -64` each slide down one slot behind it. The store has the LOWER
 * emission index, so a mere tie would be enough -- aftercycles 1 would do it.
 * Only a dependence successor can supply that, and the trace shows exactly
 * how: adding any second volatile reference to the block chains the store to
 * it (`afternodes: 1/1`, aftercycles 2), the store is picked first, and the
 * target's order appears. Every such reference costs an instruction (82
 * against the target's 81), so the residual is not that the order is
 * unreachable but that no zero-instruction source form gives that store a
 * successor. Newly measured: `volatile` on gOverlay33Allocation is itself
 * byte-flat here -- dropping the qualifier compiles to the identical object,
 * so the barrier is not what pins the current schedule.
 *
 * 2026-09-09, lane fin-misc: the cluster read out exactly. Five of the six
 * words are the same seven instructions in two orders. The target emits
 * `sw`, `beqz`, `move`(delay), `andi`, `beqz`, `li -64`(delay), `and`; the
 * candidate emits `beqz`, `sw`(delay), `andi`, `beqz`, `move`(delay),
 * `li -64`, `and`. So both branch delay slots are filled differently: the
 * target spends `original = allocation` on the first and `li at,-64` on the
 * second, the candidate spends the store on the first and the copy on the
 * second. `original` is dead on the allocation-failure path, so uopt sinks it
 * past the null test and the store becomes the only fill candidate above the
 * branch; hoisting the copy above the test instead lets uopt delete the
 * matching `move` at the join (80 words). The sixth word, the `addu` at +0xDC,
 * is the same operand-order question and never moves alone.
 *
 * 2026-09-10, lane w8-tu: MATCHED. The two halves of the closure above are
 * both right and they compose -- but only with a uopt region boundary between
 * them, which is why each alone regressed.
 *
 * Hoisting `original = allocation;` above the null test is the schedule half.
 * It does not give the volatile store a dependence successor, which is what
 * the as1 trace asked for; it puts a THIRD node in the store's basic block.
 * as1 then picks the branch (aftercycles 1), the store next on the emission
 * index, and the copy last -- and the last pick is what fills the delay slot.
 * `sw`, `beqz`, `move`(delay) is the target's order, so the cluster is a block
 * SIZE question, not a dependence one. That correction matters for the next
 * function: a two-node block always sinks its non-branch node into the slot.
 *
 * The hoist alone costs a word because uopt then knows `original` equals
 * `allocation` and deletes the else arm's `move v0,v1` at the join (80 words,
 * 52 masked). `if (1) { }` around the hoisted copy is a uopt region boundary
 * (L97), and a boundary blocks that propagation while changing no instruction.
 * A bare brace block around the same statement is byte-identical to no block
 * at all -- 52 masked again -- which is L97's own control: the lever is region
 * structure, not lexical scope.
 *
 * Measured alternatives, all at delta 0: an empty `if (1) { }` before the
 * hoisted copy and `do { original = allocation; } while (0)` are each also 0;
 * reading `original` from the volatile global instead of copying it does fix
 * the store order (the successor route the trace named) but rewrites the else
 * arm, 8 words; an `| 0` lock-break keeps the else copy alive without a region
 * and lands at 13 because it manufactures a second carrier.
 */
void overlay33InitializeBuffers(void) {
    s32 width;
    s32 height;
    volatile s32 unused;
    s32 status;
    s32 allocation;
    s32 original;

    status = 0;
    if (gOverlay33Initialized[0x3790] == 0) {
        D_8007A3EC = 0;
        D_800D1C38 = 0;
        D_800D1C34 = 2;
        gOverlay33Allocation = 0;
        viGetCurrentSize(&width, &height);
        allocation = mmAlloc((width * height * 4) + 0x40,
                                             0x87);
        gOverlay33Allocation = allocation;
        if (1) { original = allocation; }
        if (allocation != 0) {
            if (original & 0x3F) {
                allocation = (original & ~0x3F) + 0x40;
            } else {
                allocation = original;
            }
            D_800D1C28[0] = allocation;
            D_800D1C28[1] =
                (s32)allocation + (width * height * 2);
            gOverlay33ActiveBuffer =
                D_800D1C28[D_800D1C38];
            if (osRecvMesg(&D_800D2CD0,
                                                &status, 0) == 0) {
                do {
                } while (osRecvMesg(
                             &D_800D2CD0, &status, 0) == 0);
            }
            gOverlay33Initialized[0x3790] = 1;
        } else {
            overlay33ReleaseGlobal();
        }
    }
}
